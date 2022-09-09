#pragma once

#include "HAQuDA_DisplayManip_typedefs.h"

#include <WiFiUdp.h>
#include <NTPClient.h>

class HAQuDA_DisplayManip {
  private:
	static displayMode_enum DisplayMode;

	static displayEffectParams_struct DisplayEffectParams;
	static bool stopEffect;

	static displayMeasParams_struct DisplayMeasParams;

	// Define NTP Client to get time
	WiFiUDP ntpUDP;
	static NTPClient *timeClient;

	static void showMeas();

	static displayParams_enum checkBadParam(displayMeasParams_struct currUI_Params);
	static uint8_t get_nightMode_hour(nightModeTimeBorder_struct timeBorder);
	static void standardMode();
	static void multiMode(multiModeParamDisplay_struct multiModeParams);
	static void nightMode();

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

	/*
	static void christmasLightsSnake();
	static void drawStarOnTree(int middleColumn);
	static void drawTreeTrunk(uint8_t middleColumn);
	static void drawTreeTop(uint8_t middleColumn);
	static void displayChristmasTree(uint8_t speed, uint8_t treeMiddleColumn);
	*/

  public:
	HAQuDA_DisplayManip();

	static void SetBrightness(uint8_t newBrightnessPerCent);
	static void SetDisplayMode(displayMode_enum newDisplayMode);
	static void SetDisplayMeasMode(displayMeasMode_enum newDisplayMeasMode);
	static void SetDisplayMeasParam(displayParams_enum newDisplayMeasParamn);
	static void SetDisplayEffect(displayEffectMode_enum newDisplayEffect);
	static void SetDisplayEffectParams(displayEffectParams_struct newDisplayEffectParams);

	uint8_t GetBrightness();
	displayMode_enum GetDisplayMode();
	displayMeasMode_enum GetDisplayMeasMode();
	displayParams_enum GetDisplayMeasParam();
	static displayEffectMode_enum GetDisplayEffect();

	void StartNTP();

	static void ShowStaticColor(uint32_t color);
	static void ShowStaticColor(int red, int green, int blue);
	static void ShowEffectGrow(const growEffectsParams_struct params = DisplayEffectParams.growParams);
	static void ShowEffectSnake(const snakeEffectsParams_struct params = DisplayEffectParams.snakeParams);
	static void ShowEffectRandom(const randomEffectsParams_struct params = DisplayEffectParams.randomParams);
	static void ShowEffectFade(const fadeEffectsParams_struct params = DisplayEffectParams.fadeParams);
	// static void ShowEffectChristmasTree(uint8_t speed, uint8_t treeMiddleColumn);

	~HAQuDA_DisplayManip();
};