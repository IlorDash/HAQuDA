#include "HAQuDA_WebPageDisplayManip.h"
#include "HAQuDA_LEDsParams.h"
#include "HAQuDA_TimeHelper.h"

// The mapping below lists every enumerator of these firmware enums, the last one being the "none" sentinel. A new enumerator
// shifts the sentinel and stops the build here, so the switches are revisited; a value outside the list maps to what draws nothing.
static_assert(noneMode == 4, "display_modes changed: update toSource()");
static_assert(noneEffect == 5, "effect_type changed: update toEffect()");
static_assert(noneError == 5, "error_type changed: update toError()");
static_assert(noneMeas == 3, "meas_mode changed: update toLayout()");
static_assert(noneType == 6, "meas_type changed: update toKey()");
// The pure state starts with these colours, copied from the effects handler constructor
static_assert(COLOR_LIME == 0x99FF33, "default snake colour changed: update HAQuDA_WebPictureInit()");
static_assert(COLOR_AQUA == 0x00FFFF, "default effect colour changed: update HAQuDA_WebPictureInit()");
static_assert(COLOR_GREEN == 0x01FF00, "default static colour changed: update HAQuDA_WebPictureInit()");

namespace {
	// Holds the critical section for its lifetime
	class CriticalLock {
	  public:
		explicit CriticalLock(portMUX_TYPE &muxToLock) : mux(muxToLock) {
			portENTER_CRITICAL(&mux);
		}
		~CriticalLock() {
			portEXIT_CRITICAL(&mux);
		}
		CriticalLock(const CriticalLock &) = delete;
		CriticalLock &operator=(const CriticalLock &) = delete;

	  private:
		portMUX_TYPE &mux;
	};

	// Anything the lamp does not draw maps to none: the page then holds its last picture.
	web_source toSource(display_modes m) {
		switch (m) {
			case effect:
				return web_source::effect;
			case meas:
				return web_source::meas;
			case error:
				return web_source::error;
			case event:
				return web_source::event;
			case noneMode:
			default:
				return web_source::none;
		}
	}

	web_effect toEffect(effect_type e) {
		switch (e) {
			case snake:
				return web_effect::snake;
			case growing:
				return web_effect::growing;
			case randomLED:
				return web_effect::random_led;
			case fading:
				return web_effect::fading;
			case staticColor:
				return web_effect::static_color;
			case noneEffect:
			default:
				return web_effect::none;
		}
	}

	web_error toError(error_type e) {
		switch (e) {
			case failedToConnectToWiFi:
				return web_error::wifi;
			case failedToCreateAP:
				return web_error::ap;
			case failedToStartFS:
				return web_error::fs;
			case failedToStartSensors:
				return web_error::sensors;
			case failedToUpdateNTP:
				return web_error::ntp;
			case noneError:
			default:
				return web_error::none;
		}
	}

	web_meas_layout toLayout(meas_mode m) {
		switch (m) {
			case standard:
				return web_meas_layout::standard;
			case multi:
				return web_meas_layout::multi;
			case night:
				return web_meas_layout::night;
			case noneMeas:
			default:
				return web_meas_layout::none;
		}
	}

	// valid is false for noneType and for a value outside the enum: the lamp shows nothing for them.
	web_key toKey(meas_type t) {
		switch (t) {
			case total:
				return {web_meas_key::total, true};
			case eCO2:
				return {web_meas_key::eco2, true};
			case TVOC:
				return {web_meas_key::tvoc, true};
			case PM2_5:
				return {web_meas_key::pm25, true};
			case temp:
				return {web_meas_key::temp, true};
			case humid:
				return {web_meas_key::humid, true};
			case noneType:
			default:
				return {web_meas_key::total, false};
		}
	}

	web_snake_params toWeb(const snake_params &p) {
		return {p.color, p.move_pause_ms, p.tail_len};
	}

	web_growing_params toWeb(const growing_params &p) {
		return {p.color, p.grow_pause_ms};
	}

	web_random_params toWeb(const random_led_params &p) {
		return {p.pause_ms, p.end_pause_ms};
	}

	web_fading_params toWeb(const fading_params &p) {
		return {p.color, p.fade_pause_ms, p.start_bright, p.stop_bright, p.step};
	}
} // namespace

HAQuDA_WebPageDisplayManip::HAQuDA_WebPageDisplayManip() {
	HAQuDA_WebPictureInit(picture, DEFAULT_BRIGHTNESS);
}

void HAQuDA_WebPageDisplayManip::SetMode(const display_modes m) {
	CriticalLock lock(mux);
	HAQuDA_WebPictureSetSource(picture, toSource(m), toError(HAQuDA_ErrorHandler::GetCurrentError()), millis());
}

void HAQuDA_WebPageDisplayManip::SetBrightness(const uint8_t brightPerCent) {
	CriticalLock lock(mux);
	HAQuDA_WebPictureSetBrightness(picture, brightPerCent);
}

void HAQuDA_WebPageDisplayManip::SetMeasMode(const meas_mode m) {
	CriticalLock lock(mux);
	HAQuDA_WebPictureSetLayout(picture, toLayout(m), millis());
}

void HAQuDA_WebPageDisplayManip::SetStandardMeasParam(const meas_type p) {
	CriticalLock lock(mux);
	HAQuDA_WebPictureSetStandardKey(picture, toKey(p), millis());
}

void HAQuDA_WebPageDisplayManip::SetMultiMeasParam(const multi_mode_params p) {
	web_key keys[MULTI_MODE_PARAM_NUM];
	for (size_t i = 0; i < MULTI_MODE_PARAM_NUM; i++) {
		keys[i] = toKey(p.paramsArr[i]);
	}
	CriticalLock lock(mux);
	HAQuDA_WebPictureSetMultiKeys(picture, keys, millis());
}

void HAQuDA_WebPageDisplayManip::SetNightTime(const night_time t) {
	CriticalLock lock(mux);
	HAQuDA_WebPictureSetNightWindow(picture, t.start, t.finish, millis());
}

void HAQuDA_WebPageDisplayManip::SetMeasDrawn(const meas_mode layout, const uint32_t colors[3], const uint32_t whiteColor) {
	CriticalLock lock(mux);
	HAQuDA_WebPictureSetMeasDrawn(picture, toLayout(layout), colors, whiteColor, millis());
}

void HAQuDA_WebPageDisplayManip::SetEffect(const effect_type e) {
	CriticalLock lock(mux);
	HAQuDA_WebPictureSetEffect(picture, toEffect(e), millis());
}

void HAQuDA_WebPageDisplayManip::SetEffectParam(const snake_params p) {
	CriticalLock lock(mux);
	HAQuDA_WebPictureSetEffectParam(picture, toWeb(p), millis());
}

void HAQuDA_WebPageDisplayManip::SetEffectParam(const growing_params p) {
	CriticalLock lock(mux);
	HAQuDA_WebPictureSetEffectParam(picture, toWeb(p), millis());
}

void HAQuDA_WebPageDisplayManip::SetEffectParam(const fading_params p) {
	CriticalLock lock(mux);
	HAQuDA_WebPictureSetEffectParam(picture, toWeb(p), millis());
}

void HAQuDA_WebPageDisplayManip::SetEffectParam(const random_led_params p) {
	CriticalLock lock(mux);
	HAQuDA_WebPictureSetEffectParam(picture, toWeb(p), millis());
}

void HAQuDA_WebPageDisplayManip::SetEffectParam(const uint32_t p) {
	CriticalLock lock(mux);
	HAQuDA_WebPictureSetStaticColor(picture, p, millis());
}

void HAQuDA_WebPageDisplayManip::RefreshHour() {
	uint8_t hour = 0;
	hourNow.store(HAQuDA_TimeService::GetCachedHour(&hour) ? hour : HOUR_UNKNOWN);
}

void HAQuDA_WebPageDisplayManip::fillErrorEffect(error_type kind, web_display_state &state) const {
	switch (kind) {
		case failedToCreateAP:
			state.effect = web_effect::snake;
			state.snake = toWeb(HAQuDA_ErrorHandler::failedToCreateAPEffect);
			break;
		case failedToConnectToWiFi:
			state.effect = web_effect::growing;
			state.growing = toWeb(HAQuDA_ErrorHandler::failedToConnectToWiFiEffect);
			break;
		case failedToStartSensors:
			state.effect = web_effect::random_led;
			state.randomLed = toWeb(HAQuDA_ErrorHandler::failedToStartSensorsEffect);
			break;
		case failedToStartFS:
			state.effect = web_effect::fading;
			state.fading = toWeb(HAQuDA_ErrorHandler::failedToStartFSEffect);
			break;
		case failedToUpdateNTP:
			state.effect = web_effect::fading;
			state.fading = toWeb(HAQuDA_ErrorHandler::failedToUpdateNTPEffect);
			break;
		case noneError:
		default:
			state.effect = web_effect::none;
			break;
	}
}

size_t HAQuDA_WebPageDisplayManip::BuildJson(char *buf, size_t size) {
	web_picture_state copy;
	error_type currentError = noneError;
	uint32_t nowMs = 0;
	{
		CriticalLock lock(mux);
		// Plain read of a one-byte enum the loop task writes, as HAQuDA_DisplayManip::SetMode() does
		currentError = HAQuDA_ErrorHandler::GetCurrentError();
		nowMs = millis();
		HAQuDA_WebPictureCheckError(picture, toError(currentError), nowMs);
		copy = picture;
	}

	const uint8_t hour = hourNow.load();
	web_display_state state;
	HAQuDA_WebPictureBuild(copy, toError(currentError), hour != HOUR_UNKNOWN, hour, nowMs, state);
	if (state.source == web_source::error) {
		fillErrorEffect(currentError, state);
	}
	return HAQuDA_TranslateDisplayToJson(state, buf, size);
}
