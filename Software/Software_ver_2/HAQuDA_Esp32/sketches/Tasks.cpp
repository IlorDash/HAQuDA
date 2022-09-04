#include "Tasks.h"
#include "HAQuDA_ErrorHandler.h"

HAQuDA_DisplayInterface *myDisplayInterface_tasks;
uint8_t builtIn_LED;

void WS2812_ErrorDispTaskCode(void *parameter) {
	while (true) {
		switch (HAQuDA_DisplayInterface::displayError) {
			case failedToConnectToWiFi: {
				HAQuDA_DisplayManip::showEffectGrow(HAQuDA_ErrorHandler::failedToConnectToWiFi_displayParams, &HAQuDA_ErrorHandler::errorEffectDisp);
				break;
			}

			case failedToCreateAP: {
				HAQuDA_DisplayManip::showEffectSnake(HAQuDA_ErrorHandler::failedToCreateAP_displayParams, &HAQuDA_ErrorHandler::errorEffectDisp);
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
		switch (HAQuDA_DisplayInterface::displayEffect) {
			case snake: {
				HAQuDA_DisplayManip::showEffectSnake(HAQuDA_DisplayInterface::snakeEffectParams, &HAQuDA_DisplayInterface::displayEffect);
				break;
			}
			case randomPixel: {
				HAQuDA_DisplayManip::showEffectRandom(HAQuDA_DisplayInterface::randomEffectParams, &HAQuDA_DisplayInterface::displayEffect);
				break;
			}
			case fade: {
				HAQuDA_DisplayManip::showEffectFade(HAQuDA_DisplayInterface::fadeEffectParams, &HAQuDA_DisplayInterface::displayEffect);
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

	xTaskCreatePinnedToCore(WS2812_ErrorDispTaskCode, // Function that should be called
							"WS2812 Error Disp task", // Name of the task (for debugging)
							2048,					  // Stack size (bytes)
							NULL,					  // Parameter to pass
							1,						  // Task priority
							NULL,					  // Task handler
							1						  // Task core
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