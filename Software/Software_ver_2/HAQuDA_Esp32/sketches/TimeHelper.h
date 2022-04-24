#ifndef __TIME_HELPER_H
#define __TIME_HELPER_H

uint32_t GetDuration(uint32_t watchedTime);
bool PeriodInRange(uint32_t watchedTime, uint32_t minTime, uint32_t maxTime);
bool PeriodInRange(uint32_t watchedTime, uint32_t minTime);

#endif