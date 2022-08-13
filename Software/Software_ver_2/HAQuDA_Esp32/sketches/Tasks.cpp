#include "Tasks.h"

using namespace std;
HAQuDA_DisplayInterface *myDisplayInterface_tasks;
uint8_t builtIn_LED;

void WS2812_EffectsTaskCode(void *parameter) {
	while (true) {
		switch (HAQuDA_DisplayInterface::displayEffect) {
			case snake: {
//				WS2812_clear();
//				for (int i = 0; i < LED_NUM_PIXELS; i++) {
//					int pixelNum = 0;
//					for (int j = 0; j < myUI_tasks->effectParams.snakeTailLength; j++) {
//						pixelNum = j + i;
//						pixelNum = (pixelNum > LED_NUM_PIXELS) ? LED_NUM_PIXELS : pixelNum;
//						WS2812_setPixelColor(pixelNum, myUI_tasks->effectParams.snakeColor);
//						WS2812_show(myUI_tasks->effectParams.snakeSpeed);
//					}
//					WS2812_setPixelColor(i, 0);
//					if (myUI_tasks->dispEffect != snake) {
//						break;
//					}
//				}
//				break;
			}
			case randomPixel: {
//				WS2812_clear();
//				vector<int> pixelEnArr;
//				for (int i = 0; i < LED_NUM_PIXELS; i++) {
//					int pixelNum = random(0, LED_NUM_PIXELS);
//					while (find(pixelEnArr.begin(), pixelEnArr.end(), pixelNum) != pixelEnArr.end()) {
//						pixelNum = random(0, LED_NUM_PIXELS);
//					}
//					pixelEnArr.push_back(pixelNum);
//
//					WS2812_setPixelColor(pixelNum, ((uint32_t)random(0, 255) << 16) | ((uint32_t)random(0, 255) << 8) | random(0, 255));
//					WS2812_show(myUI_tasks->effectParams.randomSpeed);
//					if (myUI_tasks->dispEffect != randomPixel) {
//						break;
//					}
//				}
//				vTaskDelay(1000 / portTICK_PERIOD_MS);
//				break;
			}
			case fade: {
//				WS2812_clear();
//				WS2812_fillColor(myUI_tasks->effectParams.fadeColor);
//				for (int j = 100; j > 0; j = j - 2) {
//					WS2812_setBrightnessPerCent(j);
//					WS2812_show(myUI_tasks->effectParams.fadeSpeed);
//					if (myUI_tasks->dispEffect != fade) {
//						break;
//					}
//				}
//				break;
			}
			case christmasTree: { // Item 3
				// WS2812_ChristmasTree(400);
				break;
			}
			default:
				break;
		}
	}
}

void WS2812_ErrorDispTaskCode(void *parameter) {
	while (true) {
		switch (HAQuDA_DisplayInterface::displayError) {
			case failedToConnectToWiFi: {
//				WS2812_clear();
//				for (int i = 0; i < LED_COLUMN_NUM; i++) {
//					for (int j = (i % 2); j < LED_ROW_NUM; j += 2) {
//						uint8_t pixelNum = i * LED_ROW_NUM + j;
//						WS2812_setPixelColor(pixelNum, COLOR_RED);
//					}
//				}
//				WS2812_show();
//				break;
			}

			case failedToCreateAP: {
//				WS2812_clear();
//				vTaskDelay(100 / portTICK_PERIOD_MS);
//				for (int i = 0; i < LED_COLUMN_NUM; i += 2) {
//					for (int j = 0; j < LED_ROW_NUM; j++) {
//						uint8_t pixelNum = i * LED_ROW_NUM + j;
//						WS2812_setPixelColor(pixelNum, COLOR_RED);
//					}
//				}
//				WS2812_show();
//				break;
			}
			default:
				break;
		}
	}
}

void BuiltIn_LED_Blink_TaskCode(void *parameter) {
	while (true) {
		if (builtIn_LED >= 0) {
			digitalWrite(builtIn_LED, HIGH);
			vTaskDelay(1000 / portTICK_PERIOD_MS);
			digitalWrite(builtIn_LED, LOW);
			vTaskDelay(500 / portTICK_PERIOD_MS);
		}
	}
}

void createTasks(HAQuDA_DisplayInterface *currUI) {
	myDisplayInterface_tasks = currUI;
	builtIn_LED = atoi(BUILTIN_LED);

//	myUI_tasks->effectParams.snakeColor = COLOR_AQUA;
//	myUI_tasks->effectParams.snakeTailLength = 5;
//	myUI_tasks->effectParams.snakeSpeed = 50; // start up connection effect
	HAQuDA_DisplayInterface::displayEffect = snake;
	vTaskDelay(100 / portTICK_PERIOD_MS);
	
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