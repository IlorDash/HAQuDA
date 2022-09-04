#include "HAQuDA_DisplayInterface.h"

HAQuDA_DisplayInterface::HAQuDA_DisplayInterface() {

	DisplayMode = none;

	DisplayMeasParams.displayParam = noneParam;

	DisplayMeasParams.multiModeStruct.paramsArr[0] = noneParam;
	DisplayMeasParams.multiModeStruct.paramsArr[1] = noneParam;
	DisplayMeasParams.multiModeStruct.paramsArr[2] = noneParam;

	DisplayMeasParams.temp_divideDots = {20, 26, 30};
	DisplayMeasParams.humid_divideDots = {40, 60, 80};
	DisplayMeasParams.eCO2_divideDots = {400, 1000, 5000};
	DisplayMeasParams.TVOC_divideDots = {220, 660, 1000};
	DisplayMeasParams.PM2_5_divideDots = {15, 20, 45};
	DisplayMeasParams.currentTimeBorder = {21, 9};

	DisplayMeasParams.multiModeStruct.divideDotsArr[0] = DisplayMeasParams.temp_divideDots;
	DisplayMeasParams.multiModeStruct.divideDotsArr[1] = DisplayMeasParams.eCO2_divideDots;
	DisplayMeasParams.multiModeStruct.divideDotsArr[2] = DisplayMeasParams.PM2_5_divideDots;

	brightnessPerCent = MAX_BRIGHTNESS;

	DisplayEffectParams.growParams.color = COLOR_AQUA;
	DisplayEffectParams.growParams.speed = 200;

	DisplayEffectParams.snakeParams.color = COLOR_LIME;
	DisplayEffectParams.snakeParams.speed = 200;
	DisplayEffectParams.snakeParams.tailLength = 5;

	DisplayEffectParams.randomParams.speed = 200;
	DisplayEffectParams.randomParams.pauseTime = 1000;

	DisplayEffectParams.fadeParams.color = COLOR_AQUA;
	DisplayEffectParams.fadeParams.speed = 200;
	DisplayEffectParams.fadeParams.startBrightness = 100;
	DisplayEffectParams.fadeParams.stopBrightness = 0;
	DisplayEffectParams.fadeParams.step = 2;
}

void HAQuDA_DisplayInterface::SetStaticColor(int red, int green, int blue) {
	DisplayMode = effect;

	uint32_t color = ((uint32_t)red << 16) | ((uint32_t)green << 8) | blue;
	WS2812_fillColor(color);
	vTaskDelay(100);
}

void HAQuDA_DisplayInterface::SetBrightness(uint8_t newBrightnessPerCent) {
	WS2812_setBrightnessPerCent(newBrightnessPerCent);
}

void HAQuDA_DisplayInterface::ChangeDisplayMeasMode(uint8_t newDisplayMeasMode_num) {
	DisplayMode = meas;
	displayMeasMode_enum newDisplayMeasMode = static_cast<displayMeasMode_enum>(newDisplayMeasMode_num);

	if (newDisplayMeasMode != DisplayMeasParams.displayMode) {
		HAQuDA_DisplayManip::displayMeas(DisplayMeasParams);
	}
	DisplayMeasParams.displayMode = newDisplayMeasMode;
}

void HAQuDA_DisplayInterface::ChangeDisplayMeasParam(uint8_t newDisplayMeasParamn_num) {
	displayParams_enum newDisplayMeasParam = static_cast<displayParams_enum>(newDisplayMeasParamn_num);

	if (newDisplayMeasParam != DisplayMeasParams.displayParam) {
		HAQuDA_DisplayManip::displayMeas(DisplayMeasParams);
	}

	DisplayMeasParams.displayParam = newDisplayMeasParam;
}

void HAQuDA_DisplayInterface::ChangeDisplayEffect(uint8_t newDisplayEffect_num) {
	DisplayEffectParams.effect = static_cast<displayEffectMode_enum>(newDisplayEffect_num);
}