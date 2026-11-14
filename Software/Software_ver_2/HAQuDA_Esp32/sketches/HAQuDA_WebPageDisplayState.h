#pragma once

#include <cstddef>
#include <cstdint>

#include "HAQuDA_WebPageDisplayManip_Translator.h"

// What the lamp draws, as the web page needs it, with the bookkeeping of when it last changed: a sequence number that rises at every
// picture change and the time of that change. Hardware-free: the firmware maps its own enums onto the web_* ones, passes the time
// in as nowMs (millis()) and holds the state behind its own lock; every function here is plain arithmetic, no blocking, no heap,
// so it may run inside a critical section.
//
// The Set... functions return true when the picture changed (seq raised, start time set to nowMs) and false when they changed
// nothing visible: a value equal to the stored one, or one the current picture does not draw. A stored value that the picture does
// not draw is still kept, so it is right when the picture switches to it.

// A measurement key that may be missing: the lamp draws nothing for "no parameter" (meas_type noneType).
struct web_key {
	web_meas_key key;
	bool valid;
};

struct web_picture_state {
	uint32_t seq;
	uint32_t startMs;
	web_source source;
	web_error seenError; // error kind when the picture last changed, meaningful for web_source::error only
	web_effect effect;
	web_snake_params snake;
	web_growing_params growing;
	web_random_params randomLed;
	web_fading_params fading;
	uint32_t staticColor;
	web_meas_layout layout;
	web_key standardKey; // standard and night layouts
	web_key multiKeys[3];
	uint8_t nightStart;
	uint8_t nightFinish;
	uint8_t brightPercent;
	// What the lamp last drew for the current measurement picture (see web_display_state); measDrawn is cleared whenever the layout,
	// its keys or the source change, because the LEDs keep the previous frame until the lamp draws the new picture
	bool measDrawn;
	uint32_t measColors[3];
	uint32_t measWhiteColor;
};

// Sets the lamp's starting picture: source none, no effect, no layout, the default effect parameters of the effects handler,
// multi keys temp / eco2 / pm25, night window 21..9, seq 0. Any task.
void HAQuDA_WebPictureInit(web_picture_state &state, uint8_t brightPercent);

// Changes what the lamp draws. Entering any source but effect stops the effect (effect becomes none); entering error remembers
// currentError. No change when the source is the same. Any task.
bool HAQuDA_WebPictureSetSource(web_picture_state &state, web_source source, web_error currentError, uint32_t nowMs);

// Brightness in percent; not part of the picture change, the page repaints it on its next poll. Any task.
void HAQuDA_WebPictureSetBrightness(web_picture_state &state, uint8_t brightPercent);

// Selects the effect; a change counts only while the source is effect. Any task.
bool HAQuDA_WebPictureSetEffect(web_picture_state &state, web_effect effect, uint32_t nowMs);

// Effect parameters; a change counts only while the source is effect and that effect is the selected one. Any task.
bool HAQuDA_WebPictureSetEffectParam(web_picture_state &state, const web_snake_params &params, uint32_t nowMs);
bool HAQuDA_WebPictureSetEffectParam(web_picture_state &state, const web_growing_params &params, uint32_t nowMs);
bool HAQuDA_WebPictureSetEffectParam(web_picture_state &state, const web_random_params &params, uint32_t nowMs);
bool HAQuDA_WebPictureSetEffectParam(web_picture_state &state, const web_fading_params &params, uint32_t nowMs);
bool HAQuDA_WebPictureSetStaticColor(web_picture_state &state, uint32_t color, uint32_t nowMs);

// Selects the measurement layout; web_meas_layout::none is ignored (the lamp's meas handler keeps its mode then). A change counts
// only while the source is meas. Any task.
bool HAQuDA_WebPictureSetLayout(web_picture_state &state, web_meas_layout layout, uint32_t nowMs);

// The parameter of the standard and night layouts. A change counts only while the source is meas and the layout is standard or
// night. Any task.
bool HAQuDA_WebPictureSetStandardKey(web_picture_state &state, web_key key, uint32_t nowMs);

// The three parameters of the multi layout. A change counts only while the source is meas and the layout is multi. Any task.
bool HAQuDA_WebPictureSetMultiKeys(web_picture_state &state, const web_key keys[3], uint32_t nowMs);

// The night window (hours 0..23). A change counts only while the source is meas and the layout is night. Any task.
bool HAQuDA_WebPictureSetNightWindow(web_picture_state &state, uint8_t start, uint8_t finish, uint32_t nowMs);

// The lamp drew a measurement picture of `layout` with these full-scale 0xRRGGBB colours (the caller zeroes the ones the layout
// does not use). Ignored (false, nothing stored) unless the source is meas and `layout` is the current layout: the draw belongs
// to a picture that has been replaced since. Otherwise stored; true (seq raised, start time nowMs) when nothing was drawn for
// the picture before or a colour differs, false for the same colours again. Any task.
bool HAQuDA_WebPictureSetMeasDrawn(web_picture_state &state, web_meas_layout layout, const uint32_t colors[3], uint32_t whiteColor, uint32_t nowMs);

// Notes the error kind the firmware reports now: while the source is error, a kind other than the remembered one is a picture
// change, stamped nowMs, that is the time of the first call after the change, not of the change itself. Any task.
bool HAQuDA_WebPictureCheckError(web_picture_state &state, web_error currentError, uint32_t nowMs);

// Fills `out` for the translator from `state`: seq, age (nowMs - startMs, unsigned, survives the millis() wrap), source, brightness,
// the effect with its parameters (source effect) or the error kind (source error, the caller sets the error's effect from its own
// parameters), the layout with its keys, the night hour and the drawn colours (source meas). A layout with a missing key becomes none. hourKnown false
// gives night hour 0, as the lamp does without a time. Any task.
void HAQuDA_WebPictureBuild(const web_picture_state &state, web_error currentError, bool hourKnown, uint8_t hour, uint32_t nowMs, web_display_state &out);
