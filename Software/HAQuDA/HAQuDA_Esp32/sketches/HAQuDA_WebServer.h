#pragma once

#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include "WebPages.h"

class HAQuDA_WebServer {
  private:
  public:
	WebServer server;
	WiFiClient client;
	HAQuDA_WebServer();
	bool WiFiCredsFound = true;
	void WebServer_init();
	void setWiFiCreds();
	void handle_NotFound();
};