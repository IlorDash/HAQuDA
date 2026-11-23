#pragma once

#include <cstdint>

#include "esp_timer.h"

#include "HAQuDA_TouchPoller.h"

constexpr uint32_t TOUCH_POLL_PERIOD_MS = 10;

class HAQuDA_TouchButtons {
  public:
	// Loop task, once
	bool Begin();
	// Loop task only: the poll callback only increments the counters, only the loop takes and resets them
	uint8_t TakePresses(uint8_t index);

  private:
	static void pollCallback(void *arg);
	void poll();

	HAQuDA_TouchPoller poller;
	esp_timer_handle_t timer = nullptr;
};
