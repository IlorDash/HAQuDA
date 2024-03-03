#pragma once

#include "Arduino.h"
#include "HAQuDA_ErrorHandler.h"

#define MULTI_MODE_PARAM_NUM 3
#define DIVIDE_DOTS_NUM 3
#define DISP_PARAMS_NUM 5
#define DISP_MEAS_PERIOD_MS 1000 * 60 * 1
#define SENSORS_MEAS_PERIOD_MS 1000 * 2

typedef enum { standard, multi, night, noneMeas } meas_mode;

typedef enum { total, eCO2, TVOC, PM2_5, temp, humid, noneType } meas_type;

typedef struct {
	uint16_t first;
	uint16_t mid;
	uint16_t last;
} div_dots;

typedef struct {
	meas_type paramsArr[MULTI_MODE_PARAM_NUM];
	float dataArr[MULTI_MODE_PARAM_NUM];
	div_dots divideDotsArr[MULTI_MODE_PARAM_NUM];
} multi_mode_params;

typedef struct {
	uint8_t start;
	uint8_t finish;
} night_time;

typedef struct {
	div_dots temp;
	div_dots humid;
	div_dots eCO2;
	div_dots TVOC;
	div_dots PM2_5;
} meas_div_dots;

class HAQuDA_MeasHandler {

	static bool sens_start;
	static int meas_num;
	static bool show_meas_immed;

	static meas_mode mode;
	static meas_type standard_m_p;
	static multi_mode_params multi_m_p;
	static night_time night_m_time;
	static meas_div_dots dots;

	static void checkMeasCorrect();
	static meas_type checkBadMeas();
	static uint8_t get_nightMode_hour(night_time timeBorder);
	static void standardMode();
	static void multiMode(multi_mode_params multiModeParams);
	static void nightMode();
	static void getMeasRGB(uint8_t *_red, uint8_t *_green, uint8_t *_blue, const uint8_t brightness, const float data, const div_dots divideDots);
	static void showMeas_multi(float *data, uint8_t dataSize, div_dots *divideDots, uint8_t divideDotsSize);
	static void showMeas_total(float *data, uint8_t dataSize, div_dots *dataDivideDots, uint8_t divideDotsSize);
	static void showMeas_standard(float data, div_dots divideDots);
	static void showMeas_night(float data, div_dots divideDots, uint8_t time);

  public:
	HAQuDA_MeasHandler(HAQuDA_ErrorHandler *_err_handler);

	bool SensorsBegin();
	bool TryRebootSensors();

	static void UpdateMeas(int *sensors_meas_time);

	static void Show(int *show_meas_time);

	static void SetMeasDispMode(meas_mode m);
	meas_mode GetMeasDispMode();

	static void SetStandardMeasParam(const meas_type standard_p);
	static void SetMultiMeasParam(const multi_mode_params multi_p);
	static void SetNightTime(const night_time night_time);
	static void SetMeasDivDots(const meas_div_dots _dots);

	~HAQuDA_MeasHandler();
};