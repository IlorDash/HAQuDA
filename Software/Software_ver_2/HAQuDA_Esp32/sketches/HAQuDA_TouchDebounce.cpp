#include "HAQuDA_TouchDebounce.h"

HAQuDA_TouchDebounce::HAQuDA_TouchDebounce(bool activeLevel) : activeLevel(activeLevel) {
}

bool HAQuDA_TouchDebounce::Active() const {
	return stableActive;
}

void HAQuDA_TouchDebounce::Update(bool rawLevel, uint32_t nowMs) {
	const bool active = (rawLevel == activeLevel);

	if (!started) {
		started = true;
		candidateActive = active;
		candidateSinceMs = nowMs;
		return;
	}

	if (active != candidateActive) {
		candidateActive = active;
		candidateSinceMs = nowMs;
		return;
	}

	if ((candidateActive != stableActive) && ((nowMs - candidateSinceMs) >= TOUCH_DEBOUNCE_MS)) {
		stableActive = candidateActive;
	}
}
