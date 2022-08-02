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

#define WEB_SERVER_INDEX_PAGE "/index.html"
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

	float eCO2_meas;
	float TVOC_meas;
	float PM_1_0_meas;
	float PM_2_5_meas;
	float PM_10_meas;
	float temp_meas;
	float humid_meas;
	float O3_meas;
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
  private:
	bool SPIFFS_status;

  public:
	typedef std::function<void(String, int)> TListDirFunction;
	bool write(File &file, String &s);

	HAQuDA_FileStorage();
	~HAQuDA_FileStorage();

	bool Start(void);
	void Stop(void);
	void ListDir(const char *dirname, uint8_t levels);
	static bool DeleteFile(const char *path);
	bool Format();
	bool WriteFile(const char *path, const uint8_t *data = NULL, size_t len = 0);
	bool AppendFile(const char *path, const uint8_t *data, size_t len);
	bool Exists(const String &path);
	bool Exists(const char *path);
	File FileOpen(const String &path, const char *mode);
	static size_t FileSize(const char *path);
	
	static void ReadFileInSerial(const char *path);
	static bool ReadFileFrom(const char *path, const int bias, uint8_t *data, size_t len);
	const String Read_WiFiCreds();

	static saveNewWiFiCredsReturnMsgs SaveNew_WiFiCreds(TWiFiCreds writeWiFiNet);
	TWiFiCreds GetStored_WiFi(int num);
	static int GetStored_WiFiCredsNum();
};

#endif
