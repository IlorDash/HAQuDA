#pragma once

#include <Adafruit_NeoPixel.h>
#include "HAQuDA_LEDsParams.h"
#include "HAQuDA_ILEDs.h"

class HAQuDA_LEDs : public HAQuDA_ILEDs {

	static HAQuDA_LEDs *p_instance;

	HAQuDA_LEDs() {};
	HAQuDA_LEDs(const HAQuDA_LEDs *);
	HAQuDA_LEDs *operator=(HAQuDA_LEDs *);
	~HAQuDA_LEDs() {};

  public:
	static HAQuDA_LEDs *getInstance();

	void Begin();

	void SetBright(uint8_t bright_per_cent) override;
	uint8_t GetBright() override;

	void Clear() override;
	void Show() override;
	void Show(uint16_t delay) override;

	void SetPixelColor(uint16_t pixelNum, uint32_t color) override;

	void FillColor(uint32_t color) override;
	void FillColor(uint8_t r, uint8_t g, uint8_t b) override;
	void FillColorFrom(uint32_t color, uint8_t startPixel, uint8_t numPixels) override;
	void FillColorFrom(uint8_t r, uint8_t g, uint8_t b, uint8_t startPixel, uint8_t numPixels) override;
	uint32_t GetColor(uint8_t r, uint8_t g, uint8_t b) override;
};