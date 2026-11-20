#pragma once

#include "HAQuDA_DisplayTypes.h"
#include "HAQuDA_LampSettings.h"

// An unknown value maps to total
meas_type HAQuDA_ToMeasType(lamp_param param);

// The test HAQuDA_DisplayManip::SetMode() uses to refuse a change: the matrix shows an error and one is current
bool HAQuDA_ErrorShown(display_modes mode, error_type currentError);
