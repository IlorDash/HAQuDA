#include "Tasks.h"

void ShowTask(void *parameter) {
	static_cast<HAQuDA_DisplayService *>(parameter)->RunTask();
}

void createTasks(HAQuDA_DisplayService *display, TaskHandle_t *showTaskHandle) {

	xTaskCreatePinnedToCore(ShowTask, "WS2812 Effects task", 3000, (void *)display, 1, showTaskHandle, 0);
}
