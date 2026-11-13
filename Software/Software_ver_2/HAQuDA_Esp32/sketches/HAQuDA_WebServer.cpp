#include "HAQuDA_WebServer.h"
// Only this file includes the page data: a second include would copy it into another object file
#include "WebPages.h"
#include "HAQuDA_Version.h"

#include <esp_attr.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <esp_wifi.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>

// The escaped network name adds up to JSON_ESCAPED_SSID_LEN
constexpr size_t STATUS_JSON_LEN = 768;
constexpr size_t WIFI_LIST_JSON_LEN = 2048;
// A SSID byte is escaped to at most 6 characters (\u00XX)
constexpr size_t JSON_ESCAPED_SSID_LEN = WIFI_SSID_LEN * 6 + 1;
// state, escaped name, address, host and reason of /api/join, then the outcome of the last switch with its own escaped name
constexpr size_t JOIN_JSON_LEN = JSON_ESCAPED_SSID_LEN * 2 + 320;
// The credentials file is rewritten through this file, so a failed write leaves the old list (SPIFFS names hold 31 characters)
constexpr char FILE_NAME_WIFI_NET_TMP[] = "/WiFiNetworks.tmp";
// Longest wait for the credentials file lock; a file operation takes tens of milliseconds
constexpr uint32_t CREDS_LOCK_WAIT_MS = 2000;
static_assert(JOIN_SSID_BUF_LEN == WIFI_SSID_LEN, "the check keeps the same name length as the stored record");
constexpr char CACHE_STATIC[] = "max-age=86400";
constexpr char CACHE_NONE[] = "no-cache";

namespace {
	// Appends formatted text at pos; returns false (and keeps the buffer terminated) when it does not fit
	bool appendFormat(char *buf, size_t size, size_t &pos, const char *fmt, ...) {
		if (pos >= size) {
			return false;
		}
		va_list args;
		va_start(args, fmt);
		int written = vsnprintf(buf + pos, size - pos, fmt, args);
		va_end(args);
		if ((written < 0) || (static_cast<size_t>(written) >= size - pos)) {
			buf[pos] = '\0';
			return false;
		}
		pos += written;
		return true;
	}

	// Holds the credentials file lock for its lifetime. Held() is false when the lock was not obtained in time.
	class CredsLock {
	  public:
		explicit CredsLock(SemaphoreHandle_t mutexHandle)
			: mutex(mutexHandle), held((mutexHandle != nullptr) && (xSemaphoreTake(mutexHandle, pdMS_TO_TICKS(CREDS_LOCK_WAIT_MS)) == pdTRUE)) {
		}
		~CredsLock() {
			if (held) {
				xSemaphoreGive(mutex);
			}
		}
		CredsLock(const CredsLock &) = delete;
		CredsLock &operator=(const CredsLock &) = delete;
		bool Held() const {
			return held;
		}

	  private:
		SemaphoreHandle_t mutex;
		bool held;
	};

	void sendAsset(AsyncWebServerRequest *request, const char *mime, const uint8_t *data, size_t len, const char *cacheControl) {
		AsyncWebServerResponse *response = request->beginResponse_P(200, mime, data, len);
		response->addHeader("Content-Encoding", "gzip");
		response->addHeader("Cache-Control", cacheControl);
		request->send(response);
	}

	void sendText(AsyncWebServerRequest *request, int code, const char *text) {
		request->send(code, "text/plain", text);
	}

	// Wipes a buffer that held passwords when it goes out of scope, on every return path
	class WipeOnExit {
	  public:
		WipeOnExit(void *dataToWipe, size_t lenToWipe) : data(dataToWipe), len(lenToWipe) {
		}
		~WipeOnExit() {
			HAQuDA_WipeBytes(data, len);
		}
		WipeOnExit(const WipeOnExit &) = delete;
		WipeOnExit &operator=(const WipeOnExit &) = delete;

	  private:
		void *data;
		size_t len;
	};
} // namespace

void HAQuDA_SendIndexPage(AsyncWebServerRequest *request) {
	sendAsset(request, "text/html", WEB_INDEX_HTML_GZ, WEB_INDEX_HTML_GZ_LEN, CACHE_NONE);
}

bool HAQuDA_WebServer::WebServerStarted = false;

// The one file-scope variable: it must live in RTC memory that the start-up code leaves alone (RTC_NOINIT_ATTR), which a class
// member cannot. Written by HAQuDA_WebServer under joinMux only (one instance); holds noise after a power-on, hence the
// magic and the checksum of the record.
static RTC_NOINIT_ATTR join_last_record rtcLastJoin;

void HAQuDA_WebServer::beginWebServer() {
	restoreLastJoin();
	WebServerResponds_init();
	// WebSerial only registers its page and socket handlers, so it needs nothing from server.begin().
	// It goes before the captive handler: the server gives a request to the first handler that accepts
	// it, and the captive handler accepts every URL, so AP clients would never reach /webserial.
	MyWebSerial.begin(&server);
	MyWebSerial.msgCallback(HAQuDA_WebServer::recvMsg);
	// Must stay last: it answers every unknown URL (captive portal probes) with the index page
	server.addHandler(new HAQuDA_CaptiveRequestHandler()).setFilter(ON_AP_FILTER); // only when requested from AP
	server.begin();
	WebServerStarted = true;
}

/* Message callback of MyWebSerial */
void HAQuDA_WebServer::recvMsg(uint8_t *data, size_t len) {
	MyWebSerial.println("Received Data...");
	String d = "";
	for (int i = 0; i < len; i++) {
		d += char(data[i]);
	}
	MyWebSerial.println(d);
}

void HAQuDA_WebServer::WebSerialPrint(const char *str) {
	MyWebSerial.println(str);
}

void HAQuDA_WebServer::WebSerialPrintRaw(const char *str) {
	// Print::print(const char *) makes a single write(buffer, len) call, so one textAll()
	MyWebSerial.print(str);
}

void HAQuDA_WebServer::WebSerialPrintStoredLogs() {
	int logsNum = HAQuDA_Logger::GetStored_LogsNum();
	char logText[WEB_SERIAL_MAX_STR_LEN] = {0};
	for (int i = 0; i < logsNum; i++) {
		HAQuDA_Logger::GetStored_Log(i, logText);
		WebSerialPrint(logText);
	}
}

bool HAQuDA_WebServer::GetWebServerStarted() {
	return WebServerStarted;
}

// msg must be a string literal: only the pointer crosses the tasks. If two failures arrive within one loop() pass,
// the later one replaces the earlier one.
void HAQuDA_WebServer::postLogError(const char *msg) {
	deferredLogError.store(msg);
}

void HAQuDA_WebServer::HandleDeferredWork() {
	const char *logMsg = deferredLogError.exchange(nullptr);
	if (logMsg != nullptr) {
		HAQuDA_Logger::LogError(logMsg);
	}

	if (!restartPending.load()) {
		return;
	}
	if ((uint32_t)(millis() - restartRequestMs.load()) >= restartDelayMs.load()) {
		HAQuDA_Logger::LogInfo("Restart to apply new WiFi credentials");
		ESP.restart();
	}
}

void HAQuDA_WebServer::handle_NewWiFiCreds(AsyncWebServerRequest *request) {
	AsyncWebParameter *ssidParam = request->getParam(SSID_PARAM_NAME, true);
	AsyncWebParameter *passParam = request->getParam(PASS_PARAM_NAME, true);
	if ((ssidParam == nullptr) || (passParam == nullptr) || (ssidParam->value().length() == 0) || (passParam->value().length() == 0)) {
		sendText(request, 400, "Enter both the network name and the password.");
		return;
	}
	// The record fields hold the text and a terminator
	if (ssidParam->value().length() >= WIFI_SSID_LEN) {
		sendText(request, 400, "The network name is too long (31 bytes at most).");
		return;
	}
	if (passParam->value().length() >= WIFI_PASS_LEN) {
		sendText(request, 400, "The password is too long (31 bytes at most).");
		return;
	}

	TWiFiCreds newWiFiCreds;
	memset(&newWiFiCreds, 0, sizeof(newWiFiCreds));
	strncpy(newWiFiCreds.ssid, ssidParam->value().c_str(), sizeof(newWiFiCreds.ssid) - 1);
	strncpy(newWiFiCreds.pass, passParam->value().c_str(), sizeof(newWiFiCreds.pass) - 1);
	WipeOnExit wipeNewCreds(&newWiFiCreds, sizeof(newWiFiCreds));

	// Both modes test the network before they save it, so a wrong password costs neither the access point nor the working
	// station connection. loop() does the work; the page follows it through /api/join.
	bool accessPoint = apMode.load();
	join_queue_result queued = queueJoinRequest(newWiFiCreds);
	if (queued == join_queue_result::checking) {
		sendText(request, 409, "The lamp is already checking a network. Wait for the result.");
		return;
	}
	if (queued == join_queue_result::restarting) {
		sendText(request, 409, accessPoint ? "The network is saved. The lamp is restarting." : "The lamp is restarting. Open this page again in a minute.");
		return;
	}
	if (accessPoint) {
		sendText(request, 202, "The lamp is checking the network. Open this page again in 30 seconds.");
		return;
	}
	// Station mode: the lamp leaves its network for the check, so this page is unreachable until the lamp is back
	sendText(request, 202,
			 "The lamp is checking the network. This page may be unreachable for about a minute while the lamp tries it. "
			 "Open this page again then to see the result.");
}

// loop() restarts the lamp: waiting in the async_tcp task would block it. The time and the delay are stored before the flag.
void HAQuDA_WebServer::scheduleRestart(uint32_t delayMs) {
	if (!restartPending.load()) {
		restartRequestMs.store(millis());
		restartDelayMs.store(delayMs);
		restartPending.store(true);
	}
}

join_queue_result HAQuDA_WebServer::queueJoinRequest(const TWiFiCreds &creds) {
	join_queue_result result = join_queue_result::queued;
	portENTER_CRITICAL(&joinMux);
	if (restartPending.load() || (joinView.state == join_state::ok)) {
		result = join_queue_result::restarting;
	} else if (joinRequestPending.load() || (joinView.state == join_state::checking) || (joinView.state == join_state::switching)) {
		result = join_queue_result::checking;
	} else {
		joinRequest = creds;
		joinRequestPending.store(true);
		joinView.state = join_state::checking;
		joinView.reason = join_reason::none;
		joinView.ip = 0;
		HAQuDA_SetJoinSsid(joinView, creds.ssid, sizeof(creds.ssid));
		// The outcome of the previous switch is replaced by this one
		HAQuDA_SetJoinLast(joinView, join_last_result::none, join_reason::none, nullptr, 0);
		HAQuDA_ClearJoinLastRecord(rtcLastJoin);
	}
	portEXIT_CRITICAL(&joinMux);
	return result;
}

bool HAQuDA_WebServer::takeJoinRequest(TWiFiCreds &creds) {
	bool taken = false;
	portENTER_CRITICAL(&joinMux);
	if (joinRequestPending.load()) {
		creds = joinRequest;
		memset(&joinRequest, 0, sizeof(joinRequest));
		joinRequestPending.store(false);
		taken = true;
	}
	portEXIT_CRITICAL(&joinMux);
	return taken;
}

void HAQuDA_WebServer::publishJoinState(join_state state, join_reason reason, uint32_t ip) {
	portENTER_CRITICAL(&joinMux);
	joinView.state = state;
	joinView.reason = reason;
	joinView.ip = ip;
	portEXIT_CRITICAL(&joinMux);
}

void HAQuDA_WebServer::publishLastJoin(join_last_result result, join_reason reason, const char *ssid) {
	portENTER_CRITICAL(&joinMux);
	HAQuDA_SetJoinLast(joinView, result, reason, ssid, WIFI_SSID_LEN);
	HAQuDA_EncodeJoinLast(rtcLastJoin, joinView);
	portEXIT_CRITICAL(&joinMux);
}

void HAQuDA_WebServer::restoreLastJoin() {
	join_view restored;
	// A software restart keeps RTC memory; after a power-on, a brown-out or a crash the record is not ours to trust, even
	// if the noise happened to pass the checksum
	bool valid = (esp_reset_reason() == ESP_RST_SW) && HAQuDA_DecodeJoinLast(rtcLastJoin, restored);
	portENTER_CRITICAL(&joinMux);
	if (valid) {
		HAQuDA_SetJoinLast(joinView, restored.lastResult, restored.lastReason, restored.lastSsid, sizeof(restored.lastSsid));
	} else {
		HAQuDA_ClearJoinLastRecord(rtcLastJoin);
	}
	portEXIT_CRITICAL(&joinMux);
}

// The page has shown the outcome of the last switch: forget it, so it is not shown again
void HAQuDA_WebServer::handleJoinDismiss(AsyncWebServerRequest *request) {
	portENTER_CRITICAL(&joinMux);
	HAQuDA_SetJoinLast(joinView, join_last_result::none, join_reason::none, nullptr, 0);
	HAQuDA_ClearJoinLastRecord(rtcLastJoin);
	portEXIT_CRITICAL(&joinMux);
	request->send(204);
}

void HAQuDA_WebServer::handleJoin(AsyncWebServerRequest *request) {
	join_view view;
	portENTER_CRITICAL(&joinMux);
	view = joinView;
	portEXIT_CRITICAL(&joinMux);

	// Time left until the restart a passed check scheduled; the page counts down from it. The delay and the request
	// time are stored before the pending flag, so reading them after it is consistent.
	if ((view.state == join_state::ok) && restartPending.load()) {
		uint32_t elapsedMs = (uint32_t)(millis() - restartRequestMs.load());
		uint32_t delayMs = restartDelayMs.load();
		view.restartMs = (elapsedMs >= delayMs) ? 0 : (delayMs - elapsedMs);
	}

	char host[32];
	snprintf(host, sizeof(host), "%s.local", STA_HOSTNAME);
	char buf[JOIN_JSON_LEN];
	if (!HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host)) {
		postLogError("handleJoin: join buffer too small");
		sendText(request, 500, "Status unavailable.");
		return;
	}
	AsyncWebServerResponse *response = request->beginResponse(200, "application/json", buf);
	response->addHeader("Cache-Control", "no-store");
	request->send(response);
}

void HAQuDA_WebServer::delete_WiFiCreds(AsyncWebServerRequest *request) {
	CredsLock lock(credsMutex);
	if (!lock.Held()) {
		postLogError((credsMutex == nullptr) ? "Delete: creds lock missing" : "Delete: creds file busy");
		sendText(request, 500, "Error deleting the saved networks.");
		return;
	}
	if (!HAQuDA_FileStorage::Exists(FILE_NAME_WIFI_NET)) {
		sendText(request, 200, "There are no saved networks.");
		return;
	}
	if (HAQuDA_FileStorage::DeleteFile(FILE_NAME_WIFI_NET)) {
		sendText(request, 200, "Deleted all saved networks.");
	} else {
		postLogError("delete_WiFiCreds: cannot delete the credentials file");
		sendText(request, 500, "Error deleting the saved networks.");
	}
}

void HAQuDA_WebServer::handleStatus(AsyncWebServerRequest *request) {
	static const char *const PARAM_KEYS[SNAPSHOT_PARAM_NUM] = {"temp", "humid", "eco2", "tvoc", "pm25"};
	// Decimals shown per parameter: the DHT11 reports tenths, the others whole numbers
	static const uint8_t PARAM_DECIMALS[SNAPSHOT_PARAM_NUM] = {1, 1, 0, 0, 0};

	char buf[STATUS_JSON_LEN];
	size_t pos = 0;
	buf[0] = '\0';

	// While a new network is checked the lamp runs access point and station together, but it still reports access
	// point mode: the page must not show "WiFi connection lost" for the test join
	bool stationMode = !apMode.load() && ((WiFi.getMode() & WIFI_MODE_STA) != 0);
	bool stationUp = stationMode && (WiFi.status() == WL_CONNECTED);
	IPAddress ip = stationMode ? WiFi.localIP() : WiFi.softAPIP();
	// esp_timer counts in 64 bits, millis() would wrap after 49 days
	uint32_t uptimeS = (uint32_t)(esp_timer_get_time() / 1000000LL);

	// Name of the joined network, read from the driver (no heap, unlike WiFi.SSID()); null when not joined
	char escapedSsid[JSON_ESCAPED_SSID_LEN] = {0};
	bool hasSsid = false;
	if (stationUp) {
		wifi_ap_record_t apInfo;
		if (esp_wifi_sta_get_ap_info(&apInfo) == ESP_OK) {
			HAQuDA_EscapeJson(reinterpret_cast<const char *>(apInfo.ssid), sizeof(apInfo.ssid), escapedSsid, sizeof(escapedSsid));
			hasSsid = escapedSsid[0] != '\0';
		}
	}

	bool fits = appendFormat(buf, sizeof(buf), pos, "{\"fw\":\"%s\",\"mode\":\"%s\",\"ssid\":", FW_VERSION, stationMode ? "sta" : "ap");
	if (hasSsid) {
		fits = fits && appendFormat(buf, sizeof(buf), pos, "\"%s\"", escapedSsid);
	} else {
		fits = fits && appendFormat(buf, sizeof(buf), pos, "null");
	}
	fits = fits && appendFormat(buf, sizeof(buf), pos, ",\"ip\":\"%u.%u.%u.%u\",\"uptime\":%lu,\"rssi\":", ip[0], ip[1], ip[2], ip[3], (unsigned long)uptimeS);
	if (stationUp) {
		fits = fits && appendFormat(buf, sizeof(buf), pos, "%d", (int)WiFi.RSSI());
	} else {
		fits = fits && appendFormat(buf, sizeof(buf), pos, "null");
	}
	fits = fits && appendFormat(buf, sizeof(buf), pos, ",\"meas\":{");

	for (size_t i = 0; i < SNAPSHOT_PARAM_NUM; i++) {
		float value = 0;
		uint32_t rgb = 0;
		const char *separator = (i == 0) ? "" : ",";
		if (measSnapshot.Get(static_cast<snapshot_param>(i), value, rgb)) {
			fits = fits
				   && appendFormat(buf, sizeof(buf), pos, "%s\"%s\":{\"v\":%.*f,\"c\":\"#%06lx\"}", separator, PARAM_KEYS[i], PARAM_DECIMALS[i], value,
								   (unsigned long)rgb);
		} else {
			fits = fits && appendFormat(buf, sizeof(buf), pos, "%s\"%s\":null", separator, PARAM_KEYS[i]);
		}
	}
	fits = fits && appendFormat(buf, sizeof(buf), pos, "}}");

	if (!fits) {
		postLogError("handleStatus: status buffer too small");
		sendText(request, 500, "Status unavailable.");
		return;
	}
	AsyncWebServerResponse *response = request->beginResponse(200, "application/json", buf);
	response->addHeader("Cache-Control", "no-store");
	request->send(response);
}

void HAQuDA_WebServer::handleDisp(AsyncWebServerRequest *request) {
	char buf[WEB_DISPLAY_JSON_BUF_LEN];
	if (webDisp.BuildJson(buf, sizeof(buf)) == 0) {
		postLogError("handleDisp: picture buffer too small");
		sendText(request, 500, "Status unavailable.");
		return;
	}
	AsyncWebServerResponse *response = request->beginResponse(200, "application/json", buf);
	response->addHeader("Cache-Control", "no-store");
	request->send(response);
}

// Identifies the lamp and tells how long it has run, nothing else. The WiFi page opened on the lamp's access point reads
// it cross-origin from the new network's address after a passed check, to see that the lamp has restarted. It is the only
// route that allows cross-origin reads, so it must not carry the network name, the address or measurements. A simple
// GET, so no preflight.
void HAQuDA_WebServer::handleBoot(AsyncWebServerRequest *request) {
	char buf[96];
	// esp_timer counts in 64 bits, millis() would wrap after 49 days
	uint32_t uptimeS = (uint32_t)(esp_timer_get_time() / 1000000LL);
	int written = snprintf(buf, sizeof(buf), "{\"fw\":\"%s\",\"uptime\":%lu,\"host\":\"%s\"}", FW_VERSION, (unsigned long)uptimeS, STA_HOSTNAME);
	if ((written < 0) || (static_cast<size_t>(written) >= sizeof(buf))) {
		postLogError("handleBoot: boot buffer too small");
		sendText(request, 500, "Status unavailable.");
		return;
	}
	AsyncWebServerResponse *response = request->beginResponse(200, "application/json", buf);
	response->addHeader("Cache-Control", "no-store");
	response->addHeader("Access-Control-Allow-Origin", "*");
	request->send(response);
}

void HAQuDA_WebServer::handleWiFiList(AsyncWebServerRequest *request) {
	CredsLock lock(credsMutex);
	if (!lock.Held()) {
		postLogError((credsMutex == nullptr) ? "List: creds lock missing" : "List: creds file busy");
		sendText(request, 500, "The lamp cannot read its saved networks. Try again.");
		return;
	}
	char buf[WIFI_LIST_JSON_LEN];
	char escapedSsid[JSON_ESCAPED_SSID_LEN];
	size_t pos = 0;
	bool first = true;
	buf[0] = '\0';
	appendFormat(buf, sizeof(buf), pos, "[");

	int credsNum = GetStored_WiFiCredsNum();
	if (credsNum > MAX_WIFI_CREDS_NUM) {
		credsNum = MAX_WIFI_CREDS_NUM;
	}
	for (int i = 0; i < credsNum; i++) {
		TWiFiCreds creds;
		WipeOnExit wipeCreds(&creds, sizeof(creds));
		if (!HAQuDA_FileStorage::ReadFile(FILE_NAME_WIFI_NET, i * (int)sizeof(TWiFiCreds), (uint8_t *)&creds, sizeof(TWiFiCreds))) {
			postLogError("handleWiFiList: cannot read a saved network");
			continue;
		}
		if (strnlen(creds.ssid, sizeof(creds.ssid)) == 0) {
			continue;
		}
		HAQuDA_EscapeJson(creds.ssid, sizeof(creds.ssid), escapedSsid, sizeof(escapedSsid));
		// The passwords never leave the lamp: only the network name is copied out of the record
		if (!appendFormat(buf, sizeof(buf), pos, "%s\"%s\"", first ? "" : ",", escapedSsid)) {
			break;
		}
		first = false;
	}
	// MAX_WIFI_CREDS_NUM escaped names always fit, so the bracket cannot overflow; a failure still leaves a valid empty list
	if (!appendFormat(buf, sizeof(buf), pos, "]")) {
		strcpy(buf, "[]");
	}

	AsyncWebServerResponse *response = request->beginResponse(200, "application/json", buf);
	response->addHeader("Cache-Control", "no-store");
	request->send(response);
}

void HAQuDA_WebServer::handle_NotFound(AsyncWebServerRequest *request) {
	request->send(404);
}

void HAQuDA_WebServer::WebServerResponds_init() {
	// Routes added here are checked before the captive handler (added last), so AP clients reach them too
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) { HAQuDA_SendIndexPage(request); });
	server.on("/wifi_manager", HTTP_GET,
			  [](AsyncWebServerRequest *request) { sendAsset(request, "text/html", WEB_WIFI_MANAGER_HTML_GZ, WEB_WIFI_MANAGER_HTML_GZ_LEN, CACHE_NONE); });
	server.on("/style.css", HTTP_GET,
			  [](AsyncWebServerRequest *request) { sendAsset(request, "text/css", WEB_STYLE_CSS_GZ, WEB_STYLE_CSS_GZ_LEN, CACHE_STATIC); });
	server.on("/app.js", HTTP_GET,
			  [](AsyncWebServerRequest *request) { sendAsset(request, "application/javascript", WEB_APP_JS_GZ, WEB_APP_JS_GZ_LEN, CACHE_STATIC); });
	server.on("/wifi.js", HTTP_GET,
			  [](AsyncWebServerRequest *request) { sendAsset(request, "application/javascript", WEB_WIFI_JS_GZ, WEB_WIFI_JS_GZ_LEN, CACHE_STATIC); });

	server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) { handleStatus(request); });
	server.on("/api/disp", HTTP_GET, [this](AsyncWebServerRequest *request) { handleDisp(request); });
	server.on("/api/wifi", HTTP_GET, [this](AsyncWebServerRequest *request) { handleWiFiList(request); });
	server.on("/api/boot", HTTP_GET, [this](AsyncWebServerRequest *request) { handleBoot(request); });
	server.on("/api/join", HTTP_GET, [this](AsyncWebServerRequest *request) { handleJoin(request); });
	server.on("/api/join/dismiss", HTTP_POST, [this](AsyncWebServerRequest *request) { handleJoinDismiss(request); });

	server.on("/delete_wifi_creds", HTTP_POST, [this](AsyncWebServerRequest *request) { delete_WiFiCreds(request); });
	server.on("/add_wifi_creds", HTTP_POST, [this](AsyncWebServerRequest *request) { handle_NewWiFiCreds(request); });
	server.onNotFound(HAQuDA_WebServer::handle_NotFound);
}

bool HAQuDA_WebServer::SaveCredsFirst(const TWiFiCreds &newCreds) {
	CredsLock lock(credsMutex);
	if (!lock.Held()) {
		postLogError((credsMutex == nullptr) ? "Save first: creds lock missing" : "Save first: creds file busy");
		return false;
	}

	// Both lists hold passwords: wiped on every way out
	TWiFiCreds stored[MAX_WIFI_CREDS_NUM];
	TWiFiCreds ordered[MAX_WIFI_CREDS_NUM];
	WipeOnExit wipeStored(stored, sizeof(stored));
	WipeOnExit wipeOrdered(ordered, sizeof(ordered));
	int storedNum = GetStored_WiFiCredsNum();
	if (storedNum > MAX_WIFI_CREDS_NUM) {
		storedNum = MAX_WIFI_CREDS_NUM;
	}
	for (int i = 0; i < storedNum; i++) {
		if (!HAQuDA_FileStorage::ReadFile(FILE_NAME_WIFI_NET, i * (int)sizeof(TWiFiCreds), (uint8_t *)&stored[i], sizeof(TWiFiCreds))) {
			postLogError("Save first: cannot read a saved network");
			return false;
		}
	}

	size_t orderedNum = HAQuDA_PlaceCredsFirst(stored, storedNum, newCreds, ordered, MAX_WIFI_CREDS_NUM);
	if (orderedNum == 0) {
		postLogError("Save first: no network name");
		return false;
	}
	size_t orderedLen = orderedNum * sizeof(TWiFiCreds);

	// WriteFile() truncates, so a temporary file left by an interrupted earlier save is overwritten
	if (!HAQuDA_FileStorage::WriteFile(FILE_NAME_WIFI_NET_TMP, (const uint8_t *)ordered, orderedLen)) {
		HAQuDA_FileStorage::DeleteFile(FILE_NAME_WIFI_NET_TMP);
		postLogError("Save first: cannot write the temporary file");
		return false;
	}
	if (HAQuDA_FileStorage::Exists(FILE_NAME_WIFI_NET) && !HAQuDA_FileStorage::DeleteFile(FILE_NAME_WIFI_NET)) {
		HAQuDA_FileStorage::DeleteFile(FILE_NAME_WIFI_NET_TMP);
		postLogError("Save first: cannot replace the credentials file");
		return false;
	}
	if (HAQuDA_FileStorage::RenameFile(FILE_NAME_WIFI_NET_TMP, FILE_NAME_WIFI_NET)) {
		return true;
	}
	// The old list is already gone: write the new one directly rather than leave no file at all
	postLogError("Save first: rename failed, writing the file directly");
	bool written = HAQuDA_FileStorage::WriteFile(FILE_NAME_WIFI_NET, (const uint8_t *)ordered, orderedLen);
	HAQuDA_FileStorage::DeleteFile(FILE_NAME_WIFI_NET_TMP);
	return written;
}

// The list is missing and the temporary file holds whole records: makes the valid ones among the first records the list.
// Boot only, before the web server runs, so nothing else uses the files.
static void restoreCredsFromTmp(int tmpSize) {
	// Both buffers hold passwords: wiped on every way out
	TWiFiCreds read[CREDS_RECOVERY_READ_MAX];
	TWiFiCreds kept[MAX_WIFI_CREDS_NUM];
	WipeOnExit wipeRead(read, sizeof(read));
	WipeOnExit wipeKept(kept, sizeof(kept));

	size_t totalNum = static_cast<size_t>(tmpSize) / sizeof(TWiFiCreds);
	size_t readNum = (totalNum < CREDS_RECOVERY_READ_MAX) ? totalNum : CREDS_RECOVERY_READ_MAX;
	for (size_t i = 0; i < readNum; i++) {
		if (!HAQuDA_FileStorage::ReadFile(FILE_NAME_WIFI_NET_TMP, static_cast<int>(i * sizeof(TWiFiCreds)), (uint8_t *)&read[i], sizeof(TWiFiCreds))) {
			// Not deleted: the next boot tries again
			HAQuDA_Logger::LogError("RecoverCredsFile: cannot read the temporary file");
			return;
		}
	}
	size_t keptNum = HAQuDA_FilterRecoveredCreds(read, readNum, kept, MAX_WIFI_CREDS_NUM);

	if (keptNum == 0) {
		HAQuDA_Logger::LogError("RecoverCredsFile: no valid saved network in the temporary file, dropped");
	} else if (keptNum == totalNum) {
		// Nothing dropped: the rename cannot leave a half-written list
		if (!HAQuDA_FileStorage::RenameFile(FILE_NAME_WIFI_NET_TMP, FILE_NAME_WIFI_NET)) {
			HAQuDA_Logger::LogError("RecoverCredsFile: cannot restore the saved networks");
		} else {
			HAQuDA_Logger::LogInfo("Saved networks restored after an interrupted save");
		}
		return;
	} else {
		// Nothing else exists yet, so the list is written directly; the temporary file stays until that worked
		if (!HAQuDA_FileStorage::WriteFile(FILE_NAME_WIFI_NET, (const uint8_t *)kept, keptNum * sizeof(TWiFiCreds))) {
			HAQuDA_FileStorage::DeleteFile(FILE_NAME_WIFI_NET);
			HAQuDA_Logger::LogError("RecoverCredsFile: cannot write the restored networks");
			return;
		}
		HAQuDA_Logger::LogError("RecoverCredsFile: saved networks restored from an interrupted save, some records dropped");
	}
	if (!HAQuDA_FileStorage::DeleteFile(FILE_NAME_WIFI_NET_TMP)) {
		HAQuDA_Logger::LogError("RecoverCredsFile: cannot delete the temporary file");
	}
}

void HAQuDA_WebServer::RecoverCredsFile() {
	bool listExists = HAQuDA_FileStorage::Exists(FILE_NAME_WIFI_NET);
	bool tmpExists = HAQuDA_FileStorage::Exists(FILE_NAME_WIFI_NET_TMP);
	int tmpSize = tmpExists ? HAQuDA_FileStorage::FileSize(FILE_NAME_WIFI_NET_TMP) : 0;
	switch (HAQuDA_DecideCredsRecovery(listExists, tmpExists, tmpSize)) {
		case creds_recovery::restore_tmp:
			// The power failed between the delete of the old list and the rename: the temporary file is the new list
			restoreCredsFromTmp(tmpSize);
			break;
		case creds_recovery::delete_tmp:
			if (!HAQuDA_FileStorage::DeleteFile(FILE_NAME_WIFI_NET_TMP)) {
				HAQuDA_Logger::LogError("RecoverCredsFile: cannot delete the temporary file");
			}
			break;
		case creds_recovery::none:
		default:
			break;
	}
}

TWiFiCreds HAQuDA_WebServer::GetStored_WiFi(int num) {
	TWiFiCreds WiFiCreds;

	int pos = num * sizeof(TWiFiCreds);

	HAQuDA_FileStorage::ReadFile(FILE_NAME_WIFI_NET, pos, (uint8_t *)&WiFiCreds, sizeof(TWiFiCreds));
	return WiFiCreds;
}

int HAQuDA_WebServer::GetStored_WiFiCredsNum() {
	int fSize = HAQuDA_FileStorage::FileSize(FILE_NAME_WIFI_NET);
	if (fSize <= 0) {
		return 0;
	}
	return fSize / sizeof(TWiFiCreds);
}

HAQuDA_WebServer::~HAQuDA_WebServer() {
}
