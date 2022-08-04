#include "HAQuDA_WiFi_handler.h"

bool HAQuDA_WiFi_handler::WiFiConnected = false;

bool HAQuDA_WiFi_handler::WiFi_connect() {
	if (_myFS->GetStored_WiFiCredsNum()) {
		if (connectToStoredWiFi()) {
			return true;
		} else {
			HAQuDA_ErrorHandler::currErrorToDisp = failedToConnectToWiFi;
			vTaskDelay(3000 / portTICK_PERIOD_MS);
			return false;
		}
	}

	if (!createAP()) {
		HAQuDA_ErrorHandler::currErrorToDisp = failedToCreateAP;
		vTaskDelay(3000 / portTICK_PERIOD_MS);
		return false;
	}
	HAQuDA_WebServer::beginWebServer();
	return true;
}

void HAQuDA_WiFi_handler::WiFi_handleConnection() {

	if (WiFiConnected || !(WiFi.getMode() & WIFI_MODE_STA)) {
		return;
	}

	HAQuDA_ErrorHandler::currErrorToDisp = none;

	_myUI->currUI_Params.dispMode = effect;
	_myUI->currUI_Params.effectParams.snakeColor = COLOR_AQUA;
	_myUI->currUI_Params.effectParams.snakeSpeed = 200; // start up connection effect
	_myUI->currUI_Params.dispEffect = snake;

	while (!WiFiConnected) {
		connectToStoredWiFi();
	}

	_myUI->currUI_Params.dispEffect = noneEffect;
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
	delay(2000);

	if (!WiFi.softAPConfig(AP_ip, AP_gateway, AP_subnet)) {
		return false;
	}
	delay(100);

	return true;
}
wl_status_t foo;
bool HAQuDA_WiFi_handler:: connectToStoredWiFi() {
	int WiFiCredsNum = _myFS->GetStored_WiFiCredsNum();
	TWiFiCreds WiFiCreds;

	for (int i = 0; i < WiFiCredsNum; i++) {
		WiFiCreds = _myFS->GetStored_WiFi(i);
		if (!strlen(WiFiCreds.ssid)) {
			continue;
		}
		int j = 0;
		connectToWiFi(WiFiCreds.ssid, WiFiCreds.pass);
		while ((j < reconnectAttempts) && !WiFiConnected) {
			WiFi.reconnect();
			foo = WiFi.status();
			WiFiConnected = (WiFi.status() == WL_CONNECTED);
			if (foo == WL_CONNECTED) {
				int bar = 0;
			}
			//			uint32_t timer = millis();
			//			while (!connected && !PeriodInRange(timer, CONNECT_WIFI_TIMEOUT)) {
			//				connected = (WiFi.status() == WL_CONNECTED);
			//				delay(200);
			//			}
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
	if (!WiFi.begin(ssidLocal, passLocal, 0, 0)) {
		return false;
	}

	uint32_t timer = millis();

	while (!WiFiConnected && !PeriodInRange(timer, CONNECT_WIFI_TIMEOUT)) {
		WiFiConnected = (WiFi.status() == WL_CONNECTED);
		vTaskDelay(100);
	}

	if (!WiFiConnected) {
		return false;
	}

	WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
	WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
	// Blynk.config(BlynkAuth, BLYNK_DEFAULT_DOMAIN, BLYNK_DEFAULT_PORT);
	delay(500);
	return true;
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