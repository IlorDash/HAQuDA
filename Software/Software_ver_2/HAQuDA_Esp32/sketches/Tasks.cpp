#include "Tasks.h"
#include "HAQuDA_ErrorHandler.h"

HAQuDA_DisplayManip *myDisplayManip_tasks;
uint8_t builtIn_LED;

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
		bool isError = (HAQuDA_DisplayManip::GetDisplayMode() == error);
		displayEffectMode_enum effect = HAQuDA_DisplayManip::GetDisplayEffect();

		switch (effect) {
			case snake: {
				if (isError) {
					HAQuDA_DisplayManip::ShowEffectSnake(HAQuDA_ErrorHandler::failedToCreateAP_displayParams);
				} else {
					HAQuDA_DisplayManip::ShowEffectSnake();
				}
				break;
			}
			case randomPixel: {
				//				if (isError) {
				//					HAQuDA_DisplayManip::ShowEffectRandom(HAQuDA_ErrorHandler::failedTo....);
				//				} else {
				//					HAQuDA_DisplayManip::ShowEffectRandom();
				//				}
				HAQuDA_DisplayManip::ShowEffectRandom();
				break;
			}
			case fade: {
				if (isError) {
					HAQuDA_DisplayManip::ShowEffectFade(HAQuDA_ErrorHandler::failedToStartFS_displayParams);
				} else {
					HAQuDA_DisplayManip::ShowEffectFade();
				}
				break;
			}
			case grow: {
				if (isError) {
					HAQuDA_DisplayManip::ShowEffectGrow(HAQuDA_ErrorHandler::failedToConnectToWiFi_displayParams);
				} else {
					HAQuDA_DisplayManip::ShowEffectGrow();
				}
				break;
			}
			case upDown: {
				if (isError) {
					HAQuDA_DisplayManip::ShowEffectUpDown(HAQuDA_ErrorHandler::failedToStartSensors_displayParams);
				} else {
					HAQuDA_DisplayManip::ShowEffectUpDown();
				}
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

	xTaskCreatePinnedToCore(BuiltIn_LED_Blink_TaskCode, // Function that should be called
							"BuiltIn LED Blink task",   // Name of the task (for debugging)
							1024,						// Stack size (bytes)
							NULL,						// Parameter to pass
							1,							// Task priority
							NULL,						// Task handler
							1							// Task core
	);
}