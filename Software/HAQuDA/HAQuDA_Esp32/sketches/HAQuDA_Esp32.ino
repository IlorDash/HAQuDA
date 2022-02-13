#include "Arduino.h"

#include "HAQuDA_WebServer.h"
#include "HAQuDA_UI.h"
#include "HAQuDA_DispManip.h"

#include <BlynkSimpleEsp32.h>

#include "TimeHelper.h"
#include <SoftwareSerial.h>

#include "Sensors.h"
#include "WS2812.h"
#include "Tasks.h"

#include <EEPROM.h>


#define WIFI_CREDS_NUM 3

#define DISP_MEAS_PERIOD 300000 //=5 min in ms
#define SENSORS_MEAS_PERIOD 2000

char BlynkAuth[] = "4MdAV357utNNjm7vmCUEY2NPAdlHQMSM";
char ssid_AP[] = "HAQuDA_ESP32";
char pass_AP[] = "1234567!";

IPAddress local_ip(192, 168, 0, 198);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

char *ssidArr[] = {""};
char *passArr[] = {""};

char *AP_ssid = {"HAQuDA_1"};
char *AP_pass = {"TpF4YJ"};

bool WiFiCredsFound = true;
bool WiFiConnected = false;

WidgetTerminal terminal(V0);

HAQuDA_UI *myUI;

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

void WiFi_handleConnection() {
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

	WiFi.mode(WIFI_AP);
	WiFi.softAP(AP_ssid, AP_pass);

	WiFi_handleConnection();

	WS2812_begin();
	if (!sensorsBegin()) {
		WS2812_fillColor(COLOR_RED);
		terminal.println("FATAL ERROR: Failed to begin sensors");
		while (1) {
		}
	}

	createTasks();

	terminal.println("*************************");
	terminal.print("START LOGGING");

	// pinMode(LED_BUILTIN, OUTPUT);

	// createAP();

	// Start web server-

	// Serial.print("AP IP address: ");
	// Serial.println(ip_address);

	// Serial.println("HTTP server started");
}

uint16_t measNum = 0;
uint32_t dispMeasTimer = 0;
uint32_t sensors_meas_time = 0;

void loop() {
	// server.handleClient();
	if (!WiFiConnected) {
		WiFi_handleConnection();
	}
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

		measNum++;
		blynkPrintLog();

		sensors_meas_time = millis();
	}

	if (millis() - dispMeasTimer > DISP_MEAS_PERIOD) {
		dispParam_WS2812();
		dispMeasTimer = millis();

		temp_meas.value = 0;
		temp_meas.measNum = 0;

		humid_meas.value = 0;
		humid_meas.measNum = 0;

		eCO2_meas.value = 0;
		eCO2_meas.measNum = 0;

		TVOC_meas.value = 0;
		TVOC_meas.measNum = 0;

		PM_2_5_meas.value = 0;
		PM_2_5_meas.measNum = 0;
	}
}

 

void checkIfMeasCorrect() {
	if (!eCO2_meas.newMeasDone) {
		terminal.println("ERROR: Failed to get eCO2 measurment");
	}
	if (!TVOC_meas.newMeasDone) {
		terminal.println("ERROR: Failed to get TVOC measurment");
	}
	if (!PM_2_5_meas.newMeasDone) {
		terminal.println("ERROR: Failed to get PM 2.5 measurment");
	}
	if (!temp_meas.newMeasDone) {
		terminal.println("ERROR: Failed to get TEMP measurment");
	}
	if (!humid_meas.newMeasDone) {
		terminal.println("ERROR: Failed to get HUMID measurment");
	}
	if (!O3_meas.newMeasDone) {
		terminal.println("ERROR: Failed to get O3 measurment");
	}
	terminal.flush();

	eCO2_meas.newMeasDone = false;
	TVOC_meas.newMeasDone = false;
	PM_2_5_meas.newMeasDone = false;
	temp_meas.newMeasDone = false;
	humid_meas.newMeasDone = false;
	O3_meas.newMeasDone = false;
}

void blynkPrintLog() {
	terminal.println();
	terminal.println("----------------------------");
	terminal.print(measNum);
	terminal.println(" - measurment");

	checkIfMeasCorrect();

	terminal.print("Temp: ");
	terminal.print(temp_meas.value / temp_meas.measNum);
	terminal.println("  C");
	terminal.print("Humid: ");
	terminal.print(humid_meas.value / humid_meas.measNum);
	terminal.println("  %");
	terminal.println();

	terminal.flush();

	terminal.print("PM1.0: ");
	terminal.print(PM_1_0_meas.value / PM_1_0_meas.measNum);
	terminal.println("  ug/m3");
	terminal.print("PM2.5: ");
	terminal.print(PM_2_5_meas.value / PM_2_5_meas.measNum);
	terminal.println("  ug/m3");
	terminal.print("PM1 0: ");
	terminal.print(PM_10_meas.value / PM_10_meas.measNum);
	terminal.println("  ug/m3");
	terminal.println();

	terminal.flush();

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
	terminal.println("  ppm");
	terminal.println();

	terminal.flush();

	if (whatModeDisp == standard) {
		terminal.println("Display standard");
		switch (whatParamDisp) {
			case total:
				terminal.println("Display total quality");
				break;
			case temp:
				terminal.println("Display temperature");
				break;
			case humid:
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
	} else if (whatModeDisp == multi) {
		terminal.println("Display multi");
		for (int i = 0; i < MULTI_MODE_PARAM_NUM; i++) {
			switch (multiModeStruct.paramsArr[i]) {
				case total:
					terminal.println("Display total quality");
					break;
				case temp:
					terminal.println("Display temperature");
					break;
				case humid:
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
		}
	} else if (whatModeDisp == night) {
		switch (whatParamDisp) {
			case total:
				terminal.println("Display total quality");
				break;
			case temp:
				terminal.println("Display temperature");
				break;
			case humid:
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
	} else {
		switch (whatEffectDisp) {
			case snake:
				terminal.println("Display snake effect");
				break;
			case randomPixel:
				terminal.println("Display random pixel effect");
				break;
			case fade:
				terminal.println("Display fade effect");
				break;
			case christmasTree:
				terminal.println("Display Christmas Tree effect");
				break;
			default:
				break;
		}
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

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	WS2812_fillColor(COLOR_GREEN);
	delay(1000);
	WS2812_clear();
	delay(100);
	WiFiConnected = true;
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	WiFiConnected = false;
}

BLYNK_WRITE(V1) {
	int red = param[0].asInt();
	int green = param[1].asInt();
	int blue = param[2].asInt();

	myUI->setStaticColor(red, green, blue);
}

BLYNK_WRITE(V2) {
	myUI->setBrightness(param.asInt());
}

BLYNK_WRITE(V3) {
	myUI->changeDispMode(param.asInt());
}

BLYNK_WRITE(V4) {
	myUI->changeDispParam(param.asInt());
}

BLYNK_WRITE(V5) {
	myUI->changeDispEffect(param.asInt());
}