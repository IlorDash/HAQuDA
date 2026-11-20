#pragma once

#include "HAQuDA_Animation.h"

// ShowTask only: both calls run in the display task
class HAQuDA_IMeasSource {
  public:
	virtual ~HAQuDA_IMeasSource() = default;

	// false: no measurement animation is selected, or a value it shows has no sample yet
	virtual bool ReadAverages(anim_id &anim, meas_anim_data &data) = 0;
	virtual void RestartAveraging() = 0;
};
