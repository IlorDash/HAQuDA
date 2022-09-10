#pragma once

#include "HAQuDA_WebServer.h"
#include "TimeHelper.h"
#include "HAQuDA_DisplayManip.h"

#define CONNECT_WIFI_TIMEOUT 3000

class HAQuDA_WiFi_handler : public HAQuDA_WebServer {
  private:
	char *AP_ssid = AP_SSID;
	char *AP_pass = AP_PASS;

	static void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
	static void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

	IPAddress AP_ip = IPAddress(192, 168, 4, 22);
	IPAddress AP_gateway = IPAddress(192, 168, 4, 9);
	IPAddress AP_subnet = IPAddress(255, 255, 255, 0);
	int reconnectAttempts = 3;
	bool connectToStoredWiFi();
	bool checkStoredWiFiCreds();
	bool createAP();
	bool connectToWiFi(const char *ssidLocal, const char *passLocal);
	void waitUntilWiFiConnected();

  public:
	HAQuDA_WiFi_handler(HAQuDA_FileStorage *currFS) : HAQuDA_WebServer(currFS){};
	~HAQuDA_WiFi_handler();
	static bool WiFiConnected;
	bool Connect();
	bool CreateAP();
	void HandleConnection();
};