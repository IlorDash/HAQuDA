#include "Tasks.h"

using namespace std;
HAQuDA_UI *myUI_tasks;

void WS2812_EffectsTaskCode(void *parameter) {
	while (true) {
		switch (myUI_tasks->currUI_Params.dispEffect) {
			case snake: {
				WS2812_clear();
				vTaskDelay(100 / portTICK_PERIOD_MS);
				for (int i = 0; i < LED_NUM_PIXELS; i++) {
					int pixelNum = 0;
					for (int j = 0; j < myUI_tasks->currUI_Params.effectParams.snakeTailLength; j++) {
						pixelNum = j + i;
						pixelNum = (pixelNum > LED_NUM_PIXELS) ? LED_NUM_PIXELS : pixelNum;
						WS2812_setPixelColor(pixelNum, myUI_tasks->currUI_Params.effectParams.snakeColor);
						WS2812_show();
						vTaskDelay(myUI_tasks->currUI_Params.effectParams.snakeSpeed / portTICK_PERIOD_MS);
					}
					WS2812_setPixelColor(i, 0);
					if (myUI_tasks->currUI_Params.dispEffect != snake) {
						break;
					}
				}
				vTaskDelay(1000 / portTICK_PERIOD_MS);

				break;
			}
			case randomPixel: {
				WS2812_clear();
				vTaskDelay(100 / portTICK_PERIOD_MS);
				vector<int> pixelEnArr;
				for (int i = 0; i < LED_NUM_PIXELS; i++) {
					int pixelNum = random(0, LED_NUM_PIXELS);
					while (find(pixelEnArr.begin(), pixelEnArr.end(), pixelNum) != pixelEnArr.end()) {
						pixelNum = random(0, LED_NUM_PIXELS);
					}
					pixelEnArr.push_back(pixelNum);

					WS2812_setPixelColor(pixelNum, ((uint32_t)random(0, 255) << 16) | ((uint32_t)random(0, 255) << 8) | random(0, 255));
					WS2812_show();
					vTaskDelay(myUI_tasks->currUI_Params.effectParams.randomSpeed / portTICK_PERIOD_MS);
					if (myUI_tasks->currUI_Params.dispEffect != randomPixel) {
						break;
					}
				}
				vTaskDelay(1000 / portTICK_PERIOD_MS);
				break;
			}
			case fade: { 
				WS2812_clear();
				vTaskDelay(100 / portTICK_PERIOD_MS);
				WS2812_fillColor(myUI_tasks->currUI_Params.effectParams.fadeColor);
				for (int j = 100; j > 0; j = j - 2) {
					WS2812_setBrightnessPerCent(j);
					WS2812_show();
					vTaskDelay(myUI_tasks->currUI_Params.effectParams.fadeSpeed / portTICK_PERIOD_MS);
					if (myUI_tasks->currUI_Params.dispEffect != fade) {
						break;
					}
				}
				break;
			}
			case christmasTree: { // Item 3
				// WS2812_ChristmasTree(400);
				break;
			}
			default: {
				vTaskDelay(500 / portTICK_PERIOD_MS);
				break;
			}
		}
	}
}

void createTasks(HAQuDA_UI *currUI) {
	myUI_tasks = currUI;
	xTaskCreatePinnedToCore(WS2812_EffectsTaskCode, // Function that should be called
							"WS2812 Effects task",  // Name of the task (for debugging)
							2048,					// Stack size (bytes)
							NULL,					// Parameter to pass
							1,						// Task priority
							NULL,					// Task handler
							1						// Task core
	);
}