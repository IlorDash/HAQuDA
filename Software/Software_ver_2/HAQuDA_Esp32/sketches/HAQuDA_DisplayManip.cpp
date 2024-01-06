#include "HAQuDA_DisplayManip.h"
#include "WS2812.h"
#include "Sensors.h"
#include <vector>

curr_display HAQuDA_DisplayManip::Display;

HAQuDA_DisplayManip::HAQuDA_DisplayManip() {

	Display.brighntessPerCent = DEFAULT_BRIGHTNESS;
	Display.stopEffect = false;
	Display.forceShowMeas = false;

	Display.effectParams.mode = noneEffect;
	Display.effectParams.snakeParams.color = COLOR_LIME;
	Display.effectParams.snakeParams.speed = 200;
	Display.effectParams.snakeParams.tailLength = 5;

	Display.effectParams.growParams.color = COLOR_AQUA;
	Display.effectParams.growParams.speed = 200;

	Display.effectParams.fadeParams.color = COLOR_AQUA;
	Display.effectParams.fadeParams.speed = 10;
	Display.effectParams.fadeParams.startBrightness = 100;
	Display.effectParams.fadeParams.stopBrightness = 0;
	Display.effectParams.fadeParams.step = 5;

	Display.effectParams.randomParams.speed = 200;
	Display.effectParams.randomParams.pauseTime = 1000;

	Display.measParams.mode = noneMeas;
	Display.measParams.type = noneType;

	Display.measParams.multiModeParams.paramsArr[0] = temp;
	Display.measParams.multiModeParams.paramsArr[1] = eCO2;
	Display.measParams.multiModeParams.paramsArr[2] = PM2_5;
	Display.measParams.multiModeParams.dataArr[0] = 0;
	Display.measParams.multiModeParams.dataArr[1] = 0;
	Display.measParams.multiModeParams.dataArr[2] = 0;
	Display.measParams.multiModeParams.divideDotsArr[0] = Display.measParams.divDots.temp;
	Display.measParams.multiModeParams.divideDotsArr[1] = Display.measParams.divDots.eCO2;
	Display.measParams.multiModeParams.divideDotsArr[2] = Display.measParams.divDots.PM2_5;

	Display.measParams.nightModeTimeBorder.timeFirstBorder = 21;
	Display.measParams.nightModeTimeBorder.timeFirstBorder = 9;

	Display.measParams.divDots.temp.firstDot = 20;
	Display.measParams.divDots.temp.middleDot = 26;
	Display.measParams.divDots.temp.thirdDot = 30;

	Display.measParams.divDots.humid.firstDot = 40;
	Display.measParams.divDots.humid.middleDot = 60;
	Display.measParams.divDots.humid.thirdDot = 80;

	Display.measParams.divDots.eCO2.firstDot = 400;
	Display.measParams.divDots.eCO2.middleDot = 1000;
	Display.measParams.divDots.eCO2.thirdDot = 5000;

	Display.measParams.divDots.TVOC.firstDot = 220;
	Display.measParams.divDots.TVOC.middleDot = 660;
	Display.measParams.divDots.TVOC.thirdDot = 1000;

	Display.measParams.divDots.PM2_5.firstDot = 15;
	Display.measParams.divDots.PM2_5.middleDot = 20;
	Display.measParams.divDots.PM2_5.thirdDot = 45;
}

void HAQuDA_DisplayManip::SetBrightness(const uint8_t new_brightness) {
	Display.brighntessPerCent = new_brightness;
	WS2812_setBrightnessPerCent(Display.brighntessPerCent);
}

bool HAQuDA_DisplayManip::SetMode(const display_mode new_mode) {

	if (Display.mode == error) {
		return false;
	}

	Display.stopEffect = true;
	Display.mode = new_mode;

	switch (Display.mode) {
		case noneMode: {
			Display.effectParams.mode = noneEffect;
			Display.measParams.mode = noneMeas;
			break;
		}
		case error: {
			Display.effectParams.mode = noneEffect;
			Display.measParams.mode = noneMeas;
			break;
		}
		case meas: {
			Display.effectParams.mode = noneEffect;
			break;
		}
		case effect: {
			Display.measParams.mode = noneMeas;
			break;
		}
		default:
			break;
	}
	return true;
}

void HAQuDA_DisplayManip::SetMeasMode(const meas_mode new_meas_mode) {

	if (new_meas_mode != Display.measParams.mode) {
		Display.forceShowMeas = true;
		Display.measParams.mode = new_meas_mode;
	}

	if (Display.mode != meas)
		SetMode(meas);
}

void HAQuDA_DisplayManip::SetMeasType(const meas_type new_meas_type) {

	if (new_meas_type != Display.measParams.type) {
		Display.forceShowMeas = true;
		Display.measParams.type = new_meas_type;
	}

	if (Display.mode != meas)
		SetMode(meas);
}

void HAQuDA_DisplayManip::SetEffectMode(const effect_mode new_effect_mode) {
	Display.stopEffect = true;
	Display.effectParams.mode = new_effect_mode;
}

void HAQuDA_DisplayManip::SetEffectParam(const effects_params new_effect_param) {
	Display.stopEffect = true;
	Display.effectParams = new_effect_param;
}

void HAQuDA_DisplayManip::SetError(const effect_mode new_error) {
	HAQuDA_DisplayManip::SetMode(error);
	HAQuDA_DisplayManip::SetEffectMode(new_error);
}

void HAQuDA_DisplayManip::ClearError() {
	SetMode(noneMode);
}

uint8_t HAQuDA_DisplayManip::GetBrightness() {
	return Display.brighntessPerCent;
}

display_mode HAQuDA_DisplayManip::GetMode() {
	return Display.mode;
}

meas_mode HAQuDA_DisplayManip::GetMeasMode() {
	return Display.measParams.mode;
}

meas_type HAQuDA_DisplayManip::GetMeasType() {
	return Display.measParams.type;
}

effect_mode HAQuDA_DisplayManip::GetEffectMode() {
	return Display.effectParams.mode;
}

void HAQuDA_DisplayManip::ShowStaticColor(uint32_t color) {
	SetMode(noneMode);
	WS2812_fillColor(color);
}

void HAQuDA_DisplayManip::ShowStaticColor(int red, int green, int blue) {
	uint32_t color = ((uint32_t)red << 16) | ((uint32_t)green << 8) | blue;
	SetMode(noneMode);
	WS2812_fillColor(color);
}

void HAQuDA_DisplayManip::ShowEffectGrow(const grow_effect_params params) {
	Display.stopEffect = false;
	WS2812_clear();
	for (int i = (LED_ROW_NUM - 1); i >= 0; i--) {
		for (int j = 0; j < LED_COLUMN_NUM; j++) {
			if (Display.stopEffect) {
				return;
			}
			uint8_t pixelNum = getLedNum(j, i);
			WS2812_setPixelColor(pixelNum, COLOR_RED);
		}
		WS2812_show(params.speed);
	}
}

void HAQuDA_DisplayManip::ShowEffectSnake(const snake_effect_params params) {
	Display.stopEffect = false;
	WS2812_clear();
	for (int i = 0; i < LED_NUM_PIXELS; i++) {
		int pixelNum = 0;
		for (int j = 0; j < params.tailLength; j++) {
			pixelNum = j + i;
			pixelNum = (pixelNum > LED_NUM_PIXELS) ? LED_NUM_PIXELS : pixelNum;
			WS2812_setPixelColor(pixelNum, params.color);

			if (Display.stopEffect) {
				return;
			}
		}
		WS2812_show(params.speed);
		WS2812_setPixelColor(i, 0);
	}
}

void HAQuDA_DisplayManip::ShowEffectRandom(const random_effect_params params) {
	Display.stopEffect = false;
	WS2812_clear();
	std::vector<int> pixelEnArr;
	for (int i = 0; i < LED_NUM_PIXELS; i++) {
		int pixelNum = random(0, LED_NUM_PIXELS);
		while (find(pixelEnArr.begin(), pixelEnArr.end(), pixelNum) != pixelEnArr.end()) {
			pixelNum = random(0, LED_NUM_PIXELS);

			if (Display.stopEffect) {
				return;
			}
		}
		pixelEnArr.push_back(pixelNum);

		WS2812_setPixelColor(pixelNum, ((uint32_t)random(0, 255) << 16) | ((uint32_t)random(0, 255) << 8) | random(0, 255));
		WS2812_show(params.speed);
	}
	vTaskDelay(params.pauseTime / portTICK_PERIOD_MS);
}

void HAQuDA_DisplayManip::ShowEffectFade(const fade_effect_params params) {
	Display.stopEffect = false;
	WS2812_clear();
	WS2812_setBrightnessPerCent(params.startBrightness);
	WS2812_fillColor(params.color);
	for (int j = params.startBrightness; j > params.stopBrightness; j = j - params.step) {
		WS2812_setBrightnessPerCent(j);
		vTaskDelay(params.speed / portTICK_PERIOD_MS);
		if (Display.stopEffect) {
			WS2812_setBrightnessPerCent(Display.brighntessPerCent);
			return;
		}
	}
}

void HAQuDA_DisplayManip::ShowMeas() {
	switch (Display.measParams.mode) {
		case standard: {
			standardMode();
			break;
		}
		case multi: {
			multiMode(Display.measParams.multiModeParams);
			break;
		}
		case night: {
			nightMode();
			break;
		}
		default:
			break;
	}
}

meas_type HAQuDA_DisplayManip::checkBadMeas(meas_params DisplayMeasParams) {
	if (((temp_meas.value / temp_meas.measNum) >= DisplayMeasParams.divDots.temp.thirdDot)
		|| ((temp_meas.value / temp_meas.measNum) <= DisplayMeasParams.divDots.temp.firstDot)) {
		return temp;
	} else if (((humid_meas.value / humid_meas.measNum) >= DisplayMeasParams.divDots.humid.thirdDot)
			   || ((humid_meas.value / humid_meas.measNum) <= DisplayMeasParams.divDots.humid.firstDot)) {
		return humid;
	} else if ((eCO2_meas.value / eCO2_meas.measNum) >= DisplayMeasParams.divDots.eCO2.thirdDot) {
		return eCO2;
	} else if ((TVOC_meas.value / TVOC_meas.measNum) >= DisplayMeasParams.divDots.TVOC.thirdDot) {
		return TVOC;
	} else if ((PM_2_5_meas.value / PM_2_5_meas.measNum) >= DisplayMeasParams.divDots.PM2_5.thirdDot) {
		return PM2_5;
	}
	return noneType;
}

void HAQuDA_DisplayManip::standardMode() {
	switch (Display.measParams.type) {
		case total: {
			meas_type badParam = checkBadMeas(Display.measParams);
			switch (badParam) {
				case temp: {
					showMeas_standard(temp_meas.value / temp_meas.measNum, Display.measParams.divDots.temp);
					break;
				}
				case humid: {
					showMeas_standard(humid_meas.value / humid_meas.measNum, Display.measParams.divDots.humid);
					break;
				}
				case eCO2: {
					showMeas_standard(eCO2_meas.value / eCO2_meas.measNum, Display.measParams.divDots.eCO2);
					break;
				}
				case TVOC: {
					showMeas_standard(TVOC_meas.value / TVOC_meas.measNum, Display.measParams.divDots.TVOC);
					break;
				}
				case PM2_5: {
					showMeas_standard(PM_2_5_meas.value / PM_2_5_meas.measNum, Display.measParams.divDots.PM2_5);
					break;
				}
				case noneType: {
					float measValuesArr[DISP_PARAMS_NUM];
					div_dots measDivideDots[DISP_PARAMS_NUM];

					measValuesArr[0] = temp_meas.value / temp_meas.measNum;
					measDivideDots[0] = Display.measParams.divDots.temp;

					measValuesArr[1] = humid_meas.value / humid_meas.measNum;
					measDivideDots[1] = Display.measParams.divDots.humid;

					measValuesArr[2] = eCO2_meas.value / eCO2_meas.measNum;
					measDivideDots[2] = Display.measParams.divDots.eCO2;

					measValuesArr[3] = TVOC_meas.value / TVOC_meas.measNum;
					measDivideDots[3] = Display.measParams.divDots.TVOC;

					measValuesArr[4] = PM_2_5_meas.value / PM_2_5_meas.measNum;
					measDivideDots[4] = Display.measParams.divDots.PM2_5;

					showMeas_total(measValuesArr, sizeof(measValuesArr), measDivideDots, sizeof(measDivideDots));
					break;
				}
				default:
					break;
			}
			break;
		}
		case temp: {
			HAQuDA_DisplayManip::showMeas_standard(temp_meas.value / temp_meas.measNum, Display.measParams.divDots.temp);
			break;
		}
		case humid: {
			HAQuDA_DisplayManip::showMeas_standard(humid_meas.value / humid_meas.measNum, Display.measParams.divDots.humid);
			break;
		}
		case eCO2: {
			HAQuDA_DisplayManip::showMeas_standard(eCO2_meas.value / eCO2_meas.measNum, Display.measParams.divDots.eCO2);
			break;
		}
		case TVOC: {
			HAQuDA_DisplayManip::showMeas_standard(TVOC_meas.value / TVOC_meas.measNum, Display.measParams.divDots.TVOC);
			break;
		}
		case PM2_5: {
			HAQuDA_DisplayManip::showMeas_standard(PM_2_5_meas.value / PM_2_5_meas.measNum, Display.measParams.divDots.PM2_5);
			break;
		}
		default:
			break;
	}
}

void HAQuDA_DisplayManip::multiMode(multi_mode_params multiModeParams) {
	for (int i = 0; i < MULTI_MODE_PARAM_NUM; i++) {
		switch (multiModeParams.paramsArr[i]) {
			case temp: {
				multiModeParams.dataArr[i] = temp_meas.value / temp_meas.measNum;
				break;
			}
			case humid: {
				multiModeParams.dataArr[i] = humid_meas.value / humid_meas.measNum;
				break;
			}
			case eCO2: {
				multiModeParams.dataArr[i] = eCO2_meas.value / eCO2_meas.measNum;
				break;
			}
			case PM2_5: {
				multiModeParams.dataArr[i] = PM_2_5_meas.value / PM_2_5_meas.measNum;
				break;
			}
			case TVOC: {
				multiModeParams.dataArr[i] = TVOC_meas.value / TVOC_meas.measNum;
				break;
			}
			default:
				break;
		}
	}
	HAQuDA_DisplayManip::showMeas_multi(multiModeParams.dataArr, sizeof(multiModeParams.dataArr), multiModeParams.divideDotsArr,
										sizeof(multiModeParams.divideDotsArr));
}

uint8_t HAQuDA_DisplayManip::get_nightMode_hour(night_mode_time_border timeBorder) {
	DateTimeStruct currTime;

	if (!HAQuDA_TimeHelper_Singleton::getInstance()->GetDateTime(&currTime)) {
		return 0; // TODO: Add Get date and time error handler;
	}

	bool isBordersInDifferentDays = (timeBorder.timeSecondBorder < timeBorder.timeFirstBorder);

	if (isBordersInDifferentDays) {
		if ((currTime.hour >= timeBorder.timeFirstBorder) || (currTime.hour <= timeBorder.timeSecondBorder)) {
			int8_t hourDiff = currTime.hour - timeBorder.timeFirstBorder;
			hourDiff += (hourDiff < 0) ? 24 : 0;
			return hourDiff;
		} else {
			return 12;
		}
	} else {
		if ((currTime.hour >= timeBorder.timeFirstBorder) && (currTime.hour <= timeBorder.timeSecondBorder)) {
			int8_t hourDiff = currTime.hour - timeBorder.timeFirstBorder;
			return hourDiff;
		} else {
			return 12;
		}
	}
	return 12;
}

void HAQuDA_DisplayManip::nightMode() {
	uint8_t nightMode_hour = get_nightMode_hour(Display.measParams.nightModeTimeBorder);

	switch (Display.measParams.type) {
		case temp: {
			HAQuDA_DisplayManip::showMeas_night(temp_meas.value / temp_meas.measNum, Display.measParams.divDots.temp, nightMode_hour);
			temp_meas.value = 0;
			temp_meas.measNum = 0;
			break;
		}
		case humid: {
			HAQuDA_DisplayManip::showMeas_night(humid_meas.value / humid_meas.measNum, Display.measParams.divDots.humid, nightMode_hour);
			humid_meas.value = 0;
			humid_meas.measNum = 0;
			break;
		}
		case eCO2: {
			HAQuDA_DisplayManip::showMeas_night(eCO2_meas.value / eCO2_meas.measNum, Display.measParams.divDots.eCO2, nightMode_hour);
			eCO2_meas.value = 0;
			eCO2_meas.measNum = 0;
			break;
		}
		case TVOC: {
			HAQuDA_DisplayManip::showMeas_night(TVOC_meas.value / TVOC_meas.measNum, Display.measParams.divDots.TVOC, nightMode_hour);
			TVOC_meas.value = 0;
			TVOC_meas.measNum = 0;
			break;
		}
		case PM2_5: {
			HAQuDA_DisplayManip::showMeas_night(PM_2_5_meas.value / PM_2_5_meas.measNum, Display.measParams.divDots.PM2_5, nightMode_hour);
			PM_2_5_meas.value = 0;
			PM_2_5_meas.measNum = 0;
			break;
		}
		default:
			break;
	}
}

void HAQuDA_DisplayManip::getMeasRGB(uint8_t *_red, uint8_t *_green, uint8_t *_blue, const uint8_t brightness, const float data, const div_dots divideDots) {
	float coefficient = (brightness * 2) / (divideDots.thirdDot - divideDots.firstDot);
	if ((data < divideDots.middleDot) && (data >= divideDots.firstDot)) {
		*_blue = round(-abs(data - divideDots.firstDot) * coefficient) + MAX_BRIGHTNESS;
		*_green = round(-abs(data - divideDots.middleDot) * coefficient) + MAX_BRIGHTNESS;
		*_red = 0;
	} else if ((data >= divideDots.middleDot) && (data <= divideDots.thirdDot)) { // calculating values of LED colors (using parametres of color function)
		*_blue = 0;
		*_green = round(-abs(data - divideDots.middleDot) * coefficient) + MAX_BRIGHTNESS;
		*_red = round(-abs(data - divideDots.thirdDot) * coefficient) + MAX_BRIGHTNESS;
	} else if (data < divideDots.firstDot) {
		*_blue = MAX_BRIGHTNESS;
		*_green = 0;
		*_red = 0;
	} else if (data > divideDots.thirdDot) {
		*_blue = 0;
		*_green = 0;
		*_red = MAX_BRIGHTNESS;
	}
}

void HAQuDA_DisplayManip::showMeas_standard(float data, div_dots divideDots) {
	WS2812_clear();
	vTaskDelay(100 / portTICK_PERIOD_MS);

	uint8_t red, green, blue; // values of LED colors
	uint8_t brightness = WS2812_getBrightness();
	getMeasRGB(&red, &green, &blue, brightness, data, divideDots);
	WS2812_fillColor(WS2812_getColor(red, green, blue));
}

void HAQuDA_DisplayManip::showMeas_night(float data, div_dots divideDots, uint8_t time) {
	WS2812_clear();
	vTaskDelay(100 / portTICK_PERIOD_MS);

	uint8_t red, green, blue; // values of LED colors
	uint8_t brightness = WS2812_getBrightness();
	getMeasRGB(&red, &green, &blue, brightness, data, divideDots);

	for (int i = 0; i < LED_COLUMN_NUM; i++) {
		uint8_t whiteBrightness = brightness * 1000 / MAX_BRIGHTNESS * WHITE_BRIGHTNESS_COEFF / 1000;
		uint32_t color = (!(i % 3)) ? WS2812_getColor(red, green, blue) : WS2812_getColor(whiteBrightness, whiteBrightness, whiteBrightness);
		uint8_t pixelNum = (!(i % 3)) ? LED_ROW_NUM : time;
		uint8_t startPixel = (!(i % 2)) ? (i * LED_ROW_NUM) : (i * LED_ROW_NUM + LED_ROW_NUM - pixelNum);
		WS2812_fillColor(color, startPixel, pixelNum);
	}
}

void HAQuDA_DisplayManip::showMeas_multi(float *data, uint8_t dataSize, div_dots *divideDots, uint8_t divideDotsSize) {
	if (dataSize != MULTI_MODE_PARAM_NUM) {
		return;
	}
	if (divideDotsSize != DIVIDE_DOTS_NUM) {
		return;
	}

	WS2812_clear();
	vTaskDelay(100 / portTICK_PERIOD_MS);

	uint8_t red[MULTI_MODE_PARAM_NUM], green[MULTI_MODE_PARAM_NUM], blue[MULTI_MODE_PARAM_NUM]; // values of LED colors
	uint8_t brightness = WS2812_getBrightness();
	for (int i = 0; i < MULTI_MODE_PARAM_NUM; i++) {
		getMeasRGB(&(red[i]), &(green[i]), &(blue[i]), brightness, data[i], divideDots[i]);
	}
	for (int i = 0; i < LED_ROW_NUM; i++) {
		if (!(i % 2)) {
			WS2812_fillColor(WS2812_getColor(red[0], green[0], blue[0]), i * LED_ROW_NUM + 1, 3);
			WS2812_fillColor(WS2812_getColor(red[1], green[1], blue[1]), i * LED_ROW_NUM + 5, 3);
			WS2812_fillColor(WS2812_getColor(red[2], green[2], blue[2]), i * LED_ROW_NUM + 9, 3);
		} else {
			WS2812_fillColor(WS2812_getColor(red[2], green[2], blue[2]), i * LED_ROW_NUM, 3);
			WS2812_fillColor(WS2812_getColor(red[1], green[1], blue[1]), i * LED_ROW_NUM + 4, 3);
			WS2812_fillColor(WS2812_getColor(red[0], green[0], blue[0]), i * LED_ROW_NUM + 8, 3);
		}
	}
}

void HAQuDA_DisplayManip::showMeas_total(float *data, uint8_t dataSize, div_dots *dataDivideDots, uint8_t divideDotsSize) {
	if (dataSize != MULTI_MODE_PARAM_NUM) {
		return;
	}
	if (divideDotsSize != DIVIDE_DOTS_NUM) {
		return;
	}
	uint8_t colorsArr[DISP_PARAMS_NUM][COLORS_NUM];
	uint8_t brightness = WS2812_getBrightness();
	for (int i = 0; i < DISP_PARAMS_NUM; i++) {
		getMeasRGB(&(colorsArr[i][0]), &(colorsArr[i][1]), &(colorsArr[i][2]), brightness, data[i],
				   dataDivideDots[i]); // calculating all 3 colors for all displayable parameters
	}

	uint8_t red, green, blue; // values of LED colors
	for (int i = 0; i < DISP_PARAMS_NUM; i++) {
		red += colorsArr[i][0];
		green += colorsArr[i][1]; // calculating mean of colors of all displayable parameters
		blue += colorsArr[i][2];
	}
	red /= DISP_PARAMS_NUM;
	green /= DISP_PARAMS_NUM;
	blue /= DISP_PARAMS_NUM;
	WS2812_fillColor(WS2812_getColor(red, green, blue), 0, LED_NUM_PIXELS);
}

uint8_t HAQuDA_DisplayManip::getLedNum(uint8_t x, uint8_t y) {

	if ((x >= LED_COLUMN_NUM) || (y >= LED_ROW_NUM)) {
		return 0;
	}

	y = (x % 2 != 0) ? (LED_ROW_NUM - y - 1) : y;

	return y + LED_ROW_NUM * x;
}

HAQuDA_DisplayManip::~HAQuDA_DisplayManip() {
}