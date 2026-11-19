#include "HAQuDA_MeasAnimations.h"

namespace {
	constexpr uint32_t PERCENT_FULL = 100;
	// Clear() blocked 200 ms, then the old showMeas_*() waited 100 ms
	constexpr uint32_t MEAS_DARK_MS = ANIM_LEDS_SHOW_MS + 100;
	constexpr uint32_t MEAS_HOLD_MS = ANIM_LEDS_SHOW_MS + ANIM_PASS_PAUSE_MS;

	constexpr uint8_t MULTI_ZONES = 3;
	constexpr uint8_t MULTI_ZONE_ROWS = 3;
	constexpr uint8_t MULTI_ZONE_STRIDE = 4;

	constexpr uint8_t NIGHT_COLOUR_COLUMN_STRIDE = 3;
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
	const uint32_t percent = (context.lampBrightPercent > PERCENT_FULL) ? PERCENT_FULL : context.lampBrightPercent;
	rawBright = static_cast<uint8_t>(percent * COLOR_SCALE_MAX_CHANNEL / PERCENT_FULL);
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
	HAQuDA_GetMeasRGB(data.values[index], data.dots[index], rawBright, color);
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
	// Same integer maths order as the old showMeas_night(), so the rounding matches
	const uint8_t whiteBright = static_cast<uint8_t>(rawBright * 1000 / COLOR_SCALE_MAX_CHANNEL * ANIM_NIGHT_WHITE_PERCENT / 1000);
	const uint8_t barPixels = (data.nightHour > ANIM_ROWS) ? ANIM_ROWS : data.nightHour;

	for (uint8_t x = 0; x < ANIM_COLUMNS; x++) {
		const bool colourColumn = (x % NIGHT_COLOUR_COLUMN_STRIDE) == 0;
		const uint8_t pixelNum = colourColumn ? ANIM_ROWS : barPixels;
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
	// temp and humid are bad on both sides, the others only above last
	int8_t bad = -1;
	for (uint8_t i = 0; (i < MEAS_ANIM_MAX_PARAMS) && (bad < 0); i++) {
		const bool twoSided = i < 2;
		if ((data.values[i] >= data.dots[i].last) || (twoSided && (data.values[i] <= data.dots[i].first))) {
			bad = static_cast<int8_t>(i);
		}
	}

	rgb_color color = {0, 0, 0};
	if (bad >= 0) {
		color = colorOf(static_cast<uint8_t>(bad));
	} else {
		// 16-bit: five channels of up to 150 overflow a byte
		uint16_t rSum = 0;
		uint16_t gSum = 0;
		uint16_t bSum = 0;
		for (uint8_t i = 0; i < MEAS_ANIM_MAX_PARAMS; i++) {
			const rgb_color one = colorOf(i);
			rSum += one.r;
			gSum += one.g;
			bSum += one.b;
		}
		color = rgb_color{static_cast<uint8_t>(rSum / MEAS_ANIM_MAX_PARAMS), static_cast<uint8_t>(gSum / MEAS_ANIM_MAX_PARAMS),
						  static_cast<uint8_t>(bSum / MEAS_ANIM_MAX_PARAMS)};
	}
	leds.FillColor(color.r, color.g, color.b);
	drawn = meas_drawn{};
	drawn.colorCount = 1;
	drawn.colors[0] = color;
}
