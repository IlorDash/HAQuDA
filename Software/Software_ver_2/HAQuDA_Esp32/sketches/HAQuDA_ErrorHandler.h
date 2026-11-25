#pragma once

#include <atomic>

#include "HAQuDA_DisplayTypes.h"
#include "HAQuDA_EffectsHandler.h"

class HAQuDA_DisplayService;

class HAQuDA_ErrorHandler : HAQuDA_EffectsHandler {
	static error_type currentError;
	HAQuDA_DisplayService *display = nullptr;

	// Set by the logger's time stamp, read and cleared by UpdateNtpError(): both run in the loop task, atomic so a log call from
	// another task could not break it
	std::atomic<bool> ntpSyncFailed{false};
	// Loop task only
	bool ntpErrorPosted = false;
	bool ntpErrorLogged = false;

	static snake_params failedToCreateAPEffect;
	static growing_params failedToConnectToWiFiEffect;
	static random_led_params failedToStartSensorsEffect;
	static fading_params failedToStartFSEffect;
	static fading_params failedToUpdateNTPEffect;

  public:
	HAQuDA_ErrorHandler();

	// The errors are posted to this service. Loop task, once in setup() before the first error
	void SetDisplay(HAQuDA_DisplayService *displayService);

	static void Show();

	static error_type GetCurrentError() {
		return currentError;
	}

	// Posts the error, logs it, does not block. Loop task only (setup runs in it).
	void FailedToConnectToWiFi();
	// Posts the error, logs it, does not block. Loop task only.
	void FailedToCreateAP();
	// Posts the error, does not block and does not log (the file system is not available). Loop task only.
	void FailedToStartFS();
	// Posts the error, logs it, does not block. Loop task only.
	void FailedToStartSensors();
	// Runs on every log time stamp: must neither post (floods the queue) nor log (recurses). Loop task only.
	void FailedToUpdateNTP();
	// Blocks up to ~1 s once per minute while the error shows: a quiet lamp writes no log lines, the only other NTP retry. Loop task only.
	void UpdateNtpError(bool timeKnown);
	void FailedToGetMeas(const char *msg);
	// Ends the sensors error on the display
	void ClearError();

	~HAQuDA_ErrorHandler();
};