#pragma once

#include <cstdint>

// Largest value of one colour channel the scale produces. Equals MAX_BRIGHTNESS in HAQuDA_LEDsParams.h.
constexpr uint8_t COLOR_SCALE_MAX_CHANNEL = 150;

struct div_dots {
	uint16_t first;
	uint16_t mid;
	uint16_t last;
};

struct rgb_color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

// Maps a measured value to a colour: blue below dots.first, blue to green up to dots.mid, green to red up to dots.last,
// red above. brightness is the raw LED brightness (0..COLOR_SCALE_MAX_CHANNEL) and sets the colour gradient slope.
// color is left untouched when no range matches (data is NaN). Any task.
void HAQuDA_GetMeasRGB(float data, div_dots dots, uint8_t brightness, rgb_color &color);
