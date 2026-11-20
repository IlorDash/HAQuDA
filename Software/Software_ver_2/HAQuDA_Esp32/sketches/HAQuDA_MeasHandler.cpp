#include "HAQuDA_MeasHandler.h"
#include "HAQuDA_MeasShowPolicy.h"
#include "HAQuDA_NightProgress.h"
#include "HAQuDA_TotalColor.h"
#include "HAQuDA_TimeHelper.h"
#include "HAQuDA_LEDsParams.h"
#include "HAQuDA_LEDs.h"
#include "HAQuDA_WebPageDisplayManip.h"
#include "Sensors.h"

#include <atomic>

#define MAX_SENS_REBOOT_ATTEMPTS 3

bool HAQuDA_MeasHandler::sens_start;
int HAQuDA_MeasHandler::meas_num;
bool HAQuDA_MeasHandler::show_meas_immed;

meas_mode HAQuDA_MeasHandler::mode;
meas_type HAQuDA_MeasHandler::standard_m_p;
multi_mode_params HAQuDA_MeasHandler::multi_m_p;
night_time HAQuDA_MeasHandler::night_m_time;
meas_div_dots HAQuDA_MeasHandler::dots;

static HAQuDA_ErrorHandler *errHandler;
static HAQuDA_WebPageDisplayManip *webDisp;

HAQuDA_MeasHandler::HAQuDA_MeasHandler(HAQuDA_ErrorHandler *_err_handler, HAQuDA_MeasSnapshot &snapshot, HAQuDA_WebPageDisplayManip &webDispToFill)
	: measSnapshot(snapshot) {

	sens_start = false;

	meas_num = 0;
	show_meas_immed = false;

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
	multi_m_p.dataArr[0] = 0;
	multi_m_p.dataArr[1] = 0;
	multi_m_p.dataArr[2] = 0;
	multi_m_p.divideDotsArr[0] = dots.temp;
	multi_m_p.divideDotsArr[1] = dots.eCO2;
	multi_m_p.divideDotsArr[2] = dots.PM2_5;

	errHandler = _err_handler;
	webDisp = &webDispToFill;
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

// Scales one lamp channel (0..COLOR_SCALE_MAX_CHANNEL, the LED current limit) to the full 0..255 range of a web colour.
// The clamp keeps the result below 256, so a channel never carries into its neighbour in the packed colour.
static uint32_t webChannel(uint8_t channel) {
	uint32_t limited = (channel > COLOR_SCALE_MAX_CHANNEL) ? COLOR_SCALE_MAX_CHANNEL : channel;
	return limited * 255 / COLOR_SCALE_MAX_CHANNEL;
}

// The colour as 0xRRGGBB in the web scale, the same conversion for /api/status and for the pictures the lamp draws.
static uint32_t webColor(const rgb_color &color) {
	return (webChannel(color.r) << 16) | (webChannel(color.g) << 8) | webChannel(color.b);
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
	snapshot.Publish(param, average, webColor(color));
}

void HAQuDA_MeasHandler::publishSnapshots() {
	// The snapshot colour is the scale colour at full channel values: it does not depend on the lamp brightness.
	publishOne(measSnapshot, snapshot_param::temp, temp_meas, dots.temp);
	publishOne(measSnapshot, snapshot_param::humid, humid_meas, dots.humid);
	publishOne(measSnapshot, snapshot_param::eco2, eCO2_meas, dots.eCO2);
	publishOne(measSnapshot, snapshot_param::tvoc, TVOC_meas, dots.TVOC);
	publishOne(measSnapshot, snapshot_param::pm25, PM_2_5_meas, dots.PM2_5);
}

void HAQuDA_MeasHandler::Show(int *show_meas_time) {

	bool show_meas_time_passed = (millis() - *show_meas_time > DISP_MEAS_PERIOD_MS);

	if (show_meas_time_passed || show_meas_immed) {

		// A pass that could not draw (the shown reading has no sample yet, the first one comes within 2 s of a redraw) keeps the
		// redraw request, so the next pass tries again instead of leaving the old picture up for a minute. The period timer and the
		// sums still restart once the period has elapsed, so a reading that never gets a sample cannot let the other sums grow past
		// one period (HAQuDA_DecideShowOutcome()).
		bool consumed = true;
		switch (mode) {
			case standard: {
				consumed = standardMode();
				break;
			}
			case multi: {
				consumed = multiMode(multi_m_p);
				break;
			}
			case night: {
				consumed = nightMode();
				break;
			}
			default:
				break;
		}
		const meas_show_outcome outcome = HAQuDA_DecideShowOutcome(consumed, show_meas_time_passed);
		if (outcome.restartPeriod) {
			*show_meas_time = millis();
			ClearMeas();
		}
		if (!outcome.keepRequest) {
			show_meas_immed = false;
		}
	}
}

void HAQuDA_MeasHandler::SetMeasDispMode(meas_mode m) {
	if (m == noneMeas)
		return;
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

// The sums of one reading and its thresholds, nullptr for total and none. The caller divides by measNum only after checking it
// is not 0: no sample since the last redraw has no average (0 / 0 is NaN, which the colour scale leaves undrawn).
static meas_data *selectMeas(meas_type p, const meas_div_dots &allDots, div_dots &dotsOut) {
	switch (p) {
		case temp:
			dotsOut = allDots.temp;
			return &temp_meas;
		case humid:
			dotsOut = allDots.humid;
			return &humid_meas;
		case eCO2:
			dotsOut = allDots.eCO2;
			return &eCO2_meas;
		case TVOC:
			dotsOut = allDots.TVOC;
			return &TVOC_meas;
		case PM2_5:
			dotsOut = allDots.PM2_5;
			return &PM_2_5_meas;
		default:
			return nullptr;
	}
}

bool HAQuDA_MeasHandler::standardMode() {
	if (standard_m_p == total) {
		return totalMode();
	}
	div_dots paramDots = {0, 0, 0};
	const meas_data *data = selectMeas(standard_m_p, dots, paramDots);
	if (data == nullptr) {
		// No reading chosen: nothing will ever be drawn, so there is nothing to retry
		return true;
	}
	if (data->measNum == 0) {
		return false;
	}
	showMeas_standard(data->value / data->measNum, paramDots);
	return true;
}

bool HAQuDA_MeasHandler::totalMode() {
	static_assert(TOTAL_READING_NUM == DISP_PARAMS_NUM, "the overall rule takes one reading per displayed parameter");
	const meas_type all[TOTAL_READING_NUM] = {temp, humid, eCO2, TVOC, PM2_5};
	total_reading readings[TOTAL_READING_NUM] = {};
	for (size_t i = 0; i < TOTAL_READING_NUM; i++) {
		const meas_data *data = selectMeas(all[i], dots, readings[i].dots);
		if ((data != nullptr) && (data->measNum != 0)) {
			readings[i].hasSample = true;
			readings[i].average = data->value / data->measNum;
		}
	}
	rgb_color color = {0, 0, 0};
	if (!HAQuDA_TotalColor(readings, color)) {
		return false;
	}
	drawSolid(color);
	return true;
}

bool HAQuDA_MeasHandler::multiMode(multi_mode_params multiModeParams) {
	for (int i = 0; i < MULTI_MODE_PARAM_NUM; i++) {
		// The thresholds are taken from the current ones, so SetMeasDivDots() reaches the multi picture too
		const meas_data *data = selectMeas(multiModeParams.paramsArr[i], dots, multiModeParams.divideDotsArr[i]);
		if (data == nullptr) {
			return true;
		}
		if (data->measNum == 0) {
			return false;
		}
		multiModeParams.dataArr[i] = data->value / data->measNum;
	}
	showMeas_multi(multiModeParams.dataArr, sizeof(multiModeParams.dataArr) / sizeof(multiModeParams.dataArr[0]), multiModeParams.divideDotsArr,
				   sizeof(multiModeParams.divideDotsArr) / sizeof(multiModeParams.divideDotsArr[0]));
	return true;
}

bool HAQuDA_MeasHandler::nightMode() {
	div_dots paramDots = {0, 0, 0};
	meas_data *data = selectMeas(standard_m_p, dots, paramDots);
	if (data == nullptr) {
		return true;
	}
	if (data->measNum == 0) {
		return false;
	}

	uint8_t litCells = 0;
	DateTimeStruct currTime;
	if (HAQuDA_TimeService::getInstance()->GetDateTime(&currTime)) {
		litCells = HAQuDA_NightLitCells(night_m_time.start, night_m_time.finish, currTime.hour * 60 + currTime.min);
	}
	showMeas_night(data->value / data->measNum, paramDots, litCells);
	ClearMeasData(*data);
	return true;
}

void HAQuDA_MeasHandler::showMeas_standard(float data, div_dots divideDots) {
	rgb_color color = {0, 0, 0};
	HAQuDA_GetMeasRGB(data, divideDots, color);
	drawSolid(color);
}

void HAQuDA_MeasHandler::drawSolid(const rgb_color &color) {
	HAQuDA_LEDs::getInstance()->FillColor(color.r, color.g, color.b);
	HAQuDA_LEDs::getInstance()->Show();

	const uint32_t drawnColors[3] = {webColor(color), 0, 0};
	webDisp->SetMeasDrawn(standard, drawnColors, 0);
}

void HAQuDA_MeasHandler::showMeas_night(float data, div_dots divideDots, uint8_t litCells) {
	// Only the pixel buffer is cleared: no empty frame is sent, so the old picture is replaced by the new one in a single frame
	HAQuDA_LEDs::getInstance()->FillColor(0, 0, 0);

	rgb_color color = {0, 0, 0};
	HAQuDA_GetMeasRGB(data, divideDots, color);

	const uint8_t whiteBright = HAQuDA_NightGrey();
	for (int i = 0; i < LED_COLUMN_NUM; i++) {
		const bool colorColumn = HAQuDA_NightColumnIsColor(i);
		uint32_t pixelColor = colorColumn ? HAQuDA_LEDs::getInstance()->GetColor(color.r, color.g, color.b)
										  : HAQuDA_LEDs::getInstance()->GetColor(whiteBright, whiteBright, whiteBright);
		uint8_t pixelNum = colorColumn ? LED_ROW_NUM : HAQuDA_NightLightRows(litCells, i);
		// NeoPixel fill() with a count of 0 fills to the end of the strip
		if (pixelNum == 0) {
			continue;
		}
		uint8_t startPixel = (!(i % 2)) ? (i * LED_ROW_NUM) : (i * LED_ROW_NUM + LED_ROW_NUM - pixelNum);
		HAQuDA_LEDs::getInstance()->FillColorFrom(pixelColor, startPixel, pixelNum);
	}
	HAQuDA_LEDs::getInstance()->Show();

	// The bar's grey goes through the same 0..COLOR_SCALE_MAX_CHANNEL -> 0..255 conversion as the key colour
	const rgb_color white = {whiteBright, whiteBright, whiteBright};
	const uint32_t drawnColors[3] = {webColor(color), 0, 0};
	webDisp->SetMeasDrawn(night, drawnColors, webColor(white));
}

void HAQuDA_MeasHandler::showMeas_multi(float *data, uint8_t dataSize, div_dots *divDots, uint8_t divDotsSize) {

	if (dataSize != MULTI_MODE_PARAM_NUM) {
		return;
	}
	if (divDotsSize != DIVIDE_DOTS_NUM) {
		return;
	}

	HAQuDA_LEDs::getInstance()->FillColor(0, 0, 0);

	rgb_color colors[MULTI_MODE_PARAM_NUM];
	for (int i = 0; i < MULTI_MODE_PARAM_NUM; i++) {
		HAQuDA_GetMeasRGB(data[i], divDots[i], colors[i]);
	}
	for (int i = 0; i < LED_ROW_NUM; i++) {
		if (!(i % 2)) {
			HAQuDA_LEDs::getInstance()->FillColorFrom(colors[0].r, colors[0].g, colors[0].b, i * LED_ROW_NUM + 1, 3);
			HAQuDA_LEDs::getInstance()->FillColorFrom(colors[1].r, colors[1].g, colors[1].b, i * LED_ROW_NUM + 5, 3);
			HAQuDA_LEDs::getInstance()->FillColorFrom(colors[2].r, colors[2].g, colors[2].b, i * LED_ROW_NUM + 9, 3);
		} else {
			HAQuDA_LEDs::getInstance()->FillColorFrom(colors[2].r, colors[2].g, colors[2].b, i * LED_ROW_NUM, 3);
			HAQuDA_LEDs::getInstance()->FillColorFrom(colors[1].r, colors[1].g, colors[1].b, i * LED_ROW_NUM + 4, 3);
			HAQuDA_LEDs::getInstance()->FillColorFrom(colors[0].r, colors[0].g, colors[0].b, i * LED_ROW_NUM + 8, 3);
		}
	}
	HAQuDA_LEDs::getInstance()->Show();

	const uint32_t drawnColors[MULTI_MODE_PARAM_NUM] = {webColor(colors[0]), webColor(colors[1]), webColor(colors[2])};
	webDisp->SetMeasDrawn(multi, drawnColors, 0);
}

HAQuDA_MeasHandler::~HAQuDA_MeasHandler() {
}