#pragma once

#include "Arduino.h"

#define COLOR_AQUA 0x00FFFF
#define COLOR_LIME 0x99FF33
#define COLOR_GREEN 0x01FF00
#define COLOR_RED 0xFF0000
#define COLOR_YELLOW 0xFFFF00
#define COLOR_WHITE 0xFFFFFF

typedef enum { effect, meas, error, none } displayMode_enum;

/*******************************************************/

/*****/
/***************TYPEDEFS FOR EFFECTS********************/
/*****/

typedef enum { snake, grow, randomPixel, fade, christmasTree, staticColor, noneEffect } displayEffectMode_enum;

typedef struct {
	uint16_t speed = 0; // in ms - delay between movements
	uint8_t tailLength = 0;
	uint32_t color = 0;
} snakeEffectsParams_struct;

typedef struct {
	uint16_t speed = 0; // in ms - delay between movements
	uint32_t color = 0;
} growEffectsParams_struct;

typedef struct {
	uint16_t speed = 0; // in ms - delay between reducing brightness steps
	uint32_t color = 0;
	uint8_t startBrightness = 0; // percent
	uint8_t stopBrightness = 0;	// percent
	uint8_t step = 0;
} fadeEffectsParams_struct;

typedef struct {
	uint16_t speed = 0;	  // in ms - delay between LED ignition
	uint16_t pauseTime = 0; // in ms - pause after all LED ignited
} randomEffectsParams_struct;

typedef struct {
	uint16_t speed = 0; // in ms - delay between movements
	uint32_t color = 0;
	uint8_t length = 0;
} upDownEffectsParams_struct;

typedef struct {
	displayEffectMode_enum effect = noneEffect;

	snakeEffectsParams_struct snakeParams;
	growEffectsParams_struct growParams;
	fadeEffectsParams_struct fadeParams;
	randomEffectsParams_struct randomParams;
	upDownEffectsParams_struct upDownParams;
} displayEffectParams_struct;

/*******************************************************/

/*******************************************************/

/*****/
/***************TYPEDEFS FOR MEASURES********************/
/*****/

#define MULTI_MODE_PARAM_NUM 3
#define DIVIDE_DOTS_NUM 3
#define DISP_PARAMS_NUM 5

typedef enum { standard, multi, night, noneMode } displayMeasMode_enum;

typedef enum { total, eCO2, TVOC, PM2_5, temp, humid, noneParam } displayParams_enum;

typedef struct {
	uint16_t firstDot = 0;
	uint16_t middleDot = 0;
	uint16_t thirdDot = 0;
} measDivideDots_struct;

typedef struct {
	displayParams_enum paramsArr[MULTI_MODE_PARAM_NUM] = {noneParam, noneParam, noneParam};
	float dataArr[MULTI_MODE_PARAM_NUM] = {0, 0, 0};
	measDivideDots_struct divideDotsArr[MULTI_MODE_PARAM_NUM];
} multiModeParamDisplay_struct;

typedef struct {
	uint8_t timeFirstBorder = 0;
	uint8_t timeSecondBorder = 0;
} nightModeTimeBorder_struct;

typedef struct {
	displayMeasMode_enum mode = noneMode;

	displayParams_enum displayParam = noneParam;

	multiModeParamDisplay_struct multiModeStruct;

	measDivideDots_struct temp_divideDots;
	measDivideDots_struct humid_divideDots;
	measDivideDots_struct eCO2_divideDots;
	measDivideDots_struct TVOC_divideDots;
	measDivideDots_struct PM2_5_divideDots;
	nightModeTimeBorder_struct currentTimeBorder;
} displayMeasParams_struct;

/*******************************************************/