#include "HAQuDA_UI.h"

dispEffect_enum HAQuDA_UI::dispEffect = noneEffect;
errorTypes_enum HAQuDA_UI::dispError = noneError;

HAQuDA_UI::HAQuDA_UI(HAQuDA_DispManip &currDispManip) {
	this->myDispManip = currDispManip;

	currUI_Disp_Params.dispMode = multi;

	currUI_Disp_Params.dispParam = noneParam;

	currUI_Disp_Params.multiModeStruct.paramsArr[0] = temp;
	currUI_Disp_Params.multiModeStruct.paramsArr[1] = eCO2;
	currUI_Disp_Params.multiModeStruct.paramsArr[2] = PM2_5;

	currUI_Disp_Params.multiModeStruct.divideDotsArr[0] = currUI_Disp_Params.temp_divideDots;
	currUI_Disp_Params.multiModeStruct.divideDotsArr[1] = currUI_Disp_Params.eCO2_divideDots;
	currUI_Disp_Params.multiModeStruct.divideDotsArr[2] = currUI_Disp_Params.PM2_5_divideDots;

	currUI_Disp_Params.temp_divideDots = {20, 26, 30};
	currUI_Disp_Params.humid_divideDots = {40, 60, 80};
	currUI_Disp_Params.eCO2_divideDots = {400, 1000, 5000};
	currUI_Disp_Params.TVOC_divideDots = {220, 660, 1000};
	currUI_Disp_Params.PM2_5_divideDots = {15, 20, 45};
	currUI_Disp_Params.currentTimeBorder = {21, 9};
	brightnessPerCent = MAX_BRIGHTNESS;

	effectParams.snakeColor = COLOR_LIME;
	effectParams.fadeColor = COLOR_AQUA;
}

void HAQuDA_UI::ext_setStaticColor(int red, int green, int blue) {
	uint32_t color = ((uint32_t)red << 16) | ((uint32_t)green << 8) | blue;
	WS2812_fillColor(color);
	delay(100);
	currUI_Disp_Params.dispMode = noneMode;
	dispEffect = staticColor;
}

void HAQuDA_UI::ext_setBrightness(uint8_t newBrightnessPerCent) {
	WS2812_setBrightnessPerCent(newBrightnessPerCent);
}

void HAQuDA_UI::ext_changeDispMode(int newDispMode) {
	bool modeChanged = false;
	switch (newDispMode) {
		case 1: {
			currUI_Disp_Params.dispMode = standard;
			modeChanged = true;
			break;
		}
		case 2: {
			currUI_Disp_Params.dispMode = multi;
			modeChanged = true;
			break;
		}
		case 3: {
			currUI_Disp_Params.dispMode = night;

			modeChanged = true;
			break;
		}
		case 4: {
			currUI_Disp_Params.dispMode = noneMode;

			modeChanged = true;
			break;
		}
		default:
			currUI_Disp_Params.dispMode = noneMode;
	}
	if (modeChanged) {
		WS2812_clear();
		delay(100);
		myDispManip.displayData(currUI_Disp_Params);
	}
}

void HAQuDA_UI::ext_changeDispParam(int newDispParam) {
	bool paramChanged = false;
	switch (newDispParam) {
		case 1: {
			currUI_Disp_Params.dispParam = temp;
			paramChanged = true;
			break;
		}
		case 2: {
			currUI_Disp_Params.dispParam = humid;
			paramChanged = true;
			break;
		}
		case 3: {
			currUI_Disp_Params.dispParam = eCO2;
			paramChanged = true;
			break;
		}
		case 4: {
			currUI_Disp_Params.dispParam = TVOC;
			paramChanged = true;
			break;
		}
		case 5: {
			currUI_Disp_Params.dispParam = PM2_5;
			paramChanged = true;
			break;
		}
		case 6: {
			currUI_Disp_Params.dispParam = total;
			paramChanged = true;
			break;
		}
		default:
			currUI_Disp_Params.dispParam = noneParam;
	}
	if (paramChanged) {
		WS2812_clear();
		delay(100);
		myDispManip.displayData(currUI_Disp_Params);
	}
}

void HAQuDA_UI::ext_changeDispEffect(int newDispEffect) {
	bool effectChanged = false;
	switch (newDispEffect) {
		case 1: {
			dispEffect = snake;
			effectChanged = true;
			break;
		}
		case 2: {
			dispEffect = randomPixel;
			effectChanged = true;
			break;
		}
		case 3: {
			dispEffect = fade;
			effectChanged = true;
			break;
		}
		case 4: {
			dispEffect = christmasTree;
			effectChanged = true;
			break;
		}
		default:
			dispEffect = noneEffect;
	}
	if (effectChanged) {
		WS2812_clear();
		delay(100);
		myDispManip.displayData(currUI_Disp_Params);
	}
}