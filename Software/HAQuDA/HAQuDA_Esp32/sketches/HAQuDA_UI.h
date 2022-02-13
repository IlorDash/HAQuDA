#pragma once

#include "HAQuDA_DispManip.h"
#include "WS2812.h"

#define MULTI_MODE_PARAM_NUM 3
#define DISP_PARAMS_NUM 5

typedef enum { snake, randomPixel, fade, christmasTree, staticColor, noneEffect } dispEffects;

typedef struct {
	uint16_t firstDot;
	uint16_t secondDot;
	uint16_t thirdDot;
} measDivideDots_struct;

typedef enum { total, eCO2, TVOC, PM2_5, temp, humid, noneParam } dispParams;
typedef enum { standard, multi, night, effect, noneMode } dispMode;

typedef struct {
	dispParams paramsArr[MULTI_MODE_PARAM_NUM];
	float dataArr[MULTI_MODE_PARAM_NUM];
	measDivideDots_struct divideDotsArr[MULTI_MODE_PARAM_NUM];
} multiModeParamDisp_struct;

typedef struct {
	uint8_t timeFirstBorder;
	uint8_t timeSecondBorder;
} nightModeTimeBorder_struct;

typedef struct {
	dispMode dispMode;
	dispParams dispParam;
	dispEffects dispEffect;
	multiModeParamDisp_struct multiModeStruct;

	measDivideDots_struct temp_divideDots = {20, 26, 30};
	measDivideDots_struct humid_divideDots = {40, 60, 80};
	measDivideDots_struct eCO2_divideDots = {400, 1000, 5000};
	measDivideDots_struct TVOC_divideDots = {220, 660, 1000};
	measDivideDots_struct PM2_5_divideDots = {15, 20, 45};
	nightModeTimeBorder_struct currentTimeBorder = {21, 9};
	uint8_t brightnessPerCent;
} UI_Params;

class HAQuDA_UI {
  private:
	HAQuDA_DispManip *myDispManip;

  public:
	UI_Params currUI_Params;
	HAQuDA_UI(HAQuDA_DispManip *currDispManip);
	void setStaticColor(int red, int green, int blue);
	void setBrightness(uint8_t newBrightnessPerCent);
	void changeDispMode(int newDispMode);
	void changeDispParam(int newDispParam);
	void changeDispEffect(int newDispEffect);
};