#pragma once

#include "HAQuDA_Animation.h"

// The five effect animations. Every pass starts with a dark frame (the old Clear()), so a repeating effect flashes dark between
// passes exactly as it did. Each pass ends with the 100 ms ShowTask pause added to the wait of its last step.

// A lit run of tailLen pixels walks pixel 0 to ANIM_PIXELS - 1 in chain order, one step per movePause. The part of the tail beyond
// the last pixel is not drawn. Pass: 200 ms dark, ANIM_PIXELS steps, 100 ms.
class HAQuDA_SnakeAnim : public HAQuDA_Animation {
  public:
	explicit HAQuDA_SnakeAnim(HAQuDA_ILEDs &leds) : HAQuDA_Animation(leds) {
	}
	// Reads color, pauseMs, tailLen (>= 1). ShowTask only.
	bool Begin(const anim_params &params, const anim_context &context) override;
	// One snake step per call; returns the step wait in ms. ShowTask only.
	uint32_t Step(uint32_t nowMs) override;
	// Ends the animation; nothing outside its pixels to restore. ShowTask only.
	void Stop() override;
	// anim_id::snake. ShowTask only.
	anim_id Id() const override;

  private:
	bool running = false;
	bool darkPending = false;
	uint32_t color = 0;
	uint16_t movePauseMs = 0;
	uint8_t tailLen = 0;
	uint16_t head = 0;
};

// Fills the matrix row by row from row ANIM_ROWS - 1 down to row 0, all columns per step, one step per growPause. Pass: 200 ms dark,
// ANIM_ROWS steps, 100 ms.
class HAQuDA_GrowingAnim : public HAQuDA_Animation {
  public:
	explicit HAQuDA_GrowingAnim(HAQuDA_ILEDs &leds) : HAQuDA_Animation(leds) {
	}
	// Reads color, pauseMs. ShowTask only.
	bool Begin(const anim_params &params, const anim_context &context) override;
	// One row per call; returns the row wait in ms. ShowTask only.
	uint32_t Step(uint32_t nowMs) override;
	// Ends the animation; nothing outside its pixels to restore. ShowTask only.
	void Stop() override;
	// anim_id::growing. ShowTask only.
	anim_id Id() const override;

  private:
	bool running = false;
	bool darkPending = false;
	uint32_t color = 0;
	uint16_t growPauseMs = 0;
	uint8_t row = 0;
};

// Lights every pixel once, in a random order and a random colour each, one per pixel pause. No heap: the order is a fixed array,
// shuffled from the seed. The sequence is fully defined by the seed, so the web page replays it:
//   state = seed (0 becomes 1); order = 0..ANIM_PIXELS-1
//   for i = ANIM_PIXELS-1 down to 1: j = xorshift32(state) % (i + 1); swap order[i], order[j]
//   step k lights order[k] with r = xorshift32(state) % 255, g = xorshift32(state) % 255, b = xorshift32(state) % 255
// Every pass of one start restarts from the seed, so passes repeat the same picture. Pass: 200 ms dark, ANIM_PIXELS steps,
// endPause and 100 ms after the last.
class HAQuDA_RandomPixelsAnim : public HAQuDA_Animation {
  public:
	explicit HAQuDA_RandomPixelsAnim(HAQuDA_ILEDs &leds) : HAQuDA_Animation(leds) {
	}
	// Reads pauseMs, endPauseMs and context.seed. ShowTask only.
	bool Begin(const anim_params &params, const anim_context &context) override;
	// One pixel per call; returns the pixel wait in ms. ShowTask only.
	uint32_t Step(uint32_t nowMs) override;
	// Ends the animation; nothing outside its pixels to restore. ShowTask only.
	void Stop() override;
	// anim_id::randomPixels. ShowTask only.
	anim_id Id() const override;

	// The seed in use (never 0), what the page needs to replay the pass. ShowTask only.
	uint32_t Seed() const;

  private:
	void startPass();

	bool running = false;
	bool darkPending = false;
	uint32_t seed = 1;
	uint32_t rngState = 1;
	uint16_t pixelPauseMs = 0;
	uint16_t endPauseMs = 0;
	uint16_t step = 0;
	uint8_t order[ANIM_PIXELS] = {};
};

// Fills the matrix with a colour and moves the lamp brightness level by level from fromPercent toward toPercent (toPercent itself
// is never shown, the dark between passes stands for it): down for from > to, up for from < to. A level lasts 200 ms (the
// SetBright() frame) plus fadePause. Pass: 400 ms dark (cleared, then brightness set to from), the colour at from for 200 ms, then
// the levels from, from -/+ step, ... with the first level shown twice as the old effect did, 100 ms after the last. from == to
// shows only the 200 ms colour frame. Changes the lamp brightness: Stop() puts lampBrightPercent back. The driver's SetBright()
// still waits 200 ms itself, so on the current driver a Step() with a level takes that long inside the call.
class HAQuDA_FadingAnim : public HAQuDA_Animation {
  public:
	explicit HAQuDA_FadingAnim(HAQuDA_ILEDs &leds) : HAQuDA_Animation(leds) {
	}
	// Reads color, pauseMs, from/to/stepPercent and context.lampBrightPercent. ShowTask only.
	bool Begin(const anim_params &params, const anim_context &context) override;
	// One frame or brightness level per call; returns its wait in ms. ShowTask only.
	uint32_t Step(uint32_t nowMs) override;
	// Ends the animation and puts the lamp brightness back. ShowTask only.
	void Stop() override;
	// anim_id::fading. ShowTask only.
	anim_id Id() const override;

  private:
	enum class phase : uint8_t { dark, lit, level };

	bool running = false;
	bool brightnessChanged = false;
	phase current = phase::dark;
	uint32_t color = 0;
	uint16_t fadePauseMs = 0;
	uint8_t fromPercent = 0;
	uint8_t toPercent = 0;
	uint8_t stepPercent = 0;
	uint8_t restorePercent = 0;
	int16_t level = 0;
};

// One colour on the whole matrix, resent every 300 ms (200 ms frame + 100 ms pause as today), which also repairs a frame the driver
// dropped after its ~50 ms CanShow() wait ran out.
class HAQuDA_StaticColorAnim : public HAQuDA_Animation {
  public:
	explicit HAQuDA_StaticColorAnim(HAQuDA_ILEDs &leds) : HAQuDA_Animation(leds) {
	}
	// Reads color. ShowTask only.
	bool Begin(const anim_params &params, const anim_context &context) override;
	// Sends the colour frame; returns 300 ms. ShowTask only.
	uint32_t Step(uint32_t nowMs) override;
	// Ends the animation; nothing outside its pixels to restore. ShowTask only.
	void Stop() override;
	// anim_id::staticColor. ShowTask only.
	anim_id Id() const override;

  private:
	bool running = false;
	uint32_t color = 0;
};
