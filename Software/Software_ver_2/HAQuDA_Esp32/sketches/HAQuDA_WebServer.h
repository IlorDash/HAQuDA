#pragma once

#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include "HAQuDA_FileStorage.h"

class HAQuDA_WebServer {
  private:
  public:
	HAQuDA_FileStorage *myFS_WiFi;
	bool WiFiCredsFound = false;

	WebServer server;
	WiFiClient client;

	void beginWebServer();
	void loop();

	HAQuDA_WebServer();

	void WebServer_init();
	void getNewWiFiCreds();
	void handle_NotFound();
};