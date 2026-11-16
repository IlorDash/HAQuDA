#pragma once

// What a pass of HAQuDA_MeasHandler::Show() does with its bookkeeping once it has tried to draw. Hardware-free.
//
// A pass that could not draw (the shown reading has no sample yet) keeps the pending redraw request, so the next pass tries again.
// The sums and the period timer are still restarted when the period has elapsed: a reading that never gets a sample (a missing
// sensor) must not let the sums of the others grow for hours (the counts are 16 bit and wrap).
struct meas_show_outcome {
	bool restartPeriod; // reset the 60 s timer and clear the sums
	bool keepRequest;	// leave the redraw request (show_meas_immed) set
};

// drawn: the pass drew a picture or never will; periodElapsed: DISP_MEAS_PERIOD_MS has passed since the last restart. Any task.
inline meas_show_outcome HAQuDA_DecideShowOutcome(bool drawn, bool periodElapsed) {
	return {drawn || periodElapsed, !drawn};
}
