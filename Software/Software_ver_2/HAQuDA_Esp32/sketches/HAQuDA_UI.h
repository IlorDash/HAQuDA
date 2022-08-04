#pragma once

#include "HAQuDA_DispManip.h"
#include "HAQuDA_UI_typedefs.h"
#include "HAQuDA_ErrorHandler.h"
#include "WS2812.h"

#define COLOR_AQUA 0x00FFFF
#define COLOR_LIME 0x99FF33
#define COLOR_GREEN 0x01FF00
#define COLOR_RED 0xFF0000
#define COLOR_YELLOW 0xFFFF00
#define COLOR_WHITE 0xFFFFFF

class HAQuDA_UI {
  private:
	HAQuDA_DispManip myDispManip;

  public:

	static errorTypes_enum dispError;

	static dispEffect_enum dispEffect;
	effectsParams_struct effectParams;

	UI_Disp_Params currUI_Disp_Params;

	uint8_t brightnessPerCent;

	HAQuDA_UI(HAQuDA_DispManip &currDispManip);
	void ext_setStaticColor(int red, int green, int blue);
	void ext_setBrightness(uint8_t newBrightnessPerCent);
	void ext_changeDispMode(int newDispMode);
	void ext_changeDispParam(int newDispParam);
	void ext_changeDispEffect(int newDispEffect);
};