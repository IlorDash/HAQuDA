#pragma once
#include "HAQuDA_DisplayManip_typedefs.h"

typedef enum { failedToStartFS, failedToConnectToWiFi, failedToCreateAP, failedToStartSensors, noneError } errorTypes_enum;

#define DEFAULT_ERROR_DISPLAY_TIME 5000 // 5 sec

class HAQuDA_ErrorHandler {
	static effect_mode errorEffectDisp;

	static errorTypes_enum CurrentError;

  public:
	static grow_effect_params failedToConnectToWiFi_displayParams;
	static snake_effect_params failedToCreateAP_displayParams;

	static fade_effect_params failedToStartFS_displayParams;

	static random_effect_params failedToStartSensors_displayParams;

	HAQuDA_ErrorHandler();

	static errorTypes_enum GetCurrentError();

	void FailedToConnectToWiFi();
	void FailedToCreateAP();
	void FailedToStartFS();
	void FailedToStartSensors();
	void ClearCurrentError();

	~HAQuDA_ErrorHandler();
};