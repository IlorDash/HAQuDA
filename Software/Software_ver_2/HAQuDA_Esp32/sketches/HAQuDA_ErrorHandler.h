#pragma once
#include "HAQuDA_DisplayManip_typedefs.h"

typedef enum { failedToStartFS, failedToConnectToWiFi, failedToCreateAP, failedToStartSensors, noneError } errorTypes;

#define DEFAULT_ERROR_DISPLAY_TIME 5000 // 5 sec

class HAQuDA_ErrorHandler {
	static effect_mode errorEffectDisp;

	static errorTypes CurrentError;

  public:
	static grow_effect_params failedToConnectToWiFiEffect;
	static snake_effect_params failedToCreateAPEffect;

	static fade_effect_params failedToStartFSEffect;

	static random_effect_params failedToStartSensorsEffect;

	HAQuDA_ErrorHandler();

	static errorTypes GetCurrentError();

	void FailedToConnectToWiFi();
	void FailedToCreateAP();
	void FailedToStartFS();
	void FailedToStartSensors();
	void ClearCurrentError();

	~HAQuDA_ErrorHandler();
};