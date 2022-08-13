#pragma once

#include <Adafruit_NeoPixel.h>

#define LED_PIN 26
#define LED_NUM_PIXELS 109
#define MAX_BRIGHTNESS 150
#define LED_COLUMN_NUM 9
#define LED_ROW_NUM 12
#define WHITE_BRIGHTNESS_COEFF 70

#define CHRISTMAS_TREE_HALF_WIDTH 3
#define CHRISTMAS_TREE_HEIGHT 8
#define CHRISTMAS_TREE_COLOR 0x0F962D
#define CHRISTMAS_TREE_STAR_COLOR 0xFFFF00

#define COLORS_NUM 3

void WS2812_begin();

void WS2812_setBrightnessPerCent(uint8_t brightness);

void WS2812_clear();
void WS2812_show();
void WS2812_show(uint16_t delay);
void WS2812_setPixelColor(uint16_t pixelNum, uint32_t color);
void WS2812_fillColor(uint32_t color);
void WS2812_fillColor(uint32_t color, uint8_t startPixel, uint8_t numPixels);
uint32_t WS2812_getColor(uint8_t red, uint8_t green, uint8_t blue);

//void WS2812_showParams_standard(float data, measDivideDots_struct divideDots);
//void WS2812_showParams_standardTotal(float *data, measDivideDots_struct *dataDivideDots);
//void WS2812_showParams_multi(float *data, measDivideDots_struct *divideDots);
//void WS2812_showParams_night(float data, measDivideDots_struct divideDots, uint8_t time);
//
//void WS2812_ChristmasTree(int speed);