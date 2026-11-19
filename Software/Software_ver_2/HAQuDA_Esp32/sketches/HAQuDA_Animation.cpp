#include "HAQuDA_Animation.h"

namespace {
	const char *const ANIM_NAMES[ANIM_COUNT] = {"snake", "growing", "random", "fading", "static", "meas-standard", "meas-multi", "meas-night", "meas-total"};
} // namespace

const char *HAQuDA_Animation::Name() const {
	return HAQuDA_AnimName(Id());
}

const char *HAQuDA_AnimName(anim_id id) {
	const uint8_t index = static_cast<uint8_t>(id);
	return (index < ANIM_COUNT) ? ANIM_NAMES[index] : "?";
}

uint16_t HAQuDA_PixelOf(uint8_t x, uint8_t y) {
	if ((x >= ANIM_COLUMNS) || (y >= ANIM_ROWS)) {
		return ANIM_PIXELS;
	}
	const uint8_t rowInChain = (x % 2 != 0) ? (ANIM_ROWS - y - 1) : y;
	return rowInChain + ANIM_ROWS * x;
}

uint32_t HAQuDA_XorShift32(uint32_t &state) {
	state ^= state << 13;
	state ^= state >> 17;
	state ^= state << 5;
	return state;
}
