#include "Board.h"
#include "TimeHelper.h"

uint32_t IRAM_ATTR GetDuration(uint32_t watchedTime) {
	uint32_t duration;
	if (millis() > watchedTime) {
		duration = millis() - watchedTime;
	} else {
		duration = (4294967295U - (watchedTime - millis())) + 1;
	}
	return duration;
}

bool IRAM_ATTR PeriodInRange(uint32_t watchedTime, uint32_t minTime, uint32_t maxTime) {
	uint32_t duration = GetDuration(watchedTime);
	return duration >= minTime && duration <= maxTime;
}

bool IRAM_ATTR PeriodInRange(uint32_t watchedTime, uint32_t minTime) {
	uint32_t duration = GetDuration(watchedTime);
	return duration >= minTime;
}