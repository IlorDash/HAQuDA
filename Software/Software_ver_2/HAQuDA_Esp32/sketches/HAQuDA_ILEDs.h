#pragma once

#include <cstdint>

// Hardware seam of the WS2812B matrix. Implemented by HAQuDA_LEDs, mocked on the PC.
// Frames are sent by ShowTask only (and by setup() before ShowTask exists); the implementation does not serialise sends.
class HAQuDA_ILEDs {
  public:
	virtual ~HAQuDA_ILEDs() = default;

	// Sets brightness in percent of MAX_BRIGHTNESS (0..100) and sends the picture without waiting. ShowTask.
	virtual void SetBright(uint8_t brightPerCent) = 0;
	// Returns the raw brightness (0..MAX_BRIGHTNESS). ShowTask or loop.
	virtual uint8_t GetBright() = 0;

	// Clears the buffer and shows it. ShowTask.
	virtual void Clear() = 0;
	// Pushes the buffer to the strip, then waits 200 ms. ShowTask.
	virtual void Show() = 0;
	// Pushes the buffer to the strip, then waits delayMs. ShowTask.
	virtual void Show(uint16_t delayMs) = 0;

	// Sets one pixel (index 0..LED_NUM_PIXELS-1) to a packed 0xRRGGBB colour in the buffer. ShowTask.
	virtual void SetPixelColor(uint16_t pixelNum, uint32_t color) = 0;

	// Fills all pixels in the buffer with a packed colour. ShowTask.
	virtual void FillColor(uint32_t color) = 0;
	// Fills all pixels in the buffer with r, g, b channels (0..255). ShowTask.
	virtual void FillColor(uint8_t r, uint8_t g, uint8_t b) = 0;
	// Fills numPixels pixels from startPixel with a packed colour in the buffer. ShowTask.
	virtual void FillColorFrom(uint32_t color, uint8_t startPixel, uint8_t numPixels) = 0;
	// Fills numPixels pixels from startPixel with r, g, b channels (0..255) in the buffer. ShowTask.
	virtual void FillColorFrom(uint8_t r, uint8_t g, uint8_t b, uint8_t startPixel, uint8_t numPixels) = 0;
	// Packs r, g, b channels (0..255) into the strip's colour word. Any task.
	virtual uint32_t GetColor(uint8_t r, uint8_t g, uint8_t b) = 0;
};
