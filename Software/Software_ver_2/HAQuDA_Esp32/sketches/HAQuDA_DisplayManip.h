#pragma once

#include <atomic>

#include "HAQuDA_DisplayTypes.h"
#include "HAQuDA_MeasHandler.h"
#include "HAQuDA_EffectsHandler.h"
#include "HAQuDA_ErrorHandler.h"

class HAQuDA_WebPageDisplayManip;

class HAQuDA_DisplayManip {

	// The user's brightness in percent: written by SetBrightness() (loop task), read by ShowTask
	static std::atomic<uint8_t> brighntessPerCent;
	static display_modes mode;

	// ShowTask only: puts a pending brightness on the LEDs (see the definition)
	static void applyPendingBrightness();

  public:
	// webDisp gets a copy of every setter call, for the web page; it must outlive this object.
	HAQuDA_DisplayManip(HAQuDA_MeasHandler *_meas_handler, HAQuDA_EffectsHandler *_effects_handler, HAQuDA_ErrorHandler *_err_handler,
						HAQuDA_WebPageDisplayManip *_web_disp);

	// One ShowTask pass: puts a pending brightness on the LEDs, then draws the current mode. ShowTask only.
	static void Show(int *show_meas_time);

	// Stores the user's brightness in percent (0..100) and updates the web mirror at once; never touches the LEDs. ShowTask puts it
	// on them at the top of its next pass (Show()), after any effect that restores its own brightness has returned. loop task only.
	static void SetBrightness(const uint8_t new_brightness);

	// Makes ShowTask clear the matrix and re-apply the user's brightness once it enters the meas mode: an error effect
	// may leave the LEDs dim and its last frame lit.
	// Call before leaving the error mode. loop task only.
	static void RestoreBrightnessOnMeas();

	static bool SetMode(const display_modes new_mode);

	static display_modes GetMode() {
		return mode;
	};

	static void SetMeasMode(const meas_mode m);

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