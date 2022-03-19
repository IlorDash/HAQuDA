#ifndef _FILESTORAGE_h
#define _FILESTORAGE_h

#include <functional>
#include <Arduino.h>
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

#define MAX_WIFI_CREDS_NUM 10

const int checkPeriod = 10;
const int checksPerAccumulation = 4;
const int partTime = 21600;

typedef struct TMeasLog {
	uint32_t timeStamp = 0;
	uint16_t measNum = 0;

	float eCO2_meas;
	float TVOC_meas;
	float PM_1_0_meas;
	float PM_2_5_meas;
	float PM_10_meas;
	float temp_meas;
	float humid_meas;
	float O3_meas;
};

typedef struct {
  public:
	String ssid = "";
	String password = "";
} TWiFiCreds;

typedef enum { too_many_WiFi, error_reading_stored_WiFi_creds, error_saving_new_WiFi_creds, saved_new_WiFi_creds} saveNewWiFiCredsReturnMsgs;

class FileStorage {
  private:
	bool write(File &file, String &s);

  public:
	typedef std::function<void(String, int)> TListDirFunction;
	FileStorage();
	~FileStorage();
	bool Start(void);
	void Stop(void);
	void ListDir(const char *dirname, uint8_t levels, TListDirFunction fn);
	bool DeleteFile(const char *path);
	bool Format();
	bool WriteFile(const char *path, const uint8_t *data = NULL, size_t len = 0);
	bool AppendFile(const char *path, const uint8_t *data, size_t len);
	bool Exists(const String &path);
	bool Exists(const char *path);
	File FileOpen(const String &path, const char *mode);
	bool ReadFile(const char *path, uint8_t *data, size_t len);
	size_t FileSize(const char *path);

	bool DeleteLogData(void);

	saveNewWiFiCredsReturnMsgs SaveNewWiFiCreds(TWiFiCreds writeWiFiNet);
};
extern FileStorage fileStorage;

#endif
