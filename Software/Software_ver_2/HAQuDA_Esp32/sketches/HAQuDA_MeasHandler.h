#pragma once

#include "Arduino.h"
#include "HAQuDA_ErrorHandler.h"
#include "HAQuDA_ColorScale.h"
#include "HAQuDA_MeasSnapshot.h"
#include "HAQuDA_PMResult.h"

#define MULTI_MODE_PARAM_NUM 3
#define DIVIDE_DOTS_NUM 3
#define DISP_PARAMS_NUM 5
#define DISP_MEAS_PERIOD_MS 1000 * 60 * 1
#define SENSORS_MEAS_PERIOD_MS 1000 * 2

typedef enum { standard, multi, night, noneMeas } meas_mode;

typedef enum { total, eCO2, TVOC, PM2_5, temp, humid, noneType } meas_type;

typedef struct {
	meas_type paramsArr[MULTI_MODE_PARAM_NUM];
	float dataArr[MULTI_MODE_PARAM_NUM];
	div_dots divideDotsArr[MULTI_MODE_PARAM_NUM];
} multi_mode_params;

// The night window, minutes since midnight (it may cross midnight)
typedef struct {
	uint16_t start;
	uint16_t finish;
} night_time;

typedef struct {
	div_dots temp;
	div_dots humid;
	div_dots eCO2;
	div_dots TVOC;
	div_dots PM2_5;
} meas_div_dots;

// Declared, not included: HAQuDA_WebPageDisplayManip.h includes HAQuDA_DisplayManip.h, which includes this header.
class HAQuDA_WebPageDisplayManip;

class HAQuDA_MeasHandler {

	HAQuDA_MeasSnapshot &measSnapshot;
	void publishSnapshots();

	static bool sens_start;
	static int meas_num;
	static bool show_meas_immed;

	static meas_mode mode;
	static meas_type standard_m_p;
	static multi_mode_params multi_m_p;
	static night_time night_m_time;
	static meas_div_dots dots;

	static void checkMeasCorrect(pm_meas_result pmResult);
	static meas_type checkBadMeas();
	// The mode functions return false when they could not draw because the shown reading has no sample yet (Show() then tries
	// again on its next pass), true when they drew or never will
	static bool standardMode();
	// Standard mode with the overall reading: the first reading out of its range, else the average colour of all of them
	static bool totalMode();
	static bool multiMode(multi_mode_params multiModeParams);
	static bool nightMode();
	static void showMeas_multi(float *data, uint8_t dataSize, div_dots *divideDots, uint8_t divideDotsSize);
	static void showMeas_standard(float data, div_dots divideDots);
	// Fills the whole matrix with one colour and reports it to the web mirror. ShowTask only.
	static void drawSolid(const rgb_color &color);
	// litCells: night light cells on (HAQuDA_NightLitCells()), 0 when the time is unknown
	static void showMeas_night(float data, div_dots divideDots, uint8_t litCells);

  public:
	// snapshot receives the latest averages for the web page and webDisp the colours the lamp draws; both must outlive the handler.
	HAQuDA_MeasHandler(HAQuDA_ErrorHandler *_err_handler, HAQuDA_MeasSnapshot &snapshot, HAQuDA_WebPageDisplayManip &webDisp);

	// Starts the sensors once. loop task only (setup or loop), never from ShowTask.
	bool SensorsBegin();
	// Power-cycles and restarts the sensors up to 3 times, blocks up to ~27 s, enables UpdateMeas() on success.
	// loop task only (setup or loop), never from ShowTask.
	bool TryRebootSensors();

	// Polls the sensors every SENSORS_MEAS_PERIOD_MS and publishes the averages to the snapshot. loop only.
	void UpdateMeas(int *sensors_meas_time);

	static void Show(int *show_meas_time);

	static void SetMeasDispMode(meas_mode m);
	meas_mode GetMeasDispMode();

	static void SetStandardMeasParam(const meas_type standard_p);
	static void SetMultiMeasParam(const multi_mode_params multi_p);
	static void SetNightTime(const night_time night_time);
	static void SetMeasDivDots(const meas_div_dots _dots);

	~HAQuDA_MeasHandler();
};