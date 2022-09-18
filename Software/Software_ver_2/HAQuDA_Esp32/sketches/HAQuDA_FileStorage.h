#ifndef _FILESTORAGE_h
#define _FILESTORAGE_h

#include <functional>
#include <stdio.h>
#include <Arduino.h>
#include "Board.h"
#include "FS.h"
#include "SPIFFS.h"

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

#define WEB_SERVER_IcNDEX_PAGE "/index.html"
#define WEB_SERVER_STYLE "/style.css"
#define WEB_SERVER_WIFI_MANAGER_PAGE "/wifi_manager.html"

#define MAX_WIFI_CREDS_NUM 10
#define WIFI_SSID_LEN 32
#define WIFI_PASS_LEN 32

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

typedef struct {
	char ssid[WIFI_SSID_LEN] = "";
	char pass[WIFI_PASS_LEN] = "";
} __attribute__((__packed__)) TWiFiCreds;

typedef enum {
	error_opening_file,
	too_many_WiFi,
	error_reading_stored_WiFi_creds,
	error_saving_new_WiFi_creds,
	re_writed_WiFi_creds,
	saved_new_WiFi_creds
} saveNewWiFiCredsReturnMsgs;

class HAQuDA_FileStorage {
  public:
	HAQuDA_FileStorage();
	~HAQuDA_FileStorage();

	bool Start(void);
	void Stop(void);
	void ListDir(const char *dirname, const uint8_t levels);
	static bool DeleteFile(const char *path);
	bool Format();
	static bool WriteFile(const char *path, const uint8_t *data, const size_t len);
	static bool WriteFile(const char *path, const uint16_t fromPos, const uint8_t *data, const size_t len);
	static bool AppendFile(const char *path, const uint8_t *data, const size_t len);
	bool Exists(const String &path);
	bool Exists(const char *path);
	static int FileSize(const char *path);

	static void ReadFileInSerial(const char *path);
	static bool ReadFile(const char *path, uint8_t *data, const size_t len);
	static bool ReadFile(const char *path, const int fromPos, uint8_t *data, const size_t len);
	const String Read_WiFiCreds();
};

#endif
