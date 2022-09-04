#include "Tasks.h"
#include "HAQuDA_ErrorHandler.h"

HAQuDA_DisplayInterface *myDisplayInterface_tasks;
uint8_t builtIn_LED;

void WS2812_ErrorDisplayTaskCode(void *parameter) {
	while (true) {
		switch (HAQuDA_ErrorHandler::CurrentError) {
			case failedToConnectToWiFi: {
				HAQuDA_DisplayManip::showEffectGrow(HAQuDA_ErrorHandler::failedToConnectToWiFi_displayParams, &(HAQuDA_ErrorHandler::errorEffectDisp));
				break;
			}

			case failedToCreateAP: {
				HAQuDA_DisplayManip::showEffectSnake(HAQuDA_ErrorHandler::failedToCreateAP_displayParams, &(HAQuDA_ErrorHandler::errorEffectDisp));
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
		switch (HAQuDA_DisplayInterface::DisplayEffectParams.effect) {
			case snake: {
				HAQuDA_DisplayManip::showEffectSnake(HAQuDA_DisplayInterface::DisplayEffectParams.snakeParams,
													 &(HAQuDA_DisplayInterface::DisplayEffectParams.effect));
				break;
			}
			case randomPixel: {
				HAQuDA_DisplayManip::showEffectRandom(HAQuDA_DisplayInterface::DisplayEffectParams.randomParams,
													  &(HAQuDA_DisplayInterface::DisplayEffectParams.effect));
				break;
			}
			case fade: {
				HAQuDA_DisplayManip::showEffectFade(HAQuDA_DisplayInterface::DisplayEffectParams.fadeParams,
													&(HAQuDA_DisplayInterface::DisplayEffectParams.effect));
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

void createTasks(HAQuDA_DisplayInterface *currUI) {
	myDisplayInterface_tasks = currUI;
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