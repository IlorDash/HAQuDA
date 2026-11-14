#ifndef _FILESTORAGE_h
#define _FILESTORAGE_h

#include <functional>
#include <stdio.h>
#include <Arduino.h>
#include "Board.h"
#include "FS.h"
#include "SPIFFS.h"
#include "HAQuDA_WiFiCreds.h"

#define YEAR_DAY 365.24
#define DAY_HOUR 24
#define HOUR_SEC 3600
#define ONEYEAR (YEAR_DAY * DAY_HOUR * HOUR_SEC)

#define FILE_WRITE "w"
#define FILE_APPEND "a"
#define FILE_READ "r"

#define FILE_NAME_MEAS_LOG "/measLog.data"
#define FILE_CHECK_COUNT "/checkCount"
#define FILE_NAME_BOUNDS "/bounds"
#define FILE_NAME_WIFI_NET "/WiFiNetworks"
#define FILE_NAME_LOGS "/logs"
#define FILE_NAME_LOGS_OLD "/logs.old" // SPIFFS names are limited to 31 chars with the path

const int checkPeriod = 10;
const int checksPerAccumulation = 4;
const int partTime = 21600;

typedef struct {
	uint32_t timeStamp = 0;
	uint16_t measNum = 0;

	float eCO2_log;
	float TVOC_log;
	float PM_1_0_log;
	float PM_2_5_log;
	float PM_10_log;
	float temp_log;
	float humid_log;
	float O3_log;
} TMeasLog;

// Every function that touches SPIFFS holds the flash lock (HAQuDA_FlashLock.h) for its duration: a flash access stops the
// cache on both cores, which would corrupt an LED frame being sent. Callers: loop (logger, WiFi credentials), async_tcp
// (web handlers), setup. Not for ShowTask or ISRs; the functions may wait up to 100 ms for a running LED frame.
class HAQuDA_FileStorage {
  public:
	HAQuDA_FileStorage();
	~HAQuDA_FileStorage();

	bool Start(void);
	void Stop(void);
	void ListDir(const char *dirname, const uint8_t levels);
	static bool DeleteFile(const char *path);
	bool Format();
	// Renames a file in place, the target must not exist. loop task.
	static bool RenameFile(const char *from, const char *to);
	static bool WriteFile(const char *path, const uint8_t *data, const size_t len);
	static bool AppendFile(const char *path, const uint8_t *data, const size_t len);
	static bool Exists(const String &path);
	static bool Exists(const char *path);
	static int FileSize(const char *path);

	static void ReadFileInSerial(const char *path);
	static bool ReadFile(const char *path, uint8_t *data, const size_t len);
	static bool ReadFile(const char *path, const int fromPos, uint8_t *data, const size_t len);

  private:
	// Recursion happens here, under the single lock taken by ListDir().
	static void listDirUnlocked(const char *dirname, const uint8_t levels);
};

#endif
