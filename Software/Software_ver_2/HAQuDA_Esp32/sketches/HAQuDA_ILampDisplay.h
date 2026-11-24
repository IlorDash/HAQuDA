#pragma once

#include <cstdint>

#include "HAQuDA_LampSettings.h"

// Loop task only: the display modes behind it are driven from there
class HAQuDA_ILampDisplay {
  public:
	virtual ~HAQuDA_ILampDisplay() = default;

	virtual bool ErrorShown() = 0;
	virtual void ClearError() = 0;
	virtual void ShowSensorsError() = 0;

	virtual void SetBrightnessPercent(uint8_t brightPercent) = 0;
	virtual void ShowColor(uint32_t rgb) = 0;
	virtual void ShowStandard(lamp_param param) = 0;
	virtual void ShowMulti(const lamp_param (&bottomToTop)[LAMP_MULTI_NUM]) = 0;
	virtual void ShowNight(lamp_param param, uint16_t fromMin, uint16_t toMin) = 0;
	virtual void ShowFlash(lamp_flash kind) = 0;
};
