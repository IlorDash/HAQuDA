#pragma once

#include "HAQuDA_DisplayManip.h"
#include "HAQuDA_DisplayInterface_typedefs.h"
#include "HAQuDA_ErrorHandler.h"
#include "WS2812.h"

class HAQuDA_DisplayInterface {
  public:
	static whatDisplay_enum whatDisplay;

	static errorTypes_enum displayError;

	static displayEffect_enum displayEffect;

	static growEffectsParams_struct growEffectParams;
	static snakeEffectsParams_struct snakeEffectParams;
	static fadeEffectsParams_struct fadeEffectParams;
	static randomEffectsParams_struct randomEffectParams;

	displayMeasParams_struct displayMeasParams;

	uint8_t brightnessPerCent;

	HAQuDA_DisplayInterface();
	void ext_setStaticColor(int red, int green, int blue);
	void ext_setBrightness(uint8_t newBrightnessPerCent);
	void ext_changeDisplayMode(int newDisplayMode);
	void ext_changeDisplayParam(int newDisplayParam);
	void ext_changeDisplayEffect(int newDisplayEffect);
};