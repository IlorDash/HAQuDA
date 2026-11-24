#pragma once

#include "freertos/FreeRTOS.h"

class HAQuDA_CriticalLock {
  public:
	explicit HAQuDA_CriticalLock(portMUX_TYPE &muxToLock) : mux(muxToLock) {
		portENTER_CRITICAL(&mux);
	}
	~HAQuDA_CriticalLock() {
		portEXIT_CRITICAL(&mux);
	}
	HAQuDA_CriticalLock(const HAQuDA_CriticalLock &) = delete;
	HAQuDA_CriticalLock &operator=(const HAQuDA_CriticalLock &) = delete;

  private:
	portMUX_TYPE &mux;
};
