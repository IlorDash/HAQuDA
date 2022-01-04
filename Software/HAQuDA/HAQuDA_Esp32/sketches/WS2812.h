#pragma once

#include <Adafruit_NeoPixel.h>

#define LED_PIN 18
#define LED_NUM_PIXELS 109
#define MAX_BRIGHTNESS 150
#define LED_COLUMN_NUM 9
#define LED_ROW_NUM 12
#define WHITE_BRIGHTNESS_COEFF 70

#define CHRISTMAS_TREE_HALF_WIDTH 3
#define CHRISTMAS_TREE_HEIGHT 8
#define CHRISTMAS_TREE_COLOR 0x0F962D
#define CHRISTMAS_TREE_STAR_COLOR 0xFFFF00

typedef struct {
	uint16_t firstDot;
	uint16_t secondDot;
	uint16_t thirdDot;
} paramsDivideDots;

void WS2812_begin();

void WS2812_setBrightnessPerCent(uint8_t brightness);

void WS2812_clear();
void WS2812_show();
void WS2812_setPixelColor(uint16_t pixelNum, uint32_t color);
void WS2812_fillColor(uint32_t color);

void WS2812_showParams(float data, paramsDivideDots divideDots);

void WS2812_ChristmasTree(int speed);