#pragma once

#include <cstdint>

#include "HAQuDA_DisplayScheduler.h"

struct display_shown {
	// Rises whenever the picture changes; startMs is the engine time of that change
	uint32_t seq;
	uint32_t startMs;
	display_source source;
	display_item item;
	anim_params anim;
	// Seed of the first random pixels pass; each next pass uses xorshift32 of the previous one
	uint32_t seed;
	// Until it is set the matrix still shows the previous frame
	bool measDrawn;
	meas_type keys[MEAS_ANIM_KEYS];
	uint8_t colorCount;
	rgb_color colors[MEAS_ANIM_KEYS];
	rgb_color white;
	uint8_t nightLitCells;
};
