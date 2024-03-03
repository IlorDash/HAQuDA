#include "HAQuDA_TimeHelper.h"

#define DATE_TIME_FIELDS_NUM 6
#define FORMATTED_DATE_LEN 20

HAQuDA_TimeService *HAQuDA_TimeService::p_instance = 0;

WiFiUDP *HAQuDA_TimeService::ntpUDP = 0;
NTPClient *HAQuDA_TimeService::timeClient = 0;
bool HAQuDA_TimeService::timeClientStarted = false;

HAQuDA_TimeService *HAQuDA_TimeService::getInstance() {
	if (!p_instance) {
		p_instance = new HAQuDA_TimeService();
	}
	return p_instance;
}

void HAQuDA_TimeService::StartNTP() {
	ntpUDP = new WiFiUDP();
	timeClient = new NTPClient(*ntpUDP);
	timeClient->begin();
	timeClient->setTimeOffset(GMT_PLUS_3_TZ_SEC);
	timeClientStarted = true;
}

bool HAQuDA_TimeService::GetTimeClientStarted() {
	return timeClientStarted;
}

bool HAQuDA_TimeService::GetFormattedDate(char *_formattedDate) {
	DateTimeStruct currTime;

	if (!timeClientStarted) {
		return false;
	}

	int time_update_retries = 2;

	while ((!timeClient->update()) && (time_update_retries--)) {
		timeClient->forceUpdate();
	}
	if (time_update_retries == 0) {
		return false;
	}

	// The formattedDate comes with the following format:
	// 2018-05-28T16:00:13
	String formattedDate = timeClient->getFormattedDate();

	strncpy(_formattedDate, formattedDate.c_str(), formattedDate.length());
	return true;
}

bool HAQuDA_TimeService::GetDateTime(DateTimeStruct *_time) {

	char formattedDate[FORMATTED_DATE_LEN + 1] = {0}; // plus null term

	if (!HAQuDA_TimeService::GetFormattedDate(formattedDate)) {
		return false;
	}

	int date = 0;
	int month = 0;
	int year = 0;

	int hour = 0; // 24-h time format
	int min = 0;
	int sec = 0;

	uint8_t ret = sscanf(formattedDate, "%4d-%2d-%2dT%2d:%2d:%2d", &year, &month, &date, &hour, &min, &sec);

	if (ret != DATE_TIME_FIELDS_NUM) {
		return false;
	}
	_time->date = date;
	_time->month = month;
	_time->year = year;

	_time->hour = hour;
	_time->min = min;
	_time->sec = sec;
	return true;
}

bool HAQuDA_TimeService::GetDateTime(char *_time) {
	DateTimeStruct currTime;

	if (!HAQuDA_TimeService::GetDateTime(&currTime)) {
		return false;
	}

	char buffer[DATE_TIME_STR_LEN] = {0}; // plus null term
	uint8_t ret = snprintf(buffer, DATE_TIME_STR_LEN, "%02d.%02d.%4d %02d:%02d:%02d", currTime.date, currTime.month, currTime.year, currTime.hour, currTime.min,
						   currTime.sec);
	if (ret != DATE_TIME_STR_LEN) {
		return false;
	}
	strncpy(_time, buffer, sizeof(buffer));
	return true;
}

uint32_t IRAM_ATTR HAQuDA_TimeService::GetDuration(uint32_t watchedTime) {
	uint32_t duration;
	if (millis() > watchedTime) {
		duration = millis() - watchedTime;
	} else {
		duration = (4294967295U - (watchedTime - millis())) + 1;
	}
	return duration;
}

bool IRAM_ATTR HAQuDA_TimeService::PeriodInRange(uint32_t watchedTime, uint32_t minTime, uint32_t maxTime) {
	uint32_t duration = GetDuration(watchedTime);
	return duration >= minTime && duration <= maxTime;
}

bool IRAM_ATTR HAQuDA_TimeService::PeriodInRange(uint32_t watchedTime, uint32_t minTime) {
	uint32_t duration = GetDuration(watchedTime);
	return duration >= minTime;
}