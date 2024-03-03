#pragma once

#include "HAQuDA_MeasHandler.h"
#include "HAQuDA_EffectsHandler.h"
#include "HAQuDA_ErrorHandler.h"

typedef enum { effect, meas, error, event, noneMode } display_modes;

class HAQuDA_DisplayManip {

	static uint8_t brighntessPerCent;
	static display_modes mode;

  public:
	HAQuDA_DisplayManip(HAQuDA_MeasHandler *_meas_handler, HAQuDA_EffectsHandler *_effects_handler, HAQuDA_ErrorHandler *_err_handler);

	static void Show(int *show_meas_time);

	static void SetBrightness(const uint8_t new_brightness);
	uint8_t GetBrightness() {
		return brighntessPerCent;
	}

	static bool SetMode(const display_modes new_mode);
	
	static display_modes GetMode() {
		return mode;
	};

	static void SetMeasMode(const meas_mode m);
	meas_mode GetMeasDispMode();

	static void SetStandardMeasParam(const meas_type p);
	static void SetMultiMeasParam(const multi_mode_params p);
	static void SetNightTime(const night_time t);
	static void SetMeasDivDots(const meas_div_dots d);

	static void SetEffect(const effect_type e);
	effect_type GetEffect();

	static void SetEffectParam(const snake_params p);
	static void SetEffectParam(const growing_params p);
	static void SetEffectParam(const fading_params p);
	static void SetEffectParam(const random_led_params p);
	static void SetEffectParam(const uint32_t p);

	~HAQuDA_DisplayManip();
};