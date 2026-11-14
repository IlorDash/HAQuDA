#include <atomic>

#include "HAQuDA_DisplayManip.h"
#include "HAQuDA_LEDsParams.h"
#include "HAQuDA_LEDs.h"
#include "HAQuDA_WebPageDisplayManip.h"

uint8_t HAQuDA_DisplayManip::brighntessPerCent;
display_modes HAQuDA_DisplayManip::mode;

static HAQuDA_MeasHandler *measHandler;
static HAQuDA_EffectsHandler *effectsHandler;
static HAQuDA_ErrorHandler *errHandler;
static HAQuDA_WebPageDisplayManip *webDisp;

// Set in the loop task, consumed in ShowTask.
static std::atomic<bool> restoreBrightnessPending{false};

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

	switch (mode) {
		case effect:
			effectsHandler->Show();
			break;
		case meas:
			// The error effect has returned by now, so its own brightness restore cannot overwrite this one.
			// Clear first: the LEDs still hold the last error frame, and the first meas draw can be up to a minute away.
			if (restoreBrightnessPending.exchange(false)) {
				HAQuDA_LEDs::getInstance()->Clear();
				HAQuDA_LEDs::getInstance()->SetBright(brighntessPerCent);
			}
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
	brighntessPerCent = new_brightness;
	HAQuDA_LEDs::getInstance()->SetBright(brighntessPerCent);
	webDisp->SetBrightness(brighntessPerCent);
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