#pragma once

#include "HAQuDA_Animation.h"

struct meas_drawn {
	uint8_t colorCount;
	rgb_color colors[3];
	rgb_color white;
};

// The picture does not follow new data by itself: the caller restarts it with a fresh meas_anim_data, which shows the dark flash
// like the old once-a-minute redraw
class HAQuDA_MeasAnim : public HAQuDA_Animation {
  public:
	bool Begin(const anim_params &params, const anim_context &context) override;
	uint32_t Step(uint32_t nowMs) override;
	void Stop() override;
	anim_id Id() const override;

	const meas_drawn &Drawn() const;

  protected:
	HAQuDA_MeasAnim(HAQuDA_ILEDs &leds, anim_id id, uint8_t paramsNum) : HAQuDA_Animation(leds), id(id), paramsNum(paramsNum) {
	}

	virtual void draw() = 0;
	rgb_color colorOf(uint8_t index) const;

	meas_anim_data data = {};
	meas_drawn drawn = {};
	uint8_t rawBright = 0;

  private:
	const anim_id id;
	const uint8_t paramsNum;
	bool running = false;
	bool drawPending = false;
};

class HAQuDA_MeasStandardAnim : public HAQuDA_MeasAnim {
  public:
	explicit HAQuDA_MeasStandardAnim(HAQuDA_ILEDs &leds) : HAQuDA_MeasAnim(leds, anim_id::measStandard, 1) {
	}

  protected:
	void draw() override;
};

class HAQuDA_MeasMultiAnim : public HAQuDA_MeasAnim {
  public:
	explicit HAQuDA_MeasMultiAnim(HAQuDA_ILEDs &leds) : HAQuDA_MeasAnim(leds, anim_id::measMulti, 3) {
	}

  protected:
	void draw() override;
};

class HAQuDA_MeasNightAnim : public HAQuDA_MeasAnim {
  public:
	explicit HAQuDA_MeasNightAnim(HAQuDA_ILEDs &leds) : HAQuDA_MeasAnim(leds, anim_id::measNight, 1) {
	}

  protected:
	void draw() override;
};

class HAQuDA_MeasTotalAnim : public HAQuDA_MeasAnim {
  public:
	explicit HAQuDA_MeasTotalAnim(HAQuDA_ILEDs &leds) : HAQuDA_MeasAnim(leds, anim_id::measTotal, MEAS_ANIM_MAX_PARAMS) {
	}

  protected:
	void draw() override;
};
