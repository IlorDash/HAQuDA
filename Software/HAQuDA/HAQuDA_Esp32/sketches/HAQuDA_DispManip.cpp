#include "HAQuDA_DispManip.h"

HAQuDA_DispManip::HAQuDA_DispManip(HAQuDA_UI *currUI) {
	this->myUI_dispManip = currUI;

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

HAQuDA_DispManip::~HAQuDA_DispManip() {
	delete timeClient;
}

dispParams HAQuDA_DispManip::checkBadParam() {
	if ((temp_meas.value / temp_meas.measNum) >= myUI_dispManip->currUI_Params.temp_divideDots.thirdDot) {
		return temp;
	} else if ((humid_meas.value / humid_meas.measNum) >= myUI_dispManip->currUI_Params.humid_divideDots.thirdDot) {
		return humid;
	} else if ((eCO2_meas.value / eCO2_meas.measNum) >= myUI_dispManip->currUI_Params.eCO2_divideDots.thirdDot) {
		return eCO2;
	} else if ((TVOC_meas.value / TVOC_meas.measNum) >= myUI_dispManip->currUI_Params.TVOC_divideDots.thirdDot) {
		return TVOC;
	} else if ((PM_2_5_meas.value / PM_2_5_meas.measNum) >= myUI_dispManip->currUI_Params.PM2_5_divideDots.thirdDot) {
		return PM2_5;
	}
	return noneParam;
}

void HAQuDA_DispManip::standardMode() {
	switch (myUI_dispManip->currUI_Params.dispParam) {
		case total: {
			dispParams badParam = checkBadParam();
			switch (badParam) {
				case temp: {
					WS2812_showParams_standard(temp_meas.value / temp_meas.measNum, myUI_dispManip->currUI_Params.temp_divideDots);
					break;
				}
				case humid: {
					WS2812_showParams_standard(humid_meas.value / humid_meas.measNum, myUI_dispManip->currUI_Params.humid_divideDots);
					break;
				}
				case eCO2: {
					WS2812_showParams_standard(eCO2_meas.value / eCO2_meas.measNum, myUI_dispManip->currUI_Params.eCO2_divideDots);
					break;
				}
				case TVOC: {
					WS2812_showParams_standard(TVOC_meas.value / TVOC_meas.measNum, myUI_dispManip->currUI_Params.TVOC_divideDots);
					break;
				}
				case PM2_5: {
					WS2812_showParams_standard(PM_2_5_meas.value / PM_2_5_meas.measNum, myUI_dispManip->currUI_Params.PM2_5_divideDots);
					break;
				}
				case noneParam: {
					float measValuesArr[DISP_PARAMS_NUM];
					measDivideDots_struct measDivideDots[DISP_PARAMS_NUM];

					measValuesArr[0] = temp_meas.value / temp_meas.measNum;
					measDivideDots[0] = myUI_dispManip->currUI_Params.temp_divideDots;

					measValuesArr[1] = humid_meas.value / humid_meas.measNum;
					measDivideDots[1] = myUI_dispManip->currUI_Params.humid_divideDots;

					measValuesArr[2] = eCO2_meas.value / eCO2_meas.measNum;
					measDivideDots[2] = myUI_dispManip->currUI_Params.eCO2_divideDots;

					measValuesArr[3] = TVOC_meas.value / TVOC_meas.measNum;
					measDivideDots[3] = myUI_dispManip->currUI_Params.TVOC_divideDots;

					measValuesArr[4] = PM_2_5_meas.value / PM_2_5_meas.measNum;
					measDivideDots[4] = myUI_dispManip->currUI_Params.PM2_5_divideDots;

					WS2812_showParams_standardTotal(measValuesArr, measDivideDots);
					break;
				}
				default:
					break;
			}
			break;
		}
		case temp: {
			WS2812_showParams_standard(temp_meas.value / temp_meas.measNum, myUI_dispManip->currUI_Params.temp_divideDots);
			break;
		}
		case humid: {
			WS2812_showParams_standard(humid_meas.value / humid_meas.measNum, myUI_dispManip->currUI_Params.humid_divideDots);
			break;
		}
		case eCO2: {
			WS2812_showParams_standard(eCO2_meas.value / eCO2_meas.measNum, myUI_dispManip->currUI_Params.eCO2_divideDots);
			break;
		}
		case TVOC: {
			WS2812_showParams_standard(TVOC_meas.value / TVOC_meas.measNum, myUI_dispManip->currUI_Params.TVOC_divideDots);
			break;
		}
		case PM2_5: {
			WS2812_showParams_standard(PM_2_5_meas.value / PM_2_5_meas.measNum, myUI_dispManip->currUI_Params.PM2_5_divideDots);
			break;
		}

		default:
			break;
	}
}

uint8_t get_nightMode_hour(uint8_t curHour) {
	bool isBordersInDifferentDays = (currentTimeBorder.timeSecondBorder < currentTimeBorder.timeFirstBorder);

	if (isBordersInDifferentDays) {
		if ((curHour >= currentTimeBorder.timeFirstBorder) || (curHour <= currentTimeBorder.timeSecondBorder)) {
			int8_t hourDiff = curHour - currentTimeBorder.timeFirstBorder;
			hourDiff += (hourDiff < 0) ? 24 : 0;
			return hourDiff;
		} else {
			return 12;
		}
	} else {
		if ((curHour >= currentTimeBorder.timeFirstBorder) && (curHour <= currentTimeBorder.timeSecondBorder)) {
			int8_t hourDiff = curHour - currentTimeBorder.timeFirstBorder;
			return hourDiff;
		} else {
			return 12;
		}
	}
	return 12;
}

void nightDispParam_WS2818() {
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

	uint8_t nightMode_hour = get_nightMode_hour(curHour);

	if (whatParamDisp == temp) {
		WS2812_showParams_night(temp_meas.value / temp_meas.measNum, temp_divideDots, nightMode_hour);
		temp_meas.value = 0;
		temp_meas.measNum = 0;
	} else if (whatParamDisp == humid) {
		WS2812_showParams_night(humid_meas.value / humid_meas.measNum, humid_divideDots, nightMode_hour);
		humid_meas.value = 0;
		humid_meas.measNum = 0;
	} else if (whatParamDisp == eCO2) {
		WS2812_showParams_night(eCO2_meas.value / eCO2_meas.measNum, eCO2_divideDots, nightMode_hour);
		eCO2_meas.value = 0;
		eCO2_meas.measNum = 0;
	} else if (whatParamDisp == TVOC) {
		WS2812_showParams_night(TVOC_meas.value / TVOC_meas.measNum, TVOC_divideDots, nightMode_hour);
		TVOC_meas.value = 0;
		TVOC_meas.measNum = 0;
	} else if (whatParamDisp == PM2_5) {
		WS2812_showParams_night(PM_2_5_meas.value / PM_2_5_meas.measNum, PM2_5_divideDots, nightMode_hour);
		PM_2_5_meas.value = 0;
		PM_2_5_meas.measNum = 0;
	}
}

void HAQuDA_DispManip::dispParam_WS2812() {
	switch (whatModeDisp) {
		case standard: {
			standardDispParam_WS2818();
			break;
		}
		case multi: {
			for (int i = 0; i < MULTI_MODE_PARAM_NUM; i++) {
				switch (multiModeStruct.paramsArr[i]) {
					case temp: {
						multiModeStruct.dataArr[i] = temp_meas.value / temp_meas.measNum;
						break;
					}
					case humid: {
						multiModeStruct.dataArr[i] = humid_meas.value / humid_meas.measNum;
						break;
					}
					case eCO2: {
						multiModeStruct.dataArr[i] = eCO2_meas.value / eCO2_meas.measNum;
						break;
					}
					case PM2_5: {
						multiModeStruct.dataArr[i] = PM_2_5_meas.value / PM_2_5_meas.measNum;
						break;
					}
					case TVOC: {
						multiModeStruct.dataArr[i] = TVOC_meas.value / TVOC_meas.measNum;
						break;
					}
					default:
						break;
				}
			}
			WS2812_showParams_multi(multiModeStruct.dataArr, multiModeStruct.divideDotsArr);
			break;
		}
		case night: {
			nightDispParam_WS2818();
			break;
		}
		default:
			break;
	}
}