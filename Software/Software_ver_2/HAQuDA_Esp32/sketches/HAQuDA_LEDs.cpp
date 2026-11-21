#include "HAQuDA_LEDs.h"

HAQuDA_LEDs *HAQuDA_LEDs::p_instance = 0;

// A frame is ~3.6 ms on the wire (327 bytes x 10 us + 300 us reset) and every Show() is followed by a delay, so the previous
// frame has long left the DMA. The wait only guards against a stuck I2S interrupt.
constexpr uint32_t LED_FRAME_WAIT_MS = 50;

// The wait loop in showFrame() must sleep at least one tick (the tick rate is 1000 Hz on this core).
static_assert(pdMS_TO_TICKS(1) >= 1, "FreeRTOS tick is slower than 1 ms, the frame wait would spin");

HAQuDA_LEDs *HAQuDA_LEDs::getInstance() {
	if (!p_instance) {
		p_instance = new HAQuDA_LEDs();
	}
	return p_instance;
}

void HAQuDA_LEDs::Begin() {
	bus.Begin();
	HAQuDA_LEDs::Show();
	// Stored directly instead of through SetBright(): the picture is empty, so there is nothing to rescale, and SetBright()
	// would add a second 200 ms Show().
	brightness.store(HAQuDA_BrightPercentToRaw(DEFAULT_BRIGHTNESS) + 1);
	brightPerCent.store(DEFAULT_BRIGHTNESS);
}

void HAQuDA_LEDs::SetBright(uint8_t bright_per_cent) {
	uint8_t raw = HAQuDA_BrightPercentToRaw(bright_per_cent);
	uint8_t newBrightness = raw + 1;
	uint8_t currentBrightness = brightness.load();
	if (newBrightness != currentBrightness) {
		// Same lossy rescale of the stored picture as Adafruit_NeoPixel::setBrightness().
		uint8_t oldBrightness = currentBrightness - 1;
		uint16_t factor = 0;
		if (oldBrightness == 0) {
			factor = 0;
		} else if (raw == 255) {
			factor = 65535 / oldBrightness;
		} else {
			factor = (((uint16_t)newBrightness << 8) - 1) / oldBrightness;
		}
		uint8_t *data = bus.Pixels();
		size_t dataSize = bus.PixelsSize();
		for (size_t i = 0; i < dataSize; i++) {
			data[i] = (data[i] * factor) >> 8;
		}
		bus.Dirty();
		brightness.store(newBrightness);
	}
	brightPerCent.store(bright_per_cent > 100 ? 100 : bright_per_cent);
	// The animations count the 200 ms of the old blocking Show() in the wait they return from Step(), so waiting here as well would double it
	showFrame();
}

uint8_t HAQuDA_LEDs::GetBright() {
	return brightPerCent.load();
}

void HAQuDA_LEDs::Clear() {
	memset(bus.Pixels(), 0, bus.PixelsSize());
	bus.Dirty();
	HAQuDA_LEDs::Show();
}

// A frame that cannot be sent within its wait is lost, without a log (ShowTask must not log); the next Show() sends the
// picture again.
bool HAQuDA_LEDs::showFrame() {
	uint32_t startMs = millis();
	while (!bus.CanShow() && (millis() - startMs <= LED_FRAME_WAIT_MS)) {
		vTaskDelay(pdMS_TO_TICKS(1));
	}
	bool sent = bus.CanShow();
	if (sent) {
		// Dirty first: NeoPixelBus skips the send of an unchanged picture, but every Show() must refresh the strip.
		bus.Dirty();
		bus.Show();
	}
	return sent;
}

void HAQuDA_LEDs::Show() {
	showFrame();
	vTaskDelay(pdMS_TO_TICKS(200));
}

void HAQuDA_LEDs::Show(uint16_t delay) {
	showFrame();
	vTaskDelay(pdMS_TO_TICKS(delay));
}

// Brightness 0 (stored value before the first SetBright) leaves the channel unscaled.
uint8_t HAQuDA_LEDs::scale(uint8_t channel) const {
	uint8_t currentBrightness = brightness.load();
	if (currentBrightness == 0) {
		return channel;
	}
	return (channel * currentBrightness) >> 8;
}

void HAQuDA_LEDs::SetPixelColor(uint16_t pixelNum, uint32_t color) {
	bus.SetPixelColor(pixelNum, RgbColor(scale(color >> 16), scale(color >> 8), scale(color)));
}

void HAQuDA_LEDs::FillColor(uint32_t color) {
	HAQuDA_LEDs::FillColorFrom(color, 0, LED_NUM_PIXELS);
}

void HAQuDA_LEDs::FillColor(uint8_t r, uint8_t g, uint8_t b) {
	HAQuDA_LEDs::FillColorFrom(HAQuDA_LEDs::GetColor(r, g, b), 0, LED_NUM_PIXELS);
}

// Clipped at the end of the strip (showMeas_multi() relies on it); numPixels 0 fills to the end.
void HAQuDA_LEDs::FillColorFrom(uint32_t color, uint8_t startPixel, uint8_t numPixels) {
	if (startPixel >= LED_NUM_PIXELS) {
		return;
	}
	uint16_t end = LED_NUM_PIXELS;
	if (numPixels != 0 && startPixel + numPixels < LED_NUM_PIXELS) {
		end = startPixel + numPixels;
	}
	for (uint16_t i = startPixel; i < end; i++) {
		HAQuDA_LEDs::SetPixelColor(i, color);
	}
}

void HAQuDA_LEDs::FillColorFrom(uint8_t r, uint8_t g, uint8_t b, uint8_t startPixel, uint8_t numPixels) {
	HAQuDA_LEDs::FillColorFrom(HAQuDA_LEDs::GetColor(r, g, b), startPixel, numPixels);
}

uint32_t HAQuDA_LEDs::GetColor(uint8_t r, uint8_t g, uint8_t b) {
	return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}
