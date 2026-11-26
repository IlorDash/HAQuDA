#pragma once

#include <cstdint>

#include "HAQuDA_TouchDebounce.h"
#include "HAQuDA_TouchGestures.h"

// esp_timer task only, single caller: SetRepeatable and Poll share the gestures state without a lock.
class HAQuDA_TouchPoller {
  public:
	void SetRepeatable(uint8_t index, bool repeatable) {
		gestures.SetRepeatable(index, repeatable);
	}

	// Returns the number of events written to `events`
	uint8_t Poll(const bool rawLevels[TOUCH_BUTTONS_NUM], uint32_t nowMs, touch_gesture_event *events, uint8_t capacity) {
		bool levels[TOUCH_BUTTONS_NUM];
		for (uint8_t i = 0; i < TOUCH_BUTTONS_NUM; i++) {
			debouncers[i].Update(rawLevels[i], nowMs);
			levels[i] = debouncers[i].Active();
		}
		if (!started) {
			started = true;
			startMs = nowMs;
		}
		// The gestures take the levels present at their first call as "already touched at power-up"; the debounced levels are
		// valid only once every input has been stable for the debounce time
		if (!ready) {
			if (nowMs - startMs < TOUCH_DEBOUNCE_MS) {
				return 0;
			}
			ready = true;
		}
		return gestures.Update(levels, nowMs, events, capacity);
	}

  private:
	HAQuDA_TouchDebounce debouncers[TOUCH_BUTTONS_NUM]
		= {HAQuDA_TouchDebounce(true), HAQuDA_TouchDebounce(true), HAQuDA_TouchDebounce(true), HAQuDA_TouchDebounce(true)};
	HAQuDA_TouchGestures gestures;
	bool started = false;
	bool ready = false;
	uint32_t startMs = 0;
};
