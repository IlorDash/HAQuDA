#pragma once

#include <cstdint>

#include "HAQuDA_ColorScale.h"
#include "HAQuDA_ILEDs.h"

// Duplicated from HAQuDA_LEDsParams.h, whose LED_NUM_PIXELS 109 is one more than the matrix has
constexpr uint8_t ANIM_COLUMNS = 9;
constexpr uint8_t ANIM_ROWS = 12;
constexpr uint16_t ANIM_PIXELS = ANIM_COLUMNS * ANIM_ROWS;

// The old show_*() timing counted this long for every frame, which the driver's Show() and Clear() still block. Animations send frames
// with Show(ANIM_NO_WAIT_MS) and SetBright(), neither waits, and return these waits from Step() instead. The pass pause is what
// DisplayManip::Show() added.
constexpr uint32_t ANIM_LEDS_SHOW_MS = 200;
constexpr uint32_t ANIM_PASS_PAUSE_MS = 100;
constexpr uint16_t ANIM_NO_WAIT_MS = 0;
constexpr uint32_t ANIM_IDLE_MS = 100;

// Must equal COLOR_* in HAQuDA_LEDsParams.h (static_assert in HAQuDA_MeasHandler.cpp)
constexpr uint32_t ANIM_COLOR_OFF = 0x000000;
constexpr uint32_t ANIM_COLOR_AQUA = 0x00FFFF;
constexpr uint32_t ANIM_COLOR_LIME = 0x99FF33;
constexpr uint32_t ANIM_COLOR_GREEN = 0x01FF00;
constexpr uint32_t ANIM_COLOR_RED = 0xFF0000;
constexpr uint32_t ANIM_COLOR_YELLOW = 0xFFFF00;

enum class anim_id : uint8_t { snake, growing, randomPixels, fading, staticColor, measStandard, measMulti, measNight, measTotal, none };
constexpr uint8_t ANIM_COUNT = static_cast<uint8_t>(anim_id::none);

// One flat struct for all effects so the catalog can store it as constant data; build it with the factories below
struct anim_params {
	anim_id anim;
	uint32_t color;
	uint16_t pauseMs;
	uint16_t endPauseMs;
	uint8_t tailLen;
	// stepPercent must be 1 or more unless fromPercent == toPercent
	uint8_t fromPercent;
	uint8_t toPercent;
	uint8_t stepPercent;
};

constexpr anim_params HAQuDA_SnakeParams(uint32_t color, uint16_t movePauseMs, uint8_t tailLen) {
	return anim_params{anim_id::snake, color, movePauseMs, 0, tailLen, 0, 0, 0};
}

constexpr anim_params HAQuDA_GrowingParams(uint32_t color, uint16_t growPauseMs) {
	return anim_params{anim_id::growing, color, growPauseMs, 0, 0, 0, 0, 0};
}

constexpr anim_params HAQuDA_RandomPixelsParams(uint16_t pixelPauseMs, uint16_t endPauseMs) {
	return anim_params{anim_id::randomPixels, 0, pixelPauseMs, endPauseMs, 0, 0, 0, 0};
}

constexpr anim_params HAQuDA_FadingParams(uint32_t color, uint16_t fadePauseMs, uint8_t fromPercent, uint8_t toPercent, uint8_t stepPercent) {
	return anim_params{anim_id::fading, color, fadePauseMs, 0, 0, fromPercent, toPercent, stepPercent};
}

constexpr anim_params HAQuDA_StaticColorParams(uint32_t color) {
	return anim_params{anim_id::staticColor, color, 0, 0, 0, 0, 0, 0};
}

constexpr anim_params HAQuDA_MeasParams(anim_id measAnim) {
	return anim_params{measAnim, 0, 0, 0, 0, 0, 0, 0};
}

constexpr uint8_t MEAS_ANIM_MAX_PARAMS = 5;

// Every used dots entry must have last > first: the colour scale divides by the difference
struct meas_anim_data {
	float values[MEAS_ANIM_MAX_PARAMS];
	div_dots dots[MEAS_ANIM_MAX_PARAMS];
	// Night light cells on (HAQuDA_NightLitCells()), 0 when the time is unknown
	uint8_t nightLitCells;
};

struct anim_context {
	// Never a raw NeoPixel value
	uint8_t lampBrightPercent;
	// 0 is replaced by 1: xorshift32 sticks at 0
	uint32_t seed;
	meas_anim_data meas;
};

// ShowTask only: not thread-safe, one task owns an animation from Begin() to Stop()
class HAQuDA_Animation {
  public:
	explicit HAQuDA_Animation(HAQuDA_ILEDs &leds) : leds(leds) {
	}
	virtual ~HAQuDA_Animation() = default;

	virtual bool Begin(const anim_params &params, const anim_context &context) = 0;
	virtual uint32_t Step(uint32_t nowMs) = 0;
	virtual void Stop() = 0;
	virtual anim_id Id() const = 0;
	const char *Name() const;

  protected:
	HAQuDA_ILEDs &leds;
};

const char *HAQuDA_AnimName(anim_id id);

uint16_t HAQuDA_PixelOf(uint8_t x, uint8_t y);

// The web page runs the same generator on the seed sent in /api/disp, so it must stay exactly xorshift32 (13, 17, 5)
uint32_t HAQuDA_XorShift32(uint32_t &state);
