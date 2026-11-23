#pragma once

#include <atomic>
#include <cstdint>

class HAQuDA_PressCounter {
  public:
	// esp_timer task only, single caller. Compare-exchange keeps a concurrent Take() from being undone
	// and stops at 255 instead of wrapping.
	void Add() {
		uint8_t count = value.load();
		while ((count < UINT8_MAX) && !value.compare_exchange_weak(count, static_cast<uint8_t>(count + 1))) {
		}
	}

	// Loop task only
	uint8_t Take() {
		return value.exchange(0);
	}

  private:
	std::atomic<uint8_t> value{0};
};
