
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include "Board.h"

char auth[] = "Oi4mueBWA22xekeJ3Ppj7m95-wh0spEH";
char ssid[] = "ilor";
char pass[] = "ilor66142222!";

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
void UpdateVirtualPins();

void setup() {
	Serial.begin(115200);
	pinMode(LED_BUILTIN, OUTPUT);

	WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
	WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, pass, 0, 0);

	Blynk.config(auth, BLYNK_DEFAULT_DOMAIN, BLYNK_DEFAULT_PORT);
}

void loop() {
	wl_status_t status = WiFi.status();
	if (status != WL_CONNECTED) {
		log_i("Connecting to %s", ssid);
		WiFi.reconnect();
		uint32_t timer = millis();

		while (WiFi.status() != WL_CONNECTED && !PeriodInRange(timer, 5000)) {
			delay(200);
		}
	} else if (!Blynk.connected()) {
		Blynk.connect();
	} else {
		Blynk.run();
	}
}

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	digitalWrite(LED_BUILTIN, HIGH);
	log_i("WiFiStationConnected");
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	digitalWrite(LED_BUILTIN, LOW);
	log_i("WiFiStationDisconnected");
}