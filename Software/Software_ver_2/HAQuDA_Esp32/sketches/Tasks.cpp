#include "Tasks.h"
#include "HAQuDA_ErrorHandler.h"

#define BUILTIN_LED 2

HAQuDA_DisplayManip *myDisplayManip_tasks;

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
		bool isError = (HAQuDA_DisplayManip::GetMode() == error);
		effect_mode effect = HAQuDA_DisplayManip::GetEffectMode();

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
				if (isError) {
					HAQuDA_DisplayManip::ShowEffectRandom(HAQuDA_ErrorHandler::failedToStartSensors_displayParams);
				} else {
					HAQuDA_DisplayManip::ShowEffectRandom();
				}
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

	xTaskCreatePinnedToCore(BuiltIn_LED_Blink_TaskCode,
							"BuiltIn LED Blink task",  
							1024,					
							NULL,					
							1,						
							NULL,					
							0						
	);
}