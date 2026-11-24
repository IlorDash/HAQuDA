#include "HAQuDA_TouchDebounce.h"

HAQuDA_TouchDebounce::HAQuDA_TouchDebounce(bool activeLevel) : activeLevel(activeLevel) {
}

bool HAQuDA_TouchDebounce::Active() const {
	return stableActive;
}

bool HAQuDA_TouchDebounce::Update(bool rawLevel, uint32_t nowMs) {
	const bool active = (rawLevel == activeLevel);

	if (!started) {
		started = true;
		candidateActive = active;
		candidateSinceMs = nowMs;
		armed = !active;
		return false;
	}

	if (active != candidateActive) {
		candidateActive = active;
		candidateSinceMs = nowMs;
		return false;
	}

	if ((candidateActive == stableActive) || ((nowMs - candidateSinceMs) < TOUCH_DEBOUNCE_MS)) {
		return false;
	}

	stableActive = candidateActive;
	if (!stableActive) {
		armed = true;
		return false;
	}
	return armed;
}
