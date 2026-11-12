#include "HAQuDA_FlashLock.h"

static SemaphoreHandle_t flashMutex = nullptr;

void HAQuDA_FlashLock::Create() {
	if (flashMutex == nullptr) {
		flashMutex = xSemaphoreCreateMutex();
	}
}

bool HAQuDA_FlashLock::Take(uint32_t timeoutMs) {
	if (flashMutex == nullptr) {
		return true;
	}
	return xSemaphoreTake(flashMutex, pdMS_TO_TICKS(timeoutMs)) == pdTRUE;
}

void HAQuDA_FlashLock::Give() {
	if (flashMutex != nullptr) {
		xSemaphoreGive(flashMutex);
	}
}

bool HAQuDA_FlashLock::Exists() {
	return flashMutex != nullptr;
}
