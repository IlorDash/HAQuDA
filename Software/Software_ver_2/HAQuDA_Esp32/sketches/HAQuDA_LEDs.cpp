#include "HAQuDA_LEDs.h"
#include "HAQuDA_FlashLock.h"

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

// A flash operation can last seconds (format). The frame is skipped after this wait: the LEDs keep the previous picture and
// the frame is sent again by the next Show() or by ResendSkippedFrame().
constexpr uint32_t LED_FRAME_LOCK_TIMEOUT_MS = 500;

// Holds the flash lock only for the ~3.3 ms transmission, never for the delay that follows, so file storage is not starved.
// Returns false when the frame was skipped.
bool HAQuDA_LEDs::showFrame() {
	HAQuDA_FlashLockGuard guard(LED_FRAME_LOCK_TIMEOUT_MS);
	if (!guard.Acquired()) {
		frameSkipped = true;
		return false;
	}
	pixels->show();
	frameSkipped = false;
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

void HAQuDA_LEDs::ResendSkippedFrame() {
	// exchange() claims the retry; a skipped retry sets the flag again inside showFrame().
	if (frameSkipped.exchange(false)) {
		showFrame();
	}
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