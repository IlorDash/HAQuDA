#pragma once

#include "Arduino.h"

#define MULTI_MODE_PARAM_NUM 3
#define DIVIDE_DOTS_NUM 3
#define DISP_PARAMS_NUM 5

typedef enum {effect , meas, error, none } whatDisplay_enum;

typedef enum { snake, randomPixel, fade, christmasTree, staticColor, noneEffect } displayEffect_enum;

typedef struct {
	uint16_t snakeSpeed; // in ms - delay between movements
	uint8_t snakeTailLength;
	uint32_t snakeColor;
} snakeEffectsParams_struct;

typedef struct {
	uint16_t fadeSpeed; // in ms - delay between reducing brightness steps
	uint32_t fadeColor;
} fadeEffectsParams_struct;

typedef enum { standard, multi, night, noneMode } displayMode_enum;

typedef enum { total, eCO2, TVOC, PM2_5, temp, humid, noneParam } displayParams_enum;

typedef struct {
	uint16_t firstDot;
	uint16_t secondDot;
	uint16_t thirdDot;
} measDivideDots_struct;

typedef struct {
	displayParams_enum paramsArr[MULTI_MODE_PARAM_NUM];
	float dataArr[MULTI_MODE_PARAM_NUM];
	measDivideDots_struct divideDotsArr[MULTI_MODE_PARAM_NUM];
} multiModeParamDisplay_struct;

typedef struct {
	uint8_t timeFirstBorder;
	uint8_t timeSecondBorder;
} nightModeTimeBorder_struct;

typedef struct {
	displayMode_enum displayMode;

	displayParams_enum displayParam;

	multiModeParamDisplay_struct multiModeStruct;

	measDivideDots_struct temp_divideDots;
	measDivideDots_struct humid_divideDots;
	measDivideDots_struct eCO2_divideDots;
	measDivideDots_struct TVOC_divideDots;
	measDivideDots_struct PM2_5_divideDots;
	nightModeTimeBorder_struct currentTimeBorder;
} displayMeasParams_struct;