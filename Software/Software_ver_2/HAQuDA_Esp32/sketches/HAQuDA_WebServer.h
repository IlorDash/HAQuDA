#pragma once

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "HAQuDA_FileStorage.h"

#define WIFI_CREDS_PARAMS_NUM 4
#define SSID_PARAM_NAME "ssid"
#define PASS_PARAM_NAME "pass"
#define STATIC_IP_ARG "static_ip"
#define GATEWAY_ARG "gateway"

class HAQuDA_WebServer {
  private:
  protected:
	HAQuDA_FileStorage *_myFS;
	bool newWiFiCredsFound = false;
	AsyncWebServer server = AsyncWebServer(80);

	void beginWebServer();

	HAQuDA_WebServer(HAQuDA_FileStorage *curr_FS) : _myFS(curr_FS){};
	~HAQuDA_WebServer();

	void WebServerResponds_init();
	static void handle_NewWiFiCreds(AsyncWebServerRequest *request);
	static void handle_NotFound(AsyncWebServerRequest *request);
	static void delete_WiFiCreds(AsyncWebServerRequest *request);
	static void show_WiFiCreds(AsyncWebServerRequest *request);
	void loop();
};