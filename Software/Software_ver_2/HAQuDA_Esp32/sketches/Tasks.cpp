#include "Tasks.h"
#include "HAQuDA_ErrorHandler.h"

void ShowTask(void *parameter) {

	HAQuDA_DisplayManip *dispManip = (HAQuDA_DisplayManip *)parameter;
	int show_meas_time = 0;

	while (true)
		dispManip->Show(&show_meas_time);
}

void createTasks(HAQuDA_DisplayManip *dispManip) {

	xTaskCreatePinnedToCore(ShowTask, "WS2812 Effects task", 3000, (void *)dispManip, 1, NULL, 0);
}