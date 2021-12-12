
#include <WiFi.h>
#include <WebServer.h>
#include "WebPages.h"
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include "Arduino.h"
#include "TimeHelper.h"
#include <SoftwareSerial.h>

#include "Sensors.h"
#include "WS2812.h"
#include "Tasks.h"

#include "Main.h"

#define WIFI_CREDS_NUM 3

#define DISP_MEAS_PERIOD 300000 //=5 min in ms

char BlynkAuth[] = "4MdAV357utNNjm7vmCUEY2NPAdlHQMSM";
char ssid_AP[] = "HAQuDA_ESP32";
char pass_AP[] = "1234567!";

IPAddress local_ip(192, 168, 0, 198);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

char *ssidDefault = "ilor";
char *passDefault = "ilor66142222!";

char *ssidArr[WIFI_CREDS_NUM] = {"ilor", "ordash", "realme 7"};
char *passArr[WIFI_CREDS_NUM] = {"ilor66142222!", "or14591711", "1a387fa49c2b"};

enum dispParams { eCO2, TVOC, PM2_5, temp, humid, noneParam };
dispParams whatParamDisp = noneParam;

dispEffects whatEffectDisp = noneEffect;

bool WiFiCredsFound = true;

WidgetTerminal terminal(V1);
WebServer server(80);

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
void UpdateVirtualPins();

void connectToWiFi(char *ssidLocal, char *passLocal);
void createAP();

int firstDot;
int secondDot; // parametrs of dispParams
int thirdDot;

bool dispFirstParam = false;

void dispParam_WS2812() {
	showParamsDivideDots myDots;
	if (whatParamDisp == temp) {
		myDots.firstDot = 20;
		myDots.secondDot = 26;
		myDots.thirdDot = 30;
		WS2812_showParams(PM2_5_meas.value / PM2_5_meas.measNum, myDots);
		PM2_5_meas.value = 0;
		PM2_5_meas.measNum = 0;
	} else if (whatParamDisp == humid) {
		myDots.firstDot = 40;
		myDots.secondDot = 60;
		myDots.thirdDot = 80;
		WS2812_showParams(PM2_5_meas.value / PM2_5_meas.measNum, myDots);
		PM2_5_meas.value = 0;
		PM2_5_meas.measNum = 0;
	} else if (whatParamDisp == eCO2) {
		myDots.firstDot = 400;
		myDots.secondDot = 1000;
		myDots.thirdDot = 5000;
		WS2812_showParams(eCO2_meas.value / eCO2_meas.measNum, myDots);
		eCO2_meas.value = 0;
		eCO2_meas.measNum = 0;
	} else if (whatParamDisp == TVOC) {
		myDots.firstDot = 220;
		myDots.secondDot = 660;
		myDots.thirdDot = 1000;
		WS2812_showParams(TVOC_meas.value / TVOC_meas.measNum, myDots);
		TVOC_meas.value = 0;
		TVOC_meas.measNum = 0;
	} else if (whatParamDisp == PM2_5) {
		myDots.firstDot = 15;
		myDots.secondDot = 20;
		myDots.thirdDot = 45;
		WS2812_showParams(PM2_5_meas.value / PM2_5_meas.measNum, myDots);
		PM2_5_meas.value = 0;
		PM2_5_meas.measNum = 0;
	}
}

void WiFi_handler() {
	wl_status_t status = WiFi.status();
	int i = 0;
	int j = 0;
	while (WiFiCredsFound && (status != WL_CONNECTED) && (i < WIFI_CREDS_NUM)) {
		WS2812_clear();
		delay(100);
		connectToWiFi(ssidArr[i], passArr[i]);
		status = WiFi.status();
		if (status != WL_CONNECTED) {
			WS2812_setPixelColor(j, COLOR_YELLOW);
			log_i("Connecting to %s", ssidArr[i]);
			WiFi.reconnect();
			uint32_t timer = millis();

			while (status != WL_CONNECTED && !PeriodInRange(timer, 5000)) {
				status = WiFi.status();
				delay(200);
			}
			delay(500);
		}
		i++;
		j++;
		if (i == WIFI_CREDS_NUM) {
			i = 0;
		}
		if (j > LED_NUM_PIXELS) {
			WS2812_clear();
			delay(100);
			j = 0;
		}
	}
}

void connectToWiFi_AP() {
	int i = 0;
	wl_status_t status = WiFi.status();
	while (WiFiCredsFound && (status != WL_CONNECTED) && (i < WIFI_CREDS_NUM)) {
		connectToWiFi(ssidArr[i], passArr[i]);
		i++;
		status = WiFi.status();
		if (i == WIFI_CREDS_NUM) {
			i = 0;
		}
	}
}

void setup() {
	Serial.begin(115200);

	WS2812_begin();
	if (!sensorsBegin()) {
		WS2812_fillColor(COLOR_RED);
		while (1) {
		}
	}

	createTasks();

	// pinMode(LED_BUILTIN, OUTPUT);

	// createAP();

	// Start web server-

	// Serial.print("AP IP address: ");
	// Serial.println(ip_address);

	// server.on("/", setWiFiCreds);
	// server.onNotFound(handle_NotFound);
	// server.begin();
	// Serial.println("HTTP server started");
}

uint16_t measNum = 0;
uint32_t dispMeasTimer = 0;
uint32_t sensors_meas_time = 0;

uint8_t tempBuf[9];

void loop() {
	server.handleClient();
	WiFi_handler();
	if (!Blynk.connected()) {
		Blynk.connect();
	} else {
		Blynk.run();
	}

	delay(100);
	if ((millis() - sensors_meas_time) > SENSORS_MEAS_PERIOD) {
		if (!getDHT11_meas()) {
			terminal.println(F("Failed to read from DHT sensor!"));
		}
		sensors_meas_time = millis();
		getCCS811_meas();
		getPM_meas();

		if (!getO3_meas()) {
			terminal.println(F("Failed to read from ZE25-O3 sensor!"));
		}
	}

	bool isMeasExist = (eCO2_meas.measNum) && (TVOC_meas.measNum) && (PM01_meas.measNum) && (PM2_5_meas.measNum) && (PM10_meas.measNum) && (temp_meas.measNum)
					   && (humid_meas.measNum) && (O3_meas.measNum);

	static unsigned long OledTimer = millis();
	if ((millis() - OledTimer >= 1000) && isMeasExist) {
		measNum++;
		OledTimer = millis();

		terminal.print("	Measurment:");
		terminal.println(measNum);

		terminal.print("Temp: ");
		terminal.print(temp_meas.value / temp_meas.measNum);
		terminal.println("  C");
		terminal.print("Humid: ");
		terminal.print(humid_meas.value / humid_meas.measNum);
		terminal.println("  %");
		terminal.println();

		terminal.print("PM1.0: ");
		terminal.print(PM01_meas.value / PM01_meas.measNum);
		terminal.println("  ug/m3");
		terminal.print("PM2.5: ");
		terminal.print(PM2_5_meas.value / PM2_5_meas.measNum);
		terminal.println("  ug/m3");
		terminal.print("PM1 0: ");
		terminal.print(PM10_meas.value / PM10_meas.measNum);
		terminal.println("  ug/m3");
		terminal.println();

		terminal.print("eCO2: ");
		terminal.print(eCO2_meas.value / eCO2_meas.measNum);
		terminal.println("  ppm");
		terminal.println();

		terminal.print("TVOC: ");
		terminal.print(TVOC_meas.value / TVOC_meas.measNum);
		terminal.println("  ppb");
		terminal.println();

		terminal.print("O3: ");
		terminal.print(O3_meas.value / O3_meas.measNum);
		terminal.println("  ppb");
		terminal.println();

		switch (whatParamDisp) {
			case temp: // Item 3
				terminal.println("Display temperature");
				break;
			case humid: // Item 3
				terminal.println("Display humidity");
				break;
			case eCO2:
				terminal.println("Display eCO2");
				break;
			case TVOC:
				terminal.println("Display TVOC");
				break;
			case PM2_5:
				terminal.println("Display PM2_5");
				break;
			default:
				terminal.println("Display none parametr");
		}

		terminal.flush();
	}
	if (((millis() - dispMeasTimer > DISP_MEAS_PERIOD) || dispFirstParam) && isMeasExist) {
		dispMeasTimer = millis();
		dispParam_WS2812();
		dispFirstParam = false;
	}
	//	if (whatEffectDisp != noneEffect) {
	//		dispEffect_WS2812();
	//	}
}

void createAP() {
	WiFi.mode(WIFI_AP);
	WiFi.softAP(ssid_AP, pass_AP);
	delay(2000);
	WiFi.softAPConfig(local_ip, gateway, subnet);
	// IPAddress ip_address = WiFi.softAPIP(); // IP Address of our accesspoint
	delay(100);
}

void connectToWiFi(char *ssidLocal, char *passLocal) {
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssidLocal, passLocal, 0, 0);
	WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
	WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);

	Blynk.config(BlynkAuth, BLYNK_DEFAULT_DOMAIN, BLYNK_DEFAULT_PORT);
}

void setWiFiCreds(String ssidLocal, String passLocal) {
	if (server.hasArg("ssid") && server.hasArg("password")) {
		ssidLocal = server.arg("ssid");
		passLocal = server.arg("password");
		WiFiCredsFound = true;
		server.send(200, "text/html", WiFiCredsPage);
		// server.stop();
	} else {
		// If one of the creds is missing, go back to form page
		server.send(200, "text/html", WiFiCredsPage);
	}
}

void handle_NotFound() {
	server.send(404, "text/plain", "Not found");
}

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	WS2812_fillColor(COLOR_GREEN);
	delay(1000);
	WS2812_clear();
	delay(100);
	log_i("WiFiStationConnected");
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	log_i("WiFiStationDisconnected");
}

BLYNK_WRITE(V0) {
	int red = param[0].asInt();
	int green = param[1].asInt();
	int blue = param[2].asInt();
	log_i("V0, r: %d, g: %d, b: %d,", red, green, blue);
	uint32_t color = ((uint32_t)red << 16) | ((uint32_t)green << 8) | blue;
	WS2812_fillColor(color);
	delay(100);
	whatParamDisp = noneParam;
	whatEffectDisp = noneEffect;
}

BLYNK_WRITE(V2) {
	switch (param.asInt()) {
		case 1: { // Item 3
			whatParamDisp = temp;
			WS2812_clear();
			delay(100);
			break;
		}
		case 2: { // Item 3
			whatParamDisp = humid;
			WS2812_clear();
			delay(100);
			break;
		}
		case 3: { // Item 1
			whatParamDisp = eCO2;
			WS2812_clear();
			delay(100);
			break;
		}
		case 4: { // Item 2
			whatParamDisp = TVOC;
			WS2812_clear();
			delay(100);
			break;
		}
		case 5: { // Item 3
			whatParamDisp = PM2_5;
			WS2812_clear();
			delay(100);
			break;
		}
		default:
			whatParamDisp = noneParam;
	}
	dispFirstParam = true;
	whatEffectDisp = noneEffect;
}

BLYNK_WRITE(V3) {
	WS2812_setBrightnessPerCent(param.asInt());
}

BLYNK_WRITE(V4) {
	switch (param.asInt()) {
		case 1: { // Item 3
			whatEffectDisp = snake;
			WS2812_clear();
			delay(100);
			break;
		}
		case 2: { // Item 3
			whatEffectDisp = randomPixel;
			WS2812_clear();
			delay(100);
			break;
		}
		case 3: { // Item 1
			whatEffectDisp = fade;
			WS2812_clear();
			delay(100);
			break;
		}
		case 4: { // Item 2
			whatEffectDisp = christmasTree;
			WS2812_clear();
			delay(100);
			break;
		}
		default:
			whatEffectDisp = noneEffect;
	}
	whatParamDisp = noneParam;
}