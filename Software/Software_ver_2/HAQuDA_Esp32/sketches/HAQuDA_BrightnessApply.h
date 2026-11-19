#pragma once

// What ShowTask does with the brightness requests it has taken.
struct brightness_apply {
	bool clear;
	bool setBright;
};

// Clear the matrix first only when the meas mode restores the brightness (the LEDs still hold the last error frame); set the level
// for a restore or a pending change. restoreRequested must already be qualified by the meas mode. Any task.
constexpr brightness_apply HAQuDA_DecideBrightnessApply(bool pending, bool restoreRequested) {
	return {restoreRequested, restoreRequested || pending};
}
