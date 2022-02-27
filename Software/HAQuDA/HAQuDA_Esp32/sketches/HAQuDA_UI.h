#pragma once

#include "HAQuDA_DispManip.h"
#include "HAQuDA_UI_typedefs.h"
#include "WS2812.h"

class HAQuDA_UI {
  private:
	HAQuDA_DispManip *myDispManip;

  public:
	UI_Params currUI_Params;
	HAQuDA_UI(HAQuDA_DispManip *currDispManip);
	void ext_setStaticColor(int red, int green, int blue);
	void ext_setBrightness(uint8_t newBrightnessPerCent);
	void ext_changeDispMode(int newDispMode);
	void ext_changeDispParam(int newDispParam);
	void ext_changeDispEffect(int newDispEffect);
};