
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
#define BLUNK_LOG_PERIOD 1000
#define SENSORS_MEAS_PERIOD 2000

#define DISP_PARAMS_NUM 5

char BlynkAuth[] = "4MdAV357utNNjm7vmCUEY2NPAdlHQMSM";
char ssid_AP[] = "HAQuDA_ESP32";
char pass_AP[] = "1234567!";

IPAddress local_ip(192, 168, 0, 198);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

char *ssidDefault = "ilor";
char *passDefault = "ilor66142222!";

char *ssidArr[WIFI_CREDS_NUM] = {"ilor", "ordash", "realme 7"};
char *passArr[WIFI_CREDS_NUM] = {"ilor66142222!", "or14591711!", "1a387fa49c2b"};

enum dispParams { eCO2, TVOC, PM2_5, temp, humid, noneParam };
dispParams whatParamDisp = noneParam;

paramsDivideDots temp_divideDots = {20, 26, 30};
paramsDivideDots humid_divideDots = {40, 60, 80};
paramsDivideDots eCO2_divideDots = {400, 1000, 5000};
paramsDivideDots TVOC_divideDots = {220, 660, 1000};
paramsDivideDots PM2_5_divideDots = {15, 20, 45};

dispEffects whatEffectDisp = noneEffect;

bool WiFiCredsFound = true;

WidgetTerminal terminal(V0);
WebServer server(80);

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
void UpdateVirtualPins();

void connectToWiFi(char *ssidLocal, char *passLocal);
void createAP();

void dispParam_WS2812();
void blynkPrintLog();

int firstDot;
int secondDot; // parametrs of dispParams
int thirdDot;

bool dispFirstParam = false;

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
		terminal.println("FATAL ERROR: Failed to begin sensors");
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
uint32_t blynk_log_time = 0;

void loop() {
	server.handleClient();
	WiFi_handler();
	if (!Blynk.connected()) {
		Blynk.connect();
	} else {
		Blynk.run();
	}
	vTaskDelay(100);

	if ((millis() - sensors_meas_time) > SENSORS_MEAS_PERIOD) {
		getDHT11_meas();
		getCCS811_meas();
		getPM_meas();
		getO3_meas();
		sensors_meas_time = millis();
	}

	if (millis() - blynk_log_time >= BLUNK_LOG_PERIOD) {
		measNum++;
		blynk_log_time = millis();
		blynkPrintLog();
	}

	if (millis() - dispMeasTimer > DISP_MEAS_PERIOD) {
		dispParam_WS2812();
		dispMeasTimer = millis();
	}
}

void dispParam_WS2812() {
	if (whatParamDisp == temp) {
		WS2812_showParams(temp_meas.value / temp_meas.measNum, temp_divideDots);
		temp_meas.value = 0;
		temp_meas.measNum = 0;
	} else if (whatParamDisp == humid) {
		WS2812_showParams(humid_meas.value / humid_meas.measNum, humid_divideDots);
		humid_meas.value = 0;
		humid_meas.measNum = 0;
	} else if (whatParamDisp == eCO2) {
		WS2812_showParams(eCO2_meas.value / eCO2_meas.measNum, eCO2_divideDots);
		eCO2_meas.value = 0;
		eCO2_meas.measNum = 0;
	} else if (whatParamDisp == TVOC) {
		WS2812_showParams(TVOC_meas.value / TVOC_meas.measNum, TVOC_divideDots);
		TVOC_meas.value = 0;
		TVOC_meas.measNum = 0;
	} else if (whatParamDisp == PM2_5) {
		WS2812_showParams(PM2_5_meas.value / PM2_5_meas.measNum, PM2_5_divideDots);
		PM2_5_meas.value = 0;
		PM2_5_meas.measNum = 0;
	}
}

bool checkIfMeasCorrect() {
	bool returnVal = true;
	if (!eCO2_meas.newMeasDone) {
		terminal.println("FATAL ERROR: Failed to get eCO2 measurment");
		returnVal = false;
	}
	if (!TVOC_meas.newMeasDone) {
		terminal.println("FATAL ERROR: Failed to get TVOC measurment");
		returnVal = false;
	}
	if (!PM2_5_meas.newMeasDone) {
		terminal.println("FATAL ERROR: Failed to get PM 2.5 measurment");
		returnVal = false;
	}
	if (!temp_meas.newMeasDone) {
		terminal.println("FATAL ERROR: Failed to get TEMP measurment");
		returnVal = false;
	}
	if (!humid_meas.newMeasDone) {
		terminal.println("FATAL ERROR: Failed to get HUMID measurment");
		returnVal = false;
	}
	if (!humid_meas.newMeasDone) {
		terminal.println("FATAL ERROR: Failed to get O3 measurment");
		returnVal = false;
	}
	terminal.flush();
	return returnVal;
}

void blynkPrintLog() {
	if (!checkIfMeasCorrect) {
		return;
	}

	terminal.println();
	terminal.print("----------------------------");
	terminal.print(measNum);
	terminal.println(" - measurment");

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

BLYNK_WRITE(V1) {
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
	WS2812_setBrightnessPerCent(param.asInt());
}

BLYNK_WRITE(V3) {
	switch (param.asInt()) {
		case 1: { // Item 3
			break;
		}

		default:
			whatParamDisp = noneParam;
	}
}

BLYNK_WRITE(V4) {
	switch (param.asInt()) {
		case 1: {
			whatParamDisp = temp;
			WS2812_clear();
			delay(100);
			break;
		}
		case 2: {
			whatParamDisp = humid;
			WS2812_clear();
			delay(100);
			break;
		}
		case 3: {
			whatParamDisp = eCO2;
			WS2812_clear();
			delay(100);
			break;
		}
		case 4: {
			whatParamDisp = TVOC;
			WS2812_clear();
			delay(100);
			break;
		}
		case 5: {
			whatParamDisp = PM2_5;
			WS2812_clear();
			delay(100);
			break;
		}
		default:
			whatParamDisp = noneParam;
	}
	dispParam_WS2812();
	whatEffectDisp = noneEffect;
}

BLYNK_WRITE(V5) {
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