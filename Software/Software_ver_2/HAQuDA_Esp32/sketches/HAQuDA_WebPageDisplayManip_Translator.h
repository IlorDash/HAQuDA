#pragma once

#include <cstddef>
#include <cstdint>

// Translates what the lamp draws into the compact JSON the web page uses to mirror the LED picture on its 4 columns x 6 rows grid.
// All durations in the JSON are already converted to page cells and milliseconds for that 4 columns x 6 rows grid (the lamp is
// 9 columns x 12 rows), so the page plays them as they are.
// Hardware-free: the firmware fills web_display_state from its own enums, this module only does arithmetic and text.

// What the lamp is drawing (mirrors display_modes). Mode event and none draw nothing, the LEDs keep their last frame.
enum class web_source : uint8_t { none, effect, meas, error, event };

// Active error (mirrors error_type), meaningful only for web_source::error.
enum class web_error : uint8_t { none, wifi, ap, fs, sensors, ntp };

// Effect kind (mirrors effect_type).
enum class web_effect : uint8_t { none, snake, growing, random_led, fading, static_color };

// Measurement layout (mirrors meas_mode).
enum class web_meas_layout : uint8_t { none, standard, multi, night };

// Measured quantity (mirrors meas_type).
enum class web_meas_key : uint8_t { total, eco2, tvoc, pm25, temp, humid };

// Parameters mirror the firmware effect structs (snake_params, growing_params, random_led_params, fading_params).
struct web_snake_params {
	uint32_t color;
	uint16_t movePauseMs;
	uint8_t tailLen;
};

struct web_growing_params {
	uint32_t color;
	uint16_t growPauseMs;
};

struct web_random_params {
	uint16_t pauseMs;
	uint16_t endPauseMs;
};

struct web_fading_params {
	uint32_t color;
	uint16_t fadePauseMs;
	uint8_t startBright;
	uint8_t stopBright;
	uint8_t step;
};

struct web_display_state {
	uint32_t seq;
	uint32_t ageMs;
	web_source source;
	web_error error;
	web_effect effect;
	web_snake_params snake;
	web_growing_params growing;
	web_random_params randomLed;
	web_fading_params fading;
	uint32_t staticColor;
	web_meas_layout layout;
	web_meas_key keys[3]; // standard and night use keys[0], multi uses all three in paramsArr order
	uint8_t nightHour;	  // get_nightMode_hour(): 0..11 hours into the night window, 12 = outside it
	uint8_t brightPercent;
};

// Writes the JSON of `state` into buf (NUL-terminated) and returns its length. Returns 0 when it does not fit or buf is
// null / size is 0, when source is out of range, or when source is web_source::error and error is out of range (error is
// ignored for every other source); then buf[0] is '\0' (if size > 0) and nothing is written past size. No heap. Any task.
size_t HAQuDA_TranslateDisplayToJson(const web_display_state &state, char *buf, size_t size);
