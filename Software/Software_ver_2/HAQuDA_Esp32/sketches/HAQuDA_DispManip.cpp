#include "HAQuDA_DispManip.h"

HAQuDA_DispManip::~HAQuDA_DispManip() {
	//delete timeClient;
}

void HAQuDA_DispManip::startNTP() {

	// Initialize a NTPClient to get time
	timeClient.begin();
	// Set offset time in seconds to adjust for your timezone, for example:
	// GMT +1 = 3600
	// GMT +8 = 28800
	// GMT +3 = 10800
	// GMT 0 = 0
	timeClient.setTimeOffset(10800);
}

dispParams_enum HAQuDA_DispManip::checkBadParam(UI_Params currUI_Params) {
	if (((temp_meas.value / temp_meas.measNum) >= currUI_Params.temp_divideDots.thirdDot)
		|| ((temp_meas.value / temp_meas.measNum) <= currUI_Params.temp_divideDots.firstDot)) {
		return temp;
	} else if (((humid_meas.value / humid_meas.measNum) >= currUI_Params.humid_divideDots.thirdDot)
			   || ((humid_meas.value / humid_meas.measNum) <= currUI_Params.humid_divideDots.firstDot)) {
		return humid;
	} else if ((eCO2_meas.value / eCO2_meas.measNum) >= currUI_Params.eCO2_divideDots.thirdDot) {
		return eCO2;
	} else if ((TVOC_meas.value / TVOC_meas.measNum) >= currUI_Params.TVOC_divideDots.thirdDot) {
		return TVOC;
	} else if ((PM_2_5_meas.value / PM_2_5_meas.measNum) >= currUI_Params.PM2_5_divideDots.thirdDot) {
		return PM2_5;
	}
	return noneParam;
}

void HAQuDA_DispManip::standardMode(UI_Params currUI_Params) {
	switch (currUI_Params.dispParam) {
		case total: {
			dispParams_enum badParam = checkBadParam(currUI_Params);
			switch (badParam) {
				case temp: {
					WS2812_showParams_standard(temp_meas.value / temp_meas.measNum, currUI_Params.temp_divideDots);
					break;
				}
				case humid: {
					WS2812_showParams_standard(humid_meas.value / humid_meas.measNum, currUI_Params.humid_divideDots);
					break;
				}
				case eCO2: {
					WS2812_showParams_standard(eCO2_meas.value / eCO2_meas.measNum, currUI_Params.eCO2_divideDots);
					break;
				}
				case TVOC: {
					WS2812_showParams_standard(TVOC_meas.value / TVOC_meas.measNum, currUI_Params.TVOC_divideDots);
					break;
				}
				case PM2_5: {
					WS2812_showParams_standard(PM_2_5_meas.value / PM_2_5_meas.measNum, currUI_Params.PM2_5_divideDots);
					break;
				}
				case noneParam: {
					float measValuesArr[DISP_PARAMS_NUM];
					measDivideDots_struct measDivideDots[DISP_PARAMS_NUM];

					measValuesArr[0] = temp_meas.value / temp_meas.measNum;
					measDivideDots[0] = currUI_Params.temp_divideDots;

					measValuesArr[1] = humid_meas.value / humid_meas.measNum;
					measDivideDots[1] = currUI_Params.humid_divideDots;

					measValuesArr[2] = eCO2_meas.value / eCO2_meas.measNum;
					measDivideDots[2] = currUI_Params.eCO2_divideDots;

					measValuesArr[3] = TVOC_meas.value / TVOC_meas.measNum;
					measDivideDots[3] = currUI_Params.TVOC_divideDots;

					measValuesArr[4] = PM_2_5_meas.value / PM_2_5_meas.measNum;
					measDivideDots[4] = currUI_Params.PM2_5_divideDots;

					WS2812_showParams_standardTotal(measValuesArr, measDivideDots);
					break;
				}
				default:
					break;
			}
			break;
		}
		case temp: {
			WS2812_showParams_standard(temp_meas.value / temp_meas.measNum, currUI_Params.temp_divideDots);
			break;
		}
		case humid: {
			WS2812_showParams_standard(humid_meas.value / humid_meas.measNum, currUI_Params.humid_divideDots);
			break;
		}
		case eCO2: {
			WS2812_showParams_standard(eCO2_meas.value / eCO2_meas.measNum, currUI_Params.eCO2_divideDots);
			break;
		}
		case TVOC: {
			WS2812_showParams_standard(TVOC_meas.value / TVOC_meas.measNum, currUI_Params.TVOC_divideDots);
			break;
		}
		case PM2_5: {
			WS2812_showParams_standard(PM_2_5_meas.value / PM_2_5_meas.measNum, currUI_Params.PM2_5_divideDots);
			break;
		}

		default:
			break;
	}
}

void HAQuDA_DispManip::multiMode(UI_Params currUI_Params) {
	for (int i = 0; i < MULTI_MODE_PARAM_NUM; i++) {
		switch (currUI_Params.multiModeStruct.paramsArr[i]) {
			case temp: {
				currUI_Params.multiModeStruct.dataArr[i] = temp_meas.value / temp_meas.measNum;
				break;
			}
			case humid: {
				currUI_Params.multiModeStruct.dataArr[i] = humid_meas.value / humid_meas.measNum;
				break;
			}
			case eCO2: {
				currUI_Params.multiModeStruct.dataArr[i] = eCO2_meas.value / eCO2_meas.measNum;
				break;
			}
			case PM2_5: {
				currUI_Params.multiModeStruct.dataArr[i] = PM_2_5_meas.value / PM_2_5_meas.measNum;
				break;
			}
			case TVOC: {
				currUI_Params.multiModeStruct.dataArr[i] = TVOC_meas.value / TVOC_meas.measNum;
				break;
			}
			default:
				break;
		}
	}
	WS2812_showParams_multi(currUI_Params.multiModeStruct.dataArr, currUI_Params.multiModeStruct.divideDotsArr);
}

uint8_t HAQuDA_DispManip::get_nightMode_hour(uint8_t curHour, UI_Params currUI_Params) {
	bool isBordersInDifferentDays = (currUI_Params.currentTimeBorder.timeSecondBorder < currUI_Params.currentTimeBorder.timeFirstBorder);

	if (isBordersInDifferentDays) {
		if ((curHour >= currUI_Params.currentTimeBorder.timeFirstBorder) || (curHour <= currUI_Params.currentTimeBorder.timeSecondBorder)) {
			int8_t hourDiff = curHour - currUI_Params.currentTimeBorder.timeFirstBorder;
			hourDiff += (hourDiff < 0) ? 24 : 0;
			return hourDiff;
		} else {
			return 12;
		}
	} else {
		if ((curHour >= currUI_Params.currentTimeBorder.timeFirstBorder) && (curHour <= currUI_Params.currentTimeBorder.timeSecondBorder)) {
			int8_t hourDiff = curHour - currUI_Params.currentTimeBorder.timeFirstBorder;
			return hourDiff;
		} else {
			return 12;
		}
	}
	return 12;
}

void HAQuDA_DispManip::nightMode(UI_Params currUI_Params) {
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

	uint8_t nightMode_hour = get_nightMode_hour(curHour, currUI_Params);

	switch (currUI_Params.dispParam) {
		case temp: {
			WS2812_showParams_night(temp_meas.value / temp_meas.measNum, currUI_Params.temp_divideDots, nightMode_hour);
			temp_meas.value = 0;
			temp_meas.measNum = 0;
			break;
		}
		case humid: {
			WS2812_showParams_night(humid_meas.value / humid_meas.measNum, currUI_Params.humid_divideDots, nightMode_hour);
			humid_meas.value = 0;
			humid_meas.measNum = 0;
			break;
		}
		case eCO2: {
			WS2812_showParams_night(eCO2_meas.value / eCO2_meas.measNum, currUI_Params.eCO2_divideDots, nightMode_hour);
			eCO2_meas.value = 0;
			eCO2_meas.measNum = 0;
			break;
		}
		case TVOC: {
			WS2812_showParams_night(TVOC_meas.value / TVOC_meas.measNum, currUI_Params.TVOC_divideDots, nightMode_hour);
			TVOC_meas.value = 0;
			TVOC_meas.measNum = 0;
			break;
		}
		case PM2_5: {
			WS2812_showParams_night(PM_2_5_meas.value / PM_2_5_meas.measNum, currUI_Params.PM2_5_divideDots, nightMode_hour);
			PM_2_5_meas.value = 0;
			PM_2_5_meas.measNum = 0;
			break;
		}
		default:
			break;
	}
}

void HAQuDA_DispManip::displayData(UI_Params currUI_Params) {
	switch (currUI_Params.dispMode) {
		case standard: {
			standardMode(currUI_Params);
			break;
		}
		case multi: {
			multiMode(currUI_Params);
			break;
		}
		case night: {
			nightMode(currUI_Params);
			break;
		}
		default:
			break;
	}
}