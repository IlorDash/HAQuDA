#pragma once
#include "HAQuDA_DisplayInterface_typedefs.h"
typedef enum { noneError, failedToConnectToWiFi, failedToCreateAP } errorTypes_enum;

#define COLOR_RED 0xFF0000

class HAQuDA_ErrorHandler {
  private:
  public:
	HAQuDA_ErrorHandler();
	static displayEffect_enum errorEffectDisp;
	static growEffectsParams_struct failedToConnectToWiFi_displayParams;
	static snakeEffectsParams_struct failedToCreateAP_displayParams;

	~HAQuDA_ErrorHandler();
};