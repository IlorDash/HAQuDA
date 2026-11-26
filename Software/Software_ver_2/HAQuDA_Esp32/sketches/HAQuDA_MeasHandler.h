#pragma once

#include "Arduino.h"
#include "HAQuDA_DisplayTypes.h"
#include "HAQuDA_ErrorHandler.h"
#include "HAQuDA_IMeasSource.h"
#include "HAQuDA_ColorScale.h"
#include "HAQuDA_MeasSnapshot.h"
#include "HAQuDA_PMResult.h"

#define MULTI_MODE_PARAM_NUM 3
#define SENSORS_MEAS_PERIOD_MS 1000 * 2

typedef enum { standard, multi, night, noneMeas } meas_mode;

typedef struct {
	meas_type paramsArr[MULTI_MODE_PARAM_NUM];
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

class HAQuDA_MeasHandler : public HAQuDA_IMeasSource {

	HAQuDA_MeasSnapshot &measSnapshot;
	void publishSnapshots();

	static bool sens_start;
	static int meas_num;

	static meas_mode mode;
	static meas_type standard_m_p;
	static multi_mode_params multi_m_p;
	static night_time night_m_time;
	static meas_div_dots dots;

	static void checkMeasCorrect(pm_meas_result pmResult);

	static bool readAverage(meas_type type, float &average);
	static div_dots dotsOf(meas_type type);

  public:
	// snapshot receives the latest averages for the web page and must outlive the handler.
	HAQuDA_MeasHandler(HAQuDA_ErrorHandler *_err_handler, HAQuDA_MeasSnapshot &snapshot);

	// Starts the sensors once. loop task only (setup or loop), never from ShowTask.
	bool SensorsBegin();
	// Power-cycles and restarts the sensors up to 3 times, blocks up to ~27 s, enables UpdateMeas() on success.
	// loop task only (setup or loop), never from ShowTask.
	bool TryRebootSensors();

	// Polls the sensors every SENSORS_MEAS_PERIOD_MS and publishes the averages to the snapshot. loop only.
	void UpdateMeas(int *sensors_meas_time);

	// The animation the current mode shows, none when the mode or its parameter selects nothing
	static anim_id CurrentAnim();
	bool ReadAverages(anim_id &anim, meas_anim_data &data) override;
	void RestartAveraging() override;

	static void SetMeasDispMode(meas_mode m);

	static void SetStandardMeasParam(const meas_type standard_p);
	static void SetMultiMeasParam(const multi_mode_params multi_p);
	static void SetNightTime(const night_time night_time);

	~HAQuDA_MeasHandler();
};