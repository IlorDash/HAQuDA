#include "HAQuDA_ColorScale.h"

#include <cmath>

namespace {
	uint8_t scaleChannel(float distance, float coefficient) {
		return static_cast<uint8_t>(std::round(-std::fabs(distance) * coefficient) + COLOR_SCALE_MAX_CHANNEL);
	}
} // namespace

void HAQuDA_GetMeasRGB(float data, div_dots dots, uint8_t brightness, rgb_color &color) {
	// Both operands are ints, so the slope is truncated before it becomes a float (0 on wide ranges)
	float coefficient = static_cast<float>((brightness * 2) / (dots.last - dots.first));
	if ((data < dots.mid) && (data >= dots.first)) {
		color.b = scaleChannel(data - dots.first, coefficient);
		color.g = scaleChannel(data - dots.mid, coefficient);
		color.r = 0;
	} else if ((data >= dots.mid) && (data <= dots.last)) {
		color.b = 0;
		color.g = scaleChannel(data - dots.mid, coefficient);
		color.r = scaleChannel(data - dots.last, coefficient);
	} else if (data < dots.first) {
		color.b = COLOR_SCALE_MAX_CHANNEL;
		color.g = 0;
		color.r = 0;
	} else if (data > dots.last) {
		color.b = 0;
		color.g = 0;
		color.r = COLOR_SCALE_MAX_CHANNEL;
	}
}
