#pragma once

#include <mutex>

struct portMUX_TYPE {
	std::mutex lock;
};

#define portMUX_INITIALIZER_UNLOCKED portMUX_TYPE()
#define portENTER_CRITICAL(mux) ((mux)->lock.lock())
#define portEXIT_CRITICAL(mux) ((mux)->lock.unlock())
