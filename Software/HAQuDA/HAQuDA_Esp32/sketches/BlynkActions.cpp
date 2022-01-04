#include "BlynkActions.h"

BLYNK_WRITE(V0) {
	int red = param[0].asInt();
	int green = param[1].asInt();
	int blue = param[2].asInt();
	log_i("V0, r: %d, g: %d, b: %d,", red, green, blue);
	uint32_t color = ((uint32_t)red << 16) | ((uint32_t)green << 8) | blue;
	WS2812_fillColor(color);
	delay(100);
	whatParamDisp = noneParam;
	whatEffectDisp = noneEffect;
}

BLYNK_WRITE(V2) {
	switch (param.asInt()) {
		case 1: { // Item 3
			whatParamDisp = temp;
			WS2812_clear();
			delay(100);
			break;
		}
		case 2: { // Item 3
			whatParamDisp = humid;
			WS2812_clear();
			delay(100);
			break;
		}
		case 3: { // Item 1
			whatParamDisp = eCO2;
			WS2812_clear();
			delay(100);
			break;
		}
		case 4: { // Item 2
			whatParamDisp = TVOC;
			WS2812_clear();
			delay(100);
			break;
		}
		case 5: { // Item 3
			whatParamDisp = PM2_5;
			WS2812_clear();
			delay(100);
			break;
		}
		default:
			whatParamDisp = noneParam;
	}
	dispParam_WS2812();
	whatEffectDisp = noneEffect;
}

BLYNK_WRITE(V3) {
	WS2812_setBrightnessPerCent(param.asInt());
}

BLYNK_WRITE(V4) {
	switch (param.asInt()) {
		case 1: { // Item 3
			whatEffectDisp = snake;
			WS2812_clear();
			delay(100);
			break;
		}
		case 2: { // Item 3
			whatEffectDisp = randomPixel;
			WS2812_clear();
			delay(100);
			break;
		}
		case 3: { // Item 1
			whatEffectDisp = fade;
			WS2812_clear();
			delay(100);
			break;
		}
		case 4: { // Item 2
			whatEffectDisp = christmasTree;
			WS2812_clear();
			delay(100);
			break;
		}
		default:
			whatEffectDisp = noneEffect;
	}
	whatParamDisp = noneParam;
}