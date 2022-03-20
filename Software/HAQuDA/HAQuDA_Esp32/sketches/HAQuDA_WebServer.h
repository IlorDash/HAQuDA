#pragma once

#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>

class HAQuDA_WebServer {
  private:
  public:
	void begin();
	WebServer server;
	WiFiClient client;
	HAQuDA_WebServer();
	bool WiFiCredsFound = true;
	void WebServer_init();
	void setWiFiCreds();
	void handle_NotFound();
};