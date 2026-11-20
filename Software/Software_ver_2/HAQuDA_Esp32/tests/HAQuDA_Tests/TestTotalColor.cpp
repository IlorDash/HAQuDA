#include <gtest/gtest.h>

#include <cstdint>

#include "HAQuDA_TotalColor.h"

TEST(TotalColorTest, AllInRangeIsChannelWiseAverage) {
	const div_dots dots = {0, 10, 20};

	const rgb_color exp_color = {15, 120, 15};

	const total_reading readings[TOTAL_READING_NUM] = {{5.0f, true, dots}, {10.0f, true, dots}, {15.0f, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}};
	rgb_color color = {1, 2, 3};

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_color.r, color.r);
	EXPECT_EQ(exp_color.g, color.g);
	EXPECT_EQ(exp_color.b, color.b);
}

TEST(TotalColorTest, AverageDropsTheFractionOfEachChannel) {
	const div_dots dots = {0, 10, 20};

	const rgb_color exp_color = {0, 112, 37};

	const total_reading readings[TOTAL_READING_NUM] = {{5.0f, true, dots}, {10.0f, true, dots}, {0.0f, false, dots}, {0.0f, false, dots}, {0.0f, false, dots}};
	rgb_color color = {1, 2, 3};

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_color.r, color.r);
	EXPECT_EQ(exp_color.g, color.g);
	EXPECT_EQ(exp_color.b, color.b);
}

TEST(TotalColorTest, AverageUsesEachReadingsOwnDots) {
	const div_dots narrow_dots = {0, 10, 20};
	const div_dots wide_dots = {0, 100, 200};

	const rgb_color exp_color = {25, 150, 0};

	const total_reading readings[TOTAL_READING_NUM]
		= {{10.0f, true, narrow_dots}, {100.0f, true, wide_dots}, {0.0f, false, narrow_dots}, {0.0f, false, narrow_dots}, {0.0f, false, narrow_dots}};
	rgb_color color = {1, 2, 3};

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_color.r, color.r);
	EXPECT_EQ(exp_color.g, color.g);
	EXPECT_EQ(exp_color.b, color.b);
}

TEST(TotalColorTest, SingleSampleGivesItsOwnColour) {
	const div_dots dots = {0, 10, 20};
	const uint8_t half = COLOR_SCALE_MAX_CHANNEL / 2;

	const rgb_color exp_color = {0, half, half};

	const total_reading readings[TOTAL_READING_NUM] = {{0.0f, false, dots}, {0.0f, false, dots}, {0.0f, false, dots}, {0.0f, false, dots}, {5.0f, true, dots}};
	rgb_color color = {1, 2, 3};

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_color.r, color.r);
	EXPECT_EQ(exp_color.g, color.g);
	EXPECT_EQ(exp_color.b, color.b);
}

TEST(TotalColorTest, ReadingsWithoutSampleAreLeftOutOfTheAverage) {
	const div_dots dots = {0, 10, 20};
	const uint8_t half = COLOR_SCALE_MAX_CHANNEL / 2;

	const rgb_color exp_color = {0, half, half};

	const total_reading readings[TOTAL_READING_NUM]
		= {{5.0f, true, dots}, {10.0f, false, dots}, {10.0f, false, dots}, {10.0f, false, dots}, {10.0f, false, dots}};
	rgb_color color = {1, 2, 3};

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_color.r, color.r);
	EXPECT_EQ(exp_color.g, color.g);
	EXPECT_EQ(exp_color.b, color.b);
}

TEST(TotalColorTest, NoSampleAtAllReturnsFalse) {
	const div_dots dots = {0, 10, 20};

	const total_reading readings[TOTAL_READING_NUM] = {{5.0f, false, dots}, {5.0f, false, dots}, {5.0f, false, dots}, {5.0f, false, dots}, {5.0f, false, dots}};
	rgb_color color = {1, 2, 3};

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_FALSE(res);
}

TEST(TotalColorTest, NoSampleAtAllLeavesTheColourUntouched) {
	const div_dots dots = {0, 10, 20};

	const rgb_color exp_color = {1, 2, 3};

	const total_reading readings[TOTAL_READING_NUM] = {{5.0f, false, dots}, {5.0f, false, dots}, {5.0f, false, dots}, {5.0f, false, dots}, {5.0f, false, dots}};
	rgb_color color = exp_color;

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_EQ(exp_color.r, color.r);
	EXPECT_EQ(exp_color.g, color.g);
	EXPECT_EQ(exp_color.b, color.b);
}

TEST(TotalColorTest, NoSampleLeavesTheColourUntouchedEvenWhenAnAverageIsOutOfRange) {
	const div_dots dots = {0, 10, 20};

	const rgb_color exp_color = {1, 2, 3};

	const total_reading readings[TOTAL_READING_NUM]
		= {{99.0f, false, dots}, {99.0f, false, dots}, {99.0f, false, dots}, {99.0f, false, dots}, {99.0f, false, dots}};
	rgb_color color = exp_color;

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_FALSE(res);
	EXPECT_EQ(exp_color.r, color.r);
	EXPECT_EQ(exp_color.g, color.g);
	EXPECT_EQ(exp_color.b, color.b);
}

TEST(TotalColorTest, TempAtLastDotIsRed) {
	const div_dots dots = {0, 10, 20};
	const float last_dot = 20.0f;

	const rgb_color exp_color = {COLOR_SCALE_MAX_CHANNEL, 0, 0};

	const total_reading readings[TOTAL_READING_NUM]
		= {{last_dot, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}};
	rgb_color color = {1, 2, 3};

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_color.r, color.r);
	EXPECT_EQ(exp_color.g, color.g);
	EXPECT_EQ(exp_color.b, color.b);
}

TEST(TotalColorTest, TempAboveLastDotIsRed) {
	const div_dots dots = {0, 10, 20};
	const float above_last = 25.0f;

	const rgb_color exp_color = {COLOR_SCALE_MAX_CHANNEL, 0, 0};

	const total_reading readings[TOTAL_READING_NUM]
		= {{above_last, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}};
	rgb_color color = {1, 2, 3};

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_color.r, color.r);
	EXPECT_EQ(exp_color.g, color.g);
	EXPECT_EQ(exp_color.b, color.b);
}

TEST(TotalColorTest, TempAtFirstDotIsBlue) {
	const div_dots dots = {0, 10, 20};
	const float first_dot = 0.0f;

	const rgb_color exp_color = {0, 0, COLOR_SCALE_MAX_CHANNEL};

	const total_reading readings[TOTAL_READING_NUM]
		= {{first_dot, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}};
	rgb_color color = {1, 2, 3};

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_color.r, color.r);
	EXPECT_EQ(exp_color.g, color.g);
	EXPECT_EQ(exp_color.b, color.b);
}

TEST(TotalColorTest, TempBelowFirstDotIsBlue) {
	const div_dots dots = {0, 10, 20};
	const float below_first = -5.0f;

	const rgb_color exp_color = {0, 0, COLOR_SCALE_MAX_CHANNEL};

	const total_reading readings[TOTAL_READING_NUM]
		= {{below_first, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}};
	rgb_color color = {1, 2, 3};

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_color.r, color.r);
	EXPECT_EQ(exp_color.g, color.g);
	EXPECT_EQ(exp_color.b, color.b);
}

TEST(TotalColorTest, HumidAtLastDotIsRed) {
	const div_dots dots = {0, 10, 20};
	const float last_dot = 20.0f;

	const rgb_color exp_color = {COLOR_SCALE_MAX_CHANNEL, 0, 0};

	const total_reading readings[TOTAL_READING_NUM]
		= {{10.0f, true, dots}, {last_dot, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}};
	rgb_color color = {1, 2, 3};

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_color.r, color.r);
	EXPECT_EQ(exp_color.g, color.g);
	EXPECT_EQ(exp_color.b, color.b);
}

TEST(TotalColorTest, HumidAtFirstDotIsBlue) {
	const div_dots dots = {0, 10, 20};
	const float first_dot = 0.0f;

	const rgb_color exp_color = {0, 0, COLOR_SCALE_MAX_CHANNEL};

	const total_reading readings[TOTAL_READING_NUM]
		= {{10.0f, true, dots}, {first_dot, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}};
	rgb_color color = {1, 2, 3};

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_color.r, color.r);
	EXPECT_EQ(exp_color.g, color.g);
	EXPECT_EQ(exp_color.b, color.b);
}

TEST(TotalColorTest, Eco2AtLastDotIsRed) {
	const div_dots dots = {0, 10, 20};
	const float last_dot = 20.0f;

	const rgb_color exp_color = {COLOR_SCALE_MAX_CHANNEL, 0, 0};

	const total_reading readings[TOTAL_READING_NUM]
		= {{10.0f, true, dots}, {10.0f, true, dots}, {last_dot, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}};
	rgb_color color = {1, 2, 3};

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_color.r, color.r);
	EXPECT_EQ(exp_color.g, color.g);
	EXPECT_EQ(exp_color.b, color.b);
}

TEST(TotalColorTest, Eco2AtFirstDotIsInRangeAndAveraged) {
	const div_dots dots = {0, 10, 20};
	const float first_dot = 0.0f;

	const rgb_color exp_color = {0, 75, 75};

	const total_reading readings[TOTAL_READING_NUM]
		= {{0.0f, false, dots}, {0.0f, false, dots}, {first_dot, true, dots}, {0.0f, false, dots}, {10.0f, true, dots}};
	rgb_color color = {1, 2, 3};

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_color.r, color.r);
	EXPECT_EQ(exp_color.g, color.g);
	EXPECT_EQ(exp_color.b, color.b);
}

TEST(TotalColorTest, TvocAtLastDotIsRed) {
	const div_dots dots = {0, 10, 20};
	const float last_dot = 20.0f;

	const rgb_color exp_color = {COLOR_SCALE_MAX_CHANNEL, 0, 0};

	const total_reading readings[TOTAL_READING_NUM]
		= {{10.0f, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}, {last_dot, true, dots}, {10.0f, true, dots}};
	rgb_color color = {1, 2, 3};

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_color.r, color.r);
	EXPECT_EQ(exp_color.g, color.g);
	EXPECT_EQ(exp_color.b, color.b);
}

TEST(TotalColorTest, TvocAtFirstDotIsInRangeAndAveraged) {
	const div_dots dots = {0, 10, 20};
	const float first_dot = 0.0f;

	const rgb_color exp_color = {0, 75, 75};

	const total_reading readings[TOTAL_READING_NUM]
		= {{0.0f, false, dots}, {0.0f, false, dots}, {0.0f, false, dots}, {first_dot, true, dots}, {10.0f, true, dots}};
	rgb_color color = {1, 2, 3};

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_color.r, color.r);
	EXPECT_EQ(exp_color.g, color.g);
	EXPECT_EQ(exp_color.b, color.b);
}

TEST(TotalColorTest, Pm25AtLastDotIsRed) {
	const div_dots dots = {0, 10, 20};
	const float last_dot = 20.0f;

	const rgb_color exp_color = {COLOR_SCALE_MAX_CHANNEL, 0, 0};

	const total_reading readings[TOTAL_READING_NUM]
		= {{10.0f, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}, {last_dot, true, dots}};
	rgb_color color = {1, 2, 3};

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_color.r, color.r);
	EXPECT_EQ(exp_color.g, color.g);
	EXPECT_EQ(exp_color.b, color.b);
}

TEST(TotalColorTest, Pm25AtFirstDotIsInRangeAndAveraged) {
	const div_dots dots = {0, 10, 20};
	const float first_dot = 0.0f;

	const rgb_color exp_color = {0, 75, 75};

	const total_reading readings[TOTAL_READING_NUM]
		= {{0.0f, false, dots}, {0.0f, false, dots}, {0.0f, false, dots}, {10.0f, true, dots}, {first_dot, true, dots}};
	rgb_color color = {1, 2, 3};

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_color.r, color.r);
	EXPECT_EQ(exp_color.g, color.g);
	EXPECT_EQ(exp_color.b, color.b);
}

TEST(TotalColorTest, Pm25BelowFirstDotIsInRangeAndAveraged) {
	const div_dots dots = {0, 10, 20};
	const float below_first = -5.0f;

	const rgb_color exp_color = {0, 75, 75};

	const total_reading readings[TOTAL_READING_NUM]
		= {{0.0f, false, dots}, {0.0f, false, dots}, {0.0f, false, dots}, {10.0f, true, dots}, {below_first, true, dots}};
	rgb_color color = {1, 2, 3};

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_color.r, color.r);
	EXPECT_EQ(exp_color.g, color.g);
	EXPECT_EQ(exp_color.b, color.b);
}

TEST(TotalColorTest, TempOutOfRangeWinsOverHumid) {
	const div_dots dots = {0, 10, 20};
	const float above_last = 25.0f;
	const float below_first = -5.0f;

	const rgb_color exp_color = {COLOR_SCALE_MAX_CHANNEL, 0, 0};

	const total_reading readings[TOTAL_READING_NUM]
		= {{above_last, true, dots}, {below_first, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}};
	rgb_color color = {1, 2, 3};

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_color.r, color.r);
	EXPECT_EQ(exp_color.g, color.g);
	EXPECT_EQ(exp_color.b, color.b);
}

TEST(TotalColorTest, HumidOutOfRangeWinsOverEco2) {
	const div_dots dots = {0, 10, 20};
	const float above_last = 25.0f;
	const float below_first = -5.0f;

	const rgb_color exp_color = {0, 0, COLOR_SCALE_MAX_CHANNEL};

	const total_reading readings[TOTAL_READING_NUM]
		= {{10.0f, true, dots}, {below_first, true, dots}, {above_last, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}};
	rgb_color color = {1, 2, 3};

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_color.r, color.r);
	EXPECT_EQ(exp_color.g, color.g);
	EXPECT_EQ(exp_color.b, color.b);
}

TEST(TotalColorTest, OutOfRangeReadingWithoutSampleIsIgnored) {
	const div_dots dots = {0, 10, 20};
	const float above_last = 99.0f;

	const rgb_color exp_color = {0, COLOR_SCALE_MAX_CHANNEL, 0};

	const total_reading readings[TOTAL_READING_NUM]
		= {{above_last, false, dots}, {10.0f, true, dots}, {above_last, false, dots}, {above_last, false, dots}, {above_last, false, dots}};
	rgb_color color = {1, 2, 3};

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_color.r, color.r);
	EXPECT_EQ(exp_color.g, color.g);
	EXPECT_EQ(exp_color.b, color.b);
}

TEST(TotalColorTest, OutOfRangeTempWithoutSampleDoesNotHideALaterOutOfRangeReading) {
	const div_dots dots = {0, 10, 20};
	const float above_last = 99.0f;
	const float below_first = -5.0f;

	const rgb_color exp_color = {0, 0, COLOR_SCALE_MAX_CHANNEL};

	const total_reading readings[TOTAL_READING_NUM]
		= {{above_last, false, dots}, {below_first, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}, {10.0f, true, dots}};
	rgb_color color = {1, 2, 3};

	bool res;

	res = HAQuDA_TotalColor(readings, color);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_color.r, color.r);
	EXPECT_EQ(exp_color.g, color.g);
	EXPECT_EQ(exp_color.b, color.b);
}
