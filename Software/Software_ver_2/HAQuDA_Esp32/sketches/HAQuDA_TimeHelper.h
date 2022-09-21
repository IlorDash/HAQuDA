#pragma once
#include "Arduino.h"
#include <WiFiUdp.h>
#include <NTPClient.h>

#define DATE_TIME_FIELDS_NUM 6

typedef struct {
	uint16_t year;
	uint8_t month;
	uint8_t date;

	uint8_t hour; // 24-h time format
	uint8_t min;
	uint8_t sec;
} DateTimeStruct;

class HAQuDA_TimeHelper {
	// Define NTP Client to get time
	WiFiUDP ntpUDP;
	NTPClient *timeClient;

	bool timeClientStarted = false;

  public:
	HAQuDA_TimeHelper();

	void StartNTP();
	bool GetTimeClientStarted();
	bool GetDateTime(DateTimeStruct *_time);
	bool GetDateTime(char* _time);

	uint32_t GetDuration(uint32_t watchedTime);
	bool PeriodInRange(uint32_t watchedTime, uint32_t minTime, uint32_t maxTime);
	bool PeriodInRange(uint32_t watchedTime, uint32_t minTime);
	~HAQuDA_TimeHelper();

  private:
};