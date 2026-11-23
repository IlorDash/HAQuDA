#pragma once

#include <cstdint>

#include "HAQuDA_PressCounter.h"
#include "HAQuDA_TouchDebounce.h"

constexpr uint8_t TOUCH_BUTTONS_NUM = 4;

class HAQuDA_TouchPoller {
  public:
	// esp_timer task only, single caller
	void Poll(const bool levels[TOUCH_BUTTONS_NUM], uint32_t nowMs) {
		for (uint8_t i = 0; i < TOUCH_BUTTONS_NUM; i++) {
			if (debouncers[i].Update(levels[i], nowMs)) {
				presses[i].Add();
			}
		}
	}

	// Loop task only
	uint8_t Take(uint8_t index) {
		if (index >= TOUCH_BUTTONS_NUM) {
			return 0;
		}
		return presses[index].Take();
	}

  private:
	HAQuDA_TouchDebounce debouncers[TOUCH_BUTTONS_NUM]
		= {HAQuDA_TouchDebounce(true), HAQuDA_TouchDebounce(true), HAQuDA_TouchDebounce(true), HAQuDA_TouchDebounce(true)};
	HAQuDA_PressCounter presses[TOUCH_BUTTONS_NUM];
};
