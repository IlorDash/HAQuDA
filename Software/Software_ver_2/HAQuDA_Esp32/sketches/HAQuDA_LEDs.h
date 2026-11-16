#pragma once

#include <NeoPixelBus.h>
#include "HAQuDA_LEDsParams.h"
#include "HAQuDA_ILEDs.h"

// The LED chain is driven by the I2S1 peripheral through DMA (NeoPixelBus): a frame is copied into a DMA buffer in internal RAM
// and then clocked out by the hardware alone, from descriptors that stay in RAM. An SPI flash access (SPIFFS, WiFi NVS / PHY
// calibration) switches the flash cache off on both cores, but that no longer disturbs the bits on the wire: the I2S interrupt
// that ends the frame runs from IRAM, and if it is late the DMA only repeats the same frame. The earlier RMT driver refilled
// its 64-item buffer from an interrupt that is not in IRAM, so a cache-off period corrupted the frame.
//
// Brightness keeps the Adafruit scheme (stored value = raw + 1, colours scaled when they are written, the stored picture
// rescaled when the brightness changes), so pictures look exactly as before.
class HAQuDA_LEDs : public HAQuDA_ILEDs {

	using LedBus = NeoPixelBus<NeoGrbFeature, NeoEsp32I2s1Ws2812xMethod>;

	static HAQuDA_LEDs *p_instance;

	LedBus *bus = nullptr;

	// Raw brightness + 1, as Adafruit stores it: 0 (before the first SetBright) means "no scaling", 1 means off.
	uint8_t brightness = 0;

	// Waits (yielding, bounded) for the previous frame to leave the DMA, then sends the buffer. Returns false when the wait
	// timed out and the frame was dropped.
	bool showFrame();
	uint8_t scale(uint8_t channel) const;

	HAQuDA_LEDs() {};
	HAQuDA_LEDs(const HAQuDA_LEDs *);
	HAQuDA_LEDs *operator=(HAQuDA_LEDs *);
	~HAQuDA_LEDs() {};

  public:
	static HAQuDA_LEDs *getInstance();

	// Allocates the DMA buffer, starts I2S1 on LED_PIN, shows an empty picture and sets DEFAULT_BRIGHTNESS. Call once from
	// setup() (loop task) before ShowTask exists; the only heap allocation of this class.
	void Begin();

	// Sets brightness in percent (0..100) of MAX_BRIGHTNESS, rescales the picture and shows it. Blocks 200 ms. ShowTask or loop.
	void SetBright(uint8_t bright_per_cent) override;
	// Returns the raw brightness 0..150 (not percent). ShowTask or loop.
	uint8_t GetBright() override;

	// Clears the picture and shows it. Blocks 200 ms. ShowTask or loop.
	void Clear() override;
	// Sends the picture and then blocks 200 ms (delay) so the caller's pacing stays the same as before. ShowTask or loop.
	void Show() override;
	void Show(uint16_t delay) override;

	void SetPixelColor(uint16_t pixelNum, uint32_t color) override;

	void FillColor(uint32_t color) override;
	void FillColor(uint8_t r, uint8_t g, uint8_t b) override;
	void FillColorFrom(uint32_t color, uint8_t startPixel, uint8_t numPixels) override;
	void FillColorFrom(uint8_t r, uint8_t g, uint8_t b, uint8_t startPixel, uint8_t numPixels) override;
	uint32_t GetColor(uint8_t r, uint8_t g, uint8_t b) override;
};
