#include "HAQuDA_MeasHandler.h"
#include "HAQuDA_NightProgress.h"
#include "HAQuDA_TimeHelper.h"
#include "HAQuDA_LEDsParams.h"
#include "HAQuDA_Animation.h"
#include "Sensors.h"

#include <atomic>

#define MAX_SENS_REBOOT_ATTEMPTS 3

static_assert(ANIM_COLUMNS == LED_COLUMN_NUM, "animation and LED geometry must agree");
static_assert(ANIM_ROWS == LED_ROW_NUM, "animation and LED geometry must agree");
static_assert(ANIM_PIXELS == LED_NUM_PIXELS, "animation and LED geometry must agree");
static_assert(ANIM_COLOR_AQUA == COLOR_AQUA, "animation colours must agree with the LED colours");
static_assert(ANIM_COLOR_LIME == COLOR_LIME, "animation colours must agree with the LED colours");
static_assert(ANIM_COLOR_GREEN == COLOR_GREEN, "animation colours must agree with the LED colours");
static_assert(ANIM_COLOR_RED == COLOR_RED, "animation colours must agree with the LED colours");
static_assert(ANIM_COLOR_YELLOW == COLOR_YELLOW, "animation colours must agree with the LED colours");

bool HAQuDA_MeasHandler::sens_start;
int HAQuDA_MeasHandler::meas_num;

meas_mode HAQuDA_MeasHandler::mode;
meas_type HAQuDA_MeasHandler::standard_m_p;
multi_mode_params HAQuDA_MeasHandler::multi_m_p;
night_time HAQuDA_MeasHandler::night_m_time;
meas_div_dots HAQuDA_MeasHandler::dots;

static HAQuDA_ErrorHandler *errHandler;

HAQuDA_MeasHandler::HAQuDA_MeasHandler(HAQuDA_ErrorHandler *_err_handler, HAQuDA_MeasSnapshot &snapshot) : measSnapshot(snapshot) {

	sens_start = false;

	meas_num = 0;

	mode = noneMeas;
	standard_m_p = noneType;

	night_m_time.start = NIGHT_DEFAULT_FROM_MIN;
	night_m_time.finish = NIGHT_DEFAULT_TO_MIN;

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

	errHandler = _err_handler;
}

bool HAQuDA_MeasHandler::SensorsBegin() {
	pmSerialBegin();
	sens_start = sensorsBegin();
	return sens_start;
}

bool HAQuDA_MeasHandler::TryRebootSensors() {
	uint8_t rebootSensorsAttemptsNum = 0;
	while (!sensorsBegin()) {
		if (rebootSensorsAttemptsNum >= MAX_SENS_REBOOT_ATTEMPTS) {
			sens_start = false;
			return false;
		}
		rebootSensorsAttemptsNum++;
		sensors_reboot();
	}
	sens_start = true;
	return true;
}

void HAQuDA_MeasHandler::UpdateMeas(int *sensors_meas_time) {
	if (!sens_start)
		return;

	if ((millis() - *sensors_meas_time) > SENSORS_MEAS_PERIOD_MS) {
		getDHT22_meas();
		getCCS811_meas();
		pm_meas_result pmResult = getPM_meas();
		meas_num++;
		*sensors_meas_time = millis();

		checkMeasCorrect(pmResult);

		publishSnapshots();

		eCO2_meas.newMeasDone = false;
		TVOC_meas.newMeasDone = false;
		PM_2_5_meas.newMeasDone = false;
		temp_meas.newMeasDone = false;
		humid_meas.newMeasDone = false;
	}
}

void HAQuDA_MeasHandler::checkMeasCorrect(pm_meas_result pmResult) {
	if (!eCO2_meas.newMeasDone) {
		errHandler->FailedToGetMeas("Failed to get eCO2 meas");
	}
	if (!TVOC_meas.newMeasDone) {
		errHandler->FailedToGetMeas("Failed to get TVOC meas");
	}
	// A sample skipped during the ZH03B warm-up is expected, not a failure
	if (HAQuDA_ShouldReportPMFailure(pmResult)) {
		errHandler->FailedToGetMeas("Failed to get PM 2.5 meas");
	}
	if (!temp_meas.newMeasDone) {
		errHandler->FailedToGetMeas("Failed to get TEMP meas");
	}
	if (!humid_meas.newMeasDone) {
		errHandler->FailedToGetMeas("Failed to get HUMID meas");
	}
}

// loop task. A failed poll publishes NaN so the page shows a gap instead of a stale value.
// ShowTask zeroes count and then sum in ClearMeasData() without a lock. Here the count is read before and after
// the sum: an unchanged non-zero count means no clear started before the second read, so the sum is the old one
// and matches the count. A changed count means a clear ran in between, and that poll keeps the previous snapshot.
static void publishOne(HAQuDA_MeasSnapshot &snapshot, snapshot_param param, const meas_data &data, div_dots dotsOfParam) {
	if (!data.newMeasDone) {
		snapshot.Publish(param, NAN, 0);
		return;
	}
	// volatile: the compiler must not merge the two count reads or move them past the sum read
	const volatile uint16_t *count = &data.measNum;
	const volatile float *sumPtr = &data.value;
	uint16_t countBefore = *count;
	std::atomic_thread_fence(std::memory_order_seq_cst);
	float sum = *sumPtr;
	std::atomic_thread_fence(std::memory_order_seq_cst);
	uint16_t countAfter = *count;
	if ((countBefore == 0) || (countBefore != countAfter)) {
		return;
	}

	float average = sum / countBefore;
	rgb_color color = {0, 0, 0};
	HAQuDA_GetMeasRGB(average, dotsOfParam, color);
	snapshot.Publish(param, average, HAQuDA_ToWebColor(color));
}

void HAQuDA_MeasHandler::publishSnapshots() {
	// The snapshot colour is the scale colour at full channel values: it does not depend on the lamp brightness.
	publishOne(measSnapshot, snapshot_param::temp, temp_meas, dots.temp);
	publishOne(measSnapshot, snapshot_param::humid, humid_meas, dots.humid);
	publishOne(measSnapshot, snapshot_param::eco2, eCO2_meas, dots.eCO2);
	publishOne(measSnapshot, snapshot_param::tvoc, TVOC_meas, dots.TVOC);
	publishOne(measSnapshot, snapshot_param::pm25, PM_2_5_meas, dots.PM2_5);
}

anim_id HAQuDA_MeasHandler::CurrentAnim() {
	switch (mode) {
		case standard:
			if (standard_m_p == noneType) {
				return anim_id::none;
			}
			return (standard_m_p == total) ? anim_id::measTotal : anim_id::measStandard;
		case multi:
			return anim_id::measMulti;
		case night:
			return anim_id::measNight;
		default:
			return anim_id::none;
	}
}

bool HAQuDA_MeasHandler::readAverage(meas_type type, float &average) {
	const meas_data *data = nullptr;
	switch (type) {
		case temp:
			data = &temp_meas;
			break;
		case humid:
			data = &humid_meas;
			break;
		case eCO2:
			data = &eCO2_meas;
			break;
		case TVOC:
			data = &TVOC_meas;
			break;
		case PM2_5:
			data = &PM_2_5_meas;
			break;
		default:
			return false;
	}
	if (data->measNum == 0) {
		return false;
	}
	average = data->value / data->measNum;
	return true;
}

div_dots HAQuDA_MeasHandler::dotsOf(meas_type type) {
	switch (type) {
		case temp:
			return dots.temp;
		case humid:
			return dots.humid;
		case eCO2:
			return dots.eCO2;
		case TVOC:
			return dots.TVOC;
		case PM2_5:
			return dots.PM2_5;
		default:
			return div_dots{};
	}
}

bool HAQuDA_MeasHandler::ReadAverages(anim_id &anim, meas_anim_data &data) {
	data = meas_anim_data{};
	anim = CurrentAnim();
	switch (anim) {
		case anim_id::measStandard:
		case anim_id::measNight: {
			if (!readAverage(standard_m_p, data.values[0])) {
				return false;
			}
			data.dots[0] = dotsOf(standard_m_p);
			data.keys[0] = standard_m_p;
			if (anim == anim_id::measNight) {
				DateTimeStruct currTime;
				if (HAQuDA_TimeService::getInstance()->GetDateTime(&currTime)) {
					data.nightLitCells = HAQuDA_NightLitCells(night_m_time.start, night_m_time.finish, currTime.hour * 60 + currTime.min);
				}
			}
			return true;
		}
		case anim_id::measMulti: {
			for (uint8_t i = 0; i < MULTI_MODE_PARAM_NUM; i++) {
				if (!readAverage(multi_m_p.paramsArr[i], data.values[i])) {
					return false;
				}
				data.dots[i] = dotsOf(multi_m_p.paramsArr[i]);
				data.keys[i] = multi_m_p.paramsArr[i];
			}
			return true;
		}
		case anim_id::measTotal: {
			// The order the total animation expects: temp and humid are bad on both sides, the others above last only
			const meas_type types[MEAS_ANIM_MAX_PARAMS] = {temp, humid, eCO2, TVOC, PM2_5};
			bool anySample = false;
			for (uint8_t i = 0; i < MEAS_ANIM_MAX_PARAMS; i++) {
				// A sensor that gives nothing (CCS811 down, ZH03B warming up) must not stop the other readings from colouring the lamp
				data.noSample[i] = !readAverage(types[i], data.values[i]);
				anySample = anySample || !data.noSample[i];
				data.dots[i] = dotsOf(types[i]);
			}
			return anySample;
		}
		default:
			return false;
	}
}

void HAQuDA_MeasHandler::RestartAveraging() {
	ClearMeas();
}

void HAQuDA_MeasHandler::SetMeasDispMode(meas_mode m) {
	mode = m;
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

HAQuDA_MeasHandler::~HAQuDA_MeasHandler() {
}
