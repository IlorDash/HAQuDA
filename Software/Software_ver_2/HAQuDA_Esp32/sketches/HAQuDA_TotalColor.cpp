#include "HAQuDA_TotalColor.h"

#include <cstdint>

namespace {
	constexpr size_t LOWER_LIMIT_READING_NUM = 2;

	bool outOfRange(const total_reading &reading, bool hasLowerLimit) {
		return (reading.average >= reading.dots.last) || (hasLowerLimit && (reading.average <= reading.dots.first));
	}
} // namespace

bool HAQuDA_TotalColor(const total_reading (&readings)[TOTAL_READING_NUM], rgb_color &color) {
	for (size_t i = 0; i < TOTAL_READING_NUM; i++) {
		if (readings[i].hasSample && outOfRange(readings[i], i < LOWER_LIMIT_READING_NUM)) {
			HAQuDA_GetMeasRGB(readings[i].average, readings[i].dots, color);
			return true;
		}
	}

	uint16_t rSum = 0;
	uint16_t gSum = 0;
	uint16_t bSum = 0;
	uint8_t sampled = 0;
	for (const total_reading &reading : readings) {
		if (!reading.hasSample) {
			continue;
		}
		rgb_color readingColor = {0, 0, 0};
		HAQuDA_GetMeasRGB(reading.average, reading.dots, readingColor);
		rSum += readingColor.r;
		gSum += readingColor.g;
		bSum += readingColor.b;
		sampled++;
	}
	if (sampled == 0) {
		return false;
	}
	color = {static_cast<uint8_t>(rSum / sampled), static_cast<uint8_t>(gSum / sampled), static_cast<uint8_t>(bSum / sampled)};
	return true;
}
