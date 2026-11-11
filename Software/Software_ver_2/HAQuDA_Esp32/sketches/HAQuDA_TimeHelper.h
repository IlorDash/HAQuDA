#pragma once
#include "Arduino.h"
#include <WiFiUdp.h>
#include <NTPClient.h>

#include "HAQuDA_DateFormat.h"

#define GMT_PLUS_3_TZ_SEC 10800
#define DATE_TIME_STR_LEN 20

typedef struct {
	uint8_t date;
	uint8_t month;
	uint16_t year;

	uint8_t hour;
	uint8_t min;
	uint8_t sec;
} DateTimeStruct;

class HAQuDA_TimeService {
	static WiFiUDP *ntpUDP;
	static NTPClient *timeClient;
	static HAQuDA_TimeService *p_instance;

	HAQuDA_TimeService() {};
	HAQuDA_TimeService(const HAQuDA_TimeService *);
	HAQuDA_TimeService *operator=(HAQuDA_TimeService *);
	~HAQuDA_TimeService() {};

	static bool timeClientStarted;

	static bool GetFormattedDate(char *_formattedDate);

  public:
	static HAQuDA_TimeService *getInstance();

	static void StartNTP();

	static bool GetTimeClientStarted();
	static bool GetDateTime(DateTimeStruct *_time);
	// Writes "DD.MM.YYYY HH:MM:SS" into _time, which must hold at least DATE_TIME_STR_LEN bytes (timeSize). Returns false when NTP is not started,
	// no time could be read or the buffer is too small; _time is then not a valid stamp. Can block in forceUpdate(). Loop task.
	static bool GetDateTime(char *_time, size_t timeSize);

	static uint32_t GetDuration(uint32_t watchedTime);
	static bool PeriodInRange(uint32_t watchedTime, uint32_t minTime, uint32_t maxTime);
	static bool PeriodInRange(uint32_t watchedTime, uint32_t minTime);
};