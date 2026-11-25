#include "HAQuDA_WebPageDisplayState.h"

namespace {
	constexpr size_t MULTI_KEYS_NUM = MEAS_ANIM_KEYS;

	static_assert(sizeof(web_display_state::keys) / sizeof(web_meas_key) == MEAS_ANIM_KEYS, "the page and the engine count the same keys");
	static_assert(sizeof(web_display_state::measColors) / sizeof(uint32_t) == MEAS_ANIM_KEYS, "the page and the engine count the same colours");

	web_error errorWord(display_item item) {
		switch (item) {
			case HAQuDA_ItemOf(display_error::wifiConnectFailed):
				return web_error::wifi;
			case HAQuDA_ItemOf(display_error::apFailed):
				return web_error::ap;
			case HAQuDA_ItemOf(display_error::fsFailed):
				return web_error::fs;
			case HAQuDA_ItemOf(display_error::sensorsFailed):
				return web_error::sensors;
			case HAQuDA_ItemOf(display_error::ntpFailed):
				return web_error::ntp;
			default:
				return web_error::none;
		}
	}

	bool toKey(meas_type type, web_meas_key &key) {
		switch (type) {
			case total:
				key = web_meas_key::total;
				return true;
			case eCO2:
				key = web_meas_key::eco2;
				return true;
			case TVOC:
				key = web_meas_key::tvoc;
				return true;
			case PM2_5:
				key = web_meas_key::pm25;
				return true;
			case temp:
				key = web_meas_key::temp;
				return true;
			case humid:
				key = web_meas_key::humid;
				return true;
			default:
				return false;
		}
	}

	void fillEffect(const anim_params &anim, uint32_t seed, web_display_state &out) {
		switch (anim.anim) {
			case anim_id::snake:
				out.effect = web_effect::snake;
				out.snake = {anim.color, anim.pauseMs, anim.tailLen};
				break;
			case anim_id::growing:
				out.effect = web_effect::growing;
				out.growing = {anim.color, anim.pauseMs};
				break;
			case anim_id::randomPixels:
				out.effect = web_effect::random_led;
				out.randomLed = {anim.pauseMs, anim.endPauseMs};
				out.seed = seed;
				break;
			case anim_id::fading:
				out.effect = web_effect::fading;
				out.fading = {anim.color, anim.pauseMs, anim.fromPercent, anim.toPercent, anim.stepPercent};
				break;
			case anim_id::staticColor:
				out.effect = web_effect::static_color;
				out.staticColor = anim.color;
				break;
			default:
				out.effect = web_effect::none;
				break;
		}
	}

	void fillMeas(const display_shown &shown, web_display_state &out) {
		size_t keysNum = 1;
		switch (shown.anim.anim) {
			case anim_id::measStandard:
				out.layout = web_meas_layout::standard;
				break;
			case anim_id::measTotal:
				out.layout = web_meas_layout::standard;
				out.keys[0] = web_meas_key::total;
				break;
			case anim_id::measNight:
				out.layout = web_meas_layout::night;
				break;
			case anim_id::measMulti:
				out.layout = web_meas_layout::multi;
				keysNum = MULTI_KEYS_NUM;
				break;
			default:
				return;
		}
		if (shown.anim.anim != anim_id::measTotal) {
			for (size_t i = 0; i < keysNum; i++) {
				if (!toKey(shown.keys[i], out.keys[i])) {
					out.layout = web_meas_layout::none;
				}
			}
		}
		out.nightLit = shown.nightLitCells;
		out.measDrawn = shown.measDrawn;
		for (size_t i = 0; (i < MULTI_KEYS_NUM) && (i < shown.colorCount); i++) {
			out.measColors[i] = HAQuDA_ToWebColor(shown.colors[i]);
		}
		if (shown.anim.anim == anim_id::measNight) {
			out.measWhiteColor = HAQuDA_ToWebColor(shown.white);
		}
	}
} // namespace

void HAQuDA_WebPictureBuild(const display_shown &shown, uint8_t brightPercent, uint32_t nowMs, web_display_state &out) {
	out = web_display_state{};
	out.seq = shown.seq;
	out.ageMs = nowMs - shown.startMs;
	out.brightPercent = brightPercent;

	switch (shown.source) {
		case display_source::event:
		case display_source::waiting:
		case display_source::solid:
			out.source = web_source::effect;
			fillEffect(shown.anim, shown.seed, out);
			break;
		case display_source::error:
			out.error = errorWord(shown.item);
			out.source = (out.error == web_error::none) ? web_source::effect : web_source::error;
			fillEffect(shown.anim, shown.seed, out);
			break;
		case display_source::meas:
			out.source = web_source::meas;
			fillMeas(shown, out);
			break;
		default:
			out.source = web_source::none;
			break;
	}
}
