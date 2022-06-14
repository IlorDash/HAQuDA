#pragma once

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "HAQuDA_FileStorage.h"

#define WIFI_CREDS_PARAMS_NUM 4

class HAQuDA_WebServer {
  private:
  protected:
	HAQuDA_FileStorage *myFS_WiFi;
	bool WiFiCredsFound = false;

	AsyncWebServer server = AsyncWebServer(80);

	void beginWebServer();

	HAQuDA_WebServer();
	~HAQuDA_WebServer();

	bool foundNameInParamsArrOrder(String name);
	bool checkCorrectParams(AsyncWebServerRequest *request);
	int getParamVal(AsyncWebServerRequest *request, String param);

	void WebServer_init();
	void handle_NewWiFiCreds(AsyncWebServerRequest *request);
	void handle_NotFound(AsyncWebServerRequest *request);
};