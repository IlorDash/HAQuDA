#include "HAQuDA_LEDs.h"

Adafruit_NeoPixel *pixels;

HAQuDA_LEDs *HAQuDA_LEDs::p_instance = 0;

HAQuDA_LEDs *HAQuDA_LEDs::getInstance() {
	if (!p_instance) {
		p_instance = new HAQuDA_LEDs();
	}
	return p_instance;
}

void HAQuDA_LEDs::Begin() {
	pixels = new Adafruit_NeoPixel(LED_NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
	pixels->begin();
	pixels->clear();
	HAQuDA_LEDs::Show();
	pixels->setBrightness(DEFAULT_BRIGHTNESS);
}

void HAQuDA_LEDs::SetBright(uint8_t bright_per_cent) {
	if (bright_per_cent > 100) {
		bright_per_cent = 100;
	}
	uint16_t brightness = bright_per_cent * MAX_BRIGHTNESS / 100;
	pixels->setBrightness(brightness);
	HAQuDA_LEDs::Show();
}

uint8_t HAQuDA_LEDs::GetBright() {
	return pixels->getBrightness();
}

void HAQuDA_LEDs::Clear() {
	pixels->clear();
	HAQuDA_LEDs::Show();
}

void HAQuDA_LEDs::Show() {
	pixels->show();
	vTaskDelay(200 / portTICK_PERIOD_MS);
}

void HAQuDA_LEDs::Show(uint16_t delay) {
	pixels->show();
	vTaskDelay(delay / portTICK_PERIOD_MS);
}

void HAQuDA_LEDs::SetPixelColor(uint16_t pixelNum, uint32_t color) {
	pixels->setPixelColor(pixelNum, color);
}

void HAQuDA_LEDs::FillColor(uint32_t color) {
	pixels->fill(color, 0, LED_NUM_PIXELS);
}

void HAQuDA_LEDs::FillColor(uint8_t r, uint8_t g, uint8_t b) {
	uint32_t color = HAQuDA_LEDs::GetColor(r, g, b);
	pixels->fill(color, 0, LED_NUM_PIXELS);
}

void HAQuDA_LEDs::FillColorFrom(uint32_t color, uint8_t startPixel, uint8_t numPixels) {
	pixels->fill(color, startPixel, numPixels);
}

void HAQuDA_LEDs::FillColorFrom(uint8_t r, uint8_t g, uint8_t b, uint8_t startPixel, uint8_t numPixels) {
	uint32_t color = HAQuDA_LEDs::GetColor(r, g, b);
	pixels->fill(color, startPixel, numPixels);
}

uint32_t HAQuDA_LEDs::GetColor(uint8_t r, uint8_t g, uint8_t b) {
	return pixels->Color(r, g, b);
}