#include "HAQuDA_DisplayInterface.h"

whatDisplay_enum HAQuDA_DisplayInterface::whatDisplay = none;
displayEffect_enum HAQuDA_DisplayInterface::displayEffect = noneEffect;
errorTypes_enum HAQuDA_DisplayInterface::displayError = noneError;

HAQuDA_DisplayInterface::HAQuDA_DisplayInterface() {

	whatDisplay = meas;

	displayMeasParams.displayMode = multi;

	displayMeasParams.displayParam = noneParam;
	
	displayMeasParams.multiModeStruct.paramsArr[0] = temp;
	displayMeasParams.multiModeStruct.paramsArr[1] = eCO2;
	displayMeasParams.multiModeStruct.paramsArr[2] = PM2_5;
	
	displayMeasParams.temp_divideDots = {20, 26, 30};
	displayMeasParams.humid_divideDots = {40, 60, 80};
	displayMeasParams.eCO2_divideDots = {400, 1000, 5000};
	displayMeasParams.TVOC_divideDots = {220, 660, 1000};
	displayMeasParams.PM2_5_divideDots = {15, 20, 45};
	displayMeasParams.currentTimeBorder = {21, 9};

	displayMeasParams.multiModeStruct.divideDotsArr[0] = displayMeasParams.temp_divideDots;
	displayMeasParams.multiModeStruct.divideDotsArr[1] = displayMeasParams.eCO2_divideDots;
	displayMeasParams.multiModeStruct.divideDotsArr[2] = displayMeasParams.PM2_5_divideDots;
	
	brightnessPerCent = MAX_BRIGHTNESS;

	growEffectParams.color = COLOR_AQUA;
	growEffectParams.speed = 200;

	snakeEffectParams.color = COLOR_LIME;
	snakeEffectParams.speed = 200;
	snakeEffectParams.tailLength = 5;

	randomEffectParams.speed = 200;
	randomEffectParams.pauseTime = 1000;

	fadeEffectParams.color = COLOR_AQUA;
	fadeEffectParams.speed = 200;
	fadeEffectParams.startBrightness = 100;
	fadeEffectParams.stopBrightness = 0;
	fadeEffectParams.step = 2;
}

void HAQuDA_DisplayInterface::ext_setStaticColor(int red, int green, int blue) {
	uint32_t color = ((uint32_t)red << 16) | ((uint32_t)green << 8) | blue;
	WS2812_fillColor(color);
	delay(100);
	displayMeasParams.displayMode = noneMode;
	displayEffect = staticColor;
}

void HAQuDA_DisplayInterface::ext_setBrightness(uint8_t newBrightnessPerCent) {
	WS2812_setBrightnessPerCent(newBrightnessPerCent);
}

void HAQuDA_DisplayInterface::ext_changeDisplayMode(int newDisplayMode) {
	bool modeChanged = false;
	switch (newDisplayMode) {
		case 1: {
			displayMeasParams.displayMode = standard;
			modeChanged = true;
			break;
		}
		case 2: {
			displayMeasParams.displayMode = multi;
			modeChanged = true;
			break;
		}
		case 3: {
			displayMeasParams.displayMode = night;

			modeChanged = true;
			break;
		}
		case 4: {
			displayMeasParams.displayMode = noneMode;

			modeChanged = true;
			break;
		}
		default:
			displayMeasParams.displayMode = noneMode;
	}
	if (modeChanged) {
		WS2812_clear();
		delay(100);
		HAQuDA_DisplayManip::displayData(displayMeasParams);
	}
}

void HAQuDA_DisplayInterface::ext_changeDisplayParam(int newDisplayParam) {
	bool paramChanged = false;
	switch (newDisplayParam) {
		case 1: {
			displayMeasParams.displayParam = temp;
			paramChanged = true;
			break;
		}
		case 2: {
			displayMeasParams.displayParam = humid;
			paramChanged = true;
			break;
		}
		case 3: {
			displayMeasParams.displayParam = eCO2;
			paramChanged = true;
			break;
		}
		case 4: {
			displayMeasParams.displayParam = TVOC;
			paramChanged = true;
			break;
		}
		case 5: {
			displayMeasParams.displayParam = PM2_5;
			paramChanged = true;
			break;
		}
		case 6: {
			displayMeasParams.displayParam = total;
			paramChanged = true;
			break;
		}
		default:
			displayMeasParams.displayParam = noneParam;
	}
	if (paramChanged) {
		WS2812_clear();
		delay(100);
		HAQuDA_DisplayManip::displayData(displayMeasParams);
	}
}

void HAQuDA_DisplayInterface::ext_changeDisplayEffect(int newDisplayEffect) {
	bool effectChanged = false;
	switch (newDisplayEffect) {
		case 1: {
			displayEffect = snake;
			effectChanged = true;
			break;
		}
		case 2: {
			displayEffect = randomPixel;
			effectChanged = true;
			break;
		}
		case 3: {
			displayEffect = fade;
			effectChanged = true;
			break;
		}
		case 4: {
			displayEffect = christmasTree;
			effectChanged = true;
			break;
		}
		default:
			displayEffect = noneEffect;
	}
	if (effectChanged) {
		WS2812_clear();
		delay(100);
		HAQuDA_DisplayManip::displayData(displayMeasParams);
	}
}