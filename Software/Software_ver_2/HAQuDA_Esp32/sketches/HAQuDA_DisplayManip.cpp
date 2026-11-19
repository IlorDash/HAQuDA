#include <atomic>

#include "HAQuDA_DisplayManip.h"
#include "HAQuDA_BrightnessApply.h"
#include "HAQuDA_LEDsParams.h"
#include "HAQuDA_LEDs.h"
#include "HAQuDA_WebPageDisplayManip.h"

std::atomic<uint8_t> HAQuDA_DisplayManip::brighntessPerCent;
display_modes HAQuDA_DisplayManip::mode;

static HAQuDA_MeasHandler *measHandler;
static HAQuDA_EffectsHandler *effectsHandler;
static HAQuDA_ErrorHandler *errHandler;
static HAQuDA_WebPageDisplayManip *webDisp;

// Both are set in the loop task and consumed in ShowTask, which is the only task that may send an LED frame.
// brightnessPending: brighntessPerCent has not reached the LEDs yet.
// restoreBrightnessPending: the matrix must also be cleared when the meas mode starts.
static std::atomic<bool> brightnessPending{false};
static std::atomic<bool> restoreBrightnessPending{false};

// Puts a requested brightness on the LEDs. Runs at the top of every ShowTask pass, so an effect that restores its own saved
// brightness when it stops has already returned and cannot overwrite the user's value. This is the only thing that corrects a level
// restored by show_fading(): every path that stops an effect must be followed by SetBrightness() or RestoreBrightnessOnMeas().
void HAQuDA_DisplayManip::applyPendingBrightness() {
	const display_modes currentMode = mode;
	// The restore waits for the meas mode: the error effect has returned by then, so its own brightness restore cannot overwrite this
	// one. Clear first: the LEDs still hold the last error frame, and the first meas draw can be up to a minute away.
	const bool restore = (currentMode == meas) && restoreBrightnessPending.exchange(false);
	// The percent is read after the flag: the setter writes the value first, so a flag seen here comes with its value
	const bool pending = brightnessPending.exchange(false);
	const brightness_apply apply = HAQuDA_DecideBrightnessApply(pending, restore);
	if (apply.clear) {
		HAQuDA_LEDs::getInstance()->Clear();
	}
	if (apply.setBright) {
		HAQuDA_LEDs::getInstance()->SetBright(brighntessPerCent.load());
	}
}

HAQuDA_DisplayManip::HAQuDA_DisplayManip(HAQuDA_MeasHandler *_meas_handler, HAQuDA_EffectsHandler *_effects_handler, HAQuDA_ErrorHandler *_err_handler,
										 HAQuDA_WebPageDisplayManip *_web_disp) {
	brighntessPerCent = DEFAULT_BRIGHTNESS;
	mode = noneMode;

	measHandler = _meas_handler;
	effectsHandler = _effects_handler;
	errHandler = _err_handler;
	webDisp = _web_disp;
}

void HAQuDA_DisplayManip::Show(int *show_meas_time) {

	applyPendingBrightness();

	switch (mode) {
		case effect:
			effectsHandler->Show();
			break;
		case meas:
			measHandler->Show(show_meas_time);
			break;
		case error:
			errHandler->Show();
			break;
		default:
			break;
	}
	vTaskDelay(100 / portTICK_PERIOD_MS);
}

void HAQuDA_DisplayManip::SetBrightness(const uint8_t new_brightness) {
	brighntessPerCent.store(new_brightness);
	// After the value: ShowTask reads the value once it sees the flag
	brightnessPending.store(true);
	// The page and the status report the user's value at once, not when ShowTask gets to it
	webDisp->SetBrightness(new_brightness);
}

void HAQuDA_DisplayManip::RestoreBrightnessOnMeas() {
	restoreBrightnessPending = true;
}

bool HAQuDA_DisplayManip::SetMode(const display_modes m) {

	if ((mode == error) && (errHandler->GetCurrentError() != noneError))
		return false;

	mode = m;
	// The mirror learns the new source before ShowTask can draw in it: a meas draw reported to the mirror while it still has the
	// old source is dropped, and the page would hold until the next draw
	webDisp->SetMode(mode);

	switch (mode) {
		case noneMode: {
			effectsHandler->SetEffect(noneEffect);
			measHandler->SetMeasDispMode(noneMeas);
			break;
		}
		case error: {
			effectsHandler->SetEffect(noneEffect);
			measHandler->SetMeasDispMode(noneMeas);
			break;
		}
		case meas: {
			effectsHandler->SetEffect(noneEffect);
			break;
		}
		case effect: {
			measHandler->SetMeasDispMode(noneMeas);
			break;
		}
		case event: {
			effectsHandler->SetEffect(noneEffect);
			measHandler->SetMeasDispMode(noneMeas);
			break;
		}
		default:
			break;
	}
	return true;
}

/***************************************************************/

void HAQuDA_DisplayManip::SetMeasMode(const meas_mode m) {

	// The mirror gets the layout before the handler can draw it (see SetMode())
	webDisp->SetMeasMode(m);
	measHandler->SetMeasDispMode(m);

	if (mode != meas)
		SetMode(meas);
}

meas_mode HAQuDA_DisplayManip::GetMeasDispMode() {
	return measHandler->GetMeasDispMode();
};

void HAQuDA_DisplayManip::SetStandardMeasParam(const meas_type p) {
	// Mirror first: a draw that lands in between reports colours of the old key, and those are what the LEDs show
	webDisp->SetStandardMeasParam(p);
	measHandler->SetStandardMeasParam(p);
}
void HAQuDA_DisplayManip::SetMultiMeasParam(const multi_mode_params p) {
	webDisp->SetMultiMeasParam(p);
	measHandler->SetMultiMeasParam(p);
}

void HAQuDA_DisplayManip::SetNightTime(const night_time t) {
	measHandler->SetNightTime(t);
	webDisp->SetNightTime(t);
}

void HAQuDA_DisplayManip::SetMeasDivDots(const meas_div_dots d) {
	measHandler->SetMeasDivDots(d);
}

/***************************************************************/

void HAQuDA_DisplayManip::SetEffect(const effect_type e) {
	effectsHandler->SetEffect(e);

	if (mode != effect)
		SetMode(effect);
	webDisp->SetEffect(e);
}

effect_type HAQuDA_DisplayManip::GetEffect() {
	return effectsHandler->GetEffect();
};

void HAQuDA_DisplayManip::SetEffectParam(const snake_params p) {
	effectsHandler->SetSnakeParam(p);
	webDisp->SetEffectParam(p);
}

void HAQuDA_DisplayManip::SetEffectParam(const growing_params p) {
	effectsHandler->SetGrowParam(p);
	webDisp->SetEffectParam(p);
}

void HAQuDA_DisplayManip::SetEffectParam(const fading_params p) {
	effectsHandler->SetFadeParam(p);
	webDisp->SetEffectParam(p);
}

void HAQuDA_DisplayManip::SetEffectParam(const random_led_params p) {
	effectsHandler->SetRandomLedParam(p);
	webDisp->SetEffectParam(p);
}

void HAQuDA_DisplayManip::SetEffectParam(const uint32_t p) {
	effectsHandler->SetStaticColorParam(p);
	webDisp->SetEffectParam(p);
}

/***************************************************************/

HAQuDA_DisplayManip::~HAQuDA_DisplayManip() {
}