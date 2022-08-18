#pragma once

#include "HAQuDA_DisplayManip.h"
#include "HAQuDA_DisplayInterface_typedefs.h"
#include "HAQuDA_ErrorHandler.h"
#include "WS2812.h"

#define COLOR_AQUA 0x00FFFF
#define COLOR_LIME 0x99FF33
#define COLOR_GREEN 0x01FF00
#define COLOR_RED 0xFF0000
#define COLOR_YELLOW 0xFFFF00
#define COLOR_WHITE 0xFFFFFF

class HAQuDA_DisplayInterface {
  public:
	static whatDisplay_enum whatDisplay;

	static errorTypes_enum displayError;

	static displayEffect_enum displayEffect;
	snakeEffectsParams_struct snakeEffectParams;
	fadeEffectsParams_struct fadeEffectParams;

	uint16_t randomSpeed; // in ms - delay between LED ignition
 
	displayMeasParams_struct displayMeasParams;

	uint8_t brightnessPerCent;

	HAQuDA_DisplayInterface();
	void ext_setStaticColor(int red, int green, int blue);
	void ext_setBrightness(uint8_t newBrightnessPerCent);
	void ext_changeDisplayMode(int newDisplayMode);
	void ext_changeDisplayParam(int newDisplayParam);
	void ext_changeDisplayEffect(int newDisplayEffect);
};