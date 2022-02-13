#pragma once

#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include "WebPages.h"

class HAQuDA_WebServer {
  private:
	WebServer server;
	WiFiClient client;
	void setWiFiCreds(String ssidLocal, String passLocal);

  public:
	HAQuDA_WebServer();
	void WebServer_init();
};