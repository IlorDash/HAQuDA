#pragma once
#include "HAQuDA_DisplayInterface_typedefs.h"
typedef enum { failedToConnectToWiFi, failedToCreateAP, noneError } errorTypes_enum;

#define COLOR_RED 0xFF0000

class HAQuDA_ErrorHandler {
  public:
	static displayEffectMode_enum errorEffectDisp;
	static growEffectsParams_struct failedToConnectToWiFi_displayParams;
	static snakeEffectsParams_struct failedToCreateAP_displayParams;
	
	static errorTypes_enum CurrentError;
	
	HAQuDA_ErrorHandler();

	void FailedToConnectToWiFi();
	void FailedToCreateAP();

	~HAQuDA_ErrorHandler();
};