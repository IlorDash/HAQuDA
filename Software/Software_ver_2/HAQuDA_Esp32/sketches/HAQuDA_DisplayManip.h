#pragma once

#include "HAQuDA_DisplayInterface_typedefs.h"
#include "Sensors.h"
#include "WS2812.h"

#include <WiFiUdp.h>
#include <NTPClient.h>


class HAQuDA_DisplayManip {
  private:
	// Define NTP Client to get time
	WiFiUDP ntpUDP;
	NTPClient timeClient;

	displayParams_enum checkBadParam(displayMeasParams_struct currUI_Params);
	uint8_t get_nightMode_hour(uint8_t curHour, displayMeasParams_struct currUI_Params);
	void standardMode(displayMeasParams_struct currUI_Params);
	void multiMode(displayMeasParams_struct currUI_Params);
	void nightMode(displayMeasParams_struct currUI_Params);
	
	//void snakeEffect()

  public:
	HAQuDA_DisplayManip() : timeClient(ntpUDP){};
	void startNTP();
	void displayData(displayMeasParams_struct currUI_Params);
	~HAQuDA_DisplayManip();
};