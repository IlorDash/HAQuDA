#pragma once

#include "HAQuDA_UI_typedefs.h"
#include "Sensors.h"
#include "WS2812.h"

#include <WiFiUdp.h>
#include <NTPClient.h>


class HAQuDA_DispManip {
  private:
	// Define NTP Client to get time
	WiFiUDP ntpUDP;
	NTPClient *timeClient;

	dispParams_enum checkBadParam(UI_Params currUI_Params);
	uint8_t get_nightMode_hour(uint8_t curHour, UI_Params currUI_Params);
	void standardMode(UI_Params currUI_Params);
	void multiMode(UI_Params currUI_Params);
	void nightMode(UI_Params currUI_Params);

  public:
	HAQuDA_DispManip();

	void displayData(UI_Params currUI_Params);
	~HAQuDA_DispManip();
};