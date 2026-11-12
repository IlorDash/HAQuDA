#pragma once
#include "Arduino.h"
#include <WiFiUdp.h>
#include <NTPClient.h>

#include "HAQuDA_DateFormat.h"

#define GMT_PLUS_3_TZ_SEC 10800
#define DATE_TIME_STR_LEN 20

// Length of the NTP client's "YYYY-MM-DDTHH:MM:SSZ" (20, the client appends the Z); buffers hold FORMATTED_DATE_LEN + 1 for the terminator.
constexpr size_t FORMATTED_DATE_LEN = 20;

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

	// Copies the client's "YYYY-MM-DDTHH:MM:SSZ" into formattedDate (size >= FORMATTED_DATE_LEN + 1, always terminated). Returns false before
	// StartNTP() and while NTP has never synced; a stale time after a lost server still counts as valid. Blocks up to ≈ 1 s in forceUpdate()
	// at most once per NTP_RETRY_PERIOD_MS in every state - the attempt stamp is shared, so the loop task or ShowTask, whichever calls
	// first, pays it. Loop task and ShowTask.
	static bool GetFormattedDate(char *formattedDate, size_t size);

  public:
	static HAQuDA_TimeService *getInstance();

	static void StartNTP();

	static bool GetTimeClientStarted();
	// Fills _time from the NTP client. Returns false before StartNTP() and while NTP has never synced (the time is unknown, not the epoch).
	// Blocks up to ≈ 1 s at most once per NTP_RETRY_PERIOD_MS (60 s) in every state, whichever task calls first. Loop task (log stamps) and
	// ShowTask (night mode).
	static bool GetDateTime(DateTimeStruct *_time);
	// Writes "DD.MM.YYYY HH:MM:SS" into _time, which must hold at least DATE_TIME_STR_LEN bytes (timeSize). Returns false when NTP is not started,
	// has never synced or the buffer is too small; _time is then not a valid stamp. Blocks like GetDateTime(DateTimeStruct *). Loop task.
	static bool GetDateTime(char *_time, size_t timeSize);

	static uint32_t GetDuration(uint32_t watchedTime);
	static bool PeriodInRange(uint32_t watchedTime, uint32_t minTime, uint32_t maxTime);
	static bool PeriodInRange(uint32_t watchedTime, uint32_t minTime);
};