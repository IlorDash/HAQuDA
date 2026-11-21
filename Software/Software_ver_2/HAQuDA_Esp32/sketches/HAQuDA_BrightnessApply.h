#pragma once

struct brightness_apply {
	bool clear;
	bool setBright;
};

// The matrix is cleared first only on a restore: the LEDs still hold the last error frame. restoreRequested must already be
// qualified by the meas mode.
constexpr brightness_apply HAQuDA_DecideBrightnessApply(bool pending, bool restoreRequested) {
	return {restoreRequested, restoreRequested || pending};
}
