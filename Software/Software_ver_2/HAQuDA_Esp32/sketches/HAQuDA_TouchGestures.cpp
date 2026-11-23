#include "HAQuDA_TouchGestures.h"

namespace {
	uint8_t countBits(uint8_t mask) {
		uint8_t n = 0;
		for (; mask != 0; mask >>= 1) {
			n += mask & 1;
		}
		return n;
	}

	uint8_t bitIndex(uint8_t mask) {
		uint8_t i = 0;
		while (((mask >> i) & 1) == 0) {
			i++;
		}
		return i;
	}
} // namespace

void HAQuDA_TouchGestures::SetRepeatable(uint8_t index, bool repeatable) {
	if (index >= TOUCH_BUTTONS_NUM) {
		return;
	}
	if (repeatable) {
		repeatableMask |= (1 << index);
	} else {
		repeatableMask &= ~(1 << index);
	}
}

bool HAQuDA_TouchGestures::finishSolo(uint32_t nowMs, touch_gesture_kind &kind) const {
	if (!soloActive) {
		return false;
	}
	if (repeating) {
		kind = touch_gesture_kind::repeat_end;
		return true;
	}
	if (nowMs - soloSinceMs < TOUCH_TAP_MAX_MS) {
		kind = touch_gesture_kind::tap;
		return true;
	}
	return false;
}

uint8_t HAQuDA_TouchGestures::Update(const bool levels[TOUCH_BUTTONS_NUM], uint32_t nowMs, touch_gesture_event *events, uint8_t capacity) {
	uint8_t count = 0;
	auto emit = [&](uint8_t button, touch_gesture_kind kind) {
		if (count < capacity) {
			events[count].button = button;
			events[count].kind = kind;
			count++;
		}
	};

	uint8_t rawMask = 0;
	for (uint8_t i = 0; i < TOUCH_BUTTONS_NUM; i++) {
		if (levels[i]) {
			rawMask |= (1 << i);
		}
	}
	if (!started) {
		started = true;
		ignoredMask = rawMask;
	}
	ignoredMask &= rawMask;
	const uint8_t mask = rawMask & ~ignoredMask;

	if (mask != prevMask) {
		setSinceMs = nowMs;
		chordFired = false;

		if (mask == 0) {
			touch_gesture_kind kind = touch_gesture_kind::tap;
			if (finishSolo(nowMs, kind)) {
				emit(soloButton, kind);
			}
			soloActive = false;
			repeating = false;
			suppressed = false;
		} else if (countBits(mask) >= 2) {
			if (repeating) {
				emit(soloButton, touch_gesture_kind::repeat_end);
			}
			soloActive = false;
			repeating = false;
			suppressed = true;
		} else if (!suppressed) {
			touch_gesture_kind kind = touch_gesture_kind::tap;
			if (finishSolo(nowMs, kind)) {
				emit(soloButton, kind);
			}
			soloActive = true;
			soloButton = bitIndex(mask);
			soloSinceMs = nowMs;
			repeating = false;
			nextRepeatAfterMs = TOUCH_REPEAT_START_MS;
		}
		prevMask = mask;
		return count;
	}

	if (soloActive && ((repeatableMask >> soloButton) & 1) != 0 && nowMs - soloSinceMs >= nextRepeatAfterMs) {
		emit(soloButton, touch_gesture_kind::repeat);
		repeating = true;
		const uint32_t heldSoloMs = nowMs - soloSinceMs;
		nextRepeatAfterMs = TOUCH_REPEAT_START_MS + ((heldSoloMs - TOUCH_REPEAT_START_MS) / TOUCH_REPEAT_PERIOD_MS + 1) * TOUCH_REPEAT_PERIOD_MS;
	}

	if (!chordFired) {
		const uint32_t heldMs = nowMs - setSinceMs;
		if (mask == LOCK_MASK && heldMs >= TOUCH_LOCK_HOLD_MS) {
			emit(TOUCH_NO_BUTTON, touch_gesture_kind::lock_chord);
			chordFired = true;
		} else if (mask == AP_MASK && heldMs >= TOUCH_AP_HOLD_MS) {
			emit(TOUCH_NO_BUTTON, touch_gesture_kind::ap_chord);
			chordFired = true;
		}
	}
	return count;
}
