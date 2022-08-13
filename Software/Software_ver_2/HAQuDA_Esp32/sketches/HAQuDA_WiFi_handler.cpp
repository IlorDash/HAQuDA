#include "HAQuDA_WiFi_handler.h"

bool HAQuDA_WiFi_handler::WiFiConnected = false;

bool HAQuDA_WiFi_handler::WiFi_connect() {

	WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
	WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
	// Blynk.config(BlynkAuth, BLYNK_DEFAULT_DOMAIN, BLYNK_DEFAULT_PORT);
	vTaskDelay(500 / portTICK_PERIOD_MS);

	if (_myFS->GetStored_WiFiCredsNum()) {

		if (connectToStoredWiFi()) {
			return true;
		} else {
			HAQuDA_DisplayInterface::displayEffect = noneEffect;
			HAQuDA_DisplayInterface::displayError = failedToConnectToWiFi;
			vTaskDelay(3000 / portTICK_PERIOD_MS);
		}
	}

	if (!createAP()) {
		HAQuDA_DisplayInterface::displayEffect = noneEffect;
		HAQuDA_DisplayInterface::displayError = failedToCreateAP;
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

	HAQuDA_DisplayInterface::displayError = noneError;
	vTaskDelay(100 / portTICK_PERIOD_MS);

	_myDisplayInterface->snakeEffectParams.snakeColor = COLOR_AQUA;
	_myDisplayInterface->snakeEffectParams.snakeSpeed = 200; // start up connection effect
	HAQuDA_DisplayInterface::displayEffect = snake;
	vTaskDelay(100 / portTICK_PERIOD_MS);

	while (!WiFiConnected) {
		connectToStoredWiFi();
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
	delay(2000);

	if (!WiFi.softAPConfig(AP_ip, AP_gateway, AP_subnet)) {
		return false;
	}
	delay(100);

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
	HAQuDA_DisplayInterface::displayEffect = noneEffect;
	WiFiConnected = true;
	vTaskDelay(100 / portTICK_PERIOD_MS);
	WS2812_fillColor(COLOR_GREEN);
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	WS2812_clear();
	vTaskDelay(100 / portTICK_PERIOD_MS);
}

void HAQuDA_WiFi_handler::WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	WiFiConnected = false;
}

HAQuDA_WiFi_handler::~HAQuDA_WiFi_handler() {
}