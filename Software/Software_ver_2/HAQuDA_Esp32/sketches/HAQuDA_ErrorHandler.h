#pragma once

#include "HAQuDA_DisplayTypes.h"
#include "HAQuDA_EffectsHandler.h"

class HAQuDA_DisplayService;

class HAQuDA_ErrorHandler : HAQuDA_EffectsHandler {
	friend class HAQuDA_WebPageDisplayManip;

	static error_type currentError;
	HAQuDA_DisplayService *display = nullptr;

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
	// Only records the error for the web page and posts nothing yet, does not block. Called by the logger, so loop task only.
	void FailedToUpdateNTP();
	void FailedToGetMeas(const char *msg);
	// Ends the sensors error on the display
	void ClearError();

	~HAQuDA_ErrorHandler();
};