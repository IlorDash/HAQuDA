#pragma once

#include <cstdint>

// The CCS811 reports 0 or less than its 400 ppm floor while it warms up; such a read has no eCO2 and no TVOC worth averaging
constexpr uint16_t CCS811_MIN_ECO2_PPM = 400;

inline bool HAQuDA_IsCCS811Sample(uint16_t eCO2Ppm) {
	return eCO2Ppm >= CCS811_MIN_ECO2_PPM;
}
