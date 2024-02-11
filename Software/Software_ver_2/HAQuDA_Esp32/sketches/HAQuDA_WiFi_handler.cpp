#include "HAQuDA_WiFi_handler.h"

#define CONNECT_WIFI_TIMEOUT 3000
#define WAIT_FOR_SEND_WEB_SERIAL_MS 1000

bool HAQuDA_WiFi_handler::WiFiConnected = false;

const char *hostname = AP_SSID;
const char *DDNS_token = DDNS_TOKEN;
const char *DDNS_domain = DDNS_DOMAIN;

static HAQuDA_DisplayManip *dispManip;

HAQuDA_WiFi_handler::HAQuDA_WiFi_handler(HAQuDA_FileStorage *currFS, HAQuDA_DisplayManip *_dispManip) : HAQuDA_WebServer(currFS) {
	dispManip = _dispManip;
};

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

int web_serial_send_timeout;
bool start_send_web_serial_timer = false;

void HAQuDA_WiFi_handler::HandleConnection() {
	// Check for new public IP every 10 seconds
	//EasyDDNS.update(10000, true);

	//dnsServer.processNextRequest();

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