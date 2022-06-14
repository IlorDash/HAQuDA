#include "HAQuDA_WiFi_handler.h"

bool HAQuDA_WiFi_handler::WiFiConnected = false;

HAQuDA_WiFi_handler::HAQuDA_WiFi_handler(HAQuDA_UI *currUI_WiFi, HAQuDA_FileStorage *currFS_WiFi) {
	AP_ip = new IPAddress(192, 168, 4, 22);
	AP_gateway = new IPAddress(192, 168, 4, 9);
	AP_subnet = new IPAddress(255, 255, 255, 0);

	this->myUI_ptr = currUI_WiFi;
	this->myFS_WiFi = currFS_WiFi;
}

void HAQuDA_WiFi_handler::WiFi_connect() {
	if (checkStoredWiFiCreds()) {
		if (!tryConnectToWiFi()) {
		}
	} else {
		createAP();
	}
}

void HAQuDA_WiFi_handler::WiFi_handleConnection() {

	if (WiFiConnected) {
		return;
	}

	myUI_ptr->currUI_Params.dispMode = effect;
	myUI_ptr->currUI_Params.effectParams.snakeColor = COLOR_AQUA;
	myUI_ptr->currUI_Params.effectParams.snakeSpeed = 200; // start up connection effect
	myUI_ptr->currUI_Params.dispEffect = snake;

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
	WiFi.softAPConfig(*AP_ip, *AP_gateway, *AP_subnet);
	delay(100);

	HAQuDA_WebServer::beginWebServer();
}

bool HAQuDA_WiFi_handler::tryConnectToWiFi() {
	bool connect_result = false;
	// while ((FS != end) && (!connect_result)) {
	char *ssid, *pass;
		//ssid, pass = FS.read
	int i = 0;
	while ((i < reconnectAttempts) && !connect_result) {
		connect_result = connectToWiFi(ssid, pass);
		i++;
	}
	//}
	return connect_result;
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


HAQuDA_WiFi_handler::~HAQuDA_WiFi_handler() {
}