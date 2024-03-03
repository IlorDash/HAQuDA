#include "Arduino.h"

#include "HAQuDA_WiFi_handler.h"

#include "HAQuDA_DisplayManip.h"
#include "HAQuDA_MeasHandler.h"
#include "HAQuDA_EffectsHandler.h"
#include "HAQuDA_ErrorHandler.h"

#include "HAQuDA_FileStorage.h"
#include "HAQuDA_Logger.h"
#include "HAQuDA_TimeHelper.h"
#include "HAQuDA_LEDs.h"

#include "Tasks.h"

#define BUILTIN_LED 2

#define BUILTIN_LED_PERIOD 1000

HAQuDA_ErrorHandler ErrHandler;
HAQuDA_FileStorage FileStorage;
HAQuDA_EffectsHandler EffectsHandler;
HAQuDA_MeasHandler MeasHandler(&ErrHandler);
HAQuDA_DisplayManip DispManip(&MeasHandler, &EffectsHandler, &ErrHandler);
HAQuDA_WiFi_handler WiFiHandler(&FileStorage, &DispManip);

int meas_start_time = 0;
int builtin_blink_start_time = 0;

void setup() {

	Serial.begin(115200);
	pinMode(BUILTIN_LED, OUTPUT);

	HAQuDA_Logger::SetErrHandler(&ErrHandler);
	HAQuDA_TimeService::getInstance();
	HAQuDA_LEDs::getInstance()->Begin();
	createTasks(&DispManip);

	DispManip.SetEffect(snake);
	vTaskDelay(2000 / portTICK_PERIOD_MS);

	if (!FileStorage.Start()) {
		DispManip.SetMode(error);
		ErrHandler.FailedToStartFS();
		while (true) {
		}
	}
	HAQuDA_Logger::PrepareLogsFile();
	HAQuDA_Logger::LogInfo("Started File system");

	if (!WiFiHandler.Connect()) {
		DispManip.SetMode(error);
		ErrHandler.FailedToConnectToWiFi();

		if (!WiFiHandler.CreateAP()) {
			DispManip.SetMode(error);
			ErrHandler.FailedToCreateAP();
			while (true) {
			}
		}
		HAQuDA_Logger::LogInfo("Created Acces Point");
		DispManip.SetMode(noneMode);
		ErrHandler.ClearError();
		HAQuDA_DisplayManip::SetEffect(fading);
	} else {
		HAQuDA_TimeService::StartNTP();
		HAQuDA_Logger::LogInfo("Connected to WiFi");
	}

	if (!MeasHandler.SensorsBegin()) {
		DispManip.SetMode(error);
		ErrHandler.FailedToStartSensors();
		if (!MeasHandler.TryRebootSensors()) {
			ESP.restart();
		}
	}
	HAQuDA_Logger::LogInfo("Started sensors");
	HAQuDA_DisplayManip::SetMeasMode(multi);
	meas_start_time = 0;
}

void loop() {
	WiFiHandler.HandleConnection();
	MeasHandler.UpdateMeas(&meas_start_time);
	if ((millis() - builtin_blink_start_time) > BUILTIN_LED_PERIOD) {
		digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
		builtin_blink_start_time = millis();
	}
}