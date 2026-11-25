#pragma once

#include "HAQuDA_Animation.h"

class HAQuDA_SnakeAnim : public HAQuDA_Animation {
  public:
	explicit HAQuDA_SnakeAnim(HAQuDA_ILEDs &leds) : HAQuDA_Animation(leds) {
	}
	bool Begin(const anim_params &params, const anim_context &context) override;
	uint32_t Step(uint32_t nowMs) override;
	void Stop() override;
	anim_id Id() const override;

  private:
	bool running = false;
	bool darkPending = false;
	uint32_t color = 0;
	uint16_t movePauseMs = 0;
	uint8_t tailLen = 0;
	uint16_t head = 0;
};

class HAQuDA_GrowingAnim : public HAQuDA_Animation {
  public:
	explicit HAQuDA_GrowingAnim(HAQuDA_ILEDs &leds) : HAQuDA_Animation(leds) {
	}
	bool Begin(const anim_params &params, const anim_context &context) override;
	uint32_t Step(uint32_t nowMs) override;
	void Stop() override;
	anim_id Id() const override;

  private:
	bool running = false;
	bool darkPending = false;
	uint32_t color = 0;
	uint16_t growPauseMs = 0;
	uint8_t row = 0;
};

// The web page replays every pass from the seed of the start, so the sequence must stay exactly:
//   seed of pass 0 = the seed of Begin() (0 becomes 1); seed of pass n + 1 = xorshift32(seed of pass n)
//   state = seed of the pass; order = 0..ANIM_PIXELS-1
//   for i = ANIM_PIXELS-1 down to 1: j = xorshift32(state) % (i + 1); swap order[i], order[j]
//   step k lights order[k] with r = xorshift32(state) % 255, g = xorshift32(state) % 255, b = xorshift32(state) % 255
class HAQuDA_RandomPixelsAnim : public HAQuDA_Animation {
  public:
	explicit HAQuDA_RandomPixelsAnim(HAQuDA_ILEDs &leds) : HAQuDA_Animation(leds) {
	}
	bool Begin(const anim_params &params, const anim_context &context) override;
	uint32_t Step(uint32_t nowMs) override;
	void Stop() override;
	anim_id Id() const override;

	uint32_t Seed() const;

  private:
	void startPass();

	bool running = false;
	bool darkPending = false;
	bool passesStarted = false;
	uint32_t seed = 1;
	uint32_t rngState = 1;
	uint16_t pixelPauseMs = 0;
	uint16_t endPauseMs = 0;
	uint16_t step = 0;
	uint8_t order[ANIM_PIXELS] = {};
};

// toPercent itself is never shown: the dark between passes stands for it. Changes the lamp brightness, Stop() puts it back.
class HAQuDA_FadingAnim : public HAQuDA_Animation {
  public:
	explicit HAQuDA_FadingAnim(HAQuDA_ILEDs &leds) : HAQuDA_Animation(leds) {
	}
	bool Begin(const anim_params &params, const anim_context &context) override;
	uint32_t Step(uint32_t nowMs) override;
	void Stop() override;
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

// Resent every pass, which also repairs a frame the driver dropped after its ~50 ms CanShow() wait ran out
class HAQuDA_StaticColorAnim : public HAQuDA_Animation {
  public:
	explicit HAQuDA_StaticColorAnim(HAQuDA_ILEDs &leds) : HAQuDA_Animation(leds) {
	}
	bool Begin(const anim_params &params, const anim_context &context) override;
	uint32_t Step(uint32_t nowMs) override;
	void Stop() override;
	anim_id Id() const override;

  private:
	bool running = false;
	uint32_t color = 0;
};
