#pragma once

#include "Arduino.h"

#define MULTI_MODE_PARAM_NUM 3
#define DISP_PARAMS_NUM 5

typedef enum { snake, randomPixel, fade, christmasTree, staticColor, noneEffect } dispEffect_enum;

typedef struct {
	uint16_t firstDot;
	uint16_t secondDot;
	uint16_t thirdDot;
} measDivideDots_struct;

typedef enum { total, eCO2, TVOC, PM2_5, temp, humid, noneParam } dispParams_enum;
typedef enum { standard, multi, night, effect, noneMode } dispMode_enum;

typedef struct {
	dispParams_enum paramsArr[MULTI_MODE_PARAM_NUM];
	float dataArr[MULTI_MODE_PARAM_NUM];
	measDivideDots_struct divideDotsArr[MULTI_MODE_PARAM_NUM];
} multiModeParamDisp_struct;

typedef struct {
	uint8_t timeFirstBorder;
	uint8_t timeSecondBorder;
} nightModeTimeBorder_struct;

typedef struct {
	uint16_t snakeSpeed = 400; // in ms - delay between movements
	uint8_t snakeTailLength = 5;
	uint32_t snakeColor;

	uint16_t randomSpeed = 400; // in ms - delay between LED ignition

	uint16_t fadeSpeed = 400; // in ms - delay between reducing brightness steps
	uint32_t fadeColor;
} effectsParams_struct;

typedef struct {
	dispMode_enum dispMode;
	dispParams_enum dispParam;

	dispEffect_enum dispEffect;
	effectsParams_struct effectParams;

	multiModeParamDisp_struct multiModeStruct;

	measDivideDots_struct temp_divideDots = {20, 26, 30};
	measDivideDots_struct humid_divideDots = {40, 60, 80};
	measDivideDots_struct eCO2_divideDots = {400, 1000, 5000};
	measDivideDots_struct TVOC_divideDots = {220, 660, 1000};
	measDivideDots_struct PM2_5_divideDots = {15, 20, 45};
	nightModeTimeBorder_struct currentTimeBorder = {21, 9};
	uint8_t brightnessPerCent;
} UI_Params;