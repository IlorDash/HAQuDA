#include "Arduino.h"

#include "HAQuDA_WiFi_handler.h"

#include "HAQuDA_DisplayManip.h"
#include "HAQuDA_MeasHandler.h"
#include "HAQuDA_EffectsHandler.h"
#include "HAQuDA_ErrorHandler.h"
#include "HAQuDA_WebPageDisplayManip.h"
#include "HAQuDA_LampControl.h"
#include "HAQuDA_LampAdapters.h"
#include "HAQuDA_ButtonsControl.h"
#include "HAQuDA_ButtonActions.h"

#include "HAQuDA_FileStorage.h"
#include "HAQuDA_Logger.h"
#include "HAQuDA_TimeHelper.h"
#include "HAQuDA_LEDs.h"
#include "HAQuDA_TouchButtons.h"

#include "Tasks.h"
#include "HAQuDA_Version.h"

#define BUILTIN_LED 2

#define BUILTIN_LED_PERIOD 1000

HAQuDA_ErrorHandler ErrHandler;
HAQuDA_FileStorage FileStorage;
HAQuDA_LogDedup LogDedup;
HAQuDA_EffectsHandler EffectsHandler;
HAQuDA_MeasSnapshot MeasSnapshot;
HAQuDA_WebPageDisplayManip WebDisp;
HAQuDA_MeasHandler MeasHandler(&ErrHandler, MeasSnapshot, WebDisp);
HAQuDA_DisplayManip DispManip(&MeasHandler, &EffectsHandler, &ErrHandler, &WebDisp);
HAQuDA_FileStorageAdapter LampFiles;
HAQuDA_LampDisplayAdapter LampDisplay(ErrHandler);
HAQuDA_LoggerAdapter LampLogger;
HAQuDA_LampControl LampControl(LampFiles, LampDisplay, LampLogger);
HAQuDA_WiFi_handler WiFiHandler(&FileStorage, &DispManip, MeasSnapshot, WebDisp, LampControl);
HAQuDA_ButtonsControl ButtonsControl(LampFiles, LampLogger);

// Must live in RTC memory that the start-up code leaves alone (RTC_NOINIT_ATTR): it carries "start in access point mode" over the
// software restart. It holds noise after a power-on, hence the magic value and the reset reason check. Loop task only.
static RTC_NOINIT_ATTR uint32_t rtcApRequest;
constexpr uint32_t AP_REQUEST_MAGIC = 0x41505251;

class HAQuDA_ApRequestAdapter : public HAQuDA_IApRequest {
  public:
	bool RequestAccessPoint() override {
		if (WiFiHandler.RequestUserRestart() != restart_request_result::accepted) {
			return false;
		}
		// The restart itself comes later from loop(), so the flag is set before it can happen
		rtcApRequest = AP_REQUEST_MAGIC;
		return true;
	}
};

HAQuDA_ApRequestAdapter ApRequest;
HAQuDA_ButtonActions ButtonActions(LampControl, ButtonsControl, LampLogger, ApRequest);
HAQuDA_TouchButtons TouchButtons;

int meas_start_time = 0;
int builtin_blink_start_time = 0;

// A missing sensor is usually a loose connector or a slow CCS811 start, fixed by the user plugging it in.
// 5 minutes keeps the power-cycle block of loop() (up to ~27 s: 4 start attempts of up to ~5 s plus 3 power-offs of 2 s) rare
// and the log free of retry spam.
constexpr uint32_t SENSORS_START_RETRY_PERIOD_MS = 5 * 60 * 1000;

constexpr uint32_t WIFI_ERROR_DISPLAY_MS = 10 * 1000;

// Both written and read in the loop task only (setup runs in it too).
// LampControl keeps its own copy of sensorsRunning, set by LampControl.SensorsStarted(): any new path that starts the sensors must
// call it, or the lamp keeps showing the sensors error for the measurement modes.
bool sensorsRunning = false;
uint32_t sensorsStartRetryMs = 0;

void enableMeasurements();
void leaveSensorsError();

// Read and cleared before anything else, so the restart after the access point session is a normal one and a crash or brown-out
// during it cannot loop into the access point again
bool takeApRequest() {
	const bool requested = (esp_reset_reason() == ESP_RST_SW) && (rtcApRequest == AP_REQUEST_MAGIC);
	rtcApRequest = 0;
	return requested;
}

void setup() {

	const bool apRequested = takeApRequest();
	Serial.begin(115200);
	// Serial, not the logger: the file system is not mounted yet, so a log record could not be stored.
	Serial.printf("HAQuDA firmware v%s\r\n", FW_VERSION);
	pinMode(BUILTIN_LED, OUTPUT);

	HAQuDA_Logger::SetErrHandler(&ErrHandler);
	HAQuDA_Logger::SetDedup(&LogDedup);
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
	// Before the web server starts: the page reads the settings from it
	LampControl.Load();
	ButtonsControl.Load();
	// The buttons are optional: a failed start is only logged and the lamp runs without them
	TouchButtons.Begin();

	if (apRequested || !WiFiHandler.Connect()) {
		if (apRequested) {
			// Connect() does it on the normal path, before the stored networks are read
			WiFiHandler.RecoverCredsFile();
			WiFiHandler.SetApByButtons();
		} else {
			DispManip.SetMode(error);
			ErrHandler.FailedToConnectToWiFi();
			// CreateAP() replaces the red growing effect with the access point effect, so without this pause the user
			// never sees that the WiFi connection failed. Blocks the loop task only; ShowTask draws on the other core.
			vTaskDelay(pdMS_TO_TICKS(WIFI_ERROR_DISPLAY_MS));
		}

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
			// The error has been on the matrix for the whole start attempt. A saved lamp light or power off needs no sensors and
			// replaces it; a measurement mode keeps it.
			LampControl.Apply();
			TouchButtons.DiscardEvents();
			return;
		}
		leaveSensorsError();
	}
	enableMeasurements();
	// Touches made while setup() ran are stale by now
	TouchButtons.DiscardEvents();
}

// Loop task only. SetMode() refuses to leave the error mode while currentError is set, so clear it first; the error
// effects (NTP fade, random pixels) leave the LEDs dim and lit, so ShowTask clears them and re-applies the user's brightness
// when meas mode starts. Nothing may be logged between this call and SetMeasMode() in enableMeasurements().
void leaveSensorsError() {
	HAQuDA_DisplayManip::RestoreBrightnessOnMeas();
	ErrHandler.ClearError();
}

void enableMeasurements() {
	sensorsRunning = true;
	LampControl.SensorsStarted();
	// The user's picture: what was saved in /lamp, or changed on the web page while the error was shown (the defaults
	// otherwise: multi mode). Before the first log line: while NTP has never synced every log line sets the NTP error, and
	// SetMode() refuses to leave the error mode while an error is set. Once the mode is meas, later log lines only set a stale
	// error that nothing draws.
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

// Loop task only
void handleTouchButtons() {
	const buttons_settings buttonsCopy = ButtonsControl.CopySettings();
	TouchButtons.SetRepeatableMask(HAQuDA_RepeatableMask(buttonsCopy));

	touch_gesture_event batch[TOUCH_EVENT_QUEUE_LEN];
	uint8_t count = 0;
	while ((count < TOUCH_EVENT_QUEUE_LEN) && TouchButtons.TakeEvent(batch[count])) {
		count++;
	}
	ButtonActions.Handle(batch, count, buttonsCopy);

	const uint32_t dropped = TouchButtons.TakeDropped();
	if (dropped > 0) {
		char msg[48] = {};
		snprintf(msg, sizeof(msg), "Touch events dropped: %lu", static_cast<unsigned long>(dropped));
		HAQuDA_Logger::LogError(msg);
	}
}

void loop() {
	WiFiHandler.HandleConnection();
	retrySensorsStart();
	MeasHandler.UpdateMeas(&meas_start_time);
	// One read of the time service feeds both consumers. loop task only: the time client is written without a lock by the logger
	// (loop task) and by the night mode redraw (ShowTask) and is created lazily, so the async_tcp task must not read it; the
	// consumers hand the minute to that task through an atomic
	uint16_t minuteOfDay = 0;
	const bool timeKnown = HAQuDA_TimeService::GetCachedMinuteOfDay(&minuteOfDay);
	WebDisp.SetMinuteOfDay(timeKnown, minuteOfDay);
	LampControl.Handle(millis(), timeKnown, minuteOfDay);
	HAQuDA_Logger::FlushExpiredRepeats();
	ButtonsControl.Handle();
	handleTouchButtons();
	if ((millis() - builtin_blink_start_time) > BUILTIN_LED_PERIOD) {
		digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
		builtin_blink_start_time = millis();
	}
}