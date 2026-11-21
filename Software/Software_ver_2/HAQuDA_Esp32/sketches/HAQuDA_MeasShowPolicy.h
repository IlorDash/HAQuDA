#pragma once

struct meas_show_outcome {
	bool restartPeriod;
	bool keepRequest;
};

// The period restarts even when nothing was drawn: a reading that never gets a sample (a missing sensor) must not let the sums of the
// others grow for hours, their 16-bit counts wrap. The redraw request is kept until a picture is drawn.
inline meas_show_outcome HAQuDA_DecideShowOutcome(bool drawn, bool periodElapsed) {
	return {drawn || periodElapsed, !drawn};
}
