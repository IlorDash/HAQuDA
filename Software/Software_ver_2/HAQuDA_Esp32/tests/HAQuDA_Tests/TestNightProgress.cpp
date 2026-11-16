#include <gtest/gtest.h>

#include <cstdint>

#include "HAQuDA_NightProgress.h"

// ---------------------------------------------------------------- NightLitCells

TEST(NightProgressTest, LitCellsWindowStartIsOne) {
	const uint16_t from = 21 * 60;
	const uint16_t to = 9 * 60;
	const uint16_t now = 21 * 60;

	const uint8_t exp_lit = 1;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsFirstMinuteOfSecondCell) {
	// 18 * 1 / 720 = 0.025 rounds up to the first cell, 18 * 41 / 720 = 1.025 to the second
	const uint16_t from = 21 * 60;
	const uint16_t to = 9 * 60;
	const uint16_t now = 21 * 60 + 41;

	const uint8_t exp_lit = 2;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsExactCellBoundaryDoesNotRoundUp) {
	// 2 h of 12 h: 18 * 120 / 720 = 3 exactly
	const uint16_t from = 21 * 60;
	const uint16_t to = 9 * 60;
	const uint16_t now = 23 * 60;

	const uint8_t exp_lit = 3;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsAfterMidnightCountsFromTheStart) {
	// 00:00 is 180 minutes into 21:00..09:00: 18 * 180 / 720 = 4.5
	const uint16_t from = 21 * 60;
	const uint16_t to = 9 * 60;
	const uint16_t now = 0;

	const uint8_t exp_lit = 5;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsMiddleOfWindow) {
	const uint16_t from = 21 * 60;
	const uint16_t to = 9 * 60;
	const uint16_t now = 3 * 60;

	const uint8_t exp_lit = 9;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsLastMinuteOfWindowIsAll) {
	const uint16_t from = 21 * 60;
	const uint16_t to = 9 * 60;
	const uint16_t now = 8 * 60 + 59;

	const uint8_t exp_lit = 18;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsAtTheEndMinuteIsOutsideAndAll) {
	const uint16_t from = 21 * 60;
	const uint16_t to = 9 * 60;
	const uint16_t now = 9 * 60;

	const uint8_t exp_lit = 18;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsOutsideWindowIsAll) {
	const uint16_t from = 21 * 60;
	const uint16_t to = 9 * 60;
	const uint16_t now = 15 * 60;

	const uint8_t exp_lit = 18;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsWindowInOneDay) {
	// 240 of the 300 minutes of 01:00..06:00: 18 * 240 / 300 = 14.4
	const uint16_t from = 1 * 60;
	const uint16_t to = 6 * 60;
	const uint16_t now = 5 * 60;

	const uint8_t exp_lit = 15;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsWindowInOneDayBeforeStartIsAll) {
	const uint16_t from = 1 * 60;
	const uint16_t to = 6 * 60;
	const uint16_t now = 0;

	const uint8_t exp_lit = 18;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsShortWindowFirstMinuteIsOne) {
	const uint16_t from = 10 * 60;
	const uint16_t to = 10 * 60 + 1;
	const uint16_t now = 10 * 60;

	const uint8_t exp_lit = 1;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsEmptyWindowIsAll) {
	const uint16_t from = 7 * 60;
	const uint16_t to = 7 * 60;
	const uint16_t now = 7 * 60;

	const uint8_t exp_lit = 18;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsMinuteOfDayAboveRangeGivesZero) {
	const uint16_t from = 21 * 60;
	const uint16_t to = 9 * 60;
	const uint16_t now = MINUTES_PER_DAY;

	const uint8_t exp_lit = 0;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsFromAboveRangeGivesZero) {
	const uint16_t from = MINUTES_PER_DAY;
	const uint16_t to = 9 * 60;
	const uint16_t now = 3 * 60;

	const uint8_t exp_lit = 0;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsToAboveRangeGivesZero) {
	const uint16_t from = 21 * 60;
	const uint16_t to = 0xFFFF;
	const uint16_t now = 3 * 60;

	const uint8_t exp_lit = 0;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

// ---------------------------------------------------------------- NightColumnIsColor

TEST(NightProgressTest, ColumnZeroIsColor) {
	const bool exp_is_color = true;

	const uint8_t column0 = 0;

	bool res0;

	res0 = HAQuDA_NightColumnIsColor(column0);

	EXPECT_EQ(exp_is_color, res0);
}

TEST(NightProgressTest, ColumnsThreeAndSixAreColor) {
	const bool exp_is_color = true;

	const uint8_t column0 = 3;
	const uint8_t column1 = 6;

	bool res0;
	bool res1;

	res0 = HAQuDA_NightColumnIsColor(column0);
	res1 = HAQuDA_NightColumnIsColor(column1);

	EXPECT_EQ(exp_is_color, res0);
	EXPECT_EQ(exp_is_color, res1);
}

TEST(NightProgressTest, ColumnsOneAndTwoAreNightLight) {
	const bool exp_is_color = false;

	const uint8_t column0 = 1;
	const uint8_t column1 = 2;

	bool res0;
	bool res1;

	res0 = HAQuDA_NightColumnIsColor(column0);
	res1 = HAQuDA_NightColumnIsColor(column1);

	EXPECT_EQ(exp_is_color, res0);
	EXPECT_EQ(exp_is_color, res1);
}

TEST(NightProgressTest, ColumnsFourAndFiveAreNightLight) {
	const bool exp_is_color = false;

	const uint8_t column0 = 4;
	const uint8_t column1 = 5;

	bool res0;
	bool res1;

	res0 = HAQuDA_NightColumnIsColor(column0);
	res1 = HAQuDA_NightColumnIsColor(column1);

	EXPECT_EQ(exp_is_color, res0);
	EXPECT_EQ(exp_is_color, res1);
}

TEST(NightProgressTest, ColumnsSevenAndEightAreNightLight) {
	const bool exp_is_color = false;

	const uint8_t column0 = 7;
	const uint8_t column1 = 8;

	bool res0;
	bool res1;

	res0 = HAQuDA_NightColumnIsColor(column0);
	res1 = HAQuDA_NightColumnIsColor(column1);

	EXPECT_EQ(exp_is_color, res0);
	EXPECT_EQ(exp_is_color, res1);
}

// ---------------------------------------------------------------- NightLightRows

TEST(NightProgressTest, LightRowsNoCellsLitIsZero) {
	const uint8_t lit = 0;
	const uint8_t column = 1;

	const uint8_t exp_rows = 0;

	uint8_t res;

	res = HAQuDA_NightLightRows(lit, column);

	EXPECT_EQ(exp_rows, res);
}

TEST(NightProgressTest, LightRowsOneCellIsTwoRowsInBothColumnsOfTheGroup) {
	const uint8_t lit = 1;

	const uint8_t exp_rows = 2;

	uint8_t first;
	uint8_t second;

	first = HAQuDA_NightLightRows(lit, 1);
	second = HAQuDA_NightLightRows(lit, 2);

	EXPECT_EQ(exp_rows, first);
	EXPECT_EQ(exp_rows, second);
}

TEST(NightProgressTest, LightRowsTwoCellsFillTheStripeFromTheLeft) {
	const uint8_t lit = 2;

	const uint8_t exp_lit_rows = 2;
	const uint8_t exp_dark_rows = 0;

	uint8_t firstPair;
	uint8_t secondPair;
	uint8_t thirdPair;

	firstPair = HAQuDA_NightLightRows(lit, 2);
	secondPair = HAQuDA_NightLightRows(lit, 4);
	thirdPair = HAQuDA_NightLightRows(lit, 7);

	EXPECT_EQ(exp_lit_rows, firstPair);
	EXPECT_EQ(exp_lit_rows, secondPair);
	EXPECT_EQ(exp_dark_rows, thirdPair);
}

TEST(NightProgressTest, LightRowsSixCellsAreTwoFullStripes) {
	const uint8_t lit = 6;

	const uint8_t exp_rows = 4;

	uint8_t firstPair;
	uint8_t secondPair;
	uint8_t thirdPair;

	firstPair = HAQuDA_NightLightRows(lit, 1);
	secondPair = HAQuDA_NightLightRows(lit, 5);
	thirdPair = HAQuDA_NightLightRows(lit, 8);

	EXPECT_EQ(exp_rows, firstPair);
	EXPECT_EQ(exp_rows, secondPair);
	EXPECT_EQ(exp_rows, thirdPair);
}

TEST(NightProgressTest, LightRowsSevenCellsStartTheThirdStripeAtTheLeft) {
	const uint8_t lit = 7;

	const uint8_t exp_first_pair_rows = 6;
	const uint8_t exp_other_rows = 4;

	uint8_t firstPair;
	uint8_t secondPair;
	uint8_t thirdPair;

	firstPair = HAQuDA_NightLightRows(lit, 2);
	secondPair = HAQuDA_NightLightRows(lit, 4);
	thirdPair = HAQuDA_NightLightRows(lit, 7);

	EXPECT_EQ(exp_first_pair_rows, firstPair);
	EXPECT_EQ(exp_other_rows, secondPair);
	EXPECT_EQ(exp_other_rows, thirdPair);
}

TEST(NightProgressTest, LightRowsSeventeenCellsLeaveOnlyTheLastCellDark) {
	const uint8_t lit = 17;

	const uint8_t exp_full = 12;
	const uint8_t exp_short = 10;

	uint8_t firstPair;
	uint8_t secondPair;
	uint8_t thirdPair;

	firstPair = HAQuDA_NightLightRows(lit, 1);
	secondPair = HAQuDA_NightLightRows(lit, 4);
	thirdPair = HAQuDA_NightLightRows(lit, 8);

	EXPECT_EQ(exp_full, firstPair);
	EXPECT_EQ(exp_full, secondPair);
	EXPECT_EQ(exp_short, thirdPair);
}

TEST(NightProgressTest, LightRowsAllCellsFillTheFirstPairToTheTop) {
	const uint8_t lit = 18;

	const uint8_t exp_rows0 = 12;
	const uint8_t exp_rows1 = 12;

	uint8_t res0;
	uint8_t res1;

	res0 = HAQuDA_NightLightRows(lit, 1);
	res1 = HAQuDA_NightLightRows(lit, 2);

	EXPECT_EQ(exp_rows0, res0);
	EXPECT_EQ(exp_rows1, res1);
}

TEST(NightProgressTest, LightRowsAllCellsFillTheSecondPairToTheTop) {
	const uint8_t lit = 18;

	const uint8_t exp_rows0 = 12;
	const uint8_t exp_rows1 = 12;

	uint8_t res0;
	uint8_t res1;

	res0 = HAQuDA_NightLightRows(lit, 4);
	res1 = HAQuDA_NightLightRows(lit, 5);

	EXPECT_EQ(exp_rows0, res0);
	EXPECT_EQ(exp_rows1, res1);
}

TEST(NightProgressTest, LightRowsAllCellsFillTheThirdPairToTheTop) {
	const uint8_t lit = 18;

	const uint8_t exp_rows0 = 12;
	const uint8_t exp_rows1 = 12;

	uint8_t res0;
	uint8_t res1;

	res0 = HAQuDA_NightLightRows(lit, 7);
	res1 = HAQuDA_NightLightRows(lit, 8);

	EXPECT_EQ(exp_rows0, res0);
	EXPECT_EQ(exp_rows1, res1);
}

TEST(NightProgressTest, LightRowsFourCellsGiveTheFirstPairTwoCellsAndTheOthersOne) {
	const uint8_t lit = 4;

	const uint8_t exp_rows0 = 4;
	const uint8_t exp_rows1 = 2;
	const uint8_t exp_rows2 = 2;

	uint8_t res0;
	uint8_t res1;
	uint8_t res2;

	res0 = HAQuDA_NightLightRows(lit, 1);
	res1 = HAQuDA_NightLightRows(lit, 4);
	res2 = HAQuDA_NightLightRows(lit, 7);

	EXPECT_EQ(exp_rows0, res0);
	EXPECT_EQ(exp_rows1, res1);
	EXPECT_EQ(exp_rows2, res2);
}

TEST(NightProgressTest, LightRowsTwelveCellsFillFourCellsPerPair) {
	const uint8_t lit = 12;

	const uint8_t exp_rows0 = 8;
	const uint8_t exp_rows1 = 8;
	const uint8_t exp_rows2 = 8;

	uint8_t res0;
	uint8_t res1;
	uint8_t res2;

	res0 = HAQuDA_NightLightRows(lit, 2);
	res1 = HAQuDA_NightLightRows(lit, 5);
	res2 = HAQuDA_NightLightRows(lit, 8);

	EXPECT_EQ(exp_rows0, res0);
	EXPECT_EQ(exp_rows1, res1);
	EXPECT_EQ(exp_rows2, res2);
}

TEST(NightProgressTest, LightRowsSixteenCellsLeaveTheLastTwoCellsDark) {
	// cells 16 and 17 (second and third pair of the top stripe) are dark
	const uint8_t lit = 16;

	const uint8_t exp_rows0 = 12;
	const uint8_t exp_rows1 = 10;
	const uint8_t exp_rows2 = 10;

	uint8_t res0;
	uint8_t res1;
	uint8_t res2;

	res0 = HAQuDA_NightLightRows(lit, 1);
	res1 = HAQuDA_NightLightRows(lit, 4);
	res2 = HAQuDA_NightLightRows(lit, 7);

	EXPECT_EQ(exp_rows0, res0);
	EXPECT_EQ(exp_rows1, res1);
	EXPECT_EQ(exp_rows2, res2);
}

TEST(NightProgressTest, LightRowsMoreThanAllCellsStaysAtTwelve) {
	const uint8_t lit = 255;

	const uint8_t exp_rows = 12;

	uint8_t res;

	res = HAQuDA_NightLightRows(lit, 8);

	EXPECT_EQ(exp_rows, res);
}

// ---------------------------------------------------------------- More window edges

TEST(NightProgressTest, LitCellsLastMinuteBeforeTheSecondCellIsStillOne) {
	// 40 of 720 minutes: 18 * 40 / 720 = 1 exactly
	const uint16_t from = 21 * 60;
	const uint16_t to = 9 * 60;
	const uint16_t now = 21 * 60 + 40;

	const uint8_t exp_lit = 1;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsMinuteBeforeTheStartIsOutsideAndAll) {
	const uint16_t from = 21 * 60;
	const uint16_t to = 9 * 60;
	const uint16_t now = 20 * 60 + 59;

	const uint8_t exp_lit = 18;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsShortWindowAcrossMidnightAtItsMiddle) {
	// 30 of 60 minutes: 18 * 30 / 60 = 9 exactly
	const uint16_t from = 23 * 60 + 30;
	const uint16_t to = 30;
	const uint16_t now = 0;

	const uint8_t exp_lit = 9;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsShortWindowAcrossMidnightBeforeMidnight) {
	// 29 of 60 minutes: 18 * 29 / 60 = 8.7
	const uint16_t from = 23 * 60 + 30;
	const uint16_t to = 30;
	const uint16_t now = 23 * 60 + 59;

	const uint8_t exp_lit = 9;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsShortWindowAcrossMidnightLastMinute) {
	// 59 of 60 minutes: 18 * 59 / 60 = 17.7
	const uint16_t from = 23 * 60 + 30;
	const uint16_t to = 30;
	const uint16_t now = 29;

	const uint8_t exp_lit = 18;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsShortWindowAcrossMidnightEndMinuteIsOutside) {
	const uint16_t from = 23 * 60 + 30;
	const uint16_t to = 30;
	const uint16_t now = 30;

	const uint8_t exp_lit = 18;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsWindowStartingAtMidnightFirstMinuteIsOne) {
	const uint16_t from = 0;
	const uint16_t to = 6 * 60;
	const uint16_t now = 0;

	const uint8_t exp_lit = 1;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsWindowOfOneMinuteAtTheLastMinuteOfTheDay) {
	const uint16_t from = 23 * 60 + 59;
	const uint16_t to = 0;
	const uint16_t now = 23 * 60 + 59;

	const uint8_t exp_lit = 1;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsWindowOfOneMinuteTheNextMinuteIsOutside) {
	const uint16_t from = 23 * 60 + 59;
	const uint16_t to = 0;
	const uint16_t now = 0;

	const uint8_t exp_lit = 18;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsWindowOfEighteenMinutesSecondMinuteIsTwo) {
	// one cell per minute: 18 * 2 / 18 = 2
	const uint16_t from = 10 * 60;
	const uint16_t to = 10 * 60 + 18;
	const uint16_t now = 10 * 60 + 2;

	const uint8_t exp_lit = 2;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsWindowOfEighteenMinutesLastMinuteIsSeventeen) {
	// elapsed 17 of 18 minutes: 18 * 17 / 18 = 17, the 18th cell comes with the end minute
	const uint16_t from = 10 * 60;
	const uint16_t to = 10 * 60 + 18;
	const uint16_t now = 10 * 60 + 17;

	const uint8_t exp_lit = 17;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsWindowOfAlmostADayLastMinuteIsAll) {
	// 1438 of 1439 minutes: 18 * 1438 / 1439 = 17.99
	const uint16_t from = 5 * 60;
	const uint16_t to = 4 * 60 + 59;
	const uint16_t now = 4 * 60 + 58;

	const uint8_t exp_lit = 18;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}

TEST(NightProgressTest, LitCellsLastMinuteOfTheDayInTheDefaultWindow) {
	// 179 of 720 minutes: 18 * 179 / 720 = 4.475
	const uint16_t from = 21 * 60;
	const uint16_t to = 9 * 60;
	const uint16_t now = 23 * 60 + 59;

	const uint8_t exp_lit = 5;

	uint8_t res;

	res = HAQuDA_NightLitCells(from, to, now);

	EXPECT_EQ(exp_lit, res);
}
