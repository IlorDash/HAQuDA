#pragma once

#include <cstdint>

#include "HAQuDA_ColorScale.h"
#include "HAQuDA_ILEDs.h"

// Layer 1a of the display system: one non-blocking step machine per picture. Hardware-free (no Arduino.h), built on the PC.

// Matrix size: 9 columns x 12 rows = 108 pixels (9 x WS2812_PCB_stack x 12). HAQuDA_LEDsParams.h still says LED_NUM_PIXELS 109,
// which addresses nothing; animations never use it. Duplicated here because that header includes Arduino.h.
constexpr uint8_t ANIM_COLUMNS = 9;
constexpr uint8_t ANIM_ROWS = 12;
constexpr uint16_t ANIM_PIXELS = ANIM_COLUMNS * ANIM_ROWS;

// What HAQuDA_LEDs::Show(), Clear() and SetBright() block in the driver today. Animations never rely on it: they send frames with
// Show(ANIM_NO_WAIT_MS) and report every wait through the Step() return value, so these two values are the explicit model of the
// visible timing the old show_*() functions had.
constexpr uint32_t ANIM_LEDS_SHOW_MS = 200;
// Pause HAQuDA_DisplayManip::Show() adds after every pass of an effect (and every draw of a measurement).
constexpr uint32_t ANIM_PASS_PAUSE_MS = 100;
// Delay argument of ILEDs::Show(): the frame is sent, the driver does not wait.
constexpr uint16_t ANIM_NO_WAIT_MS = 0;
// Step() result of an animation that is not running (before Begin(), after Stop()).
constexpr uint32_t ANIM_IDLE_MS = 100;

// Colours as packed 0xRRGGBB, the same values as COLOR_* in HAQuDA_LEDsParams.h (checked there by static_assert in
// HAQuDA_MeasHandler.cpp).
constexpr uint32_t ANIM_COLOR_OFF = 0x000000;
constexpr uint32_t ANIM_COLOR_AQUA = 0x00FFFF;
constexpr uint32_t ANIM_COLOR_LIME = 0x99FF33;
constexpr uint32_t ANIM_COLOR_GREEN = 0x01FF00;
constexpr uint32_t ANIM_COLOR_RED = 0xFF0000;
constexpr uint32_t ANIM_COLOR_YELLOW = 0xFFFF00;

// White bar of the night mode as a percent of the colour brightness (WHITE_BRIGHTNESS_COEFF in HAQuDA_LEDsParams.h).
constexpr uint8_t ANIM_NIGHT_WHITE_PERCENT = 70;

enum class anim_id : uint8_t { snake, growing, randomPixels, fading, staticColor, measStandard, measMulti, measNight, measTotal, none };
constexpr uint8_t ANIM_COUNT = static_cast<uint8_t>(anim_id::none);

// Constant data of an animation, the form the catalog stores. One flat struct for all effects: build it with the factories below,
// they name the fields the animation reads. Measurement animations read no field here (their data comes in anim_context).
struct anim_params {
	anim_id anim;
	uint32_t color;
	// snake: pause per step, growing: per row, randomPixels: per pixel, fading: between two levels (on top of the 200 ms frame)
	uint16_t pauseMs;
	// randomPixels: pause after the last pixel
	uint16_t endPauseMs;
	// snake only: lit pixels behind the head, 1 or more
	uint8_t tailLen;
	// fading only: first level and the level the fade moves toward (not shown itself), percent of MAX_BRIGHTNESS 0..100. from > to
	// counts down, from < to counts up. stepPercent is the distance of two levels, 1 or more unless from == to.
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

// Parameters of a measurement animation, no catalog data.
constexpr anim_params HAQuDA_MeasParams(anim_id measAnim) {
	return anim_params{measAnim, 0, 0, 0, 0, 0, 0, 0};
}

constexpr uint8_t MEAS_ANIM_MAX_PARAMS = 5;

// The averages a measurement animation draws. Order of values/dots: temp, humid, eCO2, TVOC, PM2.5 for measTotal; the first
// value/dots for measStandard and measNight; the first three (the zones, top to bottom of the setting) for measMulti. Every used
// dots entry must have last > first (the colour scale divides by the difference).
struct meas_anim_data {
	float values[MEAS_ANIM_MAX_PARAMS];
	div_dots dots[MEAS_ANIM_MAX_PARAMS];
	// measNight: hours since the sleep window began, 0..12; 12 (outside the window) fills the whole bar
	uint8_t nightHour;
};

// What the caller supplies at each start, next to the catalog parameters.
struct anim_context {
	// Lamp brightness the caller keeps, percent of MAX_BRIGHTNESS 0..100. fading restores it on Stop(); the measurement animations
	// derive the colour-scale slope from it. Never a raw NeoPixel value.
	uint8_t lampBrightPercent;
	// randomPixels only: seed of the pixel order and colours. 0 is replaced by 1 (xorshift32 sticks at 0).
	uint32_t seed;
	// measurement animations only
	meas_anim_data meas;
};

// Common interface of all animations. Not thread-safe: one task (ShowTask) owns an animation from Begin() to Stop().
class HAQuDA_Animation {
  public:
	explicit HAQuDA_Animation(HAQuDA_ILEDs &leds) : leds(leds) {
	}
	virtual ~HAQuDA_Animation() = default;

	// Starts from the first frame, draws nothing; false (stays stopped) on a foreign anim id or invalid field. ShowTask only.
	virtual bool Begin(const anim_params &params, const anim_context &context) = 0;
	// Draws the next frame, returns the wait in ms until the next call; short passes repeat by themselves. ShowTask only, never blocks.
	virtual uint32_t Step(uint32_t nowMs) = 0;
	// Ends at once and restores what it changed outside its pixels (fading brightness); call before the next Begin(). ShowTask only.
	virtual void Stop() = 0;
	// Which animation this is. ShowTask only.
	virtual anim_id Id() const = 0;
	// Name for the mirror and for lists, e.g. "snake". Any task.
	const char *Name() const;

  protected:
	HAQuDA_ILEDs &leds;
};

// Name of an animation, "?" for anim_id::none or an invalid value. Any task.
const char *HAQuDA_AnimName(anim_id id);

// Pixel number of column x (0..ANIM_COLUMNS - 1), row y (0..ANIM_ROWS - 1) on the serpentine chain: odd columns run reversed.
// Returns ANIM_PIXELS (no pixel) outside the matrix. Any task.
uint16_t HAQuDA_PixelOf(uint8_t x, uint8_t y);

// xorshift32 step (13, 17, 5): advances state and returns it. The web page runs the same generator on the seed sent in /api/disp,
// so it must stay exactly this. state must not be 0. Any task.
uint32_t HAQuDA_XorShift32(uint32_t &state);
