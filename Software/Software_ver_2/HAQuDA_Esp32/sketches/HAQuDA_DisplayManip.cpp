#include "HAQuDA_DisplayManip.h"
#include "HAQuDA_DisplayService.h"
#include "HAQuDA_LEDsParams.h"
#include "HAQuDA_WebPageDisplayManip.h"

display_modes HAQuDA_DisplayManip::mode;

static HAQuDA_MeasHandler *measHandler;
static HAQuDA_EffectsHandler *effectsHandler;
static HAQuDA_ErrorHandler *errHandler;
static HAQuDA_WebPageDisplayManip *webDisp;
static HAQuDA_DisplayService *display;

// The WiFi handler tells its two pictures apart by the static colour it sets before SetEffect(staticColor)
static uint32_t staticColorParam = COLOR_GREEN;

// Shown until the measurements have a sample. Only the access point boot sets the fading effect, so that effect marks AP mode
static display_item waitingItem = HAQuDA_ItemOf(display_event::bootSnake);

HAQuDA_DisplayManip::HAQuDA_DisplayManip(HAQuDA_MeasHandler *_meas_handler, HAQuDA_EffectsHandler *_effects_handler, HAQuDA_ErrorHandler *_err_handler,
										 HAQuDA_WebPageDisplayManip *_web_disp, HAQuDA_DisplayService *_display) {
	mode = noneMode;

	measHandler = _meas_handler;
	effectsHandler = _effects_handler;
	errHandler = _err_handler;
	webDisp = _web_disp;
	display = _display;
}

void HAQuDA_DisplayManip::SetBrightness(const uint8_t new_brightness) {
	display->SetBrightness(new_brightness);
	webDisp->SetBrightness(new_brightness);
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

	const anim_id measAnim = measHandler->CurrentAnim();
	display->SetBackground(measAnim, waitingItem);
	display->SetMeasEnabled(measAnim != anim_id::none);
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

	switch (e) {
		case snake:
			display->Post(display_event::wifiReconnecting);
			display->End(display_end::reconnectStarted);
			break;
		case fading:
			waitingItem = HAQuDA_ItemOf(display_event::apRunning);
			display->Post(display_event::apRunning);
			break;
		case staticColor:
			if (staticColorParam == COLOR_GREEN) {
				display->Post(display_event::wifiJoined);
			} else if (staticColorParam == COLOR_RED) {
				display->Post(display_error::wifiLost);
			}
			break;
		default:
			break;
	}
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
	staticColorParam = p;
	effectsHandler->SetStaticColorParam(p);
	webDisp->SetEffectParam(p);
}

/***************************************************************/

HAQuDA_DisplayManip::~HAQuDA_DisplayManip() {
}
