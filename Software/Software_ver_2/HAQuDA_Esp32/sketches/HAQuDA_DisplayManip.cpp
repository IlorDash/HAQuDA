#include "HAQuDA_DisplayManip.h"

#include <vector>

HAQuDA_DisplayManip::~HAQuDA_DisplayManip() {
	// delete timeClient;
}

void HAQuDA_DisplayManip::startNTP() {
	timeClient = new NTPClient(ntpUDP);

	// Initialize a NTPClient to get time
	timeClient->begin();
	// Set offset time in seconds to adjust for your timezone, for example:
	// GMT +1 = 3600
	// GMT +8 = 28800
	// GMT +3 = 10800
	// GMT 0 = 0
	timeClient->setTimeOffset(10800);
}

void HAQuDA_DisplayManip::displayMeas(displayMeasParams_struct currDisplayMeasParams) {
	switch (currDisplayMeasParams.displayMode) {
		case standard: {
			standardMode(currDisplayMeasParams);
			break;
		}
		case multi: {
			multiMode(currDisplayMeasParams.multiModeStruct);
			break;
		}
		case night: {
			nightMode(currDisplayMeasParams);
			break;
		}
		default:
			break;
	}
}

displayParams_enum HAQuDA_DisplayManip::checkBadParam(displayMeasParams_struct currDisplayMeasParams) {
	if (((temp_meas.value / temp_meas.measNum) >= currDisplayMeasParams.temp_divideDots.thirdDot)
		|| ((temp_meas.value / temp_meas.measNum) <= currDisplayMeasParams.temp_divideDots.firstDot)) {
		return temp;
	} else if (((humid_meas.value / humid_meas.measNum) >= currDisplayMeasParams.humid_divideDots.thirdDot)
			   || ((humid_meas.value / humid_meas.measNum) <= currDisplayMeasParams.humid_divideDots.firstDot)) {
		return humid;
	} else if ((eCO2_meas.value / eCO2_meas.measNum) >= currDisplayMeasParams.eCO2_divideDots.thirdDot) {
		return eCO2;
	} else if ((TVOC_meas.value / TVOC_meas.measNum) >= currDisplayMeasParams.TVOC_divideDots.thirdDot) {
		return TVOC;
	} else if ((PM_2_5_meas.value / PM_2_5_meas.measNum) >= currDisplayMeasParams.PM2_5_divideDots.thirdDot) {
		return PM2_5;
	}
	return noneParam;
}

void HAQuDA_DisplayManip::standardMode(displayMeasParams_struct currDisplayMeasParams) {
	switch (currDisplayMeasParams.displayParam) {
		case total: {
			displayParams_enum badParam = checkBadParam(currDisplayMeasParams);
			switch (badParam) {
				case temp: {
					HAQuDA_DisplayManip::showMeas_standard(temp_meas.value / temp_meas.measNum, currDisplayMeasParams.temp_divideDots);
					break;
				}
				case humid: {
					HAQuDA_DisplayManip::showMeas_standard(humid_meas.value / humid_meas.measNum, currDisplayMeasParams.humid_divideDots);
					break;
				}
				case eCO2: {
					HAQuDA_DisplayManip::showMeas_standard(eCO2_meas.value / eCO2_meas.measNum, currDisplayMeasParams.eCO2_divideDots);
					break;
				}
				case TVOC: {
					HAQuDA_DisplayManip::showMeas_standard(TVOC_meas.value / TVOC_meas.measNum, currDisplayMeasParams.TVOC_divideDots);
					break;
				}
				case PM2_5: {
					HAQuDA_DisplayManip::showMeas_standard(PM_2_5_meas.value / PM_2_5_meas.measNum, currDisplayMeasParams.PM2_5_divideDots);
					break;
				}
				case noneParam: {
					float measValuesArr[DISP_PARAMS_NUM];
					measDivideDots_struct measDivideDots[DISP_PARAMS_NUM];

					measValuesArr[0] = temp_meas.value / temp_meas.measNum;
					measDivideDots[0] = currDisplayMeasParams.temp_divideDots;

					measValuesArr[1] = humid_meas.value / humid_meas.measNum;
					measDivideDots[1] = currDisplayMeasParams.humid_divideDots;

					measValuesArr[2] = eCO2_meas.value / eCO2_meas.measNum;
					measDivideDots[2] = currDisplayMeasParams.eCO2_divideDots;

					measValuesArr[3] = TVOC_meas.value / TVOC_meas.measNum;
					measDivideDots[3] = currDisplayMeasParams.TVOC_divideDots;

					measValuesArr[4] = PM_2_5_meas.value / PM_2_5_meas.measNum;
					measDivideDots[4] = currDisplayMeasParams.PM2_5_divideDots;

					HAQuDA_DisplayManip::showMeas_total(measValuesArr, sizeof(measValuesArr), measDivideDots, sizeof(measDivideDots));
					break;
				}
				default:
					break;
			}
			break;
		}
		case temp: {
			HAQuDA_DisplayManip::showMeas_standard(temp_meas.value / temp_meas.measNum, currDisplayMeasParams.temp_divideDots);
			break;
		}
		case humid: {
			HAQuDA_DisplayManip::showMeas_standard(humid_meas.value / humid_meas.measNum, currDisplayMeasParams.humid_divideDots);
			break;
		}
		case eCO2: {
			HAQuDA_DisplayManip::showMeas_standard(eCO2_meas.value / eCO2_meas.measNum, currDisplayMeasParams.eCO2_divideDots);
			break;
		}
		case TVOC: {
			HAQuDA_DisplayManip::showMeas_standard(TVOC_meas.value / TVOC_meas.measNum, currDisplayMeasParams.TVOC_divideDots);
			break;
		}
		case PM2_5: {
			HAQuDA_DisplayManip::showMeas_standard(PM_2_5_meas.value / PM_2_5_meas.measNum, currDisplayMeasParams.PM2_5_divideDots);
			break;
		}

		default:
			break;
	}
}

void HAQuDA_DisplayManip::multiMode(multiModeParamDisplay_struct multiModeParams) {
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

uint8_t HAQuDA_DisplayManip::get_nightMode_hour(nightModeTimeBorder_struct timeBorder) {
	while (!timeClient->update()) {
		timeClient->forceUpdate();
	}
	// The formattedDate comes with the following format:
	// 2018-05-28T16:00:13Z
	// We need to extract date and time
	// Variables to save date and time
	String formattedDate;
	uint8_t curHour;

	formattedDate = timeClient->getFormattedDate();
	Serial.println(formattedDate);

	// Extract date
	int splitT = formattedDate.indexOf("T");
	int splitColon = formattedDate.indexOf(":");
	// Extract time
	curHour = formattedDate.substring(splitT + 1, splitColon).toInt();

	bool isBordersInDifferentDays = (timeBorder.timeSecondBorder < timeBorder.timeFirstBorder);

	if (isBordersInDifferentDays) {
		if ((curHour >= timeBorder.timeFirstBorder) || (curHour <= timeBorder.timeSecondBorder)) {
			int8_t hourDiff = curHour - timeBorder.timeFirstBorder;
			hourDiff += (hourDiff < 0) ? 24 : 0;
			return hourDiff;
		} else {
			return 12;
		}
	} else {
		if ((curHour >= timeBorder.timeFirstBorder) && (curHour <= timeBorder.timeSecondBorder)) {
			int8_t hourDiff = curHour - timeBorder.timeFirstBorder;
			return hourDiff;
		} else {
			return 12;
		}
	}
	return 12;
}

void HAQuDA_DisplayManip::nightMode(displayMeasParams_struct currDisplayMeasParams) {
	uint8_t nightMode_hour = get_nightMode_hour(currDisplayMeasParams.currentTimeBorder);

	switch (currDisplayMeasParams.displayParam) {
		case temp: {
			HAQuDA_DisplayManip::showMeas_night(temp_meas.value / temp_meas.measNum, currDisplayMeasParams.temp_divideDots, nightMode_hour);
			temp_meas.value = 0;
			temp_meas.measNum = 0;
			break;
		}
		case humid: {
			HAQuDA_DisplayManip::showMeas_night(humid_meas.value / humid_meas.measNum, currDisplayMeasParams.humid_divideDots, nightMode_hour);
			humid_meas.value = 0;
			humid_meas.measNum = 0;
			break;
		}
		case eCO2: {
			HAQuDA_DisplayManip::showMeas_night(eCO2_meas.value / eCO2_meas.measNum, currDisplayMeasParams.eCO2_divideDots, nightMode_hour);
			eCO2_meas.value = 0;
			eCO2_meas.measNum = 0;
			break;
		}
		case TVOC: {
			HAQuDA_DisplayManip::showMeas_night(TVOC_meas.value / TVOC_meas.measNum, currDisplayMeasParams.TVOC_divideDots, nightMode_hour);
			TVOC_meas.value = 0;
			TVOC_meas.measNum = 0;
			break;
		}
		case PM2_5: {
			HAQuDA_DisplayManip::showMeas_night(PM_2_5_meas.value / PM_2_5_meas.measNum, currDisplayMeasParams.PM2_5_divideDots, nightMode_hour);
			PM_2_5_meas.value = 0;
			PM_2_5_meas.measNum = 0;
			break;
		}
		default:
			break;
	}
}

void HAQuDA_DisplayManip::getMeasRGB(uint8_t *_red, uint8_t *_green, uint8_t *_blue, const uint8_t brightness, const float data,
									 const measDivideDots_struct divideDots) {
	float coefficient = (brightness * 2) / (divideDots.thirdDot - divideDots.firstDot);
	if ((data < divideDots.secondDot) && (data >= divideDots.firstDot)) {
		*_blue = round(-abs(data - divideDots.firstDot) * coefficient) + MAX_BRIGHTNESS;
		*_green = round(-abs(data - divideDots.secondDot) * coefficient) + MAX_BRIGHTNESS;
		*_red = 0;
	} else if ((data >= divideDots.secondDot) && (data <= divideDots.thirdDot)) { // calculating values of LED colors (using parametres of color function)
		*_blue = 0;
		*_green = round(-abs(data - divideDots.secondDot) * coefficient) + MAX_BRIGHTNESS;
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

void HAQuDA_DisplayManip::showMeas_standard(float data, measDivideDots_struct divideDots) {
	WS2812_clear();
	vTaskDelay(100);

	uint8_t red, green, blue; // values of LED colors
	uint8_t brightness = WS2812_getBrightness();
	getMeasRGB(&red, &green, &blue, brightness, data, divideDots);
	WS2812_fillColor(WS2812_getColor(red, green, blue));
}

void HAQuDA_DisplayManip::showMeas_night(float data, measDivideDots_struct divideDots, uint8_t time) {
	WS2812_clear();
	vTaskDelay(100);

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

void HAQuDA_DisplayManip::showMeas_multi(float *data, uint8_t dataSize, measDivideDots_struct *divideDots, uint8_t divideDotsSize) {
	if (dataSize != MULTI_MODE_PARAM_NUM) {
		return;
	}
	if (divideDotsSize != DIVIDE_DOTS_NUM) {
		return;
	}

	WS2812_clear();
	vTaskDelay(100);

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

void HAQuDA_DisplayManip::showMeas_total(float *data, uint8_t dataSize, measDivideDots_struct *dataDivideDots, uint8_t divideDotsSize) {
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




void HAQuDA_DisplayManip::showEffectGrow(const growEffectsParams_struct params, const displayEffectMode_enum *effect) {
	WS2812_clear();

	for (uint8_t i = 0; i < LED_ROW_NUM; i++) {
		for (uint8_t j = 0; j < LED_COLUMN_NUM; j++) {
			uint8_t pixelNum = GetLedNum(i, j);
			WS2812_setPixelColor(pixelNum, COLOR_RED);
			WS2812_show();

			if (*effect != grow) {
				break;
			}
		}
	}
}

void HAQuDA_DisplayManip::showEffectSnake(const snakeEffectsParams_struct params, const displayEffectMode_enum *effect) {
	WS2812_clear();
	for (int i = 0; i < LED_NUM_PIXELS; i++) {
		int pixelNum = 0;
		for (int j = 0; j < params.tailLength; j++) {
			pixelNum = j + i;
			pixelNum = (pixelNum > LED_NUM_PIXELS) ? LED_NUM_PIXELS : pixelNum;
			WS2812_setPixelColor(pixelNum, params.color);

			if (*effect != snake) {
				break;
			}
		}
		WS2812_setPixelColor(i, 0);
		WS2812_show(params.speed);
	}
}

void HAQuDA_DisplayManip::showEffectRandom(const randomEffectsParams_struct params, const displayEffectMode_enum *effect) {
	WS2812_clear();
	std::vector<int> pixelEnArr;
	for (int i = 0; i < LED_NUM_PIXELS; i++) {
		int pixelNum = random(0, LED_NUM_PIXELS);
		while (find(pixelEnArr.begin(), pixelEnArr.end(), pixelNum) != pixelEnArr.end()) {
			pixelNum = random(0, LED_NUM_PIXELS);

			if (*effect != randomPixel) {
				break;
			}
		}
		pixelEnArr.push_back(pixelNum);

		WS2812_setPixelColor(pixelNum, ((uint32_t)random(0, 255) << 16) | ((uint32_t)random(0, 255) << 8) | random(0, 255));
		WS2812_show(params.speed);
	}
	vTaskDelay(params.pauseTime / portTICK_PERIOD_MS);
}

void HAQuDA_DisplayManip::showEffectFade(const fadeEffectsParams_struct params, const displayEffectMode_enum *effect) {
	WS2812_clear();
	WS2812_fillColor(params.color);
	for (int j = params.startBrightness; j > params.stopBrightness; j = j - params.step) {
		WS2812_setBrightnessPerCent(j);
		if (*effect != fade) {
			break;
		}
	}
}

uint8_t HAQuDA_DisplayManip::GetLedNum(int x, int y) {
	int n;

	x = (x >= LED_COLUMN_NUM) ? (x % LED_COLUMN_NUM) : x;
	y = (y >= LED_ROW_NUM) ? (y % LED_ROW_NUM) : y;

	if (x % 2 == 0) {
		n = LED_ROW_NUM - y;
	} else {
		n = y + 1;
	}
	return n + LED_ROW_NUM * x;
}

void HAQuDA_DisplayManip::christmasLightsSnake() {
	uint8_t startColumn = LED_COLUMN_NUM - 1;

	//	for (int i = 0; i < length; i++) {
	//	}
}

void HAQuDA_DisplayManip::drawStarOnTree(int middleColumn) {
	WS2812_setPixelColor(GetLedNum(middleColumn, LED_ROW_NUM - 1), CHRISTMAS_TREE_STAR_COLOR);
	WS2812_setPixelColor(GetLedNum(middleColumn, LED_ROW_NUM - 2), CHRISTMAS_TREE_STAR_COLOR);
	WS2812_setPixelColor(GetLedNum(middleColumn, LED_ROW_NUM - 3), CHRISTMAS_TREE_STAR_COLOR);
	WS2812_setPixelColor(GetLedNum(middleColumn - 1, LED_ROW_NUM - 2), CHRISTMAS_TREE_STAR_COLOR);
	WS2812_setPixelColor(GetLedNum(middleColumn + 1, LED_ROW_NUM - 2), CHRISTMAS_TREE_STAR_COLOR);
}

void HAQuDA_DisplayManip::drawTreeTrunk(uint8_t middleColumn) {
	WS2812_fillColor(trunkColor, 0, GetLedNum(middleColumn, 0));
}

void HAQuDA_DisplayManip::drawTreeTop(uint8_t middleColumn) {

	bool isLedStripFromUp = (middleColumn % 2 == 0);

	int globalMiddleColumn = LED_COLUMN_NUM / 2;
	int middleColumnOffset = globalMiddleColumn - middleColumn;
	for (int i = 0; i < (CHRISTMAS_TREE_HEIGHT / 2); i++) {
		for (int j = (globalMiddleColumn - CHRISTMAS_TREE_HALF_WIDTH + i); j <= (globalMiddleColumn + CHRISTMAS_TREE_HALF_WIDTH - i); j++) {
			WS2812_setPixelColor(GetLedNum(j + middleColumnOffset, i), CHRISTMAS_TREE_COLOR);
			WS2812_setPixelColor(GetLedNum(j + middleColumnOffset, i + 1), CHRISTMAS_TREE_COLOR);
		}
	}
}

void HAQuDA_DisplayManip::showEffectChristmasTree(uint8_t speed, uint8_t treeMiddleColumn) {
	uint8_t redBackground = redBackgroundIntensity * 1000 / MAX_BRIGHTNESS * WHITE_BRIGHTNESS_COEFF / 1000;
	uint8_t greenBackground = greenBackgroundIntensity * 1000 / MAX_BRIGHTNESS * WHITE_BRIGHTNESS_COEFF / 1000;
	uint8_t blueBackground = blueBackgroundIntensity * 1000 / MAX_BRIGHTNESS * WHITE_BRIGHTNESS_COEFF / 1000;
	uint32_t backgroundColor = WS2812_getColor(redBackground, greenBackground, blueBackground);
	WS2812_fillColor(backgroundColor); // drawing christmas tree with white-blue background

	HAQuDA_DisplayManip::drawTreeTrunk(treeMiddleColumn);
	HAQuDA_DisplayManip::drawTreeTop(treeMiddleColumn);
	HAQuDA_DisplayManip::drawStarOnTree(treeMiddleColumn);
	// uint8_t lightsEffect = rand() % 3;
}