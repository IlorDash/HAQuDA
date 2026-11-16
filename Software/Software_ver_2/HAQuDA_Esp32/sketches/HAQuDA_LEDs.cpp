#include "HAQuDA_LEDs.h"

HAQuDA_LEDs *HAQuDA_LEDs::p_instance = 0;

// A frame is ~3.6 ms on the wire (327 bytes x 10 us + 300 us reset) and every Show() is followed by a delay, so the previous
// frame has long left the DMA. The wait only guards against a stuck I2S interrupt.
constexpr uint32_t LED_FRAME_WAIT_MS = 50;

// Another task is sending a frame for ~4 ms at most, so this wait is normally far shorter.
constexpr uint32_t LED_OUTPUT_LOCK_WAIT_MS = 50;

HAQuDA_LEDs *HAQuDA_LEDs::getInstance() {
	if (!p_instance) {
		p_instance = new HAQuDA_LEDs();
	}
	return p_instance;
}

void HAQuDA_LEDs::Begin() {
	outputMutex = xSemaphoreCreateMutex();
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

// A frame that cannot be sent within its waits is lost, without a log (ShowTask must not log); the next Show() sends the
// picture again. The mutex is released before the caller's delay, so it is held for one frame at most.
bool HAQuDA_LEDs::showFrame() {
	if ((outputMutex != nullptr) && (xSemaphoreTake(outputMutex, pdMS_TO_TICKS(LED_OUTPUT_LOCK_WAIT_MS)) != pdTRUE)) {
		return false;
	}

	uint32_t startMs = millis();
	while (!bus->CanShow() && (millis() - startMs <= LED_FRAME_WAIT_MS)) {
		vTaskDelay(1);
	}
	bool sent = bus->CanShow();
	if (sent) {
		// Dirty first: NeoPixelBus skips the send of an unchanged picture, but every Show() must refresh the strip.
		bus->Dirty();
		bus->Show();
	}

	if (outputMutex != nullptr) {
		xSemaphoreGive(outputMutex);
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
