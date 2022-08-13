#include "Arduino.h"

#include "HAQuDA_WiFi_handler.h"
#include "HAQuDA_DisplayInterface.h"
#include "HAQuDA_FileStorage.h"

#include <SoftwareSerial.h>

#include "Sensors.h"
#include "Tasks.h"
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#define DISP_MEAS_PERIOD 300000 //=5 min in ms
#define SENSORS_MEAS_PERIOD 2000

#define T_SENSOR_1 32
#define T_SENSOR_2 27
#define T_SENSOR_3 33
#define T_SENSOR_4 4

char BlynkAuth[] = BLYNK_AUTH;

WidgetTerminal terminal(V0);
HAQuDA_DisplayManip myDispManip;
HAQuDA_FileStorage myFS;
HAQuDA_DisplayInterface myUI(myDispManip);
HAQuDA_WiFi_handler *myWiFi_handler;

void UpdateVirtualPins();
void blynkPrintLog();

int firstDot;
int secondDot; // parametrs of dispParams
int thirdDot;

bool dispFirstParam = false;

void setup() {
	Serial.begin(115200);
	myWiFi_handler = new HAQuDA_WiFi_handler(&myUI, &myFS);

	WS2812_begin();
	createTasks(&myUI);

	if (!myFS.Start()) {
		log_e("SPIFFS not mounted!");
		return;
	}
	log_i("SPIFFS mounted successfully");

	myWiFi_handler->WiFi_connect();

	//	if (!sensorsBegin()) {
	//		WS2812_fillColor(COLOR_RED);
	//		terminal.println("FATAL ERROR: Failed to begin sensors");
	//		while (1) {
	//		}
	//	}
	//
	//	terminal.println("*************************");
	//	terminal.print("START LOGGING");
}

uint16_t measNum = 0;
uint32_t dispMeasTimer = 0;
uint32_t sensors_meas_time = 0;

int sens_val_1;
int sens_val_2;
int sens_val_3;
int sens_val_4;

void loop() {

	sens_val_1 = touchRead(T_SENSOR_1);
	sens_val_2 = touchRead(T_SENSOR_2);
	sens_val_3 = touchRead(T_SENSOR_3);
	sens_val_4 = touchRead(T_SENSOR_4);
	

	if (sens_val_1 || sens_val_2 || sens_val_3 || sens_val_4) {
		sens_val_1 = 0;
		sens_val_2 = 0;
		sens_val_3 = 0;
		sens_val_4 = 0;
	}

	myWiFi_handler->WiFi_handleConnection();

	/*if (!Blynk.connected()) {
		Blynk.connect();
	} else {
		Blynk.run();
	}
	vTaskDelay(100);

	if ((millis() - sensors_meas_time) > SENSORS_MEAS_PERIOD) {
		getDHT11_meas();
		getCCS811_meas();
		getPM_meas();

		measNum++;
		blynkPrintLog();

		sensors_meas_time = millis();
	}

	if (millis() - dispMeasTimer > DISP_MEAS_PERIOD) {
		myDispManip->displayData(myUI->currUI_Params);
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
	}*/
}
//
// void checkIfMeasCorrect() {
//	if (!eCO2_meas.newMeasDone) {
//		terminal.println("ERROR: Failed to get eCO2 measurment");
//	}
//	if (!TVOC_meas.newMeasDone) {
//		terminal.println("ERROR: Failed to get TVOC measurment");
//	}
//	if (!PM_2_5_meas.newMeasDone) {
//		terminal.println("ERROR: Failed to get PM 2.5 measurment");
//	}
//	if (!temp_meas.newMeasDone) {
//		terminal.println("ERROR: Failed to get TEMP measurment");
//	}
//	if (!humid_meas.newMeasDone) {
//		terminal.println("ERROR: Failed to get HUMID measurment");
//	}
//	terminal.flush();
//
//	eCO2_meas.newMeasDone = false;
//	TVOC_meas.newMeasDone = false;
//	PM_2_5_meas.newMeasDone = false;
//	temp_meas.newMeasDone = false;
//	humid_meas.newMeasDone = false;
//}
//
// void blynkPrintLog() {
//	terminal.println();
//	terminal.println("----------------------------");
//	terminal.print(measNum);
//	terminal.println(" - measurment");
//
//	checkIfMeasCorrect();
//
//	terminal.print("Temp: ");
//	terminal.print(temp_meas.value / temp_meas.measNum);
//	terminal.println("  C");
//	terminal.print("Humid: ");
//	terminal.print(humid_meas.value / humid_meas.measNum);
//	terminal.println("  %");
//	terminal.println();
//
//	terminal.flush();
//
//	terminal.print("PM1.0: ");
//	terminal.print(PM_1_0_meas.value / PM_1_0_meas.measNum);
//	terminal.println("  ug/m3");
//	terminal.print("PM2.5: ");
//	terminal.print(PM_2_5_meas.value / PM_2_5_meas.measNum);
//	terminal.println("  ug/m3");
//	terminal.print("PM1 0: ");
//	terminal.print(PM_10_meas.value / PM_10_meas.measNum);
//	terminal.println("  ug/m3");
//	terminal.println();
//
//	terminal.flush();
//
//	terminal.print("eCO2: ");
//	terminal.print(eCO2_meas.value / eCO2_meas.measNum);
//	terminal.println("  ppm");
//	terminal.println();
//
//	terminal.print("TVOC: ");
//	terminal.print(TVOC_meas.value / TVOC_meas.measNum);
//	terminal.println("  ppb");
//	terminal.println();
//
//	terminal.flush();
//
//	/*if (myUI.currUI_Params.dispMode == standard) {
//		terminal.println("Display standard");
//		switch (myUI.currUI_Params.dispParam) {
//			case total:
//				terminal.println("Display total quality");
//				break;
//			case temp:
//				terminal.println("Display temperature");
//				break;
//			case humid:
//				terminal.println("Display humidity");
//				break;
//			case eCO2:
//				terminal.println("Display eCO2");
//				break;
//			case TVOC:
//				terminal.println("Display TVOC");
//				break;
//			case PM2_5:
//				terminal.println("Display PM2_5");
//				break;
//			default:
//				terminal.println("Display none parametr");
//		}
//	} else if (myUI.currUI_Params.dispMode == multi) {
//		terminal.println("Display multi");
//		for (int i = 0; i < MULTI_MODE_PARAM_NUM; i++) {
//			switch (myUI.currUI_Params.multiModeStruct.paramsArr[i]) {
//				case total:
//					terminal.println("Display total quality");
//					break;
//				case temp:
//					terminal.println("Display temperature");
//					break;
//				case humid:
//					terminal.println("Display humidity");
//					break;
//				case eCO2:
//					terminal.println("Display eCO2");
//					break;
//				case TVOC:
//					terminal.println("Display TVOC");
//					break;
//				case PM2_5:
//					terminal.println("Display PM2_5");
//					break;
//				default:
//					terminal.println("Display none parametr");
//			}
//		}
//	} else if (myUI.currUI_Params.dispMode == night) {
//		switch (myUI.currUI_Params.dispParam) {
//			case total:
//				terminal.println("Display total quality");
//				break;
//			case temp:
//				terminal.println("Display temperature");
//				break;
//			case humid:
//				terminal.println("Display humidity");
//				break;
//			case eCO2:
//				terminal.println("Display eCO2");
//				break;
//			case TVOC:
//				terminal.println("Display TVOC");
//				break;
//			case PM2_5:
//				terminal.println("Display PM2_5");
//				break;
//			default:
//				terminal.println("Display none parametr");
//		}
//	} else {
//		switch (myUI.currUI_Params.dispEffect) {
//			case snake:
//				terminal.println("Display snake effect");
//				break;
//			case randomPixel:
//				terminal.println("Display random pixel effect");
//				break;
//			case fade:
//				terminal.println("Display fade effect");
//				break;
//			case christmasTree:
//				terminal.println("Display Christmas Tree effect");
//				break;
//			default:
//				break;
//		}
//	}*/
//
//	terminal.flush();
//}
//
// BLYNK_WRITE(V1) {
//	int red = param[0].asInt();
//	int green = param[1].asInt();
//	int blue = param[2].asInt();
//
//	//myUI.ext_setStaticColor(red, green, blue);
//}
//
// BLYNK_WRITE(V2) {
//	//myUI.ext_setBrightness(param.asInt());
//}
//
// BLYNK_WRITE(V3) {
//	//myUI.ext_changeDispMode(param.asInt());
//}
//
// BLYNK_WRITE(V4) {
//	//myUI.ext_changeDispParam(param.asInt());
//}
//
// BLYNK_WRITE(V5) {
//	//myUI.ext_changeDispEffect(param.asInt());
//}