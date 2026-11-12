#include "HAQuDA_WiFi_handler.h"

#define WAIT_FOR_SEND_WEB_SERIAL_MS 1000

constexpr uint32_t DNS_POLL_PERIOD_MS = 10;

constexpr uint32_t JOINED_WIFI_DISPLAY_MS = 3000;

// A wait ends on an IP address, not on association. The router took 4.5 s from association to DHCP
// lease in the lamp log (2026-11-11), and WiFi.reconnect() after a timeout aborts the attempt in progress
// (it drops DHCP too), so the wait must outlast association + DHCP on a slow router.
constexpr uint32_t CONNECT_WIFI_TIMEOUT_MS = 8000;

std::atomic<bool> HAQuDA_WiFi_handler::WiFiConnected{false};

const char *hostname = AP_SSID;
const char *DDNS_token = DDNS_TOKEN;
const char *DDNS_domain = DDNS_DOMAIN;

static HAQuDA_DisplayManip *dispManip;

HAQuDA_WiFi_handler::HAQuDA_WiFi_handler(HAQuDA_FileStorage *currFS, HAQuDA_DisplayManip *_dispManip) : HAQuDA_WebServer(currFS) {
	dispManip = _dispManip;
};

bool HAQuDA_WiFi_handler::Connect() {

	// GOT_IP, not STA_CONNECTED: association comes 1..5 s before DHCP finishes, and NTP / DNS need the address
	WiFi.onEvent(WiFiStationConnected, ARDUINO_EVENT_WIFI_STA_GOT_IP);
	vTaskDelay(200 / portTICK_PERIOD_MS);

	if (GetStored_WiFiCredsNum()) {
		if (connectToStoredWiFi()) {
			WiFi.onEvent(WiFiStationDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
			vTaskDelay(200 / portTICK_PERIOD_MS);
			HAQuDA_WebServer::beginWebServer();
			EasyDDNS.service("duckdns");
			EasyDDNS.client(DDNS_domain, DDNS_token); // Enter your DDNS Domain & Token

			// Get Notified when your IP changes
			EasyDDNS.onUpdate([&](const char *oldIP, const char *newIP) {
				Serial.print("EasyDDNS - IP Change Detected: ");
				Serial.println(newIP);
			});
			showJoinedWiFi();
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

bool HAQuDA_WiFi_handler::CreateAP() {
	if (!createAP()) {
		return false;
	}
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

	if (WiFiConnected || !(WiFi.getMode() & WIFI_MODE_STA)) {
		return;
	}
	WiFi.removeEvent(SYSTEM_EVENT_STA_DISCONNECTED);

	dispManip->SetEffect(snake);
	vTaskDelay(2000 / portTICK_PERIOD_MS);

	while (!WiFiConnected) {
		connectToStoredWiFi();
	}
	WiFi.onEvent(WiFiStationDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

	showJoinedWiFi();
	// An effect call leaves the meas display mode stored in MeasHandler untouched, so it is still the mode that ran
	// before the disconnect (none only if measurements never started: then SetMeasMode() is skipped). While an error is
	// shown, SetMode() refuses to leave the error mode, so the error display stays.
	// Without this the matrix stays dark after the green flash.
	const meas_mode prevMeasMode = dispManip->GetMeasDispMode();
	if (prevMeasMode != noneMeas) {
		HAQuDA_DisplayManip::SetMeasMode(prevMeasMode);
	}
}

void HAQuDA_WiFi_handler::showJoinedWiFi() {
	dispManip->SetEffectParam(COLOR_GREEN);
	dispManip->SetEffect(staticColor);
	// Blocks the loop task for the whole flash; ShowTask draws on the other core, and this is not a system task
	vTaskDelay(pdMS_TO_TICKS(JOINED_WIFI_DISPLAY_MS));

	dispManip->SetEffectParam(COLOR_OFF);
	dispManip->SetEffect(staticColor);
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
	while (!WiFiConnected && !PeriodInRange(timer, CONNECT_WIFI_TIMEOUT_MS)) {
		// The core reports WL_CONNECTED only after GOT_IP, so this agrees with the event handler
		if (WiFi.status() == WL_CONNECTED) {
			WiFiConnected = true;
		}
		vTaskDelay(pdMS_TO_TICKS(100));
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
			return true;
		}
	}
	return WiFiConnected;
}

bool HAQuDA_WiFi_handler::connectToWiFi(const char *ssidLocal, const char *passLocal) {
	if (!WiFi.mode(WIFI_STA)) {
		return false;
	}

	if (!WiFi.setHostname(hostname)) { // define hostname)
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

// WiFi event task: only sets the flag. The matrix is drawn by the loop task (showJoinedWiFi()): the event task may be
// re-posted GOT_IP on a lease change, and a display write from here would land after setup() has entered meas mode.
void HAQuDA_WiFi_handler::WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	WiFiConnected = true;
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