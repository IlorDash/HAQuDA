#pragma once

#include "HAQuDA_Animation.h"

// What a measurement animation drew, for the web mirror. Channels are the lamp's 0..COLOR_SCALE_MAX_CHANNEL scale, as the colour
// scale returns them.
struct meas_drawn {
	// 1 (standard, night, total) or 3 (multi); colors[] beyond it are 0
	uint8_t colorCount;
	rgb_color colors[3];
	// night only: the grey of the sleep bar, 0 elsewhere
	rgb_color white;
};

// Shared steps of the four measurement animations. A draw is: dark frame, 300 ms later (200 ms Clear() + 100 ms pause of the old
// showMeas_*()) the picture, then the same frame is resent every 300 ms. The picture does not follow new data by itself: the caller
// starts the animation again with a fresh meas_anim_data, which shows the dark flash like the old once-a-minute redraw.
class HAQuDA_MeasAnim : public HAQuDA_Animation {
  public:
	bool Begin(const anim_params &params, const anim_context &context) override;
	uint32_t Step(uint32_t nowMs) override;
	void Stop() override;
	anim_id Id() const override;

	// The picture of the last draw, all zero before the first one. ShowTask only.
	const meas_drawn &Drawn() const;

  protected:
	HAQuDA_MeasAnim(HAQuDA_ILEDs &leds, anim_id id, uint8_t paramsNum) : HAQuDA_Animation(leds), id(id), paramsNum(paramsNum) {
	}

	// Writes the picture into the LED buffer and fills drawn.
	virtual void draw() = 0;
	// Colour of data.values[index] on the colour scale at the lamp brightness.
	rgb_color colorOf(uint8_t index) const;

	meas_anim_data data = {};
	meas_drawn drawn = {};
	// lampBrightPercent as the raw LED level (0..COLOR_SCALE_MAX_CHANNEL) the colour scale expects
	uint8_t rawBright = 0;

  private:
	const anim_id id;
	const uint8_t paramsNum;
	bool running = false;
	bool drawPending = false;
};

// One parameter on all pixels.
class HAQuDA_MeasStandardAnim : public HAQuDA_MeasAnim {
  public:
	explicit HAQuDA_MeasStandardAnim(HAQuDA_ILEDs &leds) : HAQuDA_MeasAnim(leds, anim_id::measStandard, 1) {
	}

  protected:
	void draw() override;
};

// Three parameters in three zones of three rows each (rows 1-3, 5-7, 9-11 of every column).
class HAQuDA_MeasMultiAnim : public HAQuDA_MeasAnim {
  public:
	explicit HAQuDA_MeasMultiAnim(HAQuDA_ILEDs &leds) : HAQuDA_MeasAnim(leds, anim_id::measMulti, 3) {
	}

  protected:
	void draw() override;
};

// One parameter on every third column, white bars of nightHour pixels (12 = full) on the others as the sleep progress.
class HAQuDA_MeasNightAnim : public HAQuDA_MeasAnim {
  public:
	explicit HAQuDA_MeasNightAnim(HAQuDA_ILEDs &leds) : HAQuDA_MeasAnim(leds, anim_id::measNight, 1) {
	}

  protected:
	void draw() override;
};

// All five parameters in one colour: the first value outside its comfortable range (temp, humid outside first..last, eCO2, TVOC,
// PM2.5 at or above last) is shown like measStandard, with none outside the average of the five colours.
class HAQuDA_MeasTotalAnim : public HAQuDA_MeasAnim {
  public:
	explicit HAQuDA_MeasTotalAnim(HAQuDA_ILEDs &leds) : HAQuDA_MeasAnim(leds, anim_id::measTotal, MEAS_ANIM_MAX_PARAMS) {
	}

  protected:
	void draw() override;
};
