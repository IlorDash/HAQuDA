#include "HAQuDA_PMWarmup.h"

void HAQuDA_PMWarmup::PowerOn(uint32_t nowMs) {
	powerOnMs = nowMs;
	powered = true;
}

bool HAQuDA_PMWarmup::IsWarmingUp(uint32_t nowMs) {
	if (!powered) {
		return false;
	}
	if ((uint32_t)(nowMs - powerOnMs) < PM_WARMUP_MS) {
		return true;
	}
	powered = false;
	return false;
}
