#pragma once

#include "HAQuDA_DisplayTypes.h"
#include "HAQuDA_MeasHandler.h"
#include "HAQuDA_EffectsHandler.h"
#include "HAQuDA_ErrorHandler.h"

class HAQuDA_WebPageDisplayManip;
class HAQuDA_DisplayService;

// Callers that are not converted to display posts yet go through these setters: each one also posts what the lamp must show
// to the display service, which draws it in ShowTask.
class HAQuDA_DisplayManip {

	static display_modes mode;

	static void selectLampColor(const uint32_t rgb);

  public:
	// webDisp gets a copy of every setter call, for the web page; it must outlive this object.
	HAQuDA_DisplayManip(HAQuDA_MeasHandler *_meas_handler, HAQuDA_EffectsHandler *_effects_handler, HAQuDA_ErrorHandler *_err_handler,
						HAQuDA_WebPageDisplayManip *_web_disp, HAQuDA_DisplayService *_display);

	// Posts the user's brightness in percent (0..100) to the display service, which puts it on the LEDs in ShowTask, and updates the
	// web mirror at once. loop task only.
	static void SetBrightness(const uint8_t new_brightness);

	// The lamp's own pictures are backgrounds like the measurements: events and errors show over the light, only errors over off.
	// loop task only.
	static void ShowLampLight(const uint32_t rgb);
	static void ShowLampOff();

	static bool SetMode(const display_modes new_mode);

	static display_modes GetMode() {
		return mode;
	};

	// Selects the measurement picture; the display shows the waiting picture (boot snake, access point fading in AP mode) until the first sample
	static void SetMeasMode(const meas_mode m);

	static void SetStandardMeasParam(const meas_type p);
	static void SetMultiMeasParam(const multi_mode_params p);
	static void SetNightTime(const night_time t);
	static void SetMeasDivDots(const meas_div_dots d);

	// Posts the event the effect stands for: snake = WiFi reconnecting, fading = access point running, static green = WiFi
	// joined, static red = WiFi lost; the other effects and a static colour of any other value post nothing
	static void SetEffect(const effect_type e);
	effect_type GetEffect();

	static void SetEffectParam(const snake_params p);
	static void SetEffectParam(const growing_params p);
	static void SetEffectParam(const fading_params p);
	static void SetEffectParam(const random_led_params p);
	static void SetEffectParam(const uint32_t p);

	~HAQuDA_DisplayManip();
};
