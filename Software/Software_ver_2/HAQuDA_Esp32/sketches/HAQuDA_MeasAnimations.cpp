#include "HAQuDA_MeasAnimations.h"

#include "HAQuDA_LEDsParams.h"
#include "HAQuDA_NightProgress.h"
#include "HAQuDA_TotalColor.h"

static_assert(TOTAL_READING_NUM == MEAS_ANIM_MAX_PARAMS, "the overall rule takes one reading per animation parameter");

namespace {
	// Clear() blocked 200 ms, then the old showMeas_*() waited 100 ms
	constexpr uint32_t MEAS_DARK_MS = ANIM_LEDS_SHOW_MS + 100;
	constexpr uint32_t MEAS_HOLD_MS = ANIM_LEDS_SHOW_MS + ANIM_PASS_PAUSE_MS;

	constexpr uint8_t MULTI_ZONES = 3;
	constexpr uint8_t MULTI_ZONE_ROWS = 3;
	constexpr uint8_t MULTI_ZONE_STRIDE = 4;
} // namespace

bool HAQuDA_MeasAnim::Begin(const anim_params &params, const anim_context &context) {
	running = false;
	if (params.anim != id) {
		return false;
	}
	for (uint8_t i = 0; i < paramsNum; i++) {
		if (context.meas.dots[i].last <= context.meas.dots[i].first) {
			return false;
		}
	}
	data = context.meas;
	drawn = meas_drawn{};
	drawPending = false;
	running = true;
	return true;
}

uint32_t HAQuDA_MeasAnim::Step(uint32_t nowMs) {
	if (!running) {
		return ANIM_IDLE_MS;
	}
	if (!drawPending) {
		leds.FillColor(ANIM_COLOR_OFF);
		leds.Show(ANIM_NO_WAIT_MS);
		drawPending = true;
		return MEAS_DARK_MS;
	}
	draw();
	leds.Show(ANIM_NO_WAIT_MS);
	return MEAS_HOLD_MS;
}

void HAQuDA_MeasAnim::Stop() {
	running = false;
}

anim_id HAQuDA_MeasAnim::Id() const {
	return id;
}

const meas_drawn &HAQuDA_MeasAnim::Drawn() const {
	return drawn;
}

rgb_color HAQuDA_MeasAnim::colorOf(uint8_t index) const {
	// NaN matches no range and leaves the colour black
	rgb_color color = {0, 0, 0};
	HAQuDA_GetMeasRGB(data.values[index], data.dots[index], color);
	return color;
}

void HAQuDA_MeasStandardAnim::draw() {
	const rgb_color color = colorOf(0);
	leds.FillColor(color.r, color.g, color.b);
	drawn = meas_drawn{};
	drawn.colorCount = 1;
	drawn.colors[0] = color;
}

void HAQuDA_MeasMultiAnim::draw() {
	rgb_color colors[MULTI_ZONES] = {};
	for (uint8_t i = 0; i < MULTI_ZONES; i++) {
		colors[i] = colorOf(i);
	}
	for (uint8_t x = 0; x < ANIM_COLUMNS; x++) {
		const bool reversed = (x % 2 != 0);
		for (uint8_t zone = 0; zone < MULTI_ZONES; zone++) {
			// Odd columns run the chain downward, so their zones are mirrored
			const uint8_t firstRow = reversed ? (zone * MULTI_ZONE_STRIDE) : (1 + zone * MULTI_ZONE_STRIDE);
			const rgb_color &color = reversed ? colors[MULTI_ZONES - 1 - zone] : colors[zone];
			leds.FillColorFrom(color.r, color.g, color.b, x * ANIM_ROWS + firstRow, MULTI_ZONE_ROWS);
		}
	}
	drawn = meas_drawn{};
	drawn.colorCount = MULTI_ZONES;
	for (uint8_t i = 0; i < MULTI_ZONES; i++) {
		drawn.colors[i] = colors[i];
	}
}

void HAQuDA_MeasNightAnim::draw() {
	const rgb_color color = colorOf(0);
	const uint8_t whiteBright = HAQuDA_NightGrey();

	for (uint8_t x = 0; x < ANIM_COLUMNS; x++) {
		const bool colourColumn = HAQuDA_NightColumnIsColor(x);
		const uint8_t pixelNum = colourColumn ? ANIM_ROWS : HAQuDA_NightLightRows(data.nightLitCells, x);
		// A count of 0 would fill to the end of the strip
		if (pixelNum == 0) {
			continue;
		}
		// Odd columns run reversed, so the bar starts at the far end
		const uint8_t startPixel = (x % 2 == 0) ? (x * ANIM_ROWS) : (x * ANIM_ROWS + ANIM_ROWS - pixelNum);
		if (colourColumn) {
			leds.FillColorFrom(color.r, color.g, color.b, startPixel, pixelNum);
		} else {
			leds.FillColorFrom(whiteBright, whiteBright, whiteBright, startPixel, pixelNum);
		}
	}
	drawn = meas_drawn{};
	drawn.colorCount = 1;
	drawn.colors[0] = color;
	drawn.white = rgb_color{whiteBright, whiteBright, whiteBright};
}

void HAQuDA_MeasTotalAnim::draw() {
	total_reading readings[TOTAL_READING_NUM] = {};
	for (uint8_t i = 0; i < TOTAL_READING_NUM; i++) {
		readings[i] = total_reading{data.values[i], !data.noSample[i], data.dots[i]};
	}
	rgb_color color = {0, 0, 0};
	HAQuDA_TotalColor(readings, color);
	leds.FillColor(color.r, color.g, color.b);
	drawn = meas_drawn{};
	drawn.colorCount = 1;
	drawn.colors[0] = color;
}
