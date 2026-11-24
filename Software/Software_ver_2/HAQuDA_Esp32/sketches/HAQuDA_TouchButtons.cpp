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
	queue = xQueueCreateStatic(TOUCH_EVENT_QUEUE_LEN, sizeof(touch_gesture_event), queueStorage, &queueControl);
	if (queue == nullptr) {
		HAQuDA_Logger::LogError("Touch buttons: queue create failed");
		return false;
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

void HAQuDA_TouchButtons::SetRepeatableMask(uint8_t mask) {
	repeatableMask.store(mask);
}

bool HAQuDA_TouchButtons::TakeEvent(touch_gesture_event &event) {
	if (queue == nullptr) {
		return false;
	}
	return xQueueReceive(queue, &event, 0) == pdTRUE;
}

void HAQuDA_TouchButtons::DiscardEvents() {
	if (queue != nullptr) {
		xQueueReset(queue);
	}
	dropped.store(0);
}

uint32_t HAQuDA_TouchButtons::TakeDropped() {
	return dropped.exchange(0);
}

void HAQuDA_TouchButtons::pollCallback(void *arg) {
	static_cast<HAQuDA_TouchButtons *>(arg)->poll();
}

void HAQuDA_TouchButtons::poll() {
	bool levels[TOUCH_BUTTONS_NUM];
	for (uint8_t i = 0; i < TOUCH_BUTTONS_NUM; i++) {
		levels[i] = (digitalRead(TOUCH_PINS[i]) == HIGH);
	}
	const uint8_t mask = repeatableMask.load();
	for (uint8_t i = 0; i < TOUCH_BUTTONS_NUM; i++) {
		poller.SetRepeatable(i, ((mask >> i) & 1) != 0);
	}

	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];
	const uint8_t count = poller.Poll(levels, millis(), events, TOUCH_GESTURE_MAX_EVENTS);
	for (uint8_t i = 0; i < count; i++) {
		// Never block the esp_timer task: a full queue loses the event
		if (xQueueSend(queue, &events[i], 0) != pdTRUE) {
			dropped.fetch_add(1);
		}
	}
}
