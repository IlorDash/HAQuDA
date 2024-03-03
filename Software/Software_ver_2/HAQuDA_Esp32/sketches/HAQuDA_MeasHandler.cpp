#include "HAQuDA_MeasHandler.h"
#include "HAQuDA_TimeHelper.h"
#include "HAQuDA_LEDsParams.h"
#include "HAQuDA_LEDs.h"
#include "Sensors.h"

#define MAX_SENS_REBOOT_ATTEMPTS 3

bool HAQuDA_MeasHandler::sens_start;
int HAQuDA_MeasHandler::meas_num;
bool HAQuDA_MeasHandler::show_meas_immed;
bool HAQuDA_MeasHandler::stop_meas_show;

meas_mode HAQuDA_MeasHandler::mode;
meas_type HAQuDA_MeasHandler::standard_m_p;
multi_mode_params HAQuDA_MeasHandler::multi_m_p;
night_time HAQuDA_MeasHandler::night_m_time;
meas_div_dots HAQuDA_MeasHandler::dots;

static HAQuDA_ErrorHandler *errHandler;

HAQuDA_MeasHandler::HAQuDA_MeasHandler(HAQuDA_ErrorHandler *_err_handler) {

	sens_start = false;

	meas_num = 0;
	show_meas_immed = false;

	mode = noneMeas;
	standard_m_p = noneType;

	night_m_time.start = 21;
	night_m_time.finish = 9;

	dots.temp.first = 20;
	dots.temp.mid = 26;
	dots.temp.last = 30;

	dots.humid.first = 40;
	dots.humid.mid = 60;
	dots.humid.last = 80;

	dots.eCO2.first = 400;
	dots.eCO2.mid = 1000;
	dots.eCO2.last = 5000;

	dots.TVOC.first = 220;
	dots.TVOC.mid = 660;
	dots.TVOC.last = 1000;

	dots.PM2_5.first = 15;
	dots.PM2_5.mid = 20;
	dots.PM2_5.last = 45;

	multi_m_p.paramsArr[0] = temp;
	multi_m_p.paramsArr[1] = eCO2;
	multi_m_p.paramsArr[2] = PM2_5;
	multi_m_p.dataArr[0] = 0;
	multi_m_p.dataArr[1] = 0;
	multi_m_p.dataArr[2] = 0;
	multi_m_p.divideDotsArr[0] = dots.temp;
	multi_m_p.divideDotsArr[1] = dots.eCO2;
	multi_m_p.divideDotsArr[2] = dots.PM2_5;

	errHandler = _err_handler;
}

bool HAQuDA_MeasHandler::SensorsBegin() {
	sens_start = sensorsBegin();
	return sens_start;
}

bool HAQuDA_MeasHandler::TryRebootSensors() {
	uint8_t rebootSensorsAttemptsNum = 0;
	while (!sensorsBegin()) {
		if (rebootSensorsAttemptsNum >= MAX_SENS_REBOOT_ATTEMPTS) {
			return false;
		}
		rebootSensorsAttemptsNum++;
		sensors_reboot();
	}
	return true;
}

void HAQuDA_MeasHandler::UpdateMeas(int *sensors_meas_time) {
	if (!sens_start)
		return;

	if ((millis() - *sensors_meas_time) > SENSORS_MEAS_PERIOD_MS) {
		getDHT11_meas();
		getCCS811_meas();
		getPM_meas();
		meas_num++;
		*sensors_meas_time = millis();

		checkMeasCorrect();

		eCO2_meas.newMeasDone = false;
		TVOC_meas.newMeasDone = false;
		PM_2_5_meas.newMeasDone = false;
		temp_meas.newMeasDone = false;
		humid_meas.newMeasDone = false;
	}
}

void HAQuDA_MeasHandler::checkMeasCorrect() {
	if (!eCO2_meas.newMeasDone) {
		errHandler->FailedToGetMeas("Failed to get eCO2 meas");
	}
	if (!TVOC_meas.newMeasDone) {
		errHandler->FailedToGetMeas("Failed to get TVOC meas");
	}
	if (!PM_2_5_meas.newMeasDone) {
		errHandler->FailedToGetMeas("Failed to get PM 2.5 meas");
	}
	if (!temp_meas.newMeasDone) {
		errHandler->FailedToGetMeas("Failed to get TEMP meas");
	}
	if (!humid_meas.newMeasDone) {
		errHandler->FailedToGetMeas("Failed to get HUMID meas");
	}
}

void HAQuDA_MeasHandler::Show(int *show_meas_time) {

	if ((millis() - *show_meas_time > DISP_MEAS_PERIOD_MS) || show_meas_immed || stop_meas_show) {

		if (stop_meas_show) {
			stop_meas_show = false;
			return;
		}

		switch (mode) {
			case standard: {
				standardMode();
				break;
			}
			case multi: {
				multiMode(multi_m_p);
				break;
			}
			case night: {
				nightMode();
				break;
			}
			default:
				break;
		}
		*show_meas_time = millis();
		ClearMeas();
		show_meas_immed = false;
	}
}

void HAQuDA_MeasHandler::SetMeasDispMode(meas_mode m) {
	if (m == noneMeas)
		stop_meas_show = true;
	else
		show_meas_immed = true;
	mode = m;
}

meas_mode HAQuDA_MeasHandler::GetMeasDispMode() {
	return mode;
}

void HAQuDA_MeasHandler::SetStandardMeasParam(const meas_type standard_p) {
	standard_m_p = standard_p;
}
void HAQuDA_MeasHandler::SetMultiMeasParam(const multi_mode_params multi_p) {
	multi_m_p = multi_p;
}

void HAQuDA_MeasHandler::SetNightTime(const night_time night_time) {
	night_m_time = night_time;
}

void HAQuDA_MeasHandler::SetMeasDivDots(const meas_div_dots _dots) {
	dots = _dots;
}

meas_type HAQuDA_MeasHandler::checkBadMeas() {
	if (((temp_meas.value / temp_meas.measNum) >= dots.temp.last) || ((temp_meas.value / temp_meas.measNum) <= dots.temp.first)) {
		return temp;
	} else if (((humid_meas.value / humid_meas.measNum) >= dots.humid.last) || ((humid_meas.value / humid_meas.measNum) <= dots.humid.first)) {
		return humid;
	} else if ((eCO2_meas.value / eCO2_meas.measNum) >= dots.eCO2.last) {
		return eCO2;
	} else if ((TVOC_meas.value / TVOC_meas.measNum) >= dots.TVOC.last) {
		return TVOC;
	} else if ((PM_2_5_meas.value / PM_2_5_meas.measNum) >= dots.PM2_5.last) {
		return PM2_5;
	}
	return noneType;
}

void HAQuDA_MeasHandler::standardMode() {
	switch (standard_m_p) {
		case total: {
			meas_type badParam = checkBadMeas();
			switch (badParam) {
				case temp: {
					showMeas_standard(temp_meas.value / temp_meas.measNum, dots.temp);
					break;
				}
				case humid: {
					showMeas_standard(humid_meas.value / humid_meas.measNum, dots.humid);
					break;
				}
				case eCO2: {
					showMeas_standard(eCO2_meas.value / eCO2_meas.measNum, dots.eCO2);
					break;
				}
				case TVOC: {
					showMeas_standard(TVOC_meas.value / TVOC_meas.measNum, dots.TVOC);
					break;
				}
				case PM2_5: {
					showMeas_standard(PM_2_5_meas.value / PM_2_5_meas.measNum, dots.PM2_5);
					break;
				}
				case total: {
					float measValuesArr[DISP_PARAMS_NUM];
					div_dots measDivideDots[DISP_PARAMS_NUM];

					measValuesArr[0] = temp_meas.value / temp_meas.measNum;
					measDivideDots[0] = dots.temp;

					measValuesArr[1] = humid_meas.value / humid_meas.measNum;
					measDivideDots[1] = dots.humid;

					measValuesArr[2] = eCO2_meas.value / eCO2_meas.measNum;
					measDivideDots[2] = dots.eCO2;

					measValuesArr[3] = TVOC_meas.value / TVOC_meas.measNum;
					measDivideDots[3] = dots.TVOC;

					measValuesArr[4] = PM_2_5_meas.value / PM_2_5_meas.measNum;
					measDivideDots[4] = dots.PM2_5;

					showMeas_total(measValuesArr, sizeof(measValuesArr), measDivideDots, sizeof(measDivideDots));
					break;
				}
				default:
					break;
			}
			break;
		}
		case temp: {
			showMeas_standard(temp_meas.value / temp_meas.measNum, dots.temp);
			break;
		}
		case humid: {
			showMeas_standard(humid_meas.value / humid_meas.measNum, dots.humid);
			break;
		}
		case eCO2: {
			showMeas_standard(eCO2_meas.value / eCO2_meas.measNum, dots.eCO2);
			break;
		}
		case TVOC: {
			showMeas_standard(TVOC_meas.value / TVOC_meas.measNum, dots.TVOC);
			break;
		}
		case PM2_5: {
			showMeas_standard(PM_2_5_meas.value / PM_2_5_meas.measNum, dots.PM2_5);
			break;
		}
		default:
			break;
	}
}

void HAQuDA_MeasHandler::multiMode(multi_mode_params multiModeParams) {
	for (int i = 0; i < MULTI_MODE_PARAM_NUM; i++) {
		switch (multiModeParams.paramsArr[i]) {
			case temp: {
				if (temp_meas.measNum == 0)
					return;

				multiModeParams.dataArr[i] = temp_meas.value / temp_meas.measNum;
				break;
			}
			case humid: {
				if (humid_meas.measNum == 0)
					return;

				multiModeParams.dataArr[i] = humid_meas.value / humid_meas.measNum;
				break;
			}
			case eCO2: {
				if (eCO2_meas.measNum == 0)
					return;

				multiModeParams.dataArr[i] = eCO2_meas.value / eCO2_meas.measNum;
				break;
			}
			case PM2_5: {
				if (PM_2_5_meas.measNum == 0)
					return;

				multiModeParams.dataArr[i] = PM_2_5_meas.value / PM_2_5_meas.measNum;
				break;
			}
			case TVOC: {
				if (TVOC_meas.measNum == 0)
					return;

				multiModeParams.dataArr[i] = TVOC_meas.value / TVOC_meas.measNum;
				break;
			}
			default:
				break;
		}
	}
	showMeas_multi(multiModeParams.dataArr, sizeof(multiModeParams.dataArr) / sizeof(multiModeParams.dataArr[0]), multiModeParams.divideDotsArr,
				   sizeof(multiModeParams.divideDotsArr) / sizeof(multiModeParams.divideDotsArr[0]));
}

uint8_t HAQuDA_MeasHandler::get_nightMode_hour(night_time timeBorder) {
	DateTimeStruct currTime;

	if (!HAQuDA_TimeService::getInstance()->GetDateTime(&currTime)) {
		return 0;
	}

	bool isBordersInDifferentDays = (timeBorder.finish < timeBorder.start);

	if (isBordersInDifferentDays) {
		if ((currTime.hour >= timeBorder.start) || (currTime.hour <= timeBorder.finish)) {
			int8_t hourDiff = currTime.hour - timeBorder.start;
			hourDiff += (hourDiff < 0) ? 24 : 0;
			return hourDiff;
		} else {
			return 12;
		}
	} else {
		if ((currTime.hour >= timeBorder.start) && (currTime.hour <= timeBorder.finish)) {
			int8_t hourDiff = currTime.hour - timeBorder.start;
			return hourDiff;
		} else {
			return 12;
		}
	}
	return 12;
}

void HAQuDA_MeasHandler::nightMode() {
	uint8_t nightMode_hour = get_nightMode_hour(night_m_time);

	switch (standard_m_p) {
		case temp: {
			showMeas_night(temp_meas.value / temp_meas.measNum, dots.temp, nightMode_hour);
			temp_meas.value = 0;
			temp_meas.measNum = 0;
			break;
		}
		case humid: {
			showMeas_night(humid_meas.value / humid_meas.measNum, dots.humid, nightMode_hour);
			humid_meas.value = 0;
			humid_meas.measNum = 0;
			break;
		}
		case eCO2: {
			showMeas_night(eCO2_meas.value / eCO2_meas.measNum, dots.eCO2, nightMode_hour);
			eCO2_meas.value = 0;
			eCO2_meas.measNum = 0;
			break;
		}
		case TVOC: {
			showMeas_night(TVOC_meas.value / TVOC_meas.measNum, dots.TVOC, nightMode_hour);
			TVOC_meas.value = 0;
			TVOC_meas.measNum = 0;
			break;
		}
		case PM2_5: {
			showMeas_night(PM_2_5_meas.value / PM_2_5_meas.measNum, dots.PM2_5, nightMode_hour);
			PM_2_5_meas.value = 0;
			PM_2_5_meas.measNum = 0;
			break;
		}
		default:
			break;
	}
}

void HAQuDA_MeasHandler::getMeasRGB(uint8_t *_red, uint8_t *_green, uint8_t *_blue, const uint8_t brightness, const float data, const div_dots divideDots) {
	float coefficient = (brightness * 2) / (divideDots.last - divideDots.first);
	if ((data < divideDots.mid) && (data >= divideDots.first)) {
		*_blue = round(-abs(data - divideDots.first) * coefficient) + MAX_BRIGHTNESS;
		*_green = round(-abs(data - divideDots.mid) * coefficient) + MAX_BRIGHTNESS;
		*_red = 0;
	} else if ((data >= divideDots.mid) && (data <= divideDots.last)) {
		*_blue = 0;
		*_green = round(-abs(data - divideDots.mid) * coefficient) + MAX_BRIGHTNESS;
		*_red = round(-abs(data - divideDots.last) * coefficient) + MAX_BRIGHTNESS;
	} else if (data < divideDots.first) {
		*_blue = MAX_BRIGHTNESS;
		*_green = 0;
		*_red = 0;
	} else if (data > divideDots.last) {
		*_blue = 0;
		*_green = 0;
		*_red = MAX_BRIGHTNESS;
	}
}

void HAQuDA_MeasHandler::showMeas_standard(float data, div_dots divideDots) {
	HAQuDA_LEDs::getInstance()->Clear();
	vTaskDelay(100 / portTICK_PERIOD_MS);

	uint8_t r, g, b;
	uint8_t brightness = HAQuDA_LEDs::getInstance()->GetBright();
	getMeasRGB(&r, &g, &b, brightness, data, divideDots);
	HAQuDA_LEDs::getInstance()->FillColor(r, g, b);
	HAQuDA_LEDs::getInstance()->Show();
}

void HAQuDA_MeasHandler::showMeas_night(float data, div_dots divideDots, uint8_t time) {
	HAQuDA_LEDs::getInstance()->Clear();
	vTaskDelay(100 / portTICK_PERIOD_MS);

	uint8_t r, g, b;
	uint8_t brightness = HAQuDA_LEDs::getInstance()->GetBright();
	getMeasRGB(&r, &g, &b, brightness, data, divideDots);

	for (int i = 0; i < LED_COLUMN_NUM; i++) {
		uint8_t whiteBright = brightness * 1000 / MAX_BRIGHTNESS * WHITE_BRIGHTNESS_COEFF / 1000;

		uint32_t color = (!(i % 3)) ? HAQuDA_LEDs::getInstance()->GetColor(r, g, b)
									: HAQuDA_LEDs::getInstance()->GetColor(whiteBright, whiteBright, whiteBright);
		uint8_t pixelNum = (!(i % 3)) ? LED_ROW_NUM : time;
		uint8_t startPixel = (!(i % 2)) ? (i * LED_ROW_NUM) : (i * LED_ROW_NUM + LED_ROW_NUM - pixelNum);
		HAQuDA_LEDs::getInstance()->FillColorFrom(color, startPixel, pixelNum);
	}
	HAQuDA_LEDs::getInstance()->Show();
}

void HAQuDA_MeasHandler::showMeas_multi(float *data, uint8_t dataSize, div_dots *divDots, uint8_t divDotsSize) {

	if (dataSize != MULTI_MODE_PARAM_NUM) {
		return;
	}
	if (divDotsSize != DIVIDE_DOTS_NUM) {
		return;
	}

	HAQuDA_LEDs::getInstance()->Clear();
	vTaskDelay(100 / portTICK_PERIOD_MS);

	uint8_t r[MULTI_MODE_PARAM_NUM], g[MULTI_MODE_PARAM_NUM], b[MULTI_MODE_PARAM_NUM];
	uint8_t brightness = HAQuDA_LEDs::getInstance()->GetBright();
	for (int i = 0; i < MULTI_MODE_PARAM_NUM; i++) {
		getMeasRGB(&(r[i]), &(g[i]), &(b[i]), brightness, data[i], divDots[i]);
	}
	for (int i = 0; i < LED_ROW_NUM; i++) {
		if (!(i % 2)) {
			HAQuDA_LEDs::getInstance()->FillColorFrom(r[0], g[0], b[0], i * LED_ROW_NUM + 1, 3);
			HAQuDA_LEDs::getInstance()->FillColorFrom(r[1], g[1], b[1], i * LED_ROW_NUM + 5, 3);
			HAQuDA_LEDs::getInstance()->FillColorFrom(r[2], g[2], b[2], i * LED_ROW_NUM + 9, 3);
		} else {
			HAQuDA_LEDs::getInstance()->FillColorFrom(r[2], g[2], b[2], i * LED_ROW_NUM, 3);
			HAQuDA_LEDs::getInstance()->FillColorFrom(r[1], g[1], b[1], i * LED_ROW_NUM + 4, 3);
			HAQuDA_LEDs::getInstance()->FillColorFrom(r[0], g[0], b[0], i * LED_ROW_NUM + 8, 3);
		}
	}
	HAQuDA_LEDs::getInstance()->Show();
}

void HAQuDA_MeasHandler::showMeas_total(float *data, uint8_t dataSize, div_dots *dataDivideDots, uint8_t divideDotsSize) {
	if (dataSize != MULTI_MODE_PARAM_NUM) {
		return;
	}
	if (divideDotsSize != DIVIDE_DOTS_NUM) {
		return;
	}
	uint8_t colorsArr[DISP_PARAMS_NUM][COLORS_NUM];
	uint8_t brightness = HAQuDA_LEDs::getInstance()->GetBright();
	for (int i = 0; i < DISP_PARAMS_NUM; i++) {
		getMeasRGB(&(colorsArr[i][0]), &(colorsArr[i][1]), &(colorsArr[i][2]), brightness, data[i], dataDivideDots[i]);
	}

	uint8_t r, g, b;
	for (int i = 0; i < DISP_PARAMS_NUM; i++) {
		r += colorsArr[i][0];
		g += colorsArr[i][1];
		b += colorsArr[i][2];
	}
	r /= DISP_PARAMS_NUM;
	g /= DISP_PARAMS_NUM;
	b /= DISP_PARAMS_NUM;
	HAQuDA_LEDs::getInstance()->FillColor(r, g, b);
	HAQuDA_LEDs::getInstance()->Show();
}

HAQuDA_MeasHandler::~HAQuDA_MeasHandler() {
}