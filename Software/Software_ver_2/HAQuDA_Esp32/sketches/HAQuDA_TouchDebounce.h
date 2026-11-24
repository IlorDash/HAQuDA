#pragma once

#include <cstdint>

constexpr uint32_t TOUCH_DEBOUNCE_MS = 30;

// One instance per button. Single caller only (the esp_timer task in the firmware), no locking inside.
class HAQuDA_TouchDebounce {
  public:
	explicit HAQuDA_TouchDebounce(bool activeLevel);

	// Returns true once per press, after the input has been stably active for TOUCH_DEBOUNCE_MS.
	bool Update(bool rawLevel, uint32_t nowMs);

	bool Active() const;

  private:
	bool activeLevel;
	bool started = false;
	bool candidateActive = false;
	bool stableActive = false;
	// An input already active at power-up (latched or shorted line) must not fire a press at boot,
	// so a press is reported only after a stable release has been seen.
	bool armed = false;
	uint32_t candidateSinceMs = 0;
};
