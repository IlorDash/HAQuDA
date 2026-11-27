#include "HAQuDA_EffectAnimations.h"

namespace {
	// Signed: counting down must not wrap below 0, counting up must not wrap above 255
	constexpr int16_t MAX_PERCENT = 100;

	uint32_t drawDark(HAQuDA_ILEDs &leds) {
		leds.FillColor(ANIM_COLOR_OFF);
		leds.Show(ANIM_NO_WAIT_MS);
		return ANIM_LEDS_SHOW_MS;
	}
} // namespace

bool HAQuDA_SnakeAnim::Begin(const anim_params &params, const anim_context &context) {
	running = false;
	if ((params.anim != anim_id::snake) || (params.tailLen == 0)) {
		return false;
	}
	color = params.color;
	movePauseMs = params.pauseMs;
	tailLen = params.tailLen;
	head = 0;
	darkPending = true;
	running = true;
	return true;
}

uint32_t HAQuDA_SnakeAnim::Step(uint32_t nowMs) {
	if (!running) {
		return ANIM_IDLE_MS;
	}
	if (darkPending) {
		darkPending = false;
		head = 0;
		return drawDark(leds);
	}
	for (uint8_t i = 0; i < tailLen; i++) {
		const uint16_t pixel = head + i;
		if (pixel < ANIM_PIXELS) {
			leds.SetPixelColor(pixel, color);
		}
	}
	leds.Show(ANIM_NO_WAIT_MS);
	// Cleared in the buffer only; the next frame sends it
	leds.SetPixelColor(head, ANIM_COLOR_OFF);
	head++;

	uint32_t waitMs = movePauseMs;
	if (head >= ANIM_PIXELS) {
		darkPending = true;
		waitMs += ANIM_PASS_PAUSE_MS;
	}
	return waitMs;
}

void HAQuDA_SnakeAnim::Stop() {
	running = false;
}

anim_id HAQuDA_SnakeAnim::Id() const {
	return anim_id::snake;
}

bool HAQuDA_GrowingAnim::Begin(const anim_params &params, const anim_context &context) {
	running = false;
	if (params.anim != anim_id::growing) {
		return false;
	}
	color = params.color;
	growPauseMs = params.pauseMs;
	darkPending = true;
	running = true;
	return true;
}

uint32_t HAQuDA_GrowingAnim::Step(uint32_t nowMs) {
	if (!running) {
		return ANIM_IDLE_MS;
	}
	if (darkPending) {
		darkPending = false;
		row = ANIM_ROWS - 1;
		return drawDark(leds);
	}
	for (uint8_t x = 0; x < ANIM_COLUMNS; x++) {
		leds.SetPixelColor(HAQuDA_PixelOf(x, row), color);
	}
	leds.Show(ANIM_NO_WAIT_MS);

	uint32_t waitMs = growPauseMs;
	if (row == 0) {
		darkPending = true;
		waitMs += ANIM_PASS_PAUSE_MS;
	} else {
		row--;
	}
	return waitMs;
}

void HAQuDA_GrowingAnim::Stop() {
	running = false;
}

anim_id HAQuDA_GrowingAnim::Id() const {
	return anim_id::growing;
}

bool HAQuDA_RandomPixelsAnim::Begin(const anim_params &params, const anim_context &context) {
	running = false;
	if (params.anim != anim_id::randomPixels) {
		return false;
	}
	pixelPauseMs = params.pauseMs;
	endPauseMs = params.endPauseMs;
	seed = (context.seed != 0) ? context.seed : 1;
	passesStarted = false;
	darkPending = true;
	running = true;
	return true;
}

void HAQuDA_RandomPixelsAnim::startPass() {
	if (passesStarted) {
		HAQuDA_XorShift32(seed);
	}
	passesStarted = true;
	rngState = seed;
	for (uint16_t i = 0; i < ANIM_PIXELS; i++) {
		order[i] = static_cast<uint8_t>(i);
	}
	for (uint16_t i = ANIM_PIXELS - 1; i >= 1; i--) {
		const uint16_t j = HAQuDA_XorShift32(rngState) % (i + 1);
		const uint8_t swapped = order[i];
		order[i] = order[j];
		order[j] = swapped;
	}
	step = 0;
}

uint32_t HAQuDA_RandomPixelsAnim::Step(uint32_t nowMs) {
	if (!running) {
		return ANIM_IDLE_MS;
	}
	if (darkPending) {
		darkPending = false;
		startPass();
		return drawDark(leds);
	}
	const uint32_t r = HAQuDA_XorShift32(rngState) % 255;
	const uint32_t g = HAQuDA_XorShift32(rngState) % 255;
	const uint32_t b = HAQuDA_XorShift32(rngState) % 255;
	leds.SetPixelColor(order[step], (r << 16) | (g << 8) | b);
	leds.Show(ANIM_NO_WAIT_MS);
	step++;

	uint32_t waitMs = pixelPauseMs;
	if (step >= ANIM_PIXELS) {
		darkPending = true;
		waitMs += endPauseMs + ANIM_PASS_PAUSE_MS;
	}
	return waitMs;
}

void HAQuDA_RandomPixelsAnim::Stop() {
	running = false;
}

anim_id HAQuDA_RandomPixelsAnim::Id() const {
	return anim_id::randomPixels;
}

uint32_t HAQuDA_RandomPixelsAnim::Seed() const {
	return seed;
}

bool HAQuDA_FadingAnim::Begin(const anim_params &params, const anim_context &context) {
	// Puts back the brightness of a fade still running before this one changes it
	Stop();
	if ((params.anim != anim_id::fading) || (params.fromPercent > MAX_PERCENT) || (params.toPercent > MAX_PERCENT)) {
		return false;
	}
	if ((params.fromPercent != params.toPercent) && (params.stepPercent == 0)) {
		return false;
	}
	color = params.color;
	fadePauseMs = params.pauseMs;
	fromPercent = params.fromPercent;
	toPercent = params.toPercent;
	stepPercent = params.stepPercent;
	restorePercent = (context.lampBrightPercent > MAX_PERCENT) ? static_cast<uint8_t>(MAX_PERCENT) : context.lampBrightPercent;
	current = phase::dark;
	running = true;
	return true;
}

uint32_t HAQuDA_FadingAnim::Step(uint32_t nowMs) {
	if (!running) {
		return ANIM_IDLE_MS;
	}
	switch (current) {
		case phase::dark: {
			// The old timing counted 200 ms for the dark frame and for the SetBright() frame
			leds.FillColor(ANIM_COLOR_OFF);
			leds.Show(ANIM_NO_WAIT_MS);
			leds.SetBright(fromPercent);
			brightnessChanged = true;
			current = phase::lit;
			return 2 * ANIM_LEDS_SHOW_MS;
		}
		case phase::lit: {
			leds.FillColor(color);
			leds.Show(ANIM_NO_WAIT_MS);
			level = fromPercent;
			if (fromPercent == toPercent) {
				current = phase::dark;
				return ANIM_LEDS_SHOW_MS + ANIM_PASS_PAUSE_MS;
			}
			current = phase::level;
			return ANIM_LEDS_SHOW_MS;
		}
		default: {
			// SetBright() rescales the stored picture and loses the colour when it climbs from a low level (from 0 % it stays black),
			// so the colour is drawn again at the new level; the rescaled frame is replaced a few ms later
			leds.SetBright(static_cast<uint8_t>(level));
			leds.FillColor(color);
			leds.Show(ANIM_NO_WAIT_MS);
			uint32_t waitMs = ANIM_LEDS_SHOW_MS + fadePauseMs;
			const int16_t next = (toPercent < fromPercent) ? (level - stepPercent) : (level + stepPercent);
			const bool passEnds = (toPercent < fromPercent) ? (next <= toPercent) : (next >= toPercent);
			if (passEnds) {
				current = phase::dark;
				waitMs += ANIM_PASS_PAUSE_MS;
			} else {
				level = next;
			}
			return waitMs;
		}
	}
}

void HAQuDA_FadingAnim::Stop() {
	if (running && brightnessChanged) {
		// SetBright() shows a frame; clear first so the faded colour does not flash at the restored level
		leds.FillColor(ANIM_COLOR_OFF);
		leds.SetBright(restorePercent);
	}
	brightnessChanged = false;
	running = false;
}

anim_id HAQuDA_FadingAnim::Id() const {
	return anim_id::fading;
}

bool HAQuDA_StaticColorAnim::Begin(const anim_params &params, const anim_context &context) {
	running = false;
	if (params.anim != anim_id::staticColor) {
		return false;
	}
	color = params.color;
	running = true;
	return true;
}

uint32_t HAQuDA_StaticColorAnim::Step(uint32_t nowMs) {
	if (!running) {
		return ANIM_IDLE_MS;
	}
	leds.FillColor(color);
	leds.Show(ANIM_NO_WAIT_MS);
	return ANIM_LEDS_SHOW_MS + ANIM_PASS_PAUSE_MS;
}

void HAQuDA_StaticColorAnim::Stop() {
	running = false;
}

anim_id HAQuDA_StaticColorAnim::Id() const {
	return anim_id::staticColor;
}
