#include "HAQuDA_WebServer.h"
// Only this file includes the page data: a second include would copy it into another object file
#include "WebPages.h"
#include "HAQuDA_Version.h"

#include <esp_timer.h>
#include <cstdarg>
#include <cstdio>
#include <cstring>

constexpr size_t STATUS_JSON_LEN = 512;
constexpr size_t WIFI_LIST_JSON_LEN = 2048;
// A SSID byte is escaped to at most 6 characters (\u00XX)
constexpr size_t JSON_ESCAPED_SSID_LEN = WIFI_SSID_LEN * 6 + 1;
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

	// Writes the JSON string body (no quotes) of a possibly unterminated record field into out
	void escapeJson(const char *text, size_t maxLen, char *out, size_t outSize) {
		size_t pos = 0;
		size_t textLen = strnlen(text, maxLen);
		for (size_t i = 0; i < textLen; i++) {
			unsigned char c = static_cast<unsigned char>(text[i]);
			int written = 0;
			if ((c == '"') || (c == '\\')) {
				written = snprintf(out + pos, outSize - pos, "\\%c", c);
			} else if (c < 0x20) {
				written = snprintf(out + pos, outSize - pos, "\\u%04x", c);
			} else {
				written = snprintf(out + pos, outSize - pos, "%c", c);
			}
			if ((written < 0) || (static_cast<size_t>(written) >= outSize - pos)) {
				break;
			}
			pos += written;
		}
		out[pos] = '\0';
	}

	void sendAsset(AsyncWebServerRequest *request, const char *mime, const uint8_t *data, size_t len, const char *cacheControl) {
		AsyncWebServerResponse *response = request->beginResponse_P(200, mime, data, len);
		response->addHeader("Content-Encoding", "gzip");
		response->addHeader("Cache-Control", cacheControl);
		request->send(response);
	}

	void sendText(AsyncWebServerRequest *request, int code, const char *text) {
		request->send(code, "text/plain", text);
	}
} // namespace

void HAQuDA_SendIndexPage(AsyncWebServerRequest *request) {
	sendAsset(request, "text/html", WEB_INDEX_HTML_GZ, WEB_INDEX_HTML_GZ_LEN, CACHE_NONE);
}

bool HAQuDA_WebServer::WebServerStarted = false;

void HAQuDA_WebServer::beginWebServer() {
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
	if ((uint32_t)(millis() - restartRequestMs.load()) >= RESTART_DELAY_MS) {
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

	saveNewWiFiCredsReturnMsgs saveNewWiFiCredsMsg = SaveNew_WiFiCreds(newWiFiCreds);

	switch (saveNewWiFiCredsMsg) {
		case too_many_WiFi:
			postLogError("handle_NewWiFiCreds: no room for another network");
			sendText(request, 409,
					 "The lamp stores 10 networks and all places are taken. "
					 "Press Delete all networks on the WiFi networks page, then add this network again.");
			break;
		case error_reading_stored_WiFi_creds:
			postLogError("handle_NewWiFiCreds: cannot read the saved networks");
			sendText(request, 500,
					 "The lamp cannot read its saved networks. Try again. "
					 "If it fails again, press Delete all networks on the WiFi networks page and add the network again.");
			break;
		case error_saving_new_WiFi_creds:
			postLogError("handle_NewWiFiCreds: cannot write the network");
			sendText(request, 500,
					 "The lamp cannot save the network. Try again. "
					 "If it fails again, press Delete all networks on the WiFi networks page and add the network again.");
			break;
		case re_writed_WiFi_creds:
			sendText(request, 200, "This network was already saved. The password is updated and the lamp restarts in 3 seconds.");
			scheduleRestart();
			break;
		case saved_new_WiFi_creds:
			sendText(request, 200, "Saved. The lamp restarts in 3 seconds.");
			scheduleRestart();
			break;
		default:
			postLogError("handle_NewWiFiCreds: unexpected save result");
			sendText(request, 500, "The lamp cannot save the network. Try again.");
			break;
	}
}

// loop() restarts the lamp: waiting in the async_tcp task would block it
void HAQuDA_WebServer::scheduleRestart() {
	if (!restartPending.load()) {
		restartRequestMs.store(millis());
		restartPending.store(true);
	}
}

void HAQuDA_WebServer::delete_WiFiCreds(AsyncWebServerRequest *request) {
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

	bool stationMode = (WiFi.getMode() & WIFI_MODE_STA) != 0;
	bool stationUp = stationMode && (WiFi.status() == WL_CONNECTED);
	IPAddress ip = stationMode ? WiFi.localIP() : WiFi.softAPIP();
	// esp_timer counts in 64 bits, millis() would wrap after 49 days
	uint32_t uptimeS = (uint32_t)(esp_timer_get_time() / 1000000LL);

	bool fits = appendFormat(buf, sizeof(buf), pos, "{\"fw\":\"%s\",\"mode\":\"%s\",\"ip\":\"%u.%u.%u.%u\",\"uptime\":%lu,\"rssi\":", FW_VERSION,
							 stationMode ? "sta" : "ap", ip[0], ip[1], ip[2], ip[3], (unsigned long)uptimeS);
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

void HAQuDA_WebServer::handleWiFiList(AsyncWebServerRequest *request) {
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
		if (!HAQuDA_FileStorage::ReadFile(FILE_NAME_WIFI_NET, i * (int)sizeof(TWiFiCreds), (uint8_t *)&creds, sizeof(TWiFiCreds))) {
			postLogError("handleWiFiList: cannot read a saved network");
			continue;
		}
		if (strnlen(creds.ssid, sizeof(creds.ssid)) == 0) {
			continue;
		}
		escapeJson(creds.ssid, sizeof(creds.ssid), escapedSsid, sizeof(escapedSsid));
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
	server.on("/api/wifi", HTTP_GET, [this](AsyncWebServerRequest *request) { handleWiFiList(request); });

	server.on("/delete_wifi_creds", HTTP_POST, [this](AsyncWebServerRequest *request) { delete_WiFiCreds(request); });
	server.on("/add_wifi_creds", HTTP_POST, [this](AsyncWebServerRequest *request) { handle_NewWiFiCreds(request); });
	server.onNotFound(HAQuDA_WebServer::handle_NotFound);
}

// True when one more record fits. GetStored_WiFiCredsNum() counts a missing file as zero records, whether
// SPIFFS reports its size as 0 or as -1.
bool HAQuDA_WebServer::checkStoredWiFiCredsCntLimit() {
	return GetStored_WiFiCredsNum() < MAX_WIFI_CREDS_NUM;
}

// Finds where newWiFiCreds goes: the index of the stored record with the same SSID (it is overwritten), or the
// record count (it is appended). Returns false when a record cannot be read.
bool HAQuDA_WebServer::getWiFiCredsWriteIndex(uint16_t *index, const TWiFiCreds newWiFiCreds) {
	if (index == nullptr) {
		return false;
	}
	int storedWiFiCredsCnt = GetStored_WiFiCredsNum();
	TWiFiCreds readWiFiCreds;

	for (int i = 0; i < storedWiFiCredsCnt; i++) {
		if (!HAQuDA_FileStorage::ReadFile(FILE_NAME_WIFI_NET, i * (int)sizeof(TWiFiCreds), (uint8_t *)&readWiFiCreds, sizeof(TWiFiCreds))) {
			return false;
		}
		if (strncmp(readWiFiCreds.ssid, newWiFiCreds.ssid, WIFI_SSID_LEN) == 0) {
			*index = i;
			return true;
		}
	}
	*index = storedWiFiCredsCnt;
	return true;
}

saveNewWiFiCredsReturnMsgs HAQuDA_WebServer::SaveNew_WiFiCreds(TWiFiCreds newWiFiCreds) {
	uint16_t index = 0;
	if (!getWiFiCredsWriteIndex(&index, newWiFiCreds)) {
		return error_reading_stored_WiFi_creds;
	}

	// An existing SSID is overwritten in place, so it needs no free record even when the file is full
	bool overwrite = (index < GetStored_WiFiCredsNum());
	if (overwrite) {
		if (!HAQuDA_FileStorage::WriteFile(FILE_NAME_WIFI_NET, index * sizeof(TWiFiCreds), (uint8_t *)&newWiFiCreds, sizeof(TWiFiCreds))) {
			return error_saving_new_WiFi_creds;
		}
		return re_writed_WiFi_creds;
	}

	if (!checkStoredWiFiCredsCntLimit()) {
		return too_many_WiFi;
	}
	if (!HAQuDA_FileStorage::AppendFile(FILE_NAME_WIFI_NET, (uint8_t *)&newWiFiCreds, sizeof(TWiFiCreds))) {
		return error_saving_new_WiFi_creds;
	}
	return saved_new_WiFi_creds;
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
