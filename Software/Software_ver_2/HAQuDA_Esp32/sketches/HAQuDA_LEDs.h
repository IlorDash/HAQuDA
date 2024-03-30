#pragma once

#include <Adafruit_NeoPixel.h>
#include "HAQuDA_LEDsParams.h"

class HAQuDA_LEDs {

	static HAQuDA_LEDs *p_instance;

	HAQuDA_LEDs(){};
	HAQuDA_LEDs(const HAQuDA_LEDs *);
	HAQuDA_LEDs *operator=(HAQuDA_LEDs *);
	~HAQuDA_LEDs(){};

  public:
	static HAQuDA_LEDs *getInstance();

	void Begin();

	void SetBright(uint8_t bright_per_cent);
	uint8_t GetBright();

	void Clear();
	void Show();
	void Show(uint16_t delay);

	void SetPixelColor(uint16_t pixelNum, uint32_t color);

	void FillColor(uint32_t color);
	void FillColor(uint8_t r, uint8_t g, uint8_t b);
	void FillColorFrom(uint32_t color, uint8_t startPixel, uint8_t numPixels);
	void FillColorFrom(uint8_t r, uint8_t g, uint8_t b, uint8_t startPixel, uint8_t numPixels);
	uint32_t GetColor(uint8_t r, uint8_t g, uint8_t b);
};