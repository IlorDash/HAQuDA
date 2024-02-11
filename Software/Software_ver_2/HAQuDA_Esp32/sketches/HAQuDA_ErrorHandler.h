#pragma once

#include "HAQuDA_EffectsHandler.h"

typedef enum { failedToConnectToWiFi, failedToCreateAP, failedToStartFS, failedToStartSensors, failedToUpdateNTP, noneError } error_type;

#define DEFAULT_ERROR_DISPLAY_TIME 5000 // 5 sec

class HAQuDA_ErrorHandler : HAQuDA_EffectsHandler {
	static error_type currentError;

	static snake_params failedToCreateAPEffect;
	static growing_params failedToConnectToWiFiEffect;
	static random_led_params failedToStartSensorsEffect;
	static fading_params failedToStartFSEffect;
	static fading_params failedToUpdateNTPEffect;

  public:
	HAQuDA_ErrorHandler();

	static void Show();

	static error_type GetCurrentError() { return currentError; }

	void FailedToConnectToWiFi();
	void FailedToCreateAP();
	void FailedToStartFS();
	void FailedToStartSensors();
	void FailedToUpdateNTP();
	void FailedToGetMeas(const char *msg);
	void ClearError();

	~HAQuDA_ErrorHandler();
};