#pragma once
#include "HAQuDA_DisplayManip_typedefs.h"

typedef enum { failedToStartFS, failedToConnectToWiFi, failedToCreateAP, failedToStartSensors, noneError } errorTypes_enum;

#define DEFAULT_ERROR_DISPLAY_TIME 5000 // 5 sec

class HAQuDA_ErrorHandler {
	static displayEffectMode_enum errorEffectDisp;

	static errorTypes_enum CurrentError;

  public:
	static growEffectsParams_struct failedToConnectToWiFi_displayParams;
	static snakeEffectsParams_struct failedToCreateAP_displayParams;

	static fadeEffectsParams_struct failedToStartFS_displayParams;

	static upDownEffectsParams_struct failedToStartSensors_displayParams;

	HAQuDA_ErrorHandler();

	static errorTypes_enum GetCurrentError();

	void FailedToConnectToWiFi();
	void FailedToCreateAP();
	void FailedToStartFS();
	void FailedToStartSensors();
	void ClearCurrentError();

	~HAQuDA_ErrorHandler();
};