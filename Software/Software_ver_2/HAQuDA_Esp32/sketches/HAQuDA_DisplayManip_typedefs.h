#pragma once

#include "Arduino.h"

#define COLOR_AQUA 0x00FFFF
#define COLOR_LIME 0x99FF33
#define COLOR_GREEN 0x01FF00
#define COLOR_RED 0xFF0000
#define COLOR_YELLOW 0xFFFF00
#define COLOR_WHITE 0xFFFFFF

typedef enum { effect, meas, error, noneMode } display_mode;

/***************TYPEDEFS FOR EFFECTS********************/

typedef enum { snake, grow, randomPixel, fade, upDown, staticColor, noneEffect } effect_mode;

typedef struct {
	uint16_t speed = 0; // in ms - delay between movements
	uint8_t tailLength = 0;
	uint32_t color = 0;
} snake_effect_params;

typedef struct {
	uint16_t speed = 0; // in ms - delay between movements
	uint32_t color = 0;
} grow_effect_params;

typedef struct {
	uint16_t speed = 0; // in ms - delay between reducing brightness steps
	uint32_t color = 0;
	uint8_t startBrightness = 0; // percent
	uint8_t stopBrightness = 0;  // percent
	uint8_t step = 0;
} fade_effect_params;

typedef struct {
	uint16_t speed = 0;		// in ms - delay between LED ignition
	uint16_t pauseTime = 0; // in ms - pause after all LED ignited
} random_effect_params;

typedef struct {
	uint16_t speed = 0; // in ms - delay between movements
	uint32_t color = 0;
	uint8_t length = 0;
} upDown_effects_params;

typedef struct {
	effect_mode mode = noneEffect;

	snake_effect_params snakeParams;
	grow_effect_params growParams;
	fade_effect_params fadeParams;
	random_effect_params randomParams;
	upDown_effects_params upDownParams;
} effects_params;

/***************TYPEDEFS FOR MEASURES********************/

#define MULTI_MODE_PARAM_NUM 3
#define DIVIDE_DOTS_NUM 3
#define DISP_PARAMS_NUM 5

typedef enum { standard, multi, night, noneMeas } meas_mode;

typedef enum { total, eCO2, TVOC, PM2_5, temp, humid, noneType } meas_type;

typedef struct {
	uint16_t firstDot = 0;
	uint16_t middleDot = 0;
	uint16_t thirdDot = 0;
} div_dots;

typedef struct {
	meas_type paramsArr[MULTI_MODE_PARAM_NUM] = {noneType, noneType, noneType};
	float dataArr[MULTI_MODE_PARAM_NUM] = {0, 0, 0};
	div_dots divideDotsArr[MULTI_MODE_PARAM_NUM];
} multi_mode_params;

typedef struct {
	uint8_t timeFirstBorder = 0;
	uint8_t timeSecondBorder = 0;
} night_mode_time_border;

typedef struct {
	div_dots temp;
	div_dots humid;
	div_dots eCO2;
	div_dots TVOC;
	div_dots PM2_5;
} meas_div_dots;

typedef struct {
	meas_mode mode = noneMeas;
	meas_type type = noneType;

	multi_mode_params multiModeParams;
	night_mode_time_border nightModeTimeBorder;
	meas_div_dots divDots;
} meas_params;

typedef struct {
	uint8_t brighntessPerCent;
	bool stopEffect;
	bool forceShowMeas;
	display_mode mode;
	effects_params effectParams;
	meas_params measParams;
} curr_display;