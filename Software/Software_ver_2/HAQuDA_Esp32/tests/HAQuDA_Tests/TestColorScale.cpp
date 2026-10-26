#include <gtest/gtest.h>

#include <limits>

#include "HAQuDA_ColorScale.h"

// Slope used by most tests: dots {20, 26, 30} and brightness 50 give an exact integer coefficient
// (50 * 2) / (30 - 20) = 10, so the integer division in the function does not distort the result.

TEST(ColorScaleTest, GetMeasRGBBelowFirstDotIsBlue) {
	const div_dots dots = {20, 26, 30};
	const uint8_t brightness = 50;

	const rgb_color exp_color = {0, 0, COLOR_SCALE_MAX_CHANNEL};

	const float data = 10.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, brightness, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

TEST(ColorScaleTest, GetMeasRGBAtFirstDotIsFullBlueAndDimGreen) {
	const div_dots dots = {20, 26, 30};
	const uint8_t brightness = 50;

	// blue: distance 0 -> 150; green: distance 6 * coefficient 10 -> 150 - 60 = 90
	const rgb_color exp_color = {0, 90, COLOR_SCALE_MAX_CHANNEL};

	const float data = 20.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, brightness, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

TEST(ColorScaleTest, GetMeasRGBBetweenFirstAndMidMixesBlueAndGreen) {
	const div_dots dots = {20, 26, 30};
	const uint8_t brightness = 50;

	// blue: distance 3 -> 150 - 30 = 120; green: distance 3 -> 150 - 30 = 120
	const rgb_color exp_color = {0, 120, 120};

	const float data = 23.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, brightness, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

TEST(ColorScaleTest, GetMeasRGBAtMidDotIsFullGreenAndNoBlue) {
	const div_dots dots = {20, 26, 30};
	const uint8_t brightness = 50;

	// green: distance 0 -> 150; red: distance 4 * coefficient 10 -> 150 - 40 = 110
	const rgb_color exp_color = {110, COLOR_SCALE_MAX_CHANNEL, 0};

	const float data = 26.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, brightness, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

TEST(ColorScaleTest, GetMeasRGBBetweenMidAndLastMixesGreenAndRed) {
	const div_dots dots = {20, 26, 30};
	const uint8_t brightness = 50;

	// green: distance 2 -> 150 - 20 = 130; red: distance 2 -> 150 - 20 = 130
	const rgb_color exp_color = {130, 130, 0};

	const float data = 28.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, brightness, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

TEST(ColorScaleTest, GetMeasRGBAtLastDotIsFullRedAndDimGreen) {
	const div_dots dots = {20, 26, 30};
	const uint8_t brightness = 50;

	// red: distance 0 -> 150; green: distance 4 * coefficient 10 -> 150 - 40 = 110
	const rgb_color exp_color = {COLOR_SCALE_MAX_CHANNEL, 110, 0};

	const float data = 30.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, brightness, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

TEST(ColorScaleTest, GetMeasRGBAboveLastDotIsRed) {
	const div_dots dots = {20, 26, 30};
	const uint8_t brightness = 50;

	const rgb_color exp_color = {COLOR_SCALE_MAX_CHANNEL, 0, 0};

	const float data = 35.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, brightness, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

TEST(ColorScaleTest, GetMeasRGBMaxBrightnessAtMidDotIsPureGreen) {
	// mid is halfway: coefficient (150 * 2) / (30 - 20) = 30, red distance 5 * 30 = 150 -> 0
	const div_dots dots = {20, 25, 30};
	const uint8_t brightness = COLOR_SCALE_MAX_CHANNEL;

	const rgb_color exp_color = {0, COLOR_SCALE_MAX_CHANNEL, 0};

	const float data = 25.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, brightness, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

TEST(ColorScaleTest, GetMeasRGBMaxBrightnessAtFirstDotReachesZeroWithoutWrap) {
	// coefficient 30, green distance 5 * 30 = 150 -> exactly 0, still no wrap
	const div_dots dots = {20, 25, 30};
	const uint8_t brightness = COLOR_SCALE_MAX_CHANNEL;

	const rgb_color exp_color = {0, 0, COLOR_SCALE_MAX_CHANNEL};

	const float data = 20.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, brightness, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

// Reproducer: with a wide range the slope (brightness * 2) / (last - first) is truncated to 0 by the
// integer division, so the gradient is flat. Correct slope is 100 / 4600; red at mid:
// 150 - round(4000 * 100 / 4600) = 150 - 87 = 63. The function returns 150.
TEST(ColorScaleTest, DISABLED_GetMeasRGBWideRangeKeepsGradient) {
	const div_dots dots = {400, 1000, 5000};
	const uint8_t brightness = 50;

	const rgb_color exp_color = {63, COLOR_SCALE_MAX_CHANNEL, 0};

	const float data = 1000.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, brightness, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

// Reproducer: at brightness 150 and dots {20, 26, 30} the slope is 30, green at the first dot is
// 150 - 6 * 30 = -30, which wraps in uint8_t to 226 (a bright green instead of dark). The channel
// must stay in 0..150; clamping to 0 is assumed as the fix.
TEST(ColorScaleTest, DISABLED_GetMeasRGBHighBrightnessDoesNotWrapChannel) {
	const div_dots dots = {20, 26, 30};
	const uint8_t brightness = COLOR_SCALE_MAX_CHANNEL;

	const rgb_color exp_color = {0, 0, COLOR_SCALE_MAX_CHANNEL};

	const float data = 20.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, brightness, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

// Reproducer: NaN matches no range, so the output colour keeps whatever it held before (in the
// firmware an uninitialised local, a random lamp colour). A defined colour must be written; lamp
// off (all channels 0) is assumed as the fix.
TEST(ColorScaleTest, DISABLED_GetMeasRGBNaNInputWritesDefinedColor) {
	const div_dots dots = {20, 26, 30};
	const uint8_t brightness = 50;

	const rgb_color exp_color = {0, 0, 0};

	const float data = std::numeric_limits<float>::quiet_NaN();

	rgb_color res = {1, 2, 3};

	HAQuDA_GetMeasRGB(data, dots, brightness, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

// Reproducer: first == last makes (brightness * 2) / (last - first) an integer division by zero
// (hardware exception on the ESP32 and on the host). The call must return; with the single dot hit
// exactly, both distances are 0, so green and red are full and blue is 0.
TEST(ColorScaleTest, DISABLED_GetMeasRGBEqualFirstAndLastDoesNotDivideByZero) {
	const div_dots dots = {20, 20, 20};
	const uint8_t brightness = 50;

	const rgb_color exp_color = {COLOR_SCALE_MAX_CHANNEL, COLOR_SCALE_MAX_CHANNEL, 0};

	const float data = 20.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, brightness, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}
