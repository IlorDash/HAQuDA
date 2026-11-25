#pragma once

#include <cstddef>
#include <cstdint>

// Translates what the lamp draws into the compact JSON the web page uses to mirror the LED picture on its 4 columns x 6 rows grid.
// All durations in the JSON are already converted to page cells and milliseconds for that 4 columns x 6 rows grid (the lamp is
// 9 columns x 12 rows), so the page plays them as they are.
// Timing fields of the effects: "dark" is the time the matrix is dark at the start of every cycle (the lamp clears it first),
// "step" the time of one page step, "end" the extra time the last frame stays at the end of the cycle; a fading effect also has
// "first", the time of its first level (the lamp shows the start level twice). A fading cycle is exactly
// dark + first + (steps - 1) x step + end, and equals the lamp's cycle.
// Hardware-free: the firmware fills web_display_state from what the display engine shows, this module only does arithmetic and text.
// The random effect also carries "seed", the seed of its first pass. Pass n (0 is the pass that starts with the picture) uses the seed
// s(n): s(0) = seed, s(n + 1) = xorshift32(s(n)) with the shifts 13, 17, 5 on 32 bits. From a pass seed the page derives the order and
// the colours as HAQuDA_RandomPixelsAnim documents.

// What the lamp is drawing. none draws nothing, the LEDs keep their last frame. effect is every picture without an error word: the
// start and WiFi pictures, the lamp light and off. error is an error picture with its word.
enum class web_source : uint8_t { none, effect, meas, error };

// Active error, meaningful only for web_source::error.
enum class web_error : uint8_t { none, wifi, ap, fs, sensors, ntp };

enum class web_effect : uint8_t { none, snake, growing, random_led, fading, static_color };

enum class web_meas_layout : uint8_t { none, standard, multi, night };

enum class web_meas_key : uint8_t { total, eco2, tvoc, pm25, temp, humid };

// Parameters of the catalog animations (HAQuDA_Animation.h).
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
	uint32_t seed;
	web_fading_params fading;
	uint32_t staticColor;
	web_meas_layout layout;
	web_meas_key keys[3]; // standard and night use keys[0], multi uses all three in paramsArr order
	uint8_t nightLit;	  // night light cells lit, 0..18 (HAQuDA_NightLitCells(), 0 when the time is unknown)
	uint8_t brightPercent;
	// The colours the lamp drew for the current measurement picture, full-scale 0xRRGGBB (the lamp's 0..150 channels scaled to
	// 0..255): measColors[0] for standard, [0] for night, all three for multi in the order of keys. measWhiteColor is the grey of
	// the night bar. measDrawn false: the lamp has not drawn this picture yet (its LEDs still show the previous frame).
	bool measDrawn;
	uint32_t measColors[3];
	uint32_t measWhiteColor;
};

// Measurement picture ("fx":"meas"): "layout", "keys" (1 for standard and night, 3 for multi), "colors" (the colours the lamp drew,
// "#rrggbb": one per key for multi in the order of keys, one for standard and night), and for night "lit" (cells of the night light
// on, 0..18: the page lights them bottom up in its three right columns, left column first) and "whiteColor" ("#rrggbb" of the
// light). While the lamp has not drawn the picture (measDrawn false) or the layout is none the
// output is "fx":"hold": the LEDs show their previous frame.
//
// Buffer size that holds every JSON the translator writes: the longest one (an error with a fading effect, every number at its
// maximum) is about 200 bytes.
constexpr size_t WEB_DISPLAY_JSON_BUF_LEN = 256;

// Writes the JSON of `state` into buf (NUL-terminated) and returns its length. Returns 0 when it does not fit or buf is
// null / size is 0, when source is out of range, or when source is web_source::error and error is out of range (error is
// ignored for every other source); then buf[0] is '\0' (if size > 0) and nothing is written past size. No heap. Any task.
size_t HAQuDA_TranslateDisplayToJson(const web_display_state &state, char *buf, size_t size);
