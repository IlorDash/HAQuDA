#pragma once

#include "HAQuDA_DisplayManip.h"
#include "HAQuDA_DisplayInterface_typedefs.h"
#include "HAQuDA_ErrorHandler.h"
#include "WS2812.h"

class HAQuDA_DisplayInterface {
  public:
	static displayMode_enum DisplayMode;

	static displayEffectParams_struct DisplayEffectParams;
	
	displayMeasParams_struct DisplayMeasParams;

	uint8_t brightnessPerCent;

	HAQuDA_DisplayInterface();
	void SetStaticColor(int red, int green, int blue);
	void SetBrightness(uint8_t newBrightnessPerCent);
	
	void ChangeDisplayMeasMode(uint8_t newDisplayMeasMode_num);
	void ChangeDisplayMeasParam(uint8_t newDisplayMeasParamn_num);

	void ChangeDisplayEffect(uint8_t newDisplayEffect_num);
};