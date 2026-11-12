#pragma once

#include <atomic>

#include <WiFi.h>
#include <DNSServer.h>
#include <EasyDDNS.h>
#include "HTTPClient.h"

#include "TimeHelper.h"
#include "HAQuDA_WebServer.h"
#include "HAQuDA_DisplayManip.h"

#include "HAQuDA_LEDsParams.h"

class HAQuDA_WiFi_handler : public HAQuDA_WebServer {
  private:
	char *AP_ssid = AP_SSID;
	char *AP_pass = AP_PASS;

	DNSServer dnsServer;
	// Set in CreateAP() (setup), read in HandleConnection() (loop): same task, no sharing
	bool dnsServerRunning = false;
	// loop() only
	uint32_t dnsPollMs = 0;

	static void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
	static void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

	IPAddress AP_ip = IPAddress(192, 168, 4, 22);
	IPAddress AP_subnet = IPAddress(255, 255, 255, 0);
	int reconnectAttempts = 3;
	bool connectToStoredWiFi();
	bool checkStoredWiFiCreds();
	bool createAP();
	bool connectToWiFi(const char *ssidLocal, const char *passLocal);
	void waitUntilWiFiConnected();
	// Green matrix for 3 s, then off. Loop task only (setup / HandleConnection): blocks for the flash.
	void showJoinedWiFi();

  public:
	HAQuDA_WiFi_handler(HAQuDA_FileStorage *currFS, HAQuDA_DisplayManip *_dispManip);
	~HAQuDA_WiFi_handler();
	// True from GOT_IP until DISCONNECTED (LOST_IP does not clear it). Written by the WiFi event task and by
	// waitUntilWiFiConnected() in the loop task, read in the loop task.
	static std::atomic<bool> WiFiConnected;
	bool Connect();
	bool CreateAP();
	void HandleConnection();
};