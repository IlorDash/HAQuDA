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
#include "HAQuDA_FlashLock.h"

#include "Tasks.h"
#include "HAQuDA_Version.h"

#define BUILTIN_LED 2

#define BUILTIN_LED_PERIOD 1000

HAQuDA_ErrorHandler ErrHandler;
HAQuDA_FileStorage FileStorage;
HAQuDA_LogDedup LogDedup;
HAQuDA_EffectsHandler EffectsHandler;
HAQuDA_MeasHandler MeasHandler(&ErrHandler);
HAQuDA_DisplayManip DispManip(&MeasHandler, &EffectsHandler, &ErrHandler);
HAQuDA_WiFi_handler WiFiHandler(&FileStorage, &DispManip);

int meas_start_time = 0;
int builtin_blink_start_time = 0;

// A missing sensor is usually a loose connector or a slow CCS811 start, fixed by the user plugging it in.
// 5 minutes keeps the ~8 s power-cycle block of loop() rare and the log free of retry spam.
constexpr uint32_t SENSORS_START_RETRY_PERIOD_MS = 5 * 60 * 1000;

constexpr uint32_t WIFI_ERROR_DISPLAY_MS = 10 * 1000;

// Both written and read in the loop task only (setup runs in it too).
bool sensorsRunning = false;
uint32_t sensorsStartRetryMs = 0;

void enableMeasurements();
void leaveSensorsError();

void setup() {

	Serial.begin(115200);
	// Serial, not the logger: the file system is not mounted yet, so a log record could not be stored.
	Serial.printf("HAQuDA firmware v%s\r\n", FW_VERSION);
	pinMode(BUILTIN_LED, OUTPUT);

	HAQuDA_Logger::SetErrHandler(&ErrHandler);
	HAQuDA_Logger::SetDedup(&LogDedup);
	HAQuDA_TimeService::getInstance();
	// Before the LEDs and the file storage start: they share it so a flash access never overlaps an LED frame.
	HAQuDA_FlashLock::Create();
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
		// CreateAP() replaces the red growing effect with the access point effect, so without this pause the user
		// never sees that the WiFi connection failed. Blocks the loop task only; ShowTask draws on the other core.
		vTaskDelay(pdMS_TO_TICKS(WIFI_ERROR_DISPLAY_MS));

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
			// No restart: it would take the access point, web server and WebSerial down with the lamp.
			// The lamp keeps the sensors error on the matrix and loop() retries.
			HAQuDA_Logger::LogError("Sensors off, retry in 5 min");
			sensorsStartRetryMs = millis();
			return;
		}
		leaveSensorsError();
	}
	enableMeasurements();
}

// Loop task only. SetMode() refuses to leave the error mode while currentError is set, so clear it first; the error
// effects (NTP fade, random pixels) leave the LEDs dim and lit, so ShowTask clears them and re-applies the user's brightness
// when meas mode starts.
void leaveSensorsError() {
	HAQuDA_DisplayManip::RestoreBrightnessOnMeas();
	ErrHandler.ClearError();
}

void enableMeasurements() {
	sensorsRunning = true;
	HAQuDA_Logger::LogInfo("Started sensors");
	// A mode picked on the web page while the error was shown is kept. The mode is none only if nothing set one before
	// boot (the constructor default), so none falls back to multi.
	const meas_mode selectedMode = DispManip.GetMeasDispMode();
	HAQuDA_DisplayManip::SetMeasMode((selectedMode == noneMeas) ? multi : selectedMode);
	meas_start_time = 0;
}

// Called from loop(). TryRebootSensors() blocks loop for up to ~8 s, so retries are rare.
void retrySensorsStart() {
	if (sensorsRunning || ((millis() - sensorsStartRetryMs) < SENSORS_START_RETRY_PERIOD_MS)) {
		return;
	}
	sensorsStartRetryMs = millis();
	if (MeasHandler.TryRebootSensors()) {
		leaveSensorsError();
		enableMeasurements();
	} else {
		// LogError, not LogInfo: failures go through it, and every log line already runs the time check.
		HAQuDA_Logger::LogError("Sensors retry failed");
	}
}

void loop() {
	WiFiHandler.HandleConnection();
	retrySensorsStart();
	MeasHandler.UpdateMeas(&meas_start_time);
	HAQuDA_Logger::FlushExpiredRepeats();
	if ((millis() - builtin_blink_start_time) > BUILTIN_LED_PERIOD) {
		digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
		builtin_blink_start_time = millis();
	}
}