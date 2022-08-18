#pragma once

#include "HAQuDA_DisplayInterface_typedefs.h"
#include "Sensors.h"
#include "WS2812.h"

#include <WiFiUdp.h>
#include <NTPClient.h>

class HAQuDA_DisplayManip {
  private:
	// Define NTP Client to get time
	static WiFiUDP ntpUDP;
	static NTPClient *timeClient;

	static displayParams_enum checkBadParam(displayMeasParams_struct currUI_Params);
	static uint8_t get_nightMode_hour(nightModeTimeBorder_struct timeBorder);
	static void standardMode(displayMeasParams_struct currUI_Params);
	static void multiMode(multiModeParamDisplay_struct multiModeParams);
	static void nightMode(displayMeasParams_struct currUI_Params);

	static void getMeasRGB(uint8_t *_red, uint8_t *_green, uint8_t *_blue, const uint8_t brightness, const float data, const measDivideDots_struct divideDots);
	static void showMeas_multi(float *data, uint8_t dataSize, measDivideDots_struct *divideDots, uint8_t divideDotsSize);
	static void showMeas_total(float *data, uint8_t dataSize, measDivideDots_struct *dataDivideDots, uint8_t divideDotsSize);
	static void showMeas_standard(float data, measDivideDots_struct divideDots);
	static void showMeas_night(float data, measDivideDots_struct divideDots, uint8_t time);

	static const uint32_t trunkColor = 0xBE551E;
	static const uint8_t christmasTreeMaxWidth = 7;
	static const uint8_t redBackgroundIntensity = 190;
	static const uint8_t greenBackgroundIntensity = 190;
	static const uint8_t blueBackgroundIntensity = 230;

	static uint8_t GetLedNum(int x, int y);
	static void christmasLightsSnake();
	static void drawStarOnTree(int middleColumn);
	static void drawTreeTrunk(uint8_t middleColumn);
	static void drawTreeTop(uint8_t middleColumn);
	static void displayChristmasTree(uint8_t speed, uint8_t treeMiddleColumn);
	// void snakeEffect()

  public:
	HAQuDA_DisplayManip();
	static void startNTP();
	static void displayData(displayMeasParams_struct currUI_Params);
	static void showEffect_ChristmasTree(uint8_t speed, uint8_t treeMiddleColumn);
	~HAQuDA_DisplayManip();
};