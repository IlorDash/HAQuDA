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
	// Set and read in startMdns() only (called from Connect() or CreateAP() in setup: loop task, no sharing)
	bool mdnsStartTried = false;
	// Id of the STA_CONNECTED registration made by Connect(); 0 = not registered. Connect() and CreateAP(): loop task.
	wifi_event_id_t staConnectedEventId = 0;

	// Check of a new network in access point mode, switch to a new network in station mode. All members below: loop task,
	// except the atomics.
	HAQuDA_JoinCheck joinCheck;
	// The network under test; the password is wiped when the check ends
	TWiFiCreds joinCreds;
	// Station mode: a switch is under way or has ended in a restart that is still to come. It keeps HandleConnection() from
	// reconnecting to the stored networks while the lamp has left its network on purpose. Cleared only when the scan
	// found nothing (the lamp never left); otherwise the restart ends it.
	bool stationSwitchActive = false;
	// True once registerDisconnectRecorder() has run
	bool disconnectRecorderRegistered = false;
	// The disconnect handler is registered once (Connect() on the station path, CreateAP() on the access point path) and
	// never removed: the core keeps its handler list in an unlocked std::vector that the WiFi event task walks, so loop()
	// must not change it while the lamp runs.
	// The handler records reasons only while this flag is set. Written by loop(), read by the WiFi event task.
	std::atomic<bool> joinActive{false};
	// Written by the WiFi event task, reset and read by loop(). Holds the most telling disconnect reason of the check
	// (see HAQuDA_MergeDisconnectReason); 0 = none. loop() resets it only while joinActive is false.
	std::atomic<uint8_t> joinDisconnectReason{0};
	// Consecutive same-class disconnect events (HAQuDA_AddDisconnectEvent); the event task is its only writer while
	// joinActive is set, loop() clears it before setting joinActive.
	std::atomic<uint32_t> joinDisconnectStreak{0};

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
	// Starts the mDNS responder so the page opens as http://haquda.local/ in the home network and on the lamp's own
	// access point. The access point needs it too: iOS, macOS and newer Android resolve *.local only by multicast
	// mDNS, so the captive DNS that answers every name does not help there. Tries once; later calls do
	// nothing. A failure is logged and is not fatal. Loop task only (it logs); never from the WiFi event task.
	void startMdns();

	// Runs the check of a new network: takes a queued request, joins it with the access point kept up, saves it on
	// success. Never waits: every call does one step. loop only.
	void handleJoinCheck();
	void startJoinCheck(const TWiFiCreds &creds);
	void pollJoinCheck();
	void finishJoinOk();
	void finishJoinFail();
	// Called from the WiFi event task - must not block
	void recordDisconnectReason(uint8_t reason);
	// Registers recordDisconnectReason() for the station disconnect event, once; later calls do nothing. setup() only.
	void registerDisconnectRecorder();

	// Station mode switch, see HAQuDA_WiFiJoin.h. Each call does one step and never waits. loop only.
	// Starts the scan for the network; the lamp stays connected.
	void startStationSwitch(const TWiFiCreds &creds);
	void pollStationSwitch();
	// Leaves the old network and joins the new one
	void beginSwitchJoin();
	// The scan found nothing or failed: the lamp never left its network, nothing restarts
	void finishSwitchScanFail();
	// The network works and is saved first: restart soon
	void finishSwitchOk();
	// The join or the save failed after the lamp left its network: nothing is saved, restart so the stored networks are tried
	void finishSwitchFail();
	// True when the station is connected to the network called ssid (not to the one it left)
	bool joinedNetwork(const char *ssid) const;

  public:
	// snapshot feeds the web page with the latest measurements; it must outlive the handler.
	HAQuDA_WiFi_handler(HAQuDA_FileStorage *currFS, HAQuDA_DisplayManip *_dispManip, const HAQuDA_MeasSnapshot &snapshot);
	~HAQuDA_WiFi_handler();
	static bool WiFiConnected;
	// Joins a stored network, registers the disconnect-reason recorder and starts the web server. setup() (loop task) only.
	bool Connect();
	// Starts the access point, the captive DNS and the web server, and switches the web server to access point mode
	// (new networks are checked before they are saved). Drops the station-connected display handler: a test join
	// must not replace the access point effect. setup() (loop task) only.
	bool CreateAP();
	// Serves the captive DNS, the deferred web work, the check of a new network (access point mode) or the switch to it
	// (station mode), and the station reconnect, which is skipped during a switch. loop task only.
	void HandleConnection();
};