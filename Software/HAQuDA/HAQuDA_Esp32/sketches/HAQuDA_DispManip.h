#pragma once

#include "Arduino.h"

#include "HAQuDA_UI.h"
#include "Sensors.h"
#include "WS2812.h"

#include <WiFiUdp.h>
#include <NTPClient.h>

class HAQuDA_DispManip {
  private:
	// Define NTP Client to get time
	WiFiUDP ntpUDP;
	NTPClient *timeClient;
	HAQuDA_UI *myUI_dispManip;
	dispParams checkBadParam();
	void standardMode();

  public:
	HAQuDA_DispManip(HAQuDA_UI *currUI);

	void dispParam_WS2812();
	~HAQuDA_DispManip();
};