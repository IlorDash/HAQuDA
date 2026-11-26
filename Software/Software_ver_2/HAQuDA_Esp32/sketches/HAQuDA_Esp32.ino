#include "Arduino.h"

#include "HAQuDA_WiFi_handler.h"

#include "HAQuDA_DisplayService.h"
#include "HAQuDA_MeasHandler.h"
#include "HAQuDA_ErrorHandler.h"
#include "HAQuDA_WebPageDisplayManip.h"
#include "HAQuDA_LampControl.h"
#include "HAQuDA_LampAdapters.h"

#include "HAQuDA_FileStorage.h"
#include "HAQuDA_Logger.h"
#include "HAQuDA_TimeHelper.h"
#include "HAQuDA_LEDs.h"

#include "Tasks.h"
#include "HAQuDA_Version.h"

#define BUILTIN_LED 2

#define BUILTIN_LED_PERIOD 1000

HAQuDA_ErrorHandler ErrHandler;
HAQuDA_FileStorage FileStorage;
HAQuDA_LogDedup LogDedup;
HAQuDA_MeasSnapshot MeasSnapshot;
HAQuDA_WebPageDisplayManip WebDisp;
HAQuDA_MeasHandler MeasHandler(&ErrHandler, MeasSnapshot);
HAQuDA_DisplayService Display(*HAQuDA_LEDs::getInstance(), MeasHandler, WebDisp, DEFAULT_BRIGHTNESS);
HAQuDA_FileStorageAdapter LampFiles;
HAQuDA_LampDisplayAdapter LampDisplay(ErrHandler, Display, MeasHandler);
HAQuDA_LoggerAdapter LampLogger;
HAQuDA_LampControl LampControl(LampFiles, LampDisplay, LampLogger);
HAQuDA_WiFi_handler WiFiHandler(&FileStorage, Display, MeasSnapshot, WebDisp, LampControl);

int meas_start_time = 0;
int builtin_blink_start_time = 0;

// A missing sensor is usually a loose connector or a slow CCS811 start, fixed by the user plugging it in.
// 5 minutes keeps the power-cycle block of loop() (up to ~27 s: 4 start attempts of up to ~5 s plus 3 power-offs of 2 s) rare
// and the log free of retry spam.
constexpr uint32_t SENSORS_START_RETRY_PERIOD_MS = 5 * 60 * 1000;

// Both written and read in the loop task only (setup runs in it too).
// LampControl keeps its own copy of sensorsRunning, set by LampControl.SensorsStarted(): any new path that starts the sensors must
// call it, or the lamp keeps showing the sensors error for the measurement modes.
bool sensorsRunning = false;
uint32_t sensorsStartRetryMs = 0;

// The boot display has run by then, so the first value already covers the deepest effect code paths.
constexpr uint32_t SHOW_STACK_FIRST_LOG_MS = 10 * 1000;
constexpr uint32_t SHOW_STACK_CHECK_PERIOD_MS = 1000;

TaskHandle_t showTaskHandle = nullptr;
// Both written and read in the loop task only.
uint32_t showStackMinLoggedBytes = UINT32_MAX;
uint32_t showStackCheckMs = 0;

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
	HAQuDA_LEDs::getInstance()->Begin();
	ErrHandler.SetDisplay(&Display);
	Display.Begin();
	createTasks(&Display, &showTaskHandle);

	Display.Post(display_event::bootSnake);

	if (!FileStorage.Start()) {
		ErrHandler.FailedToStartFS();
		while (true) {
		}
	}
	HAQuDA_Logger::PrepareLogsFile();
	HAQuDA_Logger::LogInfo("Started File system");
	// Before the web server starts: the page reads the settings from it
	LampControl.Load();

	if (!WiFiHandler.Connect()) {
		ErrHandler.FailedToConnectToWiFi();

		if (!WiFiHandler.CreateAP()) {
			ErrHandler.FailedToCreateAP();
			while (true) {
			}
		}
		HAQuDA_Logger::LogInfo("Created Acces Point");
		ErrHandler.ClearError();
		LampDisplay.ShowAccessPointRunning();
	} else {
		HAQuDA_TimeService::StartNTP();
		HAQuDA_Logger::LogInfo("Connected to WiFi");
	}

	if (!MeasHandler.SensorsBegin()) {
		ErrHandler.FailedToStartSensors();
		if (!MeasHandler.TryRebootSensors()) {
			// No restart: it would take the access point, web server and WebSerial down with the lamp.
			// The lamp keeps the sensors error on the matrix and loop() retries.
			HAQuDA_Logger::LogError("Sensors off, retry in 5 min");
			sensorsStartRetryMs = millis();
			// The error has been on the matrix for the whole start attempt. A saved lamp light or power off needs no sensors and
			// replaces it; a measurement mode keeps it.
			LampControl.Apply();
			return;
		}
		leaveSensorsError();
	}
	enableMeasurements();
}

void leaveSensorsError() {
	ErrHandler.ClearError();
}

void enableMeasurements() {
	sensorsRunning = true;
	LampControl.SensorsStarted();
	// The user's picture: what was saved in /lamp, or changed on the web page while the error was shown (the defaults
	// otherwise: multi mode)
	LampControl.Apply();
	meas_start_time = 0;
	HAQuDA_Logger::LogInfo("Started sensors");
}

// Called from loop(). TryRebootSensors() blocks loop for up to ~27 s, so retries are rare.
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

// Loop task only: ShowTask must not log. StackType_t is uint8_t on the ESP32 port, so the value is in bytes.
void logShowTaskStack() {
	if ((showTaskHandle == nullptr) || (millis() < SHOW_STACK_FIRST_LOG_MS) || ((millis() - showStackCheckMs) < SHOW_STACK_CHECK_PERIOD_MS)) {
		return;
	}
	showStackCheckMs = millis();
	const uint32_t freeMinBytes = uxTaskGetStackHighWaterMark(showTaskHandle);
	if (freeMinBytes >= showStackMinLoggedBytes) {
		return;
	}
	showStackMinLoggedBytes = freeMinBytes;
	// A log record keeps 31 characters; with the 4-digit value (the stack is 3000 B) this text is exactly 31
	char msg[48] = {};
	snprintf(msg, sizeof(msg), "ShowTask stack min free: %lu B", static_cast<unsigned long>(freeMinBytes));
	HAQuDA_Logger::LogInfo(msg);
}

void loop() {
	WiFiHandler.HandleConnection();
	logShowTaskStack();
	retrySensorsStart();
	MeasHandler.UpdateMeas(&meas_start_time);
	// loop task only: the time client is written without a lock by the logger (loop task) and by the night mode redraw (ShowTask)
	// and is created lazily, so the async_tcp task must not read it; LampControl hands the minute to that task through an atomic
	uint16_t minuteOfDay = 0;
	const bool timeKnown = HAQuDA_TimeService::GetCachedMinuteOfDay(&minuteOfDay);
	LampControl.Handle(timeKnown, minuteOfDay);
	ErrHandler.UpdateNtpError(timeKnown);
	HAQuDA_Logger::FlushExpiredRepeats();
	if ((millis() - builtin_blink_start_time) > BUILTIN_LED_PERIOD) {
		digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
		builtin_blink_start_time = millis();
	}
}