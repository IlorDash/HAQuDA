#pragma once

#include <atomic>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "HAQuDA_MyWebSerial.h"
#include "HAQuDA_FileStorage.h"
#include "HAQuDA_CaptiveRequestHandler.h"
#include "HAQuDA_Logger.h"
#include "HAQuDA_MeasSnapshot.h"

#define SSID_PARAM_NAME "ssid"
#define PASS_PARAM_NAME "pass"
#define WEB_SERIAL_MAX_STR_LEN 64

// Delay between the "saved" reply and the restart, so the browser receives the reply first
constexpr uint32_t RESTART_DELAY_MS = 3000;
// TCP port of the web server; the mDNS http service advertises the same port
constexpr uint16_t WEB_SERVER_PORT = 80;

class HAQuDA_WebServer {
	static void recvMsg(uint8_t *data, size_t len);

	static bool checkStoredWiFiCredsCntLimit();
	static bool getWiFiCredsWriteIndex(uint16_t *index, const TWiFiCreds newWiFiCreds);

	static saveNewWiFiCredsReturnMsgs SaveNew_WiFiCreds(TWiFiCreds writeWiFiNet);

  protected:
	HAQuDA_FileStorage *_myFS;
	AsyncWebServer server = AsyncWebServer(WEB_SERVER_PORT);

	bool newWiFiCredsFound = false;
	static bool WebServerStarted;

	const HAQuDA_MeasSnapshot &measSnapshot;
	// Written by the async_tcp task (handle_NewWiFiCreds), read by loop(). The time is stored before the flag.
	std::atomic<bool> restartPending{false};
	std::atomic<uint32_t> restartRequestMs{0};
	// A log call writes to SPIFFS and WebSerial and the logger has no lock, so the async_tcp task does not call it:
	// it posts a string literal here and loop() logs it in HandleDeferredWork().
	std::atomic<const char *> deferredLogError{nullptr};

	// async_tcp task only; msg must be a string literal
	void postLogError(const char *msg);
	void scheduleRestart();

	void beginWebServer();

	HAQuDA_WebServer(HAQuDA_FileStorage *curr_FS, const HAQuDA_MeasSnapshot &snapshot) : _myFS(curr_FS), measSnapshot(snapshot) {};
	~HAQuDA_WebServer();

	TWiFiCreds GetStored_WiFi(int num);
	static int GetStored_WiFiCredsNum();

	void WebServerResponds_init();
	// The handlers below run in the async_tcp task and must not block
	void handle_NewWiFiCreds(AsyncWebServerRequest *request);
	static void handle_NotFound(AsyncWebServerRequest *request);
	void delete_WiFiCreds(AsyncWebServerRequest *request);
	void handleStatus(AsyncWebServerRequest *request);
	void handleWiFiList(AsyncWebServerRequest *request);
	void WebSerialPrintStoredLogs();

	// Writes the log record a web handler posted and restarts the lamp once the delay after saving new WiFi
	// credentials has passed. loop only.
	void HandleDeferredWork();

  public:
	// Sends str plus CRLF as two websocket frames (println). Loop task only: a call from a web callback would race with the logger's sends.
	static void WebSerialPrint(const char *str);
	// Sends str as is in one websocket frame; the caller appends the line end. Loop task only.
	static void WebSerialPrintRaw(const char *str);
	static bool GetWebServerStarted();
};