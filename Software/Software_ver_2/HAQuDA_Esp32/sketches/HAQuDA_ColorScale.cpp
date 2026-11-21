#include "HAQuDA_ColorScale.h"

#include <cmath>

namespace {
	uint8_t scaleChannel(float distance, float coefficient) {
		const float level = std::round(-std::fabs(distance) * coefficient) + COLOR_SCALE_MAX_CHANNEL;
		// With a mid dot off the middle of the range the far end of the shorter half goes below 0, and a negative float cast to
		// uint8_t is undefined (the channel wrapped to a bright colour)
		return static_cast<uint8_t>((level < 0.0f) ? 0.0f : level);
	}
} // namespace

void HAQuDA_GetMeasRGB(float data, div_dots dots, rgb_color &color) {
	// Both operands are ints, so the slope is truncated before it becomes a float (0 on wide ranges)
	float coefficient = static_cast<float>((COLOR_SCALE_MAX_CHANNEL * 2) / (dots.last - dots.first));
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
