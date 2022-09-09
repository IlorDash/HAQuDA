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
	uint16_t speed; // in ms - delay between movements
	uint8_t tailLength;
	uint32_t color;
} snakeEffectsParams_struct;

typedef struct {
	uint16_t speed; // in ms - delay between movements
	uint32_t color;
} growEffectsParams_struct;

typedef struct {
	uint16_t speed; // in ms - delay between reducing brightness steps
	uint32_t color;
	uint8_t startBrightness; // percent
	uint8_t stopBrightness;  // percent
	uint8_t step;
} fadeEffectsParams_struct;

typedef struct {
	uint16_t speed;		// in ms - delay between LED ignition
	uint16_t pauseTime; // in ms - pause after all LED ignited
} randomEffectsParams_struct;

struct displayEffectParams_struct {
	displayEffectMode_enum effect;

	snakeEffectsParams_struct snakeParams;
	growEffectsParams_struct growParams;
	fadeEffectsParams_struct fadeParams;
	randomEffectsParams_struct randomParams;
	displayEffectParams_struct() {
		growParams.color = COLOR_AQUA;
		growParams.speed = 200;

		snakeParams.color = COLOR_LIME;
		snakeParams.speed = 200;
		snakeParams.tailLength = 5;

		randomParams.speed = 200;
		randomParams.pauseTime = 1000;

		fadeParams.color = COLOR_AQUA;
		fadeParams.speed = 200;
		fadeParams.startBrightness = 100;
		fadeParams.stopBrightness = 0;
		fadeParams.step = 2;
	}
};

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

struct displayMeasParams_struct {
	displayMeasMode_enum mode;

	displayParams_enum displayParam;

	multiModeParamDisplay_struct multiModeStruct;

	measDivideDots_struct temp_divideDots;
	measDivideDots_struct humid_divideDots;
	measDivideDots_struct eCO2_divideDots;
	measDivideDots_struct TVOC_divideDots;
	measDivideDots_struct PM2_5_divideDots;
	nightModeTimeBorder_struct currentTimeBorder;

	displayMeasParams_struct() {
		mode = noneMode;

		displayParam = noneParam;

		temp_divideDots = {20, 26, 30};
		humid_divideDots = {40, 60, 80};
		eCO2_divideDots = {400, 1000, 5000};
		TVOC_divideDots = {220, 660, 1000};
		PM2_5_divideDots = {15, 20, 45};
		currentTimeBorder = {21, 9};

		multiModeStruct.paramsArr[0] = temp;
		multiModeStruct.paramsArr[1] = eCO2;
		multiModeStruct.paramsArr[2] = PM2_5;

		multiModeStruct.divideDotsArr[0] = temp_divideDots;
		multiModeStruct.divideDotsArr[1] = eCO2_divideDots;
		multiModeStruct.divideDotsArr[2] = PM2_5_divideDots;
	}
};

/*******************************************************/