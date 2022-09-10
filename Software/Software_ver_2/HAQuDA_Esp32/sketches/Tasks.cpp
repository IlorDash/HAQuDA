#include "Tasks.h"
#include "HAQuDA_ErrorHandler.h"

HAQuDA_DisplayManip *myDisplayManip_tasks;
uint8_t builtIn_LED;

void WS2812_ErrorDisplayTaskCode(void *parameter) {
	while (true) {
		errorTypes_enum e = HAQuDA_ErrorHandler::GetCurrentError();
		switch (e) {
			case failedToStartFS: {
				HAQuDA_DisplayManip::ShowEffectFade(HAQuDA_ErrorHandler::failedToStartFS_displayParams);
				break;
			}
			case failedToConnectToWiFi: {
				HAQuDA_DisplayManip::ShowEffectGrow(HAQuDA_ErrorHandler::failedToConnectToWiFi_displayParams);
				break;
			}

			case failedToCreateAP: {
				HAQuDA_DisplayManip::ShowEffectSnake(HAQuDA_ErrorHandler::failedToCreateAP_displayParams);
				break;
			}

			case failedToStartSensors: {
				HAQuDA_DisplayManip::ShowEffectUpDown(HAQuDA_ErrorHandler::failedToStartSensors_displayParams);
				break;
			}

			default:
				break;
		}
	}
}

void BuiltIn_LED_Blink_TaskCode(void *parameter) {
	while (true) {
		digitalWrite(builtIn_LED, HIGH);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		digitalWrite(builtIn_LED, LOW);
		vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}

void WS2812_EffectsTaskCode(void *parameter) {
	while (true) {
		HAQuDA_DisplayManip::stopEffect = false;
		switch (HAQuDA_DisplayManip::GetDisplayEffect()) {
			case snake: {
				HAQuDA_DisplayManip::ShowEffectSnake();
				break;
			}
			case randomPixel: {
				HAQuDA_DisplayManip::ShowEffectRandom();
				break;
			}
			case fade: {
				HAQuDA_DisplayManip::ShowEffectFade();
				break;
			}
			case christmasTree: {
				// WS2812_ChristmasTree(400);
				break;
			}
			default:
				break;
		}
	}
}

void createTasks() {
	builtIn_LED = atoi(BUILTIN_LED);
	pinMode(builtIn_LED, OUTPUT);

	xTaskCreatePinnedToCore(WS2812_EffectsTaskCode, // Function that should be called
							"WS2812 Effects task",  // Name of the task (for debugging)
							2048,					// Stack size (bytes)
							NULL,					// Parameter to pass
							1,						// Task priority
							NULL,					// Task handler
							1						// Task core
	);

	xTaskCreatePinnedToCore(WS2812_ErrorDisplayTaskCode, // Function that should be called
							"WS2812 Error Disp task",	// Name of the task (for debugging)
							2048,						 // Stack size (bytes)
							NULL,						 // Parameter to pass
							1,							 // Task priority
							NULL,						 // Task handler
							1							 // Task core
	);

	xTaskCreatePinnedToCore(BuiltIn_LED_Blink_TaskCode, // Function that should be called
							"BuiltIn LED Blink task",   // Name of the task (for debugging)
							1024,						// Stack size (bytes)
							NULL,						// Parameter to pass
							1,							// Task priority
							NULL,						// Task handler
							1							// Task core
	);
}