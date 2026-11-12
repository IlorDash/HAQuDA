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
#include "HAQuDA_WiFiJoin.h"

#define SSID_PARAM_NAME "ssid"
#define PASS_PARAM_NAME "pass"
#define WEB_SERIAL_MAX_STR_LEN 64

// Delay between the "saved" reply and the restart, so the browser receives the reply first
constexpr uint32_t RESTART_DELAY_MS = 3000;
// TCP port of the web server; the mDNS http service advertises the same port
constexpr uint16_t WEB_SERVER_PORT = 80;

// Hostname: sent in the station's DHCP request, so routers list the lamp under it, and answered by the mDNS responder
// as <name>.local in station mode and on the access point. Fixed, so the name does not depend on the build flags.
constexpr char STA_HOSTNAME[] = "haquda";

class HAQuDA_WebServer {
	static void recvMsg(uint8_t *data, size_t len);

	static bool checkStoredWiFiCredsCntLimit();
	static bool getWiFiCredsWriteIndex(uint16_t *index, const TWiFiCreds newWiFiCreds);

  protected:
	HAQuDA_FileStorage *_myFS;
	AsyncWebServer server = AsyncWebServer(WEB_SERVER_PORT);

	bool newWiFiCredsFound = false;
	static bool WebServerStarted;

	const HAQuDA_MeasSnapshot &measSnapshot;
	// Written by the async_tcp task (handle_NewWiFiCreds), read by loop(). The time is stored before the flag.
	std::atomic<bool> restartPending{false};
	std::atomic<uint32_t> restartRequestMs{0};
	std::atomic<uint32_t> restartDelayMs{RESTART_DELAY_MS};
	// A log call writes to SPIFFS and WebSerial and the logger has no lock, so the async_tcp task does not call it:
	// it posts a string literal here and loop() logs it in HandleDeferredWork().
	std::atomic<const char *> deferredLogError{nullptr};

	// True when the lamp booted into access point mode. Set once by the WiFi handler in setup(), read by the web server
	// task and loop(). In this mode a new network is checked before it is saved.
	std::atomic<bool> apMode{false};

	// Check of a new network, see HAQuDA_WiFiJoin.h. The web server task queues the request and reads the view, loop()
	// takes the request and writes the view; both sit behind joinMux, held for a copy only (no calls inside).
	portMUX_TYPE joinMux = portMUX_INITIALIZER_UNLOCKED;
	join_view joinView;
	TWiFiCreds joinRequest;
	// Written under joinMux; loop() reads it without the lock as a cheap "anything to take" test
	std::atomic<bool> joinRequestPending{false};

	// Serialises the operations on the WiFi credentials file between loop() and the web server task: a save in loop()
	// must not interleave with a delete or a list read in a handler. A plain mutex: nothing that holds it takes it again.
	// nullptr when the heap could not provide it; the users then refuse the operation and log that the lock is missing.
	SemaphoreHandle_t credsMutex = xSemaphoreCreateMutex();

	// async_tcp task only; msg must be a string literal
	void postLogError(const char *msg);
	// Restarts the lamp delayMs after the call; the first call wins. Any task, the restart itself happens in loop().
	void scheduleRestart(uint32_t delayMs);

	// Queues a check of the network for loop(). Refuses with checking when a check is running or queued, and with
	// restarting when a restart is pending (a passed check schedules one). Web server task.
	join_queue_result queueJoinRequest(const TWiFiCreds &creds);
	// Moves the queued request to creds and returns true; false when none is queued. loop() only.
	bool takeJoinRequest(TWiFiCreds &creds);
	// Publishes the progress of the running check to /api/join. loop() only.
	void publishJoinState(join_state state, join_reason reason, uint32_t ip);

	// Saves the network, or overwrites the record of the same name. Any task; takes the credentials lock.
	saveNewWiFiCredsReturnMsgs SaveNew_WiFiCreds(TWiFiCreds writeWiFiNet);

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
	void handleJoin(AsyncWebServerRequest *request);
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