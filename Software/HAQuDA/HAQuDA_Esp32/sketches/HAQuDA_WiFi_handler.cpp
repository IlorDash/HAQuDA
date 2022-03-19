#include "HAQuDA_WiFi_handler.h"

bool HAQuDA_WiFi_handler::WiFiConnected = false;

HAQuDA_WiFi_handler::HAQuDA_WiFi_handler(HAQuDA_UI *currUI_WiFi, FileStorage *currFS_WiFi) {
	AP_ip = new IPAddress(192, 168, 0, 198);
	AP_gateway = new IPAddress(192, 168, 0, 1);
	AP_subnet = new IPAddress(255, 255, 255, 0);

	this->myUI_WiFi = currUI_WiFi;
	this->myFS_WiFi = currFS_WiFi;
}

HAQuDA_WiFi_handler::~HAQuDA_WiFi_handler() {}

void handleSubmit() {
	String response_success = "<h1>Success</h1>";
	response_success += "<h2>Device will restart in 3 seconds</h2>";

	String response_error = "<h1>Error</h1>";
	response_error += "<h2><a href='/'>Go back</a>to try again";

	if (writeToMemory(String(server.arg("ssid")), String(server.arg("password")))) {
		server.send(200, "text/html", response_success);
		EEPROM.commit();
		delay(3000);
		ESP.restart();
	} else {
		server.send(200, "text/html", response_error);
	}
}

bool HAQuDA_WiFi_handler::checkStoredWiFiCreds() {
	
	
}

void HAQuDA_WiFi_handler::WiFi_handleConnection() {
	if (WiFiConnected) {
		return;
	}

	myUI_WiFi->currUI_Params.dispMode = effect;
	myUI_WiFi->currUI_Params.effectParams.snakeColor = COLOR_AQUA;
	myUI_WiFi->currUI_Params.effectParams.snakeSpeed = 200; // start up connection effect
	myUI_WiFi->currUI_Params.dispEffect = snake;

	wl_status_t status = WiFi.status();
	int i = 0;
	while (WiFiCredsFound && (status != WL_CONNECTED) && (i < WIFI_CREDS_NUM)) {
		// connectToWiFi(ssidArr[i], passArr[i]);
		status = WiFi.status();
		if (status != WL_CONNECTED) {
			WiFi.reconnect();
			uint32_t timer = millis();

			while (status != WL_CONNECTED && !PeriodInRange(timer, 5000)) {
				status = WiFi.status();
				delay(200);
			}
			delay(500);
		}
		i++;
	}
}

void HAQuDA_WiFi_handler::createAP() {
	WiFi.mode(WIFI_AP);
	WiFi.softAP(AP_ssid, AP_pass);
	delay(2000);
	WiFi.softAPConfig(*AP_ip, *AP_gateway, *AP_subnet);
	delay(100);
}

bool HAQuDA_WiFi_handler::connectToWiFi(char *ssidLocal, char *passLocal) {
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssidLocal, passLocal, 0, 0);

	WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
	WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
	return (WiFi.status() == WL_CONNECTED);
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
