#include "HAQuDA_TimeHelper.h"

#define DATE_TIME_FIELDS_NUM 6
#define FORMATTED_DATE_LEN 20

HAQuDA_TimeHelper_Singleton *HAQuDA_TimeHelper_Singleton::p_instance = 0;
HAQuDA_TimeHelper_SingletonDestroyer HAQuDA_TimeHelper_Singleton::destroyer;

WiFiUDP *HAQuDA_TimeHelper_Singleton::ntpUDP = 0;
NTPClient *HAQuDA_TimeHelper_Singleton::timeClient = 0;
bool HAQuDA_TimeHelper_Singleton::timeClientStarted = false;

HAQuDA_TimeHelper_SingletonDestroyer::~HAQuDA_TimeHelper_SingletonDestroyer() {
	delete p_instance;
}

void HAQuDA_TimeHelper_SingletonDestroyer::initialize(HAQuDA_TimeHelper_Singleton *p) {
	p_instance = p;
}

HAQuDA_TimeHelper_Singleton *HAQuDA_TimeHelper_Singleton::getInstance() {
	if (!p_instance) {
		p_instance = new HAQuDA_TimeHelper_Singleton();
		destroyer.initialize(p_instance);
	}
	return p_instance;
}

void HAQuDA_TimeHelper_Singleton::StartNTP() {
	ntpUDP = new WiFiUDP();
	timeClient = new NTPClient(*ntpUDP);
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

bool HAQuDA_TimeHelper_Singleton::GetTimeClientStarted() {
	return timeClientStarted;
}

bool HAQuDA_TimeHelper_Singleton::GetFormattedDate(char *_formattedDate) {
	DateTimeStruct currTime;

	if (!timeClientStarted) {
		return false;
	}
	while (!timeClient->update()) {
		timeClient->forceUpdate();
	}
	// The formattedDate comes with the following format:
	// 2018-05-28T16:00:13
	String formattedDate = timeClient->getFormattedDate();

	strncpy(_formattedDate, formattedDate.c_str(), formattedDate.length());
	return true;
}

bool HAQuDA_TimeHelper_Singleton::GetDateTime(DateTimeStruct *_time) {

	char formattedDate[FORMATTED_DATE_LEN + 1] = {0}; // plus null term

	if (!HAQuDA_TimeHelper_Singleton::GetFormattedDate(formattedDate)) {
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

bool HAQuDA_TimeHelper_Singleton::GetDateTime(char *_time) {
	DateTimeStruct currTime;

	if (!HAQuDA_TimeHelper_Singleton::GetDateTime(&currTime)) {
		return false;
	}

	char buffer[DATE_TIME_STR_LEN + 1] = {0}; // plus null term
	uint8_t ret = snprintf(buffer, DATE_TIME_STR_LEN, "%2d.%2d.%4d %2d:%2d:%2d", currTime.date, currTime.month, currTime.year, currTime.hour, currTime.min,
						   currTime.sec);
	if (ret != DATE_TIME_STR_LEN) {
		return false;
	}
	strncpy(_time, buffer, sizeof(buffer));
	return true;
}

// bool HAQuDA_TimeHelper_Singleton::GetDateTime(char *_time) {
//	DateTimeStruct currTime;
//	*_time = {0};
//
//	if (!timeClientStarted) {
//		return false;
//	}
//	while (!timeClient->update()) {
//		timeClient->forceUpdate();
//	}
//	// The formattedDate comes with the following format:
//	// 2018-05-28T16:00:13
//	String formattedDate = timeClient->getFormattedDate();
//
//	const char *foo = formattedDate.c_str();
//	size_t bar = formattedDate.length();
//
//	strncpy(_time, foo, bar);
//	return true;
//}

uint32_t IRAM_ATTR HAQuDA_TimeHelper_Singleton::GetDuration(uint32_t watchedTime) {
	uint32_t duration;
	if (millis() > watchedTime) {
		duration = millis() - watchedTime;
	} else {
		duration = (4294967295U - (watchedTime - millis())) + 1;
	}
	return duration;
}

bool IRAM_ATTR HAQuDA_TimeHelper_Singleton::PeriodInRange(uint32_t watchedTime, uint32_t minTime, uint32_t maxTime) {
	uint32_t duration = GetDuration(watchedTime);
	return duration >= minTime && duration <= maxTime;
}

bool IRAM_ATTR HAQuDA_TimeHelper_Singleton::PeriodInRange(uint32_t watchedTime, uint32_t minTime) {
	uint32_t duration = GetDuration(watchedTime);
	return duration >= minTime;
}