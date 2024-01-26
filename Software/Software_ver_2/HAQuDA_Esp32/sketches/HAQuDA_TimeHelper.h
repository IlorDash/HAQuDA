#pragma once
#include "Arduino.h"
#include <WiFiUdp.h>
#include <NTPClient.h>

#define DATE_TIME_STR_LEN 19 // without null term

typedef struct {
	uint8_t date;
	uint8_t month;
	uint16_t year;

	uint8_t hour; // 24-h time format
	uint8_t min;
	uint8_t sec;
} DateTimeStruct;

class HAQuDA_TimeHelper_Singleton;

class HAQuDA_TimeHelper_SingletonDestroyer {
  private:
	HAQuDA_TimeHelper_Singleton *p_instance;

  public:
	~HAQuDA_TimeHelper_SingletonDestroyer();
	void initialize(HAQuDA_TimeHelper_Singleton *p);
};

class HAQuDA_TimeHelper_Singleton {
	// Define NTP Client to get time
	static WiFiUDP *ntpUDP;
	static NTPClient *timeClient;

	static bool timeClientStarted;

	static HAQuDA_TimeHelper_Singleton *p_instance;
	static HAQuDA_TimeHelper_SingletonDestroyer destroyer;

	static bool GetFormattedDate(char *_formattedDate);

  protected:
	HAQuDA_TimeHelper_Singleton(){};
	HAQuDA_TimeHelper_Singleton(const HAQuDA_TimeHelper_Singleton *);
	HAQuDA_TimeHelper_Singleton *operator=(HAQuDA_TimeHelper_Singleton *);
	~HAQuDA_TimeHelper_Singleton(){};
	friend class HAQuDA_TimeHelper_SingletonDestroyer;

  public:
	static HAQuDA_TimeHelper_Singleton *getInstance();

	static void StartNTP();

	static bool GetTimeClientStarted();
	static bool GetDateTime(DateTimeStruct *_time);
	static bool GetDateTime(char *_time);

	static uint32_t GetDuration(uint32_t watchedTime);
	static bool PeriodInRange(uint32_t watchedTime, uint32_t minTime, uint32_t maxTime);
	static bool PeriodInRange(uint32_t watchedTime, uint32_t minTime);
};