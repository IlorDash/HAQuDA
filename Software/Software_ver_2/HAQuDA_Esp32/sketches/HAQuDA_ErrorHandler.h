#pragma once

#include <atomic>

class HAQuDA_DisplayService;

class HAQuDA_ErrorHandler {
	HAQuDA_DisplayService *display = nullptr;

	// Set by the logger's time stamp, read and cleared by UpdateNtpError(): both run in the loop task, atomic so a log call from
	// another task could not break it
	std::atomic<bool> ntpSyncFailed{false};
	// Loop task only
	bool ntpErrorPosted = false;
	bool ntpErrorLogged = false;

  public:
	HAQuDA_ErrorHandler() = default;

	// The errors are posted to this service. Loop task, once in setup() before the first error
	void SetDisplay(HAQuDA_DisplayService *displayService);

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
