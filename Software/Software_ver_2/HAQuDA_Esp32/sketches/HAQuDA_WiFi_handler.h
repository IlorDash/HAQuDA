#pragma once

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
	// Set and read in startMdns() only (called from Connect() in setup: loop task, no sharing)
	bool mdnsStartTried = false;

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
	// Starts the mDNS responder so the page opens as http://haquda.local/ in the home network. Station mode
	// only: in access point mode the captive DNS already answers every name. Tries once; later calls do
	// nothing. A failure is logged and is not fatal. Loop task only (it logs); never from the WiFi event task.
	void startMdns();

  public:
	// snapshot feeds the web page with the latest measurements; it must outlive the handler.
	HAQuDA_WiFi_handler(HAQuDA_FileStorage *currFS, HAQuDA_DisplayManip *_dispManip, const HAQuDA_MeasSnapshot &snapshot);
	~HAQuDA_WiFi_handler();
	static bool WiFiConnected;
	bool Connect();
	bool CreateAP();
	void HandleConnection();
};