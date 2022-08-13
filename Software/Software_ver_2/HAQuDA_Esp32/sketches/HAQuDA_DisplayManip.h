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
	static NTPClient timeClient;

	displayParams_enum checkBadParam(displayMeasParams_struct currUI_Params);
	uint8_t get_nightMode_hour(uint8_t curHour, displayMeasParams_struct currUI_Params);
	void standardMode(displayMeasParams_struct currUI_Params);
	void multiMode(displayMeasParams_struct currUI_Params);
	void nightMode(displayMeasParams_struct currUI_Params);
	void showMeas_multi(float *data, uint8_t dataSize, measDivideDots_struct *divideDots, uint8_t divideDotsSize);
	void showMeas_standardTotal(float *data, uint8_t dataSize, measDivideDots_struct *dataDivideDots, uint8_t divideDotsSize);
	
	
	void getMeasRGB(uint8_t *_red, uint8_t *_green, uint8_t *_blue, uint8_t brightness, float data, measDivideDots_struct divideDots);
	void showMeas_standard(float data, measDivideDots_struct divideDots);
	void showMeas_night(float data, measDivideDots_struct divideDots, uint8_t brightness, uint8_t time);

	const uint32_t trunkColor = WS2812_getColor(190, 85, 30);
	const uint8_t christmasTreeMaxWidth = 7;
	uint8_t *christmasTreePixelsArr;
	const uint8_t redBackgroundIntensity = 190;
	const uint8_t greenBackgroundIntensity = 190;
	const uint8_t blueBackgroundIntensity = 230;
	

	uint8_t GetLedNum(int x, int y);
	void christmasLightsSnake();
	void drawStarOnTree(int middleColumn);
	void drawTreeTrunk(uint8_t middleColumn);
	void drawTreeTop(uint8_t middleColumn);
	void displayChristmasTree(uint8_t speed, uint8_t treeMiddleColumn);
	//void snakeEffect()

  public:
	HAQuDA_DisplayManip();
	static void startNTP();
	static void displayData(displayMeasParams_struct currUI_Params);
	static void showEffect_ChristmasTree(uint8_t speed, uint8_t treeMiddleColumn);
	~HAQuDA_DisplayManip();
};