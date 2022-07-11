#include "HAQuDA_WiFi_handler.h"

bool HAQuDA_WiFi_handler::WiFiConnected = false;

void HAQuDA_WiFi_handler::WiFi_connect() {
	if (_myFS.storedWiFiCredsExists()) {
		if (!tryConnectToWiFi()) {
		}
	} else {
		createAP();
	}
	HAQuDA_WebServer::beginWebServer();
}

void HAQuDA_WiFi_handler::WiFi_handleConnection() {

	if (WiFiConnected) {
		return;
	}

	_myUI.currUI_Params.dispMode = effect;
	_myUI.currUI_Params.effectParams.snakeColor = COLOR_AQUA;
	_myUI.currUI_Params.effectParams.snakeSpeed = 200; // start up connection effect
	_myUI.currUI_Params.dispEffect = snake;

	wl_status_t status = WiFi.status();
	while (WiFiCredsFound && (status != WL_CONNECTED)) {
		// connectToWiFi(ssidArr[i], passArr[i]);
		status = WiFi.status();
		if (status != WL_CONNECTED) {
			WiFi.reconnect();
			uint32_t timer = millis();

			while (status != WL_CONNECTED && !PeriodInRange(timer, 5000)) {
				status = WiFi.status();
				delay(200);
			}
			delay(100);
		}
	}
}

bool HAQuDA_WiFi_handler::checkStoredWiFiCreds() {
	return false;
}

void HAQuDA_WiFi_handler::createAP() {
	WiFi.mode(WIFI_AP);
	WiFi.softAP(AP_ssid, AP_pass);
	delay(2000);
	WiFi.softAPConfig(AP_ip, AP_gateway, AP_subnet);
	delay(100);
}

bool HAQuDA_WiFi_handler::tryConnectToWiFi() {
	bool connected = false;
	int WiFiCredsNum = _myFS.getstoredWiFiCredsNum();
	TWiFiCreds WiFiCreds;
	for (int i = 0; i < WiFiCredsNum; i++) {
		_myFS.getstoredWiFiCreds(i);
		if (!strlen(WiFiCreds.ssid)) {
			continue;
		}
		int j = 0;
		while ((j < reconnectAttempts) && !connected) {
			connected = connectToWiFi(WiFiCreds.ssid, WiFiCreds.password);
			j++;
		}
		if (connected) {
			return connected;
		}
	}
	return connected;
}

bool HAQuDA_WiFi_handler::connectToWiFi(const char *ssidLocal, const char *passLocal) {
	//	WiFi.mode(WIFI_STA);
	//	WiFi.begin(ssidLocal, passLocal, 0, 0);
	//
	//	WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
	//	WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
	//	return (WiFi.status() == WL_CONNECTED);
	return false;
	// Blynk.config(BlynkAuth, BLYNK_DEFAULT_DOMAIN, BLYNK_DEFAULT_PORT);
}

void HAQuDA_WiFi_handler::WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	WS2812_fillColor(COLOR_GREEN);
	delay(1000);
	WS2812_clear();
	delay(100);
	WiFiConnected = true;
}

void HAQuDA_WiFi_handler::WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	WiFiConnected = false;
}

HAQuDA_WiFi_handler::~HAQuDA_WiFi_handler() {
}