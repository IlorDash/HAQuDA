#include "Tasks.h"
#include "HAQuDA_ErrorHandler.h"

#define BUILTIN_LED 2

void BuiltIn_LED_Blink_TaskCode(void *parameter) {
	while (true) {
		digitalWrite(BUILTIN_LED, HIGH);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		digitalWrite(BUILTIN_LED, LOW);
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}

void WS2812_EffectsTaskCode(void *parameter) {
	while (true) {
		display_mode mode = HAQuDA_DisplayManip::GetMode();

		switch (mode) {
			case effect:
				HAQuDA_DisplayManip::ShowEffect();
				break;
			case meas:
				break;
			case error:
				HAQuDA_ErrorHandler::ShowError();
				break;
			default:
				break;
		}
	}
}

void createTasks() {
	pinMode(BUILTIN_LED, OUTPUT);

	xTaskCreatePinnedToCore(WS2812_EffectsTaskCode, // Function that should be called
							"WS2812 Effects task",  // Name of the task (for debugging)
							2048,					// Stack size (bytes)
							NULL,					// Parameter to pass
							1,						// Task priority
							NULL,					// Task handler
							0						// Task core
	);

	xTaskCreatePinnedToCore(BuiltIn_LED_Blink_TaskCode, "BuiltIn LED Blink task", 1024, NULL, 1, NULL, 0);
}