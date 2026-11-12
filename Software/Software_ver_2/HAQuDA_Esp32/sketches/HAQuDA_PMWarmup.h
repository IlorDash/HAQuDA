#pragma once

#include <cstdint>

// For 10 s after power-on the ZH03B PM2.5 field holds its firmware version number, not a reading (datasheet p.5 note 1).
// 12 s = 10 s + one 1 s frame period (a frame sent just before the end of the window can still be waiting in the buffer
// at the first read after it) + 1 s for the module's boot, since its own 10 s start after SENS_POW goes HIGH.
constexpr uint32_t PM_WARMUP_MS = 12 * 1000;

// Tracks the ZH03B warm-up window after the latest sensor power-on. The caller passes the current millis() value.
// Not thread-safe: used from the loop task only.
class HAQuDA_PMWarmup {
  public:
	// Starts a new warm-up window; call at every sensor power-on (first start, retries, reboots). loop task only.
	void PowerOn(uint32_t nowMs);

	// True inside the warm-up window after the latest PowerOn(). Once the window has passed the state is cleared for good:
	// the unsigned difference becomes small again every 2^32 ms (49.7 days) and would reopen the window. loop task only.
	bool IsWarmingUp(uint32_t nowMs);

  private:
	uint32_t powerOnMs = 0;
	bool powered = false;
};
