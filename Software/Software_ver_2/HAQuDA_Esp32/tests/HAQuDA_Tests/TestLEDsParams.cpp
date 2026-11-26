#include <gtest/gtest.h>

#include <cstdint>

#include "HAQuDA_ColorScale.h"
#include "HAQuDA_LEDsParams.h"

TEST(LEDsParamsTest, DefaultBrightnessIs35Percent) {
	const uint8_t exp_bright = 35;

	const uint8_t res = DEFAULT_BRIGHTNESS;

	EXPECT_EQ(exp_bright, res);
}

TEST(LEDsParamsTest, BrightPercentToRawZeroIsZero) {
	const uint8_t exp_raw = 0;

	uint8_t res;

	res = HAQuDA_BrightPercentToRaw(0);

	EXPECT_EQ(exp_raw, res);
}

TEST(LEDsParamsTest, BrightPercentToRawOnePercentIsOne) {
	// 1 * 150 / 100 = 1.5 -> 1
	const uint8_t exp_raw = 1;

	uint8_t res;

	res = HAQuDA_BrightPercentToRaw(1);

	EXPECT_EQ(exp_raw, res);
}

TEST(LEDsParamsTest, BrightPercentToRawFivePercentRoundsDown) {
	// 5 * 150 / 100 = 7.5 -> 7
	const uint8_t exp_raw = 7;

	uint8_t res;

	res = HAQuDA_BrightPercentToRaw(5);

	EXPECT_EQ(exp_raw, res);
}

TEST(LEDsParamsTest, BrightPercentToRawDefaultBrightnessIs52) {
	// 35 * 150 / 100 = 52.5 -> 52
	const uint8_t exp_raw = 52;

	uint8_t res;

	res = HAQuDA_BrightPercentToRaw(DEFAULT_BRIGHTNESS);

	EXPECT_EQ(exp_raw, res);
}

TEST(LEDsParamsTest, BrightPercentToRawFiftyPercentIs75) {
	const uint8_t exp_raw = 75;

	uint8_t res;

	res = HAQuDA_BrightPercentToRaw(50);

	EXPECT_EQ(exp_raw, res);
}

TEST(LEDsParamsTest, BrightPercentToRawNinetyNinePercentRoundsDown) {
	// 99 * 150 / 100 = 148.5 -> 148, one step below the limit
	const uint8_t exp_raw = 148;

	uint8_t res;

	res = HAQuDA_BrightPercentToRaw(99);

	EXPECT_EQ(exp_raw, res);
}

TEST(LEDsParamsTest, BrightPercentToRawHundredPercentIsTheCurrentLimit) {
	const uint8_t exp_raw = MAX_BRIGHTNESS;

	uint8_t res;

	res = HAQuDA_BrightPercentToRaw(100);

	EXPECT_EQ(exp_raw, res);
}

TEST(LEDsParamsTest, BrightPercentToRawHundredAndOnePercentIsClamped) {
	const uint8_t exp_raw = MAX_BRIGHTNESS;

	uint8_t res;

	res = HAQuDA_BrightPercentToRaw(101);

	EXPECT_EQ(exp_raw, res);
}

TEST(LEDsParamsTest, BrightPercentToRawMaxByteIsClamped) {
	const uint8_t exp_raw = MAX_BRIGHTNESS;

	uint8_t res;

	res = HAQuDA_BrightPercentToRaw(255);

	EXPECT_EQ(exp_raw, res);
}

TEST(LEDsParamsTest, NightGreyIs70PercentOfMaxChannel) {
	// The night animation uses HAQuDA_NightGrey(): 150 * 70 / 100 = 105
	const uint8_t exp_grey = 105;

	uint8_t res;

	res = HAQuDA_NightGrey();

	EXPECT_EQ(exp_grey, res);
}
