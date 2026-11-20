#pragma once

#include <cstddef>

#include "HAQuDA_ColorScale.h"

struct total_reading {
	float average;
	bool hasSample;
	div_dots dots;
};

constexpr size_t TOTAL_READING_NUM = 5;

// Order matters: temp, humid, eCO2, TVOC, PM2.5. False (color untouched) when no reading has a sample.
bool HAQuDA_TotalColor(const total_reading (&readings)[TOTAL_READING_NUM], rgb_color &color);
