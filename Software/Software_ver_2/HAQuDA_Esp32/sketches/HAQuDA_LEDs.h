#pragma once

#include <Adafruit_NeoPixel.h>
#include "HAQuDA_LEDsParams.h"

class HAQuDA_LEDs_Singleton;

class HAQuDA_LEDs_SingletonDestroyer {
  private:
	HAQuDA_LEDs_Singleton *p_instance;

  public:
	~HAQuDA_LEDs_SingletonDestroyer();
	void initialize(HAQuDA_LEDs_Singleton *p);
};

class HAQuDA_LEDs_Singleton {

	static HAQuDA_LEDs_Singleton *p_instance;
	static HAQuDA_LEDs_SingletonDestroyer destroyer;

  protected:
	HAQuDA_LEDs_Singleton(){};
	HAQuDA_LEDs_Singleton(const HAQuDA_LEDs_Singleton *);
	~HAQuDA_LEDs_Singleton(){};
	friend class HAQuDA_LEDs_SingletonDestroyer;

  public:
	static HAQuDA_LEDs_Singleton *getInstance();

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