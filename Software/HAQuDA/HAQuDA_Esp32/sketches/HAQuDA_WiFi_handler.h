#pragma once

#include "HAQuDA_WebServer.h"
#include "TimeHelper.h"
#include "WS2812.h"
#include "HAQuDA_UI.h"
#include "FileStorage.h"


class HAQuDA_WiFi_handler : public HAQuDA_WebServer {
  private:
	char *AP_ssid = {"HAQuDA_1"};
	char *AP_pass = {"TpF4YJ"};

	HAQuDA_UI *myUI_WiFi;

	static void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
	static void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

	IPAddress *AP_ip;
	IPAddress *AP_gateway;
	IPAddress *AP_subnet;
  public:
	HAQuDA_WiFi_handler(HAQuDA_UI *currUI_WiFi, FileStorage *currFS_WiFi);
	~HAQuDA_WiFi_handler();
	static bool WiFiConnected;
	void WiFi_handleConnection();
	void createAP();
	bool connectToWiFi(char *ssidLocal, char *passLocal);
};