#pragma once

#include <Adafruit_NeoPixel.h>
#include "Main.h"

#define LED_PIN 18
#define LED_NUM_PIXELS 109
#define MAX_BRIGHTNESS 150
#define LED_COLUMN_NUM 9
#define LED_ROW_NUM 12
#define WHITE_BRIGHTNESS_COEFF 70

#define MULTI_MODE_PARAM_NUM 3

#define CHRISTMAS_TREE_HALF_WIDTH 3
#define CHRISTMAS_TREE_HEIGHT 8
#define CHRISTMAS_TREE_COLOR 0x0F962D
#define CHRISTMAS_TREE_STAR_COLOR 0xFFFF00

#define COLORS_NUM 3

void WS2812_begin();

void WS2812_setBrightnessPerCent(uint8_t brightness);

void WS2812_clear();
void WS2812_show();
void WS2812_setPixelColor(uint16_t pixelNum, uint32_t color);
void WS2812_fillColor(uint32_t color);

void WS2812_showParams_standard(float data, paramsDivideDots divideDots);
void WS2812_showParams_standardTotal(float *data);
void WS2812_showParams_multi(float *data, paramsDivideDots *divideDots);
void WS2812_showParams_night(float data, paramsDivideDots divideDots, uint8_t time);

void WS2812_ChristmasTree(int speed);