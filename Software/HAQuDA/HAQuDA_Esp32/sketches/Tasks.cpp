#include "Tasks.h"

using namespace std;

void WS2812_EffectsTaskCode(void *parameter) {
	while (true) {
		switch (whatEffectDisp) {
			case snake: { // Item 3
				uint8_t tailLength = 5;
				uint16_t SnakeSpeed = 400;
				WS2812_clear();
				vTaskDelay(100 / portTICK_PERIOD_MS);
				for (int i = 0; i < LED_NUM_PIXELS; i++) {
					int pixelNum = 0;
					for (int j = 0; j < tailLength; j++) {
						pixelNum = j + i;
						pixelNum = (pixelNum > LED_NUM_PIXELS) ? LED_NUM_PIXELS : pixelNum;
						WS2812_setPixelColor(pixelNum, COLOR_LIME);
						WS2812_show();
						vTaskDelay(SnakeSpeed / portTICK_PERIOD_MS);
					}
					WS2812_setPixelColor(i, 0);
					if (whatEffectDisp != snake) {
						break;
					}
				}
				vTaskDelay(1000 / portTICK_PERIOD_MS);

				break;
			}
			case randomPixel: { // Item 3
				uint16_t RandomSpeed = 400;
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
					vTaskDelay(RandomSpeed / portTICK_PERIOD_MS);
					if (whatEffectDisp != randomPixel) {
						break;
					}
				}
				vTaskDelay(1000 / portTICK_PERIOD_MS);
				break;
			}
			case fade: { // Item 3
				uint16_t FadeSpeed = 400;
				WS2812_clear();
				vTaskDelay(100 / portTICK_PERIOD_MS);
				WS2812_fillColor(COLOR_AQUA);
				for (int j = 100; j > 0; j = j - 2) {
					WS2812_setBrightnessPerCent(j);
					WS2812_show();
					vTaskDelay(FadeSpeed / portTICK_PERIOD_MS);
					if (whatEffectDisp != fade) {
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

void createTasks() {
	xTaskCreatePinnedToCore(WS2812_EffectsTaskCode, // Function that should be called
							"WS2812 Effects task",  // Name of the task (for debugging)
							2048,					// Stack size (bytes)
							NULL,					// Parameter to pass
							1,						// Task priority
							NULL,					// Task handler
							1						// Task core
	);
}