#include "HAQuDA_TimeHelper.h"

HAQuDA_TimeHelper::HAQuDA_TimeHelper() {
}

void HAQuDA_TimeHelper::StartNTP() {
	timeClient = new NTPClient(ntpUDP);
	// Initialize a NTPClient to get time
	timeClient->begin();
	// Set offset time in seconds to adjust for your timezone, for example:
	// GMT +1 = 3600
	// GMT +8 = 28800
	// GMT +3 = 10800
	// GMT 0 = 0
	timeClient->setTimeOffset(10800);

	timeClientStarted = true;
}

bool HAQuDA_TimeHelper::GetTimeClientStarted() {
	return timeClientStarted;
}

bool HAQuDA_TimeHelper::GetDateTime(DateTimeStruct *_time) {
	DateTimeStruct currTime;
	*_time = {0};
	if (!timeClientStarted) {
		return false;
	}
	while (!timeClient->update()) {
		timeClient->forceUpdate();
	}
	// The formattedDate comes with the following format:
	// 2018-05-28T16:00:13
	String formattedDate = timeClient->getFormattedDate();

	uint8_t ret = sscanf(formattedDate.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d", &currTime.year, &currTime.month, &currTime.date, &currTime.hour, &currTime.min,
						 &currTime.sec);

	if (ret != DATE_TIME_FIELDS_NUM) {
		return false;
	}
	_time = &currTime;
	return true;
}

bool HAQuDA_TimeHelper::GetDateTime(char *_time) {
	DateTimeStruct currTime;
	*_time = {0};

	if (!timeClientStarted) {
		return false;
	}
	while (!timeClient->update()) {
		timeClient->forceUpdate();
	}
	// The formattedDate comes with the following format:
	// 2018-05-28T16:00:13
	String formattedDate = timeClient->getFormattedDate();

	strncpy(_time, formattedDate.c_str(), formattedDate.length());
	return true;
}

uint32_t IRAM_ATTR HAQuDA_TimeHelper::GetDuration(uint32_t watchedTime) {
	uint32_t duration;
	if (millis() > watchedTime) {
		duration = millis() - watchedTime;
	} else {
		duration = (4294967295U - (watchedTime - millis())) + 1;
	}
	return duration;
}

bool IRAM_ATTR HAQuDA_TimeHelper::PeriodInRange(uint32_t watchedTime, uint32_t minTime, uint32_t maxTime) {
	uint32_t duration = GetDuration(watchedTime);
	return duration >= minTime && duration <= maxTime;
}

bool IRAM_ATTR HAQuDA_TimeHelper::PeriodInRange(uint32_t watchedTime, uint32_t minTime) {
	uint32_t duration = GetDuration(watchedTime);
	return duration >= minTime;
}

HAQuDA_TimeHelper::~HAQuDA_TimeHelper() {
}