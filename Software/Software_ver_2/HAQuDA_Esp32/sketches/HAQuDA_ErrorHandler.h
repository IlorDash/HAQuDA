#pragma once

#include "HAQuDA_EffectsHandler.h"

typedef enum { failedToConnectToWiFi, failedToCreateAP, failedToStartFS, failedToStartSensors, failedToUpdateNTP, noneError } error_type;

class HAQuDA_ErrorHandler : HAQuDA_EffectsHandler {
	friend class HAQuDA_WebPageDisplayManip;

	static error_type currentError;

	static snake_params failedToCreateAPEffect;
	static growing_params failedToConnectToWiFiEffect;
	static random_led_params failedToStartSensorsEffect;
	static fading_params failedToStartFSEffect;
	static fading_params failedToUpdateNTPEffect;

  public:
	HAQuDA_ErrorHandler();

	static void Show();

	static error_type GetCurrentError() {
		return currentError;
	}

	// Sets the error effect and logs it, does not block. Loop task only (setup runs in it).
	void FailedToConnectToWiFi();
	// Sets the error effect and logs it, does not block. Loop task only.
	void FailedToCreateAP();
	// Sets the error effect, does not block and does not log (the file system is not available). Loop task only.
	void FailedToStartFS();
	// Sets the error effect and logs it, does not block. Loop task only.
	void FailedToStartSensors();
	// Only sets the error effect, does not block. Called by the logger, so loop task only.
	void FailedToUpdateNTP();
	void FailedToGetMeas(const char *msg);
	void ClearError();

	~HAQuDA_ErrorHandler();
};