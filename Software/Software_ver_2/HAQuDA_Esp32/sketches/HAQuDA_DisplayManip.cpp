#include "HAQuDA_DisplayManip.h"
#include "HAQuDA_DisplayService.h"
#include "HAQuDA_LEDsParams.h"

display_modes HAQuDA_DisplayManip::mode;

static HAQuDA_MeasHandler *measHandler;
static HAQuDA_EffectsHandler *effectsHandler;
static HAQuDA_ErrorHandler *errHandler;
static HAQuDA_DisplayService *display;

static uint32_t staticColorParam = COLOR_GREEN;

// Shown until the measurements have a sample. Only the access point boot sets the fading effect, so that effect marks AP mode
static display_item waitingItem = HAQuDA_ItemOf(display_event::bootSnake);

HAQuDA_DisplayManip::HAQuDA_DisplayManip(HAQuDA_MeasHandler *_meas_handler, HAQuDA_EffectsHandler *_effects_handler, HAQuDA_ErrorHandler *_err_handler,
										 HAQuDA_DisplayService *_display) {
	mode = noneMode;

	measHandler = _meas_handler;
	effectsHandler = _effects_handler;
	errHandler = _err_handler;
	display = _display;
}

void HAQuDA_DisplayManip::SetBrightness(const uint8_t new_brightness) {
	display->SetBrightness(new_brightness);
}

void HAQuDA_DisplayManip::selectLampColor(const uint32_t rgb) {
	effectsHandler->SetStaticColorParam(rgb);
	effectsHandler->SetEffect(staticColor);
	if (mode != effect)
		SetMode(effect);
	display->SetMeasEnabled(false);
}

void HAQuDA_DisplayManip::ShowLampLight(const uint32_t rgb) {
	selectLampColor(rgb);
	display->SetLampLight(rgb);
}

void HAQuDA_DisplayManip::ShowLampOff() {
	selectLampColor(COLOR_OFF);
	display->SetLampOff();
}

bool HAQuDA_DisplayManip::SetMode(const display_modes m) {

	if ((mode == error) && (errHandler->GetCurrentError() != noneError))
		return false;

	mode = m;

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

	measHandler->SetMeasDispMode(m);

	if (mode != meas)
		SetMode(meas);

	const anim_id measAnim = measHandler->CurrentAnim();
	display->SetBackground(measAnim, waitingItem);
	display->SetMeasEnabled(measAnim != anim_id::none);
}

void HAQuDA_DisplayManip::SetStandardMeasParam(const meas_type p) {
	measHandler->SetStandardMeasParam(p);
}
void HAQuDA_DisplayManip::SetMultiMeasParam(const multi_mode_params p) {
	measHandler->SetMultiMeasParam(p);
}

void HAQuDA_DisplayManip::SetNightTime(const night_time t) {
	measHandler->SetNightTime(t);
}

void HAQuDA_DisplayManip::SetMeasDivDots(const meas_div_dots d) {
	measHandler->SetMeasDivDots(d);
}

/***************************************************************/

void HAQuDA_DisplayManip::SetEffect(const effect_type e) {
	effectsHandler->SetEffect(e);

	if (mode != effect)
		SetMode(effect);

	switch (e) {
		case snake:
			display->Post(display_event::wifiReconnecting);
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
}

void HAQuDA_DisplayManip::SetEffectParam(const growing_params p) {
	effectsHandler->SetGrowParam(p);
}

void HAQuDA_DisplayManip::SetEffectParam(const fading_params p) {
	effectsHandler->SetFadeParam(p);
}

void HAQuDA_DisplayManip::SetEffectParam(const random_led_params p) {
	effectsHandler->SetRandomLedParam(p);
}

void HAQuDA_DisplayManip::SetEffectParam(const uint32_t p) {
	staticColorParam = p;
	effectsHandler->SetStaticColorParam(p);
}

/***************************************************************/

HAQuDA_DisplayManip::~HAQuDA_DisplayManip() {
}
