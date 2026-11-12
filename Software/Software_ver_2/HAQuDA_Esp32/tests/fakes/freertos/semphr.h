#pragma once

// Host stand-in for the FreeRTOS mutex API: one fake mutex with no real blocking. Taking a mutex that is already
// held fails at once, as a take that ran into its timeout would. FakeFreeRTOS::Reset() clears the state.
#include "FreeRTOS.h"

struct FakeSemaphore {
	bool held = false;
	int takeCalls = 0;
	int giveCalls = 0;
	TickType_t lastTakeTicks = 0;
};

struct FakeFreeRTOS {
	static inline FakeSemaphore mutex;
	static inline bool failCreate = false;
	static inline int createCalls = 0;

	static void Reset() {
		mutex = FakeSemaphore();
		failCreate = false;
		createCalls = 0;
	}
};

typedef FakeSemaphore *SemaphoreHandle_t;

inline SemaphoreHandle_t xSemaphoreCreateMutex() {
	FakeFreeRTOS::createCalls++;
	return FakeFreeRTOS::failCreate ? nullptr : &FakeFreeRTOS::mutex;
}

inline BaseType_t xSemaphoreTake(SemaphoreHandle_t handle, TickType_t ticks) {
	handle->takeCalls++;
	handle->lastTakeTicks = ticks;
	if (handle->held) {
		return pdFALSE;
	}
	handle->held = true;
	return pdTRUE;
}

inline BaseType_t xSemaphoreGive(SemaphoreHandle_t handle) {
	handle->giveCalls++;
	handle->held = false;
	return pdTRUE;
}
