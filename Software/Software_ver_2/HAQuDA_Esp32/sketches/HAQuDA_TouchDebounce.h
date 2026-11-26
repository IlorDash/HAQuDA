#pragma once

#include <cstdint>

constexpr uint32_t TOUCH_DEBOUNCE_MS = 30;

// One instance per button. Single caller only (the esp_timer task in the firmware), no locking inside.
class HAQuDA_TouchDebounce {
  public:
	explicit HAQuDA_TouchDebounce(bool activeLevel);

	// Active() follows the input once it has been stable for TOUCH_DEBOUNCE_MS
	void Update(bool rawLevel, uint32_t nowMs);

	bool Active() const;

  private:
	bool activeLevel;
	bool started = false;
	bool candidateActive = false;
	bool stableActive = false;
	uint32_t candidateSinceMs = 0;
};
