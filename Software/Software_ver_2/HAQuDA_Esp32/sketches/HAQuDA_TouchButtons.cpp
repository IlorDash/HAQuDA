#include "Arduino.h"

#include "Board.h"
#include "HAQuDA_Logger.h"
#include "HAQuDA_TouchButtons.h"

namespace {
	constexpr uint8_t TOUCH_PINS[TOUCH_BUTTONS_NUM] = {TOUCH_2_PIN, TOUCH_6_PIN, TOUCH_8_PIN, TOUCH_9_PIN};
}

bool HAQuDA_TouchButtons::Begin() {
	if (timer != nullptr) {
		return true;
	}
	for (uint8_t i = 0; i < TOUCH_BUTTONS_NUM; i++) {
		pinMode(TOUCH_PINS[i], INPUT_PULLDOWN);
	}

	esp_timer_create_args_t args = {};
	args.callback = &HAQuDA_TouchButtons::pollCallback;
	args.arg = this;
	args.dispatch_method = ESP_TIMER_TASK;
	args.name = "touch_poll";
	if (esp_timer_create(&args, &timer) != ESP_OK) {
		timer = nullptr;
		HAQuDA_Logger::LogError("Touch buttons: timer create failed");
		return false;
	}
	if (esp_timer_start_periodic(timer, TOUCH_POLL_PERIOD_MS * 1000ULL) != ESP_OK) {
		esp_timer_delete(timer);
		timer = nullptr;
		HAQuDA_Logger::LogError("Touch buttons: timer start failed");
		return false;
	}
	return true;
}

uint8_t HAQuDA_TouchButtons::TakePresses(uint8_t index) {
	return poller.Take(index);
}

void HAQuDA_TouchButtons::pollCallback(void *arg) {
	static_cast<HAQuDA_TouchButtons *>(arg)->poll();
}

void HAQuDA_TouchButtons::poll() {
	bool levels[TOUCH_BUTTONS_NUM];
	for (uint8_t i = 0; i < TOUCH_BUTTONS_NUM; i++) {
		levels[i] = (digitalRead(TOUCH_PINS[i]) == HIGH);
	}
	poller.Poll(levels, millis());
}
