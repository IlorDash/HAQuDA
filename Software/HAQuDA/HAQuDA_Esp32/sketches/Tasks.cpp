#include "Tasks.h"

TaskHandle_t WS2812_SnakeTask;
TaskHandle_t WS2812_RandomTask;
TaskHandle_t WS2812_FadeTask;
TaskHandle_t WS2812_ChristmasTask;

void WS2812_Snake_taskCode(void *parameter) {
	while (1) {
		if (whatEffectDisp == snake) {
			WS2812_Snake(400, 5, COLOR_LIME);
		}
	}
}

void WS2812_Random_taskCode(void *parameter) {
	while (1) {
		if (whatEffectDisp == randomPixel) {
			WS2812_Random(400);
		}
	}
}

void WS2812_Fade_taskCode(void *parameter) {
	while (1) {
		if (whatEffectDisp == fade) {
			WS2812_Fade(400, 5, COLOR_AQUA);
		}
	}
}

void WS2812_ChristmasTree_taskCode(void *parameter) {
	while (1) {
		if (whatEffectDisp == christmasTree) {
			WS2812_ChristmasTree(400);
		}
	}
}

void createTasks() {
	xTaskCreate(WS2812_Snake_taskCode, // Function that should be called
				"WS2812 Snake task",   // Name of the task (for debugging)
				1000,				   // Stack size (bytes)
				NULL,				   // Parameter to pass
				1,					   // Task priority
				&WS2812_SnakeTask			 // Task handle
	);

	xTaskCreate(WS2812_Random_taskCode, // Function that should be called
				"WS2812 Random task",   // Name of the task (for debugging)
				1000,					// Stack size (bytes)
				NULL,					// Parameter to pass
				1,						// Task priority
				&WS2812_RandomTask			 // Task handle
	);

	xTaskCreate(WS2812_Fade_taskCode, // Function that should be called
				"WS2812 Fade task",   // Name of the task (for debugging)
				1000,				  // Stack size (bytes)
				NULL,				  // Parameter to pass
				1,					  // Task priority
				&WS2812_FadeTask			 // Task handle
	);

	xTaskCreate(WS2812_ChristmasTree_taskCode, // Function that should be called
				"WS2812 Christmas task",	   // Name of the task (for debugging)
				1000,						   // Stack size (bytes)
				NULL,						   // Parameter to pass
				1,							   // Task priority
				&WS2812_ChristmasTask			 // Task handle
	);
}

void deleteTasks() {
	if (WS2812_SnakeTask != NULL) {
		vTaskDelete(WS2812_SnakeTask);
	}

	if (WS2812_RandomTask != NULL) {
		vTaskDelete(WS2812_RandomTask);
	}

	if (WS2812_FadeTask != NULL) {
		vTaskDelete(WS2812_FadeTask);
	}

	if (WS2812_ChristmasTask != NULL) {
		vTaskDelete(WS2812_ChristmasTask);
	}
}