#include "HAQuDA_WiFi_handler.h"

bool HAQuDA_WiFi_handler::WiFiConnected = false;

const char *hostname = AP_SSID;
const char *DDNS_token = DDNS_TOKEN;
const char *DDNS_domain = DDNS_DOMAIN;


bool HAQuDA_WiFi_handler::Connect() {

	WiFi.onEvent(WiFiStationConnected, ARDUINO_EVENT_WIFI_STA_CONNECTED);
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
	dnsServer.start(53, "*", WiFi.softAPIP());
	HAQuDA_WebServer::beginWebServer();
	return true;
}

void HAQuDA_WiFi_handler::HandleConnection() {
	// Check for new public IP every 10 seconds
	EasyDDNS.update(10000, true);

	dnsServer.processNextRequest();

	if (MyWebSerial.GetWebSerialOpened()) {
		WebSerialPrintStoredLogs();
		MyWebSerial.ResetWebSerialOpened();
	}

	if (WiFiConnected || !(WiFi.getMode() & WIFI_MODE_STA)) {
		return;
	}
	WiFi.removeEvent(SYSTEM_EVENT_STA_DISCONNECTED);
	HAQuDA_DisplayManip::SetDisplayEffect(snake); // start up connection effect

	while (!WiFiConnected) {
		connectToStoredWiFi();
	}
	WiFi.onEvent(WiFiStationDisconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
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

	if (!WiFi.softAPConfig(AP_ip, AP_gateway, AP_subnet)) {
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

void HAQuDA_WiFi_handler::WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	HAQuDA_DisplayManip::SetDisplayMode(none);
	WiFiConnected = true;
	vTaskDelay(100 / portTICK_PERIOD_MS);

	HAQuDA_DisplayManip::ShowStaticColor(COLOR_GREEN);
	vTaskDelay(3000 / portTICK_PERIOD_MS);
	HAQuDA_DisplayManip::ShowStaticColor(0, 0, 0);
	vTaskDelay(100 / portTICK_PERIOD_MS);
}

void HAQuDA_WiFi_handler::WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	HAQuDA_DisplayManip::SetDisplayMode(none);
	WiFiConnected = false;
	vTaskDelay(100 / portTICK_PERIOD_MS);

	HAQuDA_DisplayManip::ShowStaticColor(COLOR_RED);
	vTaskDelay(3000 / portTICK_PERIOD_MS);
	HAQuDA_DisplayManip::ShowStaticColor(0, 0, 0);
	vTaskDelay(100 / portTICK_PERIOD_MS);
}

HAQuDA_WiFi_handler::~HAQuDA_WiFi_handler() {
}