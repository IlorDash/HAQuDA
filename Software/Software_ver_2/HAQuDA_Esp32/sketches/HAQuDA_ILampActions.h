#pragma once

#include <cstdint>

#include "HAQuDA_LampSettings.h"

constexpr uint32_t LAMP_SHOW_READING_MS = 10 * 1000;
constexpr uint32_t LAMP_FLASH_MS = 500;

// What the touch buttons may do to the lamp. Loop task only: the display modes behind it are driven from there, and the settings are
// shared with the web page's writer (async_tcp task) under the lock of the implementation.
class HAQuDA_ILampActions {
  public:
	virtual ~HAQuDA_ILampActions() = default;

	virtual void TogglePower() = 0;

	// Saves nothing, so a held button changes the brightness smoothly; CommitBrightness() saves the result
	virtual bool StepBrightness(bool up) = 0;
	virtual void CommitBrightness() = 0;

	virtual void NextMode() = 0;
	virtual bool SetMode(lamp_mode mode) = 0;

	// Leaves the settings and the flash memory untouched
	virtual bool ShowReading(lamp_param param) = 0;

	virtual void Flash(lamp_flash kind) = 0;
};
