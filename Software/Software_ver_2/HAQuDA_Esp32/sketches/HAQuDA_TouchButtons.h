#pragma once

#include <atomic>
#include <cstdint>

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "HAQuDA_TouchPoller.h"

constexpr uint32_t TOUCH_POLL_PERIOD_MS = 10;
constexpr uint8_t TOUCH_EVENT_QUEUE_LEN = 16;

class HAQuDA_TouchButtons {
  public:
	// Loop task, once
	bool Begin();

	// Loop task only. Bit i is button i (pads 2, 6, 8, 9); the timer task applies it before every poll.
	void SetRepeatableMask(uint8_t mask);

	// Loop task only
	bool TakeEvent(touch_gesture_event &event);

	// Loop task only: drops the queued events and the drop count without acting on them
	void DiscardEvents();

	// Loop task only: events the full queue refused since the last call
	uint32_t TakeDropped();

  private:
	static void pollCallback(void *arg);
	void poll();

	HAQuDA_TouchPoller poller;
	esp_timer_handle_t timer = nullptr;
	QueueHandle_t queue = nullptr;
	StaticQueue_t queueControl;
	uint8_t queueStorage[TOUCH_EVENT_QUEUE_LEN * sizeof(touch_gesture_event)];
	std::atomic<uint8_t> repeatableMask{0};
	std::atomic<uint32_t> dropped{0};
};
