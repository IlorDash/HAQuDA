#include "Arduino.h"
#include "Main.h"

#include <WiFi.h>
#include <WebServer.h>
#include "WebPages.h"
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include "TimeHelper.h"
#include <SoftwareSerial.h>

#include "Sensors.h"
#include "WS2812.h"
#include "Tasks.h"

#include <NTPClient.h>
#include <WiFiUdp.h>

char BlynkAuth[] = "4MdAV357utNNjm7vmCUEY2NPAdlHQMSM";
char ssid_AP[] = "HAQuDA_ESP32";
char pass_AP[] = "1234567!";

IPAddress local_ip(192, 168, 0, 198);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

char *ssidArr[WIFI_CREDS_NUM] = {"ENTER", "YOUR", "SSIDs"};
char *passArr[WIFI_CREDS_NUM] = {"ENTER!", "YOUR!", "PASSWORDs"};

enum dispParams { total, eCO2, TVOC, PM2_5, temp, humid, noneParam };
dispParams whatParamDisp = noneParam;

enum dispMode { standard, multi, night, noneMode };
dispMode whatModeDisp = standard;

typedef struct {
	dispParams paramsArr[MULTI_MODE_PARAM_NUM];
	float dataArr[MULTI_MODE_PARAM_NUM];
	paramsDivideDots divideDotsArr[MULTI_MODE_PARAM_NUM];
} multiModeParamDispStruct;

multiModeParamDispStruct multiModeStruct;

paramsDivideDots temp_divideDots = {20, 26, 30};
paramsDivideDots humid_divideDots = {40, 60, 80};
paramsDivideDots eCO2_divideDots = {400, 1000, 5000};
paramsDivideDots TVOC_divideDots = {220, 660, 1000};
paramsDivideDots PM2_5_divideDots = {15, 20, 45};

dispEffects whatEffectDisp = noneEffect;

typedef struct {
	uint8_t timeFirstBorder;
	uint8_t timeSecondBorder;
} nightMode_timeBorderStruct;

nightMode_timeBorderStruct currentTimeBorder;

bool WiFiCredsFound = true;
bool WiFiConnected = false;

WidgetTerminal terminal(V0);
WebServer server(80);

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

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

	currentTimeBorder.timeFirstBorder = 21;
	currentTimeBorder.timeSecondBorder = 9;

	WiFi_handleConnection();

	WS2812_begin();
	if (!sensorsBegin()) {
		WS2812_fillColor(COLOR_RED);
		terminal.println("FATAL ERROR: Failed to begin sensors");
		while (1) {
		}
	}

	createTasks();

	// Initialize a NTPClient to get time
	timeClient.begin();
	// Set offset time in seconds to adjust for your timezone, for example:
	// GMT +1 = 3600
	// GMT +8 = 28800
	// GMT +3 = 10800
	// GMT 0 = 0
	timeClient.setTimeOffset(10800);

	terminal.println("*************************");
	terminal.print("START LOGGING");

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

void loop() {
	server.handleClient();
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

dispParams checkBadParam() {
	if ((temp_meas.value / temp_meas.measNum) >= temp_divideDots.thirdDot) {
		return temp;
	} else if ((humid_meas.value / humid_meas.measNum) >= humid_divideDots.thirdDot) {
		return humid;
	} else if ((eCO2_meas.value / eCO2_meas.measNum) >= eCO2_divideDots.thirdDot) {
		return eCO2;
	} else if ((TVOC_meas.value / TVOC_meas.measNum) >= TVOC_divideDots.thirdDot) {
		return TVOC;
	} else if ((PM_2_5_meas.value / PM_2_5_meas.measNum) >= PM2_5_divideDots.thirdDot) {
		return PM2_5;
	}
	return noneParam;
}

void standardDispParam_WS2818() {
	switch (whatParamDisp) {
		case total: {
			dispParams badParam = checkBadParam();
			switch (badParam) {
				case temp: {
					WS2812_showParams_standard(temp_meas.value / temp_meas.measNum, temp_divideDots);
					break;
				}
				case humid: {
					WS2812_showParams_standard(humid_meas.value / humid_meas.measNum, humid_divideDots);
					break;
				}
				case eCO2: {
					WS2812_showParams_standard(eCO2_meas.value / eCO2_meas.measNum, eCO2_divideDots);
					break;
				}
				case TVOC: {
					WS2812_showParams_standard(TVOC_meas.value / TVOC_meas.measNum, TVOC_divideDots);
					break;
				}
				case PM2_5: {
					WS2812_showParams_standard(PM_2_5_meas.value / PM_2_5_meas.measNum, PM2_5_divideDots);
					break;
				}
				case noneParam: {
					float paramsValuesArr[DISP_PARAMS_NUM];
					paramsValuesArr[0] = temp_meas.value / temp_meas.measNum;
					paramsValuesArr[1] = humid_meas.value / humid_meas.measNum;
					paramsValuesArr[2] = eCO2_meas.value / eCO2_meas.measNum;
					paramsValuesArr[3] = TVOC_meas.value / TVOC_meas.measNum;
					paramsValuesArr[4] = PM_2_5_meas.value / PM_2_5_meas.measNum;
					WS2812_showParams_standardTotal(paramsValuesArr);
					break;
				}
				default:
					break;
			}
			break;
		}
		case temp: {
			WS2812_showParams_standard(temp_meas.value / temp_meas.measNum, temp_divideDots);
			break;
		}
		case humid: {
			WS2812_showParams_standard(humid_meas.value / humid_meas.measNum, humid_divideDots);
			break;
		}
		case eCO2: {
			WS2812_showParams_standard(eCO2_meas.value / eCO2_meas.measNum, eCO2_divideDots);
			break;
		}
		case TVOC: {
			WS2812_showParams_standard(TVOC_meas.value / TVOC_meas.measNum, TVOC_divideDots);
			break;
		}
		case PM2_5: {
			WS2812_showParams_standard(PM_2_5_meas.value / PM_2_5_meas.measNum, PM2_5_divideDots);
			break;
		}

		default:
			break;
	}
}

uint8_t get_nightMode_hour(uint8_t curHour) {
	bool isBordersInDifferentDays = (currentTimeBorder.timeSecondBorder < currentTimeBorder.timeFirstBorder);

	if (isBordersInDifferentDays) {
		if ((curHour >= currentTimeBorder.timeFirstBorder) || (curHour <= currentTimeBorder.timeSecondBorder)) {
			int8_t hourDiff = curHour - currentTimeBorder.timeFirstBorder;
			hourDiff += (hourDiff < 0) ? 24 : 0;
			return hourDiff;
		} else {
			return 12;
		}
	} else {
		if ((curHour >= currentTimeBorder.timeFirstBorder) && (curHour <= currentTimeBorder.timeSecondBorder)) {
			int8_t hourDiff = curHour - currentTimeBorder.timeFirstBorder;
			return hourDiff;
		} else {
			return 12;
		}
	}
	return 12;
}

void nightDispParam_WS2818() {
	while (!timeClient.update()) {
		timeClient.forceUpdate();
	}
	// The formattedDate comes with the following format:
	// 2018-05-28T16:00:13Z
	// We need to extract date and time
	// Variables to save date and time
	String formattedDate;
	uint8_t curHour;

	formattedDate = timeClient.getFormattedDate();
	Serial.println(formattedDate);

	// Extract date
	int splitT = formattedDate.indexOf("T");
	int splitColon = formattedDate.indexOf(":");
	// Extract time
	curHour = formattedDate.substring(splitT + 1, splitColon).toInt();

	uint8_t nightMode_hour = get_nightMode_hour(curHour);

	if (whatParamDisp == temp) {
		WS2812_showParams_night(temp_meas.value / temp_meas.measNum, temp_divideDots, nightMode_hour);
		temp_meas.value = 0;
		temp_meas.measNum = 0;
	} else if (whatParamDisp == humid) {
		WS2812_showParams_night(humid_meas.value / humid_meas.measNum, humid_divideDots, nightMode_hour);
		humid_meas.value = 0;
		humid_meas.measNum = 0;
	} else if (whatParamDisp == eCO2) {
		WS2812_showParams_night(eCO2_meas.value / eCO2_meas.measNum, eCO2_divideDots, nightMode_hour);
		eCO2_meas.value = 0;
		eCO2_meas.measNum = 0;
	} else if (whatParamDisp == TVOC) {
		WS2812_showParams_night(TVOC_meas.value / TVOC_meas.measNum, TVOC_divideDots, nightMode_hour);
		TVOC_meas.value = 0;
		TVOC_meas.measNum = 0;
	} else if (whatParamDisp == PM2_5) {
		WS2812_showParams_night(PM_2_5_meas.value / PM_2_5_meas.measNum, PM2_5_divideDots, nightMode_hour);
		PM_2_5_meas.value = 0;
		PM_2_5_meas.measNum = 0;
	}
}

void dispParam_WS2812() {
	switch (whatModeDisp) {
		case standard: {
			standardDispParam_WS2818();
			break;
		}
		case multi: {
			for (int i = 0; i < MULTI_MODE_PARAM_NUM; i++) {
				switch (multiModeStruct.paramsArr[i]) {
					case temp: {
						multiModeStruct.dataArr[i] = temp_meas.value / temp_meas.measNum;
						break;
					}
					case humid: {
						multiModeStruct.dataArr[i] = humid_meas.value / humid_meas.measNum;
						break;
					}
					case eCO2: {
						multiModeStruct.dataArr[i] = eCO2_meas.value / eCO2_meas.measNum;
						break;
					}
					case PM2_5: {
						multiModeStruct.dataArr[i] = PM_2_5_meas.value / PM_2_5_meas.measNum;
						break;
					}
					case TVOC: {
						multiModeStruct.dataArr[i] = TVOC_meas.value / TVOC_meas.measNum;
						break;
					}
					default:
						break;
				}
			}
			WS2812_showParams_multi(multiModeStruct.dataArr, multiModeStruct.divideDotsArr);
			break;
		}
		case night: {
			nightDispParam_WS2818();
			break;
		}
		default:
			break;
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
	WiFiConnected = true;
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	WiFiConnected = false;
}

BLYNK_WRITE(V1) {
	int red = param[0].asInt();
	int green = param[1].asInt();
	int blue = param[2].asInt();
	log_i("V0, r: %d, g: %d, b: %d,", red, green, blue);
	uint32_t color = ((uint32_t)red << 16) | ((uint32_t)green << 8) | blue;
	WS2812_fillColor(color);
	delay(100);
	whatModeDisp = noneMode;
	whatEffectDisp = noneEffect;
}

BLYNK_WRITE(V2) {
	WS2812_setBrightnessPerCent(param.asInt());
}

BLYNK_WRITE(V3) {
	switch (param.asInt()) {
		case 1: {
			whatModeDisp = standard;
			WS2812_clear();
			delay(100);
			break;
		}
		case 2: {
			whatModeDisp = multi;

			multiModeStruct.paramsArr[0] = temp;
			multiModeStruct.paramsArr[1] = eCO2;
			multiModeStruct.paramsArr[2] = PM2_5;

			multiModeStruct.divideDotsArr[0] = temp_divideDots;
			multiModeStruct.divideDotsArr[1] = eCO2_divideDots;
			multiModeStruct.divideDotsArr[2] = PM2_5_divideDots;

			WS2812_clear();
			delay(100);
			break;
		}
		case 3: {
			whatModeDisp = night;
			WS2812_clear();
			delay(100);
			break;
		}
		default:
			whatModeDisp = noneMode;
	}
	dispParam_WS2812();
	whatEffectDisp = noneEffect;
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
		case 6: {
			whatParamDisp = total;
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
	dispParam_WS2812();
	whatParamDisp = noneParam;
}