#include "HAQuDA_LEDs.h"

HAQuDA_LEDs *HAQuDA_LEDs::p_instance = 0;

// A frame is ~3.6 ms on the wire (327 bytes x 10 us + 300 us reset) and every Show() is followed by a delay, so the previous
// frame has long left the DMA. The wait only guards against a stuck I2S interrupt.
constexpr uint32_t LED_FRAME_WAIT_MS = 50;

HAQuDA_LEDs *HAQuDA_LEDs::getInstance() {
	if (!p_instance) {
		p_instance = new HAQuDA_LEDs();
	}
	return p_instance;
}

void HAQuDA_LEDs::Begin() {
	bus = new LedBus(LED_NUM_PIXELS, LED_PIN);
	bus->Begin();
	HAQuDA_LEDs::Show();
	brightness = DEFAULT_BRIGHTNESS + 1;
}

void HAQuDA_LEDs::SetBright(uint8_t bright_per_cent) {
	if (bright_per_cent > 100) {
		bright_per_cent = 100;
	}
	uint16_t raw = bright_per_cent * MAX_BRIGHTNESS / 100;
	uint8_t newBrightness = raw + 1;
	if (newBrightness != brightness) {
		// Same lossy rescale of the stored picture as Adafruit_NeoPixel::setBrightness().
		uint8_t oldBrightness = brightness - 1;
		uint16_t factor = 0;
		if (oldBrightness == 0) {
			factor = 0;
		} else if (raw == 255) {
			factor = 65535 / oldBrightness;
		} else {
			factor = (((uint16_t)newBrightness << 8) - 1) / oldBrightness;
		}
		uint8_t *data = bus->Pixels();
		size_t dataSize = bus->PixelsSize();
		for (size_t i = 0; i < dataSize; i++) {
			data[i] = (data[i] * factor) >> 8;
		}
		bus->Dirty();
		brightness = newBrightness;
	}
	HAQuDA_LEDs::Show();
}

uint8_t HAQuDA_LEDs::GetBright() {
	return brightness - 1;
}

void HAQuDA_LEDs::Clear() {
	memset(bus->Pixels(), 0, bus->PixelsSize());
	bus->Dirty();
	HAQuDA_LEDs::Show();
}

bool HAQuDA_LEDs::showFrame() {
	uint32_t startMs = millis();
	while (!bus->CanShow()) {
		if (millis() - startMs > LED_FRAME_WAIT_MS) {
			return false;
		}
		vTaskDelay(1);
	}
	// Dirty first: NeoPixelBus skips the send of an unchanged picture, the earlier driver always sent it.
	bus->Dirty();
	bus->Show();
	return true;
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
	if (brightness == 0) {
		return channel;
	}
	return (channel * brightness) >> 8;
}

void HAQuDA_LEDs::SetPixelColor(uint16_t pixelNum, uint32_t color) {
	bus->SetPixelColor(pixelNum, RgbColor(scale(color >> 16), scale(color >> 8), scale(color)));
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
