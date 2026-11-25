#include <gtest/gtest.h>

#include <limits>

#include "HAQuDA_ColorScale.h"

// Slope used by most tests: dots {20, 26, 30} give an exact integer coefficient
// (COLOR_SCALE_MAX_CHANNEL * 2) / (30 - 20) = 300 / 10 = 30, so the integer division in the function does not distort the result.

TEST(ColorScaleTest, GetMeasRGBBelowFirstDotIsBlue) {
	const div_dots dots = {20, 26, 30};

	const rgb_color exp_color = {0, 0, COLOR_SCALE_MAX_CHANNEL};

	const float data = 10.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

TEST(ColorScaleTest, GetMeasRGBAtFirstDotIsFullBlueAndNoGreen) {
	const div_dots dots = {20, 26, 30};

	// blue: distance 0 -> 150; green: distance 6 * coefficient 30 -> 150 - 180 = -30, held at 0
	const rgb_color exp_color = {0, 0, COLOR_SCALE_MAX_CHANNEL};

	const float data = 20.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

TEST(ColorScaleTest, GetMeasRGBBetweenFirstAndMidMixesBlueAndGreen) {
	const div_dots dots = {20, 26, 30};

	// blue: distance 3 -> 150 - 90 = 60; green: distance 3 -> 150 - 90 = 60
	const rgb_color exp_color = {0, 60, 60};

	const float data = 23.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

TEST(ColorScaleTest, GetMeasRGBAtMidDotIsFullGreenAndNoBlue) {
	const div_dots dots = {20, 26, 30};

	// green: distance 0 -> 150; red: distance 4 * coefficient 30 -> 150 - 120 = 30
	const rgb_color exp_color = {30, COLOR_SCALE_MAX_CHANNEL, 0};

	const float data = 26.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

TEST(ColorScaleTest, GetMeasRGBBetweenMidAndLastMixesGreenAndRed) {
	const div_dots dots = {20, 26, 30};

	// green: distance 2 -> 150 - 60 = 90; red: distance 2 -> 150 - 60 = 90
	const rgb_color exp_color = {90, 90, 0};

	const float data = 28.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

TEST(ColorScaleTest, GetMeasRGBAtLastDotIsFullRedAndDimGreen) {
	const div_dots dots = {20, 26, 30};

	// red: distance 0 -> 150; green: distance 4 * coefficient 30 -> 150 - 120 = 30
	const rgb_color exp_color = {COLOR_SCALE_MAX_CHANNEL, 30, 0};

	const float data = 30.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

TEST(ColorScaleTest, GetMeasRGBAboveLastDotIsRed) {
	const div_dots dots = {20, 26, 30};

	const rgb_color exp_color = {COLOR_SCALE_MAX_CHANNEL, 0, 0};

	const float data = 35.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

TEST(ColorScaleTest, GetMeasRGBMidInTheMiddleOfTheRangeAtMidDotIsPureGreen) {
	// mid is halfway: coefficient (150 * 2) / (30 - 20) = 30, red distance 5 * 30 = 150 -> 0
	const div_dots dots = {20, 25, 30};

	const rgb_color exp_color = {0, COLOR_SCALE_MAX_CHANNEL, 0};

	const float data = 25.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

TEST(ColorScaleTest, GetMeasRGBMidInTheMiddleOfTheRangeAtFirstDotReachesZeroWithoutWrap) {
	// coefficient 30, green distance 5 * 30 = 150 -> exactly 0, still no wrap
	const div_dots dots = {20, 25, 30};

	const rgb_color exp_color = {0, 0, COLOR_SCALE_MAX_CHANNEL};

	const float data = 20.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

TEST(ColorScaleTest, GetMeasRGBMidOffCentreHoldsNegativeBlueAtZero) {
	const div_dots dots = {20, 26, 30};

	// blue: distance 5.5 * 30 = 165 -> 150 - 165 = -15, held at 0 (a cast of the negative float wrapped to 241);
	// green: distance 0.5 * 30 = 15 -> 135
	const rgb_color exp_color = {0, 135, 0};

	const float data = 25.5f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

TEST(ColorScaleTest, GetMeasRGBMidOffCentreAtMidDotHoldsNegativeRedAtZero) {
	// Default PM2.5 dots: coefficient 300 / 30 = 10; red: distance 25 * 10 = 250 -> 150 - 250 = -100, held at 0
	const div_dots dots = {15, 20, 45};

	const rgb_color exp_color = {0, COLOR_SCALE_MAX_CHANNEL, 0};

	const float data = 20.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

TEST(ColorScaleTest, GetMeasRGBMidOffCentreHoldsNegativeGreenAtZero) {
	// Default PM2.5 dots: red: distance 5 * 10 = 50 -> 100; green: distance 20 * 10 = 200 -> 150 - 200 = -50, held at 0
	const div_dots dots = {15, 20, 45};

	const rgb_color exp_color = {100, 0, 0};

	const float data = 40.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

TEST(ColorScaleTest, GetMeasRGBJustAboveLastDotIsRed) {
	// 30.5 is above the last dot only by a fraction: a data value truncated to int would land on the dot and give green 30
	const div_dots dots = {20, 26, 30};

	const rgb_color exp_color = {COLOR_SCALE_MAX_CHANNEL, 0, 0};

	const float data = 30.5f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

TEST(ColorScaleTest, GetMeasRGBJustBelowMidDotKeepsAlmostFullGreen) {
	const div_dots dots = {20, 26, 30};

	// green: distance 0.1 * 30 = 3 -> 147; blue: distance 5.9 * 30 = 177 -> negative, held at 0
	const rgb_color exp_color = {0, 147, 0};

	const float data = 25.9f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

TEST(ColorScaleTest, GetMeasRGBMidOffCentreGreenExactlyZeroIsNotClamped) {
	const div_dots dots = {20, 26, 30};

	// green: distance 5 * 30 = 150 -> 150 - 150 = 0 exactly; blue: distance 1 * 30 = 30 -> 120
	const rgb_color exp_color = {0, 0, 120};

	const float data = 21.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

// Reproducer: with a wide range the slope (COLOR_SCALE_MAX_CHANNEL * 2) / (last - first) is truncated to 0 by the
// integer division, so the gradient is flat. Correct slope is 300 / 4600; at 3000 both
// green and red are 150 - round(2000 * 300 / 4600) = 150 - 130 = 20. The function returns 150 for both.
TEST(ColorScaleTest, DISABLED_GetMeasRGBWideRangeKeepsGradient) {
	const div_dots dots = {400, 1000, 5000};

	const rgb_color exp_color = {20, 20, 0};

	const float data = 3000.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

// Reproducer: NaN matches no range, so the output colour keeps whatever it held before (in the
// firmware an uninitialised local, a random lamp colour). A defined colour must be written; lamp
// off (all channels 0) is assumed as the fix.
TEST(ColorScaleTest, DISABLED_GetMeasRGBNaNInputWritesDefinedColor) {
	const div_dots dots = {20, 26, 30};

	const rgb_color exp_color = {0, 0, 0};

	const float data = std::numeric_limits<float>::quiet_NaN();

	rgb_color res = {1, 2, 3};

	HAQuDA_GetMeasRGB(data, dots, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}

// Reproducer: first == last makes (COLOR_SCALE_MAX_CHANNEL * 2) / (last - first) an integer division by zero
// (hardware exception on the ESP32 and on the host). The call must return; with the single dot hit
// exactly, both distances are 0, so green and red are full and blue is 0.
TEST(ColorScaleTest, ToWebColorStretchesTheFullChannelTo255) {
	const uint32_t exp_color = 0xFF00FFu;

	uint32_t res;

	res = HAQuDA_ToWebColor(rgb_color{COLOR_SCALE_MAX_CHANNEL, 0, COLOR_SCALE_MAX_CHANNEL});

	EXPECT_EQ(exp_color, res);
}

TEST(ColorScaleTest, ToWebColorScalesAMidChannelDown) {
	// 105 * 255 / 150 = 178, 75 * 255 / 150 = 127
	const uint32_t exp_color = 0xB27F00u;

	uint32_t res;

	res = HAQuDA_ToWebColor(rgb_color{105, 75, 0});

	EXPECT_EQ(exp_color, res);
}

TEST(ColorScaleTest, ToWebColorClampsAChannelAboveTheMaximum) {
	const uint32_t exp_color = 0x00FF00u;

	uint32_t res;

	res = HAQuDA_ToWebColor(rgb_color{0, 255, 0});

	EXPECT_EQ(exp_color, res);
}

TEST(ColorScaleTest, DISABLED_GetMeasRGBEqualFirstAndLastDoesNotDivideByZero) {
	const div_dots dots = {20, 20, 20};

	const rgb_color exp_color = {COLOR_SCALE_MAX_CHANNEL, COLOR_SCALE_MAX_CHANNEL, 0};

	const float data = 20.0f;

	rgb_color res;

	HAQuDA_GetMeasRGB(data, dots, res);

	EXPECT_EQ(exp_color.r, res.r);
	EXPECT_EQ(exp_color.g, res.g);
	EXPECT_EQ(exp_color.b, res.b);
}
