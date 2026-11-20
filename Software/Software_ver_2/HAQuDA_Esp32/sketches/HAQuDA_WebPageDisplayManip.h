#pragma once

#include "Arduino.h"

#include <atomic>

#include "HAQuDA_DisplayManip.h"
#include "HAQuDA_WebPageDisplayState.h"
#include "HAQuDA_WebPageDisplayManip_Translator.h"

// Mirrors what HAQuDA_DisplayManip shows, so the web page can draw the same picture on its grid. Every setter here is called by the
// setter of the same name in HAQuDA_DisplayManip, after that one has done its work. This class only maps the firmware enums onto
// the web_* ones, reads millis() and holds the state behind a portMUX critical section (held for a copy or one trivial call of the
// pure code in HAQuDA_WebPageDisplayState.h, the same pattern as HAQuDA_MeasSnapshot); what counts as a picture change, seq and
// the start time are decided there. The page plays the effect on its own from the parameters and the time of the last change.
//
// HAQuDA_ErrorHandler::ClearError() also stops the effect, without passing through HAQuDA_DisplayManip, so it is not mirrored. That
// is harmless: the error picture then shows error none (nothing drawn), and every caller follows it with SetMode() or SetEffect(),
// which reset the mirrored effect too.
class HAQuDA_WebPageDisplayManip {
  public:
	HAQuDA_WebPageDisplayManip();

	// Setters: see the class comment. SetMeasDivDots is not mirrored: the colours are not part of the picture layout.
	// Called wherever HAQuDA_DisplayManip::SetMode() is called (loop task, WiFi event task).
	void SetMode(const display_modes m);
	// Brightness in percent, as HAQuDA_DisplayManip::SetBrightness(). Any task.
	void SetBrightness(const uint8_t brightPerCent);
	// Selects the measurement layout; noneMeas is ignored like the meas handler does. Any task.
	void SetMeasMode(const meas_mode m);
	// Parameter of the standard and night layouts. Any task.
	void SetStandardMeasParam(const meas_type p);
	// The three parameters of the multi layout. Any task.
	void SetMultiMeasParam(const multi_mode_params p);
	// Night window in minutes since midnight. Any task.
	void SetNightTime(const night_time t);
	// The lamp has drawn a measurement picture: the colours it put on the LEDs as full-scale 0xRRGGBB (the lamp's 0..150 channels
	// scaled to 0..255), the same scale /api/status uses. colors[0] for standard and night, all three for multi (the bands in the
	// order of the keys), unused ones 0; whiteColor is the grey of the night bar, 0 for the others. Ignored when the picture
	// has changed since (another layout or source). Called by HAQuDA_MeasHandler::showMeas_*() in ShowTask, after the LEDs
	// are lit; the portMUX critical section makes it safe next to the loop and async_tcp tasks.
	void SetMeasDrawn(const meas_mode layout, const uint32_t colors[3], const uint32_t whiteColor);
	// Selects the effect. Any task.
	void SetEffect(const effect_type e);
	// Effect parameters; the picture changes only if that effect is the one shown. Any task.
	void SetEffectParam(const snake_params p);
	void SetEffectParam(const growing_params p);
	void SetEffectParam(const fading_params p);
	void SetEffectParam(const random_led_params p);
	void SetEffectParam(const uint32_t p);

	// Stores the current minute of the day (0..1439) for the night light, read by the loop task from the time service's cached
	// time; minuteOfDay is valid only when timeKnown is true. The value reaches BuildJson() through an atomic. A read that overlapped
	// a sync can be a wrong minute for that one pass (harmless, the next pass corrects it). Cheap enough to call on every pass.
	// loop task only.
	void SetMinuteOfDay(bool timeKnown, uint16_t minuteOfDay);

	// Writes the JSON of the current picture into buf and returns its length, 0 when it does not fit (use
	// WEB_DISPLAY_JSON_BUF_LEN bytes). Reads the error kind now, so an error raised outside HAQuDA_DisplayManip is seen while the
	// mode is error: the change is stamped at the first call after it, up to one page poll after the lamp changed. async_tcp task only.
	size_t BuildJson(char *buf, size_t size);

  private:
	// Sets the effect of an error as the lamp runs it. Reads the private effect parameters of HAQuDA_ErrorHandler.
	void fillErrorEffect(error_type kind, web_display_state &state) const;

	portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
	web_picture_state picture;

	// Minute of the day (0..1439) stored by SetMinuteOfDay(), or MINUTE_UNKNOWN. Written by the loop task, read by the async_tcp task.
	static constexpr uint16_t MINUTE_UNKNOWN = 0xFFFF;
	std::atomic<uint16_t> minuteNow{MINUTE_UNKNOWN};
};
