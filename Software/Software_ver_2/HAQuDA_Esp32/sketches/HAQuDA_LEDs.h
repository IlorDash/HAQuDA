#pragma once

#include <NeoPixelBus.h>
#include <atomic>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "HAQuDA_LEDsParams.h"
#include "HAQuDA_ILEDs.h"

// The LED chain is driven by the I2S1 peripheral through DMA (NeoPixelBus): a frame is copied into a DMA buffer in internal RAM
// and then clocked out by the hardware alone, from descriptors that stay in RAM. An SPI flash access (SPIFFS, WiFi NVS / PHY
// calibration) switches the flash cache off on both cores, but that does not disturb the bits on the wire: the I2S interrupt
// that ends the frame runs from IRAM, and if it is late the DMA only repeats the same frame.
//
// Frames are sent by ShowTask (and by setup() before ShowTask exists). The class does not serialise sends: a send from another
// task while ShowTask runs is not supported, because NeoPixelBus can tear a frame if two tasks pass CanShow() together.
// A frame send waits up to about 50 ms for the previous DMA frame and is dropped after that; the next Show() repaints the
// picture. The picture buffer is not locked either; only GetBright() may be called from the loop task.
//
// Brightness uses the same scheme as Adafruit_NeoPixel (stored value = raw + 1, colours scaled when they are written, the
// stored picture rescaled when the brightness changes).
class HAQuDA_LEDs : public HAQuDA_ILEDs {

	using LedBus = NeoPixelBus<NeoGrbFeature, NeoEsp32I2s1Ws2812xMethod>;

	static HAQuDA_LEDs *p_instance;

	// The constructor allocates the pixel buffer and the DMA buffer; it runs on the first getInstance(), which is the global
	// display service's constructor argument, before setup(). It touches no peripheral: the DMA descriptors and the I2S
	// interrupt are allocated later, in Begin().
	LedBus bus{LED_NUM_PIXELS, LED_PIN};

	// Raw brightness + 1: 0 (before the first SetBright) means "no scaling", 1 means off. Written by ShowTask (SetBright),
	// read by the loop task through GetBright().
	std::atomic<uint8_t> brightness{0};

	// Sends the buffer after the previous frame has left the DMA. Returns false when the DMA did not become free in time and
	// the frame was lost. ShowTask.
	bool showFrame();
	uint8_t scale(uint8_t channel) const;

	HAQuDA_LEDs() {};
	HAQuDA_LEDs(const HAQuDA_LEDs *);
	HAQuDA_LEDs *operator=(HAQuDA_LEDs *);
	~HAQuDA_LEDs() {};

  public:
	static HAQuDA_LEDs *getInstance();

	// Starts I2S1 on LED_PIN, shows an empty picture and sets DEFAULT_BRIGHTNESS. Call once from setup() (loop task) before
	// ShowTask exists. Allocates the DMA descriptors and the I2S interrupt; the pixel and DMA buffers come from the
	// constructor. All of it runs once in setup(), none at runtime.
	void Begin();

	// Sets brightness in percent (0..100) of MAX_BRIGHTNESS, rescales the picture and sends it without waiting. ShowTask.
	void SetBright(uint8_t bright_per_cent) override;
	// Returns the raw brightness 0..150 (not percent). ShowTask or loop.
	uint8_t GetBright() override;

	// Clears the picture and shows it. Blocks 200 ms. ShowTask.
	void Clear() override;
	// Sends the picture and then blocks 200 ms (the caller's pacing). A frame that cannot be sent in time is lost. ShowTask.
	void Show() override;
	void Show(uint16_t delay) override;

	void SetPixelColor(uint16_t pixelNum, uint32_t color) override;

	void FillColor(uint32_t color) override;
	void FillColor(uint8_t r, uint8_t g, uint8_t b) override;
	void FillColorFrom(uint32_t color, uint8_t startPixel, uint8_t numPixels) override;
	void FillColorFrom(uint8_t r, uint8_t g, uint8_t b, uint8_t startPixel, uint8_t numPixels) override;
	uint32_t GetColor(uint8_t r, uint8_t g, uint8_t b) override;
};
