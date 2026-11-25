#pragma once

#include <cstdint>

constexpr uint8_t TOUCH_BUTTONS_NUM = 4;

constexpr uint32_t TOUCH_TAP_MAX_MS = 600;
constexpr uint32_t TOUCH_REPEAT_START_MS = 600;
constexpr uint32_t TOUCH_REPEAT_PERIOD_MS = 200;
constexpr uint32_t TOUCH_LOCK_HOLD_MS = 3000;
constexpr uint32_t TOUCH_AP_HOLD_MS = 10000;
constexpr uint8_t TOUCH_NO_BUTTON = 0xFF;

enum class touch_gesture_kind : uint8_t { tap, repeat, repeat_end, lock_chord, ap_chord };

struct touch_gesture_event {
	uint8_t button;
	touch_gesture_kind kind;
};

// One Update call emits at most one event (a mask change finishes one gesture, a tick
// emits one repeat or one chord, never both); callers size their array from this.
constexpr uint8_t TOUCH_GESTURE_MAX_EVENTS = 1;

// Buttons 0..3 are pads 2, 6, 8, 9; lock chord is buttons 1 + 2, AP chord is buttons 0 + 2.
// Update and SetRepeatable run in the same task (the esp_timer task); no locking inside.
class HAQuDA_TouchGestures {
  public:
	void SetRepeatable(uint8_t index, bool repeatable);

	uint8_t Update(const bool levels[TOUCH_BUTTONS_NUM], uint32_t nowMs, touch_gesture_event *events, uint8_t capacity);

  private:
	bool finishSolo(uint32_t nowMs, touch_gesture_kind &kind) const;

	static constexpr uint8_t LOCK_MASK = (1 << 1) | (1 << 2);
	static constexpr uint8_t AP_MASK = (1 << 0) | (1 << 2);

	bool started = false;
	// An input already touched at power-up is ignored until it is released once.
	uint8_t ignoredMask = 0;
	uint8_t repeatableMask = 0;
	uint8_t prevMask = 0;
	// Two or more buttons were touched since the last full release: no tap or repeat until then.
	bool suppressed = false;
	bool soloActive = false;
	uint8_t soloButton = 0;
	uint32_t soloSinceMs = 0;
	bool repeating = false;
	uint32_t nextRepeatAfterMs = 0;
	uint32_t setSinceMs = 0;
	bool chordFired = false;
};
