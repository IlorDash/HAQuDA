#pragma once

#include "HAQuDA_WebServer.h"
#include "TimeHelper.h"
#include "WS2812.h"
#include "HAQuDA_UI.h"


class HAQuDA_WiFi_handler : public HAQuDA_WebServer {
  private:
	char *AP_ssid = AP_SSID;
	char *AP_pass = AP_PASS;
	HAQuDA_UI _myUI;

	static void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
	static void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);


	IPAddress AP_ip = IPAddress(192, 168, 4, 22);
	IPAddress AP_gateway = IPAddress(192, 168, 4, 9);
	IPAddress AP_subnet = IPAddress(255, 255, 255, 0);
	int reconnectAttempts = 3;
	bool tryConnectToWiFi();
	bool checkStoredWiFiCreds();
	void createAP();
	bool connectToWiFi(const char *ssidLocal, const char *passLocal);
  public:
	HAQuDA_WiFi_handler(HAQuDA_UI &currUI, HAQuDA_FileStorage &currFS) : _myUI(currUI), HAQuDA_WebServer(currFS){};
	~HAQuDA_WiFi_handler();
	static bool WiFiConnected;
	void WiFi_connect();
	void WiFi_handleConnection();

};