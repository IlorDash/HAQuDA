#include "WS2812.h"

Adafruit_NeoPixel *pixels;

void WS2812_begin() {
	pixels = new Adafruit_NeoPixel(LED_NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
	pixels->begin();
	pixels->clear();
	WS2812_show();
	pixels->setBrightness(MAX_BRIGHTNESS);
}

void WS2812_setBrightnessPerCent(uint8_t brightnessPerCent) {
	brightnessPerCent = (brightnessPerCent > 100) ? 100 : brightnessPerCent;
	uint16_t brightness = brightnessPerCent * MAX_BRIGHTNESS / 100;
	pixels->setBrightness(brightness);
	WS2812_show();
}

uint8_t WS2812_getBrightness() {
	return pixels->getBrightness();
}

void WS2812_clear() {
	pixels->clear();
	WS2812_show();
}

void WS2812_show() {
	pixels->show();
	vTaskDelay(200 / portTICK_PERIOD_MS);
}

void WS2812_show(uint16_t delay) {
	pixels->show();
	vTaskDelay(delay / portTICK_PERIOD_MS);
}

void WS2812_setPixelColor(uint16_t pixelNum, uint32_t color) {
	pixels->setPixelColor(pixelNum, color);
}

void WS2812_fillColor(uint32_t color) {
	pixels->fill(color, 0, LED_NUM_PIXELS);
	WS2812_show();
}

void WS2812_fillColor(uint32_t color, uint8_t startPixel, uint8_t numPixels) {
	pixels->fill(color, startPixel, numPixels);
	WS2812_show();
}

uint32_t WS2812_getColor(uint8_t red, uint8_t green, uint8_t blue) {
	return pixels->Color(red, green, blue);
}