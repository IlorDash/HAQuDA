#include "HAQuDA_DisplayManip.h"

HAQuDA_DisplayManip::~HAQuDA_DisplayManip() {
	//delete timeClient;
}

void HAQuDA_DisplayManip::startNTP() {

	// Initialize a NTPClient to get time
	timeClient.begin();
	// Set offset time in seconds to adjust for your timezone, for example:
	// GMT +1 = 3600
	// GMT +8 = 28800
	// GMT +3 = 10800
	// GMT 0 = 0
	timeClient.setTimeOffset(10800);
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
					WS2812_showParams_standard(temp_meas.value / temp_meas.measNum, currDisplayMeasParams.temp_divideDots);
					break;
				}
				case humid: {
					WS2812_showParams_standard(humid_meas.value / humid_meas.measNum, currDisplayMeasParams.humid_divideDots);
					break;
				}
				case eCO2: {
					WS2812_showParams_standard(eCO2_meas.value / eCO2_meas.measNum, currDisplayMeasParams.eCO2_divideDots);
					break;
				}
				case TVOC: {
					WS2812_showParams_standard(TVOC_meas.value / TVOC_meas.measNum, currDisplayMeasParams.TVOC_divideDots);
					break;
				}
				case PM2_5: {
					WS2812_showParams_standard(PM_2_5_meas.value / PM_2_5_meas.measNum, currDisplayMeasParams.PM2_5_divideDots);
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

					WS2812_showParams_standardTotal(measValuesArr, measDivideDots);
					break;
				}
				default:
					break;
			}
			break;
		}
		case temp: {
			WS2812_showParams_standard(temp_meas.value / temp_meas.measNum, currDisplayMeasParams.temp_divideDots);
			break;
		}
		case humid: {
			WS2812_showParams_standard(humid_meas.value / humid_meas.measNum, currDisplayMeasParams.humid_divideDots);
			break;
		}
		case eCO2: {
			WS2812_showParams_standard(eCO2_meas.value / eCO2_meas.measNum, currDisplayMeasParams.eCO2_divideDots);
			break;
		}
		case TVOC: {
			WS2812_showParams_standard(TVOC_meas.value / TVOC_meas.measNum, currDisplayMeasParams.TVOC_divideDots);
			break;
		}
		case PM2_5: {
			WS2812_showParams_standard(PM_2_5_meas.value / PM_2_5_meas.measNum, currDisplayMeasParams.PM2_5_divideDots);
			break;
		}

		default:
			break;
	}
}

void HAQuDA_DisplayManip::multiMode(displayMeasParams_struct currDisplayMeasParams) {
	for (int i = 0; i < MULTI_MODE_PARAM_NUM; i++) {
		switch (currDisplayMeasParams.multiModeStruct.paramsArr[i]) {
			case temp: {
				currDisplayMeasParams.multiModeStruct.dataArr[i] = temp_meas.value / temp_meas.measNum;
				break;
			}
			case humid: {
				currDisplayMeasParams.multiModeStruct.dataArr[i] = humid_meas.value / humid_meas.measNum;
				break;
			}
			case eCO2: {
				currDisplayMeasParams.multiModeStruct.dataArr[i] = eCO2_meas.value / eCO2_meas.measNum;
				break;
			}
			case PM2_5: {
				currDisplayMeasParams.multiModeStruct.dataArr[i] = PM_2_5_meas.value / PM_2_5_meas.measNum;
				break;
			}
			case TVOC: {
				currDisplayMeasParams.multiModeStruct.dataArr[i] = TVOC_meas.value / TVOC_meas.measNum;
				break;
			}
			default:
				break;
		}
	}
	WS2812_showParams_multi(currDisplayMeasParams.multiModeStruct.dataArr, currDisplayMeasParams.multiModeStruct.divideDotsArr);
}

uint8_t HAQuDA_DisplayManip::get_nightMode_hour(uint8_t curHour, displayMeasParams_struct currDisplayMeasParams) {
	bool isBordersInDifferentDays = (currDisplayMeasParams.currentTimeBorder.timeSecondBorder < currDisplayMeasParams.currentTimeBorder.timeFirstBorder);

	if (isBordersInDifferentDays) {
		if ((curHour >= currDisplayMeasParams.currentTimeBorder.timeFirstBorder) || (curHour <= currDisplayMeasParams.currentTimeBorder.timeSecondBorder)) {
			int8_t hourDiff = curHour - currDisplayMeasParams.currentTimeBorder.timeFirstBorder;
			hourDiff += (hourDiff < 0) ? 24 : 0;
			return hourDiff;
		} else {
			return 12;
		}
	} else {
		if ((curHour >= currDisplayMeasParams.currentTimeBorder.timeFirstBorder) && (curHour <= currDisplayMeasParams.currentTimeBorder.timeSecondBorder)) {
			int8_t hourDiff = curHour - currDisplayMeasParams.currentTimeBorder.timeFirstBorder;
			return hourDiff;
		} else {
			return 12;
		}
	}
	return 12;
}

void HAQuDA_DisplayManip::nightMode(displayMeasParams_struct currDisplayMeasParams) {
	while (!timeClient.update()) {
		timeClient.forceUpdate();
	}
	// The formattedDate comes with the following format:
	// 2018-05-28T16:00:13Z
	// We need to extract date and time
	// Variables to save date and time
	String formattedDate;
	uint8_t curHour;

	formattedDate = timeClient.getFormattedDate();
	Serial.println(formattedDate);

	// Extract date
	int splitT = formattedDate.indexOf("T");
	int splitColon = formattedDate.indexOf(":");
	// Extract time
	curHour = formattedDate.substring(splitT + 1, splitColon).toInt();

	uint8_t nightMode_hour = get_nightMode_hour(curHour, currDisplayMeasParams);

	switch (currDisplayMeasParams.displayParam) {
		case temp: {
			WS2812_showParams_night(temp_meas.value / temp_meas.measNum, currDisplayMeasParams.temp_divideDots, nightMode_hour);
			temp_meas.value = 0;
			temp_meas.measNum = 0;
			break;
		}
		case humid: {
			WS2812_showParams_night(humid_meas.value / humid_meas.measNum, currDisplayMeasParams.humid_divideDots, nightMode_hour);
			humid_meas.value = 0;
			humid_meas.measNum = 0;
			break;
		}
		case eCO2: {
			WS2812_showParams_night(eCO2_meas.value / eCO2_meas.measNum, currDisplayMeasParams.eCO2_divideDots, nightMode_hour);
			eCO2_meas.value = 0;
			eCO2_meas.measNum = 0;
			break;
		}
		case TVOC: {
			WS2812_showParams_night(TVOC_meas.value / TVOC_meas.measNum, currDisplayMeasParams.TVOC_divideDots, nightMode_hour);
			TVOC_meas.value = 0;
			TVOC_meas.measNum = 0;
			break;
		}
		case PM2_5: {
			WS2812_showParams_night(PM_2_5_meas.value / PM_2_5_meas.measNum, currDisplayMeasParams.PM2_5_divideDots, nightMode_hour);
			PM_2_5_meas.value = 0;
			PM_2_5_meas.measNum = 0;
			break;
		}
		default:
			break;
	}
}

void HAQuDA_DisplayManip::displayData(displayMeasParams_struct currDisplayMeasParams) {
	switch (currDisplayMeasParams.displayMode) {
		case standard: {
			standardMode(currDisplayMeasParams);
			break;
		}
		case multi: {
			multiMode(currDisplayMeasParams);
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