#include "HAQuDA_WebPageDisplayState.h"

namespace {
	constexpr size_t MULTI_KEYS_NUM = 3;

	bool same(const web_snake_params &a, const web_snake_params &b) {
		return (a.color == b.color) && (a.movePauseMs == b.movePauseMs) && (a.tailLen == b.tailLen);
	}

	bool same(const web_growing_params &a, const web_growing_params &b) {
		return (a.color == b.color) && (a.growPauseMs == b.growPauseMs);
	}

	bool same(const web_random_params &a, const web_random_params &b) {
		return (a.pauseMs == b.pauseMs) && (a.endPauseMs == b.endPauseMs);
	}

	bool same(const web_fading_params &a, const web_fading_params &b) {
		return (a.color == b.color) && (a.fadePauseMs == b.fadePauseMs) && (a.startBright == b.startBright) && (a.stopBright == b.stopBright)
			   && (a.step == b.step);
	}

	// A missing key equals another missing key whatever its stored value is.
	bool same(const web_key &a, const web_key &b) {
		return (a.valid == b.valid) && (!a.valid || (a.key == b.key));
	}

	void markChanged(web_picture_state &state, uint32_t nowMs) {
		state.seq++;
		state.startMs = nowMs;
	}

	bool isMeas(const web_picture_state &state) {
		return state.source == web_source::meas;
	}

	// Stores `params` into `field`; true when it differs and the picture draws it now.
	template <typename T> bool storeEffectParam(web_picture_state &state, T &field, const T &params, web_effect kind, uint32_t nowMs) {
		const bool changed = !same(field, params);
		field = params;
		if (changed && (state.source == web_source::effect) && (state.effect == kind)) {
			markChanged(state, nowMs);
			return true;
		}
		return false;
	}
} // namespace

void HAQuDA_WebPictureInit(web_picture_state &state, uint8_t brightPercent) {
	state = web_picture_state{};
	// The starting values of HAQuDA_EffectsHandler, HAQuDA_MeasHandler and HAQuDA_DisplayManip, whose members are private there
	state.source = web_source::none;
	state.seenError = web_error::none;
	state.effect = web_effect::none;
	state.snake = {0x99FF33, 200, 5};
	state.growing = {0x00FFFF, 200};
	state.randomLed = {200, 1000};
	state.fading = {0x00FFFF, 10, 100, 0, 5};
	state.staticColor = 0x01FF00;
	state.layout = web_meas_layout::none;
	state.standardKey = {web_meas_key::total, false};
	state.multiKeys[0] = {web_meas_key::temp, true};
	state.multiKeys[1] = {web_meas_key::eco2, true};
	state.multiKeys[2] = {web_meas_key::pm25, true};
	state.nightStart = 21;
	state.nightFinish = 9;
	state.brightPercent = brightPercent;
}

bool HAQuDA_WebPictureSetSource(web_picture_state &state, web_source source, web_error currentError, uint32_t nowMs) {
	if (state.source == source) {
		return false;
	}
	state.source = source;
	state.measDrawn = false;
	if (source == web_source::error) {
		state.seenError = currentError;
	}
	// The display manipulator stops the effect in every mode but effect
	if (source != web_source::effect) {
		state.effect = web_effect::none;
	}
	markChanged(state, nowMs);
	return true;
}

void HAQuDA_WebPictureSetBrightness(web_picture_state &state, uint8_t brightPercent) {
	state.brightPercent = brightPercent;
}

bool HAQuDA_WebPictureSetEffect(web_picture_state &state, web_effect effect, uint32_t nowMs) {
	if (state.effect == effect) {
		return false;
	}
	state.effect = effect;
	if (state.source == web_source::effect) {
		markChanged(state, nowMs);
		return true;
	}
	return false;
}

bool HAQuDA_WebPictureSetEffectParam(web_picture_state &state, const web_snake_params &params, uint32_t nowMs) {
	return storeEffectParam(state, state.snake, params, web_effect::snake, nowMs);
}

bool HAQuDA_WebPictureSetEffectParam(web_picture_state &state, const web_growing_params &params, uint32_t nowMs) {
	return storeEffectParam(state, state.growing, params, web_effect::growing, nowMs);
}

bool HAQuDA_WebPictureSetEffectParam(web_picture_state &state, const web_random_params &params, uint32_t nowMs) {
	return storeEffectParam(state, state.randomLed, params, web_effect::random_led, nowMs);
}

bool HAQuDA_WebPictureSetEffectParam(web_picture_state &state, const web_fading_params &params, uint32_t nowMs) {
	return storeEffectParam(state, state.fading, params, web_effect::fading, nowMs);
}

bool HAQuDA_WebPictureSetStaticColor(web_picture_state &state, uint32_t color, uint32_t nowMs) {
	const bool changed = (state.staticColor != color);
	state.staticColor = color;
	if (changed && (state.source == web_source::effect) && (state.effect == web_effect::static_color)) {
		markChanged(state, nowMs);
		return true;
	}
	return false;
}

bool HAQuDA_WebPictureSetLayout(web_picture_state &state, web_meas_layout layout, uint32_t nowMs) {
	if ((layout == web_meas_layout::none) || (state.layout == layout)) {
		return false;
	}
	state.layout = layout;
	state.measDrawn = false;
	if (isMeas(state)) {
		markChanged(state, nowMs);
		return true;
	}
	return false;
}

bool HAQuDA_WebPictureSetStandardKey(web_picture_state &state, web_key key, uint32_t nowMs) {
	const bool changed = !same(state.standardKey, key);
	state.standardKey = key;
	if (changed && isMeas(state) && ((state.layout == web_meas_layout::standard) || (state.layout == web_meas_layout::night))) {
		state.measDrawn = false;
		markChanged(state, nowMs);
		return true;
	}
	return false;
}

bool HAQuDA_WebPictureSetMultiKeys(web_picture_state &state, const web_key keys[3], uint32_t nowMs) {
	bool changed = false;
	for (size_t i = 0; i < MULTI_KEYS_NUM; i++) {
		changed = changed || !same(state.multiKeys[i], keys[i]);
		state.multiKeys[i] = keys[i];
	}
	if (changed && isMeas(state) && (state.layout == web_meas_layout::multi)) {
		state.measDrawn = false;
		markChanged(state, nowMs);
		return true;
	}
	return false;
}

bool HAQuDA_WebPictureSetNightWindow(web_picture_state &state, uint8_t start, uint8_t finish, uint32_t nowMs) {
	const bool changed = (state.nightStart != start) || (state.nightFinish != finish);
	state.nightStart = start;
	state.nightFinish = finish;
	if (changed && isMeas(state) && (state.layout == web_meas_layout::night)) {
		markChanged(state, nowMs);
		return true;
	}
	return false;
}

bool HAQuDA_WebPictureSetMeasDrawn(web_picture_state &state, web_meas_layout layout, const uint32_t colors[3], uint32_t whiteColor, uint32_t nowMs) {
	if ((colors == nullptr) || !isMeas(state) || (state.layout != layout)) {
		return false;
	}
	bool changed = !state.measDrawn || (state.measWhiteColor != whiteColor);
	for (size_t i = 0; i < MULTI_KEYS_NUM; i++) {
		changed = changed || (state.measColors[i] != colors[i]);
		state.measColors[i] = colors[i];
	}
	state.measWhiteColor = whiteColor;
	state.measDrawn = true;
	if (changed) {
		markChanged(state, nowMs);
	}
	return changed;
}

bool HAQuDA_WebPictureCheckError(web_picture_state &state, web_error currentError, uint32_t nowMs) {
	if ((state.source != web_source::error) || (state.seenError == currentError)) {
		return false;
	}
	state.seenError = currentError;
	markChanged(state, nowMs);
	return true;
}

void HAQuDA_WebPictureBuild(const web_picture_state &state, web_error currentError, bool hourKnown, uint8_t hour, uint32_t nowMs, web_display_state &out) {
	out = web_display_state{};
	out.seq = state.seq;
	out.ageMs = nowMs - state.startMs;
	out.source = state.source;
	out.brightPercent = state.brightPercent;

	if (state.source == web_source::effect) {
		out.effect = state.effect;
		out.snake = state.snake;
		out.growing = state.growing;
		out.randomLed = state.randomLed;
		out.fading = state.fading;
		out.staticColor = state.staticColor;
	} else if (state.source == web_source::error) {
		out.error = currentError;
	} else if (state.source == web_source::meas) {
		out.layout = state.layout;
		if ((state.layout == web_meas_layout::standard) || (state.layout == web_meas_layout::night)) {
			if (state.standardKey.valid) {
				out.keys[0] = state.standardKey.key;
			} else {
				out.layout = web_meas_layout::none;
			}
		} else if (state.layout == web_meas_layout::multi) {
			for (size_t i = 0; i < MULTI_KEYS_NUM; i++) {
				if (state.multiKeys[i].valid) {
					out.keys[i] = state.multiKeys[i].key;
				} else {
					out.layout = web_meas_layout::none;
				}
			}
		}
		out.nightHour = hourKnown ? HAQuDA_NightHour(state.nightStart, state.nightFinish, hour) : 0;
		out.measDrawn = state.measDrawn;
		for (size_t i = 0; i < MULTI_KEYS_NUM; i++) {
			out.measColors[i] = state.measColors[i];
		}
		out.measWhiteColor = state.measWhiteColor;
	}
}
