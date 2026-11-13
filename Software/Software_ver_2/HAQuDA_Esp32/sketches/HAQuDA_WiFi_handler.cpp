#include <ESPmDNS.h>
#include <esp_wifi.h>

#include "HAQuDA_WiFi_handler.h"

#define CONNECT_WIFI_TIMEOUT 3000
#define WAIT_FOR_SEND_WEB_SERIAL_MS 1000

constexpr uint32_t DNS_POLL_PERIOD_MS = 10;

bool HAQuDA_WiFi_handler::WiFiConnected = false;

const char *DDNS_token = DDNS_TOKEN;
const char *DDNS_domain = DDNS_DOMAIN;

static HAQuDA_DisplayManip *dispManip;

HAQuDA_WiFi_handler::HAQuDA_WiFi_handler(HAQuDA_FileStorage *currFS, HAQuDA_DisplayManip *_dispManip, const HAQuDA_MeasSnapshot &snapshot,
										 HAQuDA_WebPageDisplayManip &webDispIn)
	: HAQuDA_WebServer(currFS, snapshot, webDispIn) {
	dispManip = _dispManip;
};

bool HAQuDA_WiFi_handler::Connect() {

	staConnectedEventId = WiFi.onEvent(WiFiStationConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED);
	vTaskDelay(200 / portTICK_PERIOD_MS);

	// An interrupted save may have left the list in the temporary file: it must be back before the records are counted
	RecoverCredsFile();
	if (GetStored_WiFiCredsNum()) {
		if (connectToStoredWiFi()) {
			// Before the display handler: that one blocks the event task for 3 s, and the reason must be recorded first
			registerDisconnectRecorder();
			WiFi.onEvent(WiFiStationDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
			vTaskDelay(200 / portTICK_PERIOD_MS);
			HAQuDA_WebServer::beginWebServer();
			startMdns();
			EasyDDNS.service("duckdns");
			EasyDDNS.client(DDNS_domain, DDNS_token); // Enter your DDNS Domain & Token

			// Get Notified when your IP changes
			EasyDDNS.onUpdate([&](const char *oldIP, const char *newIP) {
				Serial.print("EasyDDNS - IP Change Detected: ");
				Serial.println(newIP);
			});
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

bool HAQuDA_WiFi_handler::CreateAP() {
	// Connect() registered the display handler for STA_CONNECTED even though it failed. The test join of a new network
	// fires that event too, and the handler would set the display to static green and then off, replacing the access
	// point effect. Removed by id: core 2.0.11 can remove a std::function registration only that way.
	if (staConnectedEventId != 0) {
		WiFi.removeEvent(staConnectedEventId);
		staConnectedEventId = 0;
	}
	// Registered once, before the access point serves anyone: it only records the reason of a station disconnect while a
	// check runs. See joinActive for why it is never removed.
	registerDisconnectRecorder();
	if (!createAP()) {
		return false;
	}
	apMode.store(true);
	// The captive DNS cannot serve http://haquda.local/: iOS, macOS and newer Android resolve *.local names only by
	// multicast mDNS and never ask the unicast DNS server. The AP is up here, so the responder attaches to the AP interface.
	startMdns();
	dnsServerRunning = dnsServer.start(53, "*", WiFi.softAPIP());
	if (!dnsServerRunning) {
		HAQuDA_Logger::LogError("CreateAP: captive DNS not started");
	}
	HAQuDA_WebServer::beginWebServer();
	return true;
}

int web_serial_send_timeout;
bool start_send_web_serial_timer = false;

void HAQuDA_WiFi_handler::HandleConnection() {
	// Check for new public IP every 10 seconds
	// EasyDDNS.update(10000, true);

	HandleDeferredWork();
	handleJoinCheck();

	// Captive portal: answers every DNS query with the AP address so the phone's connectivity probe
	// reaches the web server. AP mode only; station mode runs no DNS server.
	// Polled every DNS_POLL_PERIOD_MS only: in core 2.0.11 each WiFiUDP::parsePacket() allocates and frees a
	// 1460-byte buffer even when no packet waits, so polling on every loop() pass churns the heap.
	if (dnsServerRunning && (millis() - dnsPollMs >= DNS_POLL_PERIOD_MS)) {
		dnsPollMs = millis();
		dnsServer.processNextRequest();
	}

	if ((MyWebSerial.GetWebSerialOpened()) && !start_send_web_serial_timer) {
		start_send_web_serial_timer = true;
		web_serial_send_timeout = millis();
	}

	if (start_send_web_serial_timer && (millis() - web_serial_send_timeout > WAIT_FOR_SEND_WEB_SERIAL_MS)) {
		WebSerialPrintStoredLogs();
		start_send_web_serial_timer = false;
		MyWebSerial.ResetWebSerialOpened();
	}

	// The reconnect loop below is for station-only mode. In access point mode the station is up only for the check of
	// a new network (WIFI_AP_STA): the loop would block loop() forever and WiFi.mode(WIFI_STA) would drop the access point.
	// During a station mode switch the lamp has left its network on purpose and the loop would rejoin it, undoing the switch
	// and blocking loop() so the check could not run.
	if (WiFiConnected || apMode.load() || stationSwitchActive || !(WiFi.getMode() & WIFI_MODE_STA)) {
		return;
	}
	WiFi.removeEvent(SYSTEM_EVENT_STA_DISCONNECTED);

	dispManip->SetEffect(snake);
	vTaskDelay(2000 / portTICK_PERIOD_MS);

	while (!WiFiConnected) {
		connectToStoredWiFi();
	}
	WiFi.onEvent(WiFiStationDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
}

void HAQuDA_WiFi_handler::handleJoinCheck() {
	if (joinCheck.Active()) {
		pollJoinCheck();
		return;
	}
	if (!joinRequestPending.load()) {
		return;
	}
	TWiFiCreds creds;
	if (takeJoinRequest(creds)) {
		if (apMode.load()) {
			startJoinCheck(creds);
		} else {
			startStationSwitch(creds);
		}
	}
	memset(&creds, 0, sizeof(creds));
}

void HAQuDA_WiFi_handler::startJoinCheck(const TWiFiCreds &creds) {
	joinCreds = creds;
	// A streak left by the previous check must not count towards this one
	joinDisconnectReason.store(0);
	joinDisconnectStreak.store(0);
	joinActive.store(true);
	joinCheck.Start(millis());
	HAQuDA_Logger::LogInfo("WiFi check started");

	// A failed check switches the core's auto-reconnect off; the core retries a join by itself, which is what the
	// check wants while it runs
	WiFi.setAutoReconnect(true);

	// WiFi.setHostname() only stores the name; WiFi.mode() hands it to the station interface when it enables the
	// station, and the DHCP request carries it. So the name must be set before the mode call. The check always starts
	// from WIFI_AP, so the station is enabled here; had it been enabled already, mode() would return early and keep the old name.
	if (!WiFi.setHostname(STA_HOSTNAME)) {
		HAQuDA_Logger::LogError("WiFi check: hostname not set");
	}
	// A failure here is not fatal: without a station the check runs into its timeout and reports it
	// Access point and captive DNS stay up: WIFI_AP_STA only adds the station
	if (!WiFi.mode(WIFI_AP_STA)) {
		HAQuDA_Logger::LogError("WiFi check: AP_STA mode failed");
	}
	if (WiFi.begin(joinCreds.ssid, joinCreds.pass) == WL_CONNECT_FAILED) {
		HAQuDA_Logger::LogError("WiFi check: begin failed");
	}
}

void HAQuDA_WiFi_handler::pollJoinCheck() {
	if (stationSwitchActive) {
		pollStationSwitch();
		return;
	}
	bool connected = (WiFi.status() == WL_CONNECTED);
	uint32_t ip = connected ? static_cast<uint32_t>(WiFi.localIP()) : 0;
	join_action action = joinCheck.Poll(millis(), connected, ip, joinDisconnectReason.load(), joinDisconnectStreak.load());

	if (action == join_action::save_network) {
		// First record: after the restart connectToStoredWiFi() tries the records in file order, and each unreachable older
		// network would cost up to ≈ 32 s before the new one. A full list loses its oldest record instead of failing.
		bool saved = SaveCredsFirst(joinCreds);
		action = joinCheck.SaveFinished(saved);
		if (saved) {
			finishJoinOk();
			return;
		}
	}
	if (action == join_action::leave_station) {
		finishJoinFail();
	}
}

void HAQuDA_WiFi_handler::finishJoinOk() {
	joinActive.store(false);
	memset(&joinCreds, 0, sizeof(joinCreds));
	// The restart goes first: a POST that arrives between the two steps must already see it pending
	scheduleRestart(JOIN_OK_HOLD_MS);
	publishJoinState(join_state::ok, join_reason::none, joinCheck.Ip());
	HAQuDA_Logger::LogInfo("WiFi check: ok, network saved");
}

void HAQuDA_WiFi_handler::finishJoinFail() {
	joinActive.store(false);
	memset(&joinCreds, 0, sizeof(joinCreds));
	// Reasons such as NO_AP_FOUND make the core call disconnect() and begin() from the event task, which would bring the
	// station back. Auto-reconnect off, and the station config erased so a begin() already under way has nothing to join.
	WiFi.setAutoReconnect(false);
	// Station only: WiFi.mode(WIFI_AP) leaves the access point, its IP configuration and the DHCP server as they are
	WiFi.disconnect(false, true);
	if (!WiFi.mode(WIFI_AP)) {
		HAQuDA_Logger::LogError("WiFi check: AP mode failed");
	}
	join_reason reason = joinCheck.Reason();
	publishJoinState(join_state::fail, reason, 0);

	char msg[MSG_DATA_MAX_LEN] = {0};
	snprintf(msg, sizeof(msg), "WiFi check: %s", HAQuDA_JoinReasonName(reason));
	HAQuDA_Logger::LogError(msg);
}

void HAQuDA_WiFi_handler::recordDisconnectReason(uint8_t reason) {
	if (!joinActive.load()) {
		return;
	}
	// The only writer while a check runs, so the read-merge-store needs no loop
	joinDisconnectReason.store(HAQuDA_MergeDisconnectReason(joinDisconnectReason.load(), reason));
	joinDisconnectStreak.store(HAQuDA_AddDisconnectEvent(joinDisconnectStreak.load(), reason));
}

void HAQuDA_WiFi_handler::registerDisconnectRecorder() {
	if (disconnectRecorderRegistered) {
		return;
	}
	disconnectRecorderRegistered = true;
	WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) { recordDisconnectReason(info.wifi_sta_disconnected.reason); },
				 ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
}

void HAQuDA_WiFi_handler::startStationSwitch(const TWiFiCreds &creds) {
	// Set only once the switch really started: while it is set HandleConnection() does not reconnect
	// (joinCreds belongs to the running check then, so it is left alone)
	if (!joinCheck.StartSwitch(millis())) {
		publishJoinState(join_state::fail, join_reason::scan_failed, 0);
		HAQuDA_Logger::LogError("WiFi switch: not started, a check is already running");
		return;
	}
	joinCreds = creds;
	stationSwitchActive = true;
	HAQuDA_Logger::LogInfo("WiFi switch: scanning for the network");

	// Asked for this one name, so the driver sends directed probe requests and a hidden network answers too; the result
	// then holds only this network. The scan runs next to the open connection and does not drop it. Asynchronous: the
	// result is polled in pollStationSwitch(). A start that fails shows up there as a failed scan.
	WiFi.scanNetworks(true, true, false, 300, 0, joinCreds.ssid);
}

void HAQuDA_WiFi_handler::pollStationSwitch() {
	uint32_t nowMs = millis();
	bool wasScanning = joinCheck.Scanning();
	join_scan_status scan = wasScanning ? HAQuDA_MapScanResult(WiFi.scanComplete()) : join_scan_status::running;
	join_action action = joinCheck.PollSwitch(nowMs, scan, WiFi.status() == WL_CONNECTED);

	if (wasScanning && !joinCheck.Scanning()) {
		// Frees the result list the core keeps until the next scan
		WiFi.scanDelete();
		if (joinCheck.State() == join_state::fail) {
			finishSwitchScanFail();
			return;
		}
		publishJoinState(join_state::switching, join_reason::none, 0);
		HAQuDA_Logger::LogInfo("WiFi switch: network found, leaving the current one");
		return;
	}
	if (action == join_action::disconnect_old) {
		// Station only, the settings stay: the core does not retry after its own voluntary disconnect
		WiFi.disconnect(false, false);
		return;
	}
	if (action == join_action::begin_join) {
		beginSwitchJoin();
		return;
	}
	if (!joinCheck.Joining()) {
		return;
	}

	bool connected = joinedNetwork(joinCreds.ssid);
	uint32_t ip = connected ? static_cast<uint32_t>(WiFi.localIP()) : 0;
	action = joinCheck.Poll(nowMs, connected, ip, joinDisconnectReason.load(), joinDisconnectStreak.load());
	if (action == join_action::save_network) {
		bool saved = SaveCredsFirst(joinCreds);
		action = joinCheck.SaveFinished(saved);
		if (saved) {
			finishSwitchOk();
			return;
		}
	}
	if (action == join_action::leave_station) {
		finishSwitchFail();
	}
}

void HAQuDA_WiFi_handler::beginSwitchJoin() {
	// A streak left by an earlier check must not count towards this one
	joinDisconnectReason.store(0);
	joinDisconnectStreak.store(0);
	joinActive.store(true);

	// The core retries a join by itself, which is what the check wants while it runs
	WiFi.setAutoReconnect(true);
	// The station is on already, so begin() does not call WiFi.mode(): the hostname set when the station was first enabled
	// stays. Set again so that it is right if the mode ever was switched off in between.
	if (!WiFi.setHostname(STA_HOSTNAME)) {
		HAQuDA_Logger::LogError("WiFi switch: hostname not set");
	}
	if (WiFi.begin(joinCreds.ssid, joinCreds.pass) == WL_CONNECT_FAILED) {
		HAQuDA_Logger::LogError("WiFi switch: begin failed");
	}
}

void HAQuDA_WiFi_handler::finishSwitchScanFail() {
	stationSwitchActive = false;
	memset(&joinCreds, 0, sizeof(joinCreds));
	join_reason reason = joinCheck.Reason();
	publishJoinState(join_state::fail, reason, 0);

	char msg[MSG_DATA_MAX_LEN] = {0};
	snprintf(msg, sizeof(msg), "WiFi switch: %s, staying on the current network", HAQuDA_JoinReasonName(reason));
	HAQuDA_Logger::LogError(msg);
}

void HAQuDA_WiFi_handler::finishSwitchOk() {
	joinActive.store(false);
	char ssid[WIFI_SSID_LEN] = {0};
	strncpy(ssid, joinCreds.ssid, sizeof(ssid) - 1);
	memset(&joinCreds, 0, sizeof(joinCreds));

	// The outcome and the restart go first: a POST that arrives between the steps must already see the restart pending
	publishLastJoin(join_last_result::ok, join_reason::none, ssid);
	scheduleRestart(SWITCH_RESTART_DELAY_MS);
	publishJoinState(join_state::ok, join_reason::none, joinCheck.Ip());
	HAQuDA_Logger::LogInfo("WiFi switch: ok, network saved first");
}

void HAQuDA_WiFi_handler::finishSwitchFail() {
	joinActive.store(false);
	char ssid[WIFI_SSID_LEN] = {0};
	strncpy(ssid, joinCreds.ssid, sizeof(ssid) - 1);
	memset(&joinCreds, 0, sizeof(joinCreds));
	join_reason reason = joinCheck.Reason();

	// The core would keep joining the refused network until the restart, and each disconnect flashes the display
	WiFi.setAutoReconnect(false);
	publishLastJoin(join_last_result::fail, reason, ssid);
	scheduleRestart(SWITCH_RESTART_DELAY_MS);
	publishJoinState(join_state::fail, reason, 0);

	char msg[MSG_DATA_MAX_LEN] = {0};
	snprintf(msg, sizeof(msg), "WiFi switch: %s, restarting to rejoin the saved networks", HAQuDA_JoinReasonName(reason));
	HAQuDA_Logger::LogError(msg);
}

bool HAQuDA_WiFi_handler::joinedNetwork(const char *ssid) const {
	if (WiFi.status() != WL_CONNECTED) {
		return false;
	}
	wifi_ap_record_t apInfo;
	if (esp_wifi_sta_get_ap_info(&apInfo) != ESP_OK) {
		return false;
	}
	// The driver terminates the name in its 33-byte field
	return strncmp(reinterpret_cast<const char *>(apInfo.ssid), ssid, sizeof(apInfo.ssid)) == 0;
}

void HAQuDA_WiFi_handler::startMdns() {
	if (mdnsStartTried) {
		return;
	}
	// One attempt only: the page stays reachable by IP, and the responder re-announces by itself after a reconnect.
	mdnsStartTried = true;

	if (!MDNS.begin(STA_HOSTNAME)) {
		HAQuDA_Logger::LogError("startMdns: mDNS responder not started");
		return;
	}
	if (!MDNS.addService("http", "tcp", WEB_SERVER_PORT)) {
		HAQuDA_Logger::LogError("startMdns: http service not added to mDNS");
	}
}

bool HAQuDA_WiFi_handler::checkStoredWiFiCreds() {
	return false;
}

bool HAQuDA_WiFi_handler::createAP() {
	if (!WiFi.mode(WIFI_AP)) {
		return false;
	}

	if (!WiFi.softAP(AP_ssid, AP_pass)) {
		return false;
	}
	vTaskDelay(2000 / portTICK_PERIOD_MS);

	// The DHCP server hands the gateway to clients as their router. The lamp is the only host in the
	// AP network, so the gateway is the AP address. The default gateway 192.168.4.9 did not exist, and
	// iOS checks after joining that its router answers, so the gateway must be a real host.
	if (!WiFi.softAPConfig(AP_ip, AP_ip, AP_subnet)) {
		return false;
	}
	vTaskDelay(200 / portTICK_PERIOD_MS);

	return true;
}

void HAQuDA_WiFi_handler::waitUntilWiFiConnected() {
	uint32_t timer = millis();
	while (!WiFiConnected && !PeriodInRange(timer, CONNECT_WIFI_TIMEOUT)) {
		WiFiConnected = (WiFi.status() == WL_CONNECTED);
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

bool HAQuDA_WiFi_handler::connectToStoredWiFi() {
	int WiFiCredsNum = GetStored_WiFiCredsNum();
	TWiFiCreds WiFiCreds;

	for (int i = 0; i < WiFiCredsNum; i++) {
		WiFiCreds = GetStored_WiFi(i);
		if (!strlen(WiFiCreds.ssid)) {
			continue;
		}
		int j = 0;
		connectToWiFi(WiFiCreds.ssid, WiFiCreds.pass);
		while ((j < reconnectAttempts) && !WiFiConnected) {
			WiFi.reconnect();

			waitUntilWiFiConnected();

			j++;
		}
		if (WiFiConnected) {
			HAQuDA_WipeBytes(&WiFiCreds, sizeof(WiFiCreds));
			return true;
		}
	}
	HAQuDA_WipeBytes(&WiFiCreds, sizeof(WiFiCreds));
	return WiFiConnected;
}

bool HAQuDA_WiFi_handler::connectToWiFi(const char *ssidLocal, const char *passLocal) {
	// WiFi.setHostname() only stores the name; WiFi.mode() hands it to the station interface when it enables the
	// station, and the DHCP request carries it. So the name goes first. When the station is already enabled, mode()
	// returns early; the name set at the first enable stays, and it is the same constant.
	if (!WiFi.setHostname(STA_HOSTNAME)) {
		return false;
	}

	if (!WiFi.mode(WIFI_STA)) {
		return false;
	}

	if (!WiFi.begin(ssidLocal, passLocal, 0, 0)) {
		return false;
	}

	waitUntilWiFiConnected();

	if (!WiFiConnected) {
		return false;
	}
	return true;
}

void HAQuDA_WiFi_handler::WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	WiFiConnected = true;
	vTaskDelay(100 / portTICK_PERIOD_MS);

	dispManip->SetEffectParam(COLOR_GREEN);
	dispManip->SetEffect(staticColor);
	vTaskDelay(3000 / portTICK_PERIOD_MS);

	dispManip->SetEffectParam(COLOR_OFF);
	dispManip->SetEffect(staticColor);
	vTaskDelay(100 / portTICK_PERIOD_MS);
}

void HAQuDA_WiFi_handler::WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	WiFiConnected = false;
	vTaskDelay(100 / portTICK_PERIOD_MS);

	dispManip->SetEffectParam(COLOR_RED);
	dispManip->SetEffect(staticColor);
	vTaskDelay(3000 / portTICK_PERIOD_MS);

	dispManip->SetEffectParam(COLOR_OFF);
	dispManip->SetEffect(staticColor);
	vTaskDelay(100 / portTICK_PERIOD_MS);
}

HAQuDA_WiFi_handler::~HAQuDA_WiFi_handler() {
}