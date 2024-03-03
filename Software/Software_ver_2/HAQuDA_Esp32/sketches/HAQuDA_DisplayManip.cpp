#include "HAQuDA_DisplayManip.h"
#include "HAQuDA_LEDsParams.h"
#include "HAQuDA_LEDs.h"

uint8_t HAQuDA_DisplayManip::brighntessPerCent;
display_modes HAQuDA_DisplayManip::mode;

static HAQuDA_MeasHandler *measHandler;
static HAQuDA_EffectsHandler *effectsHandler;
static HAQuDA_ErrorHandler *errHandler;

HAQuDA_DisplayManip::HAQuDA_DisplayManip(HAQuDA_MeasHandler *_meas_handler, HAQuDA_EffectsHandler *_effects_handler, HAQuDA_ErrorHandler *_err_handler) {
	brighntessPerCent = DEFAULT_BRIGHTNESS;
	mode = noneMode;

	measHandler = _meas_handler;
	effectsHandler = _effects_handler;
	errHandler = _err_handler;
}

void HAQuDA_DisplayManip::Show(int *show_meas_time) {

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
	brighntessPerCent = new_brightness;
	HAQuDA_LEDs::getInstance()->SetBright(brighntessPerCent);
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
}

meas_mode HAQuDA_DisplayManip::GetMeasDispMode() {
	return measHandler->GetMeasDispMode();
};

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
	effectsHandler->SetStaticColorParam(p);
}

/***************************************************************/

HAQuDA_DisplayManip::~HAQuDA_DisplayManip() {
}