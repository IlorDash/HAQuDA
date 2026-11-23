#include <gtest/gtest.h>

#include <cstdint>

#include "HAQuDA_TouchPoller.h"

TEST(TouchPollerTest, TakeOnNewPollerIsZeroForButton0) {
	const uint8_t exp_count = 0;

	HAQuDA_TouchPoller poller;

	uint8_t res;

	res = poller.Take(0);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchPollerTest, TakeOnNewPollerIsZeroForButton1) {
	const uint8_t exp_count = 0;

	HAQuDA_TouchPoller poller;

	uint8_t res;

	res = poller.Take(1);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchPollerTest, TakeOnNewPollerIsZeroForButton2) {
	const uint8_t exp_count = 0;

	HAQuDA_TouchPoller poller;

	uint8_t res;

	res = poller.Take(2);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchPollerTest, TakeOnNewPollerIsZeroForButton3) {
	const uint8_t exp_count = 0;

	HAQuDA_TouchPoller poller;

	uint8_t res;

	res = poller.Take(3);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchPollerTest, DebouncedPressOnButton0CountsOnlyForButton0) {
	const uint32_t idleMs = 0;
	const uint32_t pressMs = 10;
	const uint32_t stableMs = pressMs + TOUCH_DEBOUNCE_MS;

	const uint8_t exp_pressed = 1;
	const uint8_t exp_others = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pressed[TOUCH_BUTTONS_NUM] = {true, false, false, false};

	HAQuDA_TouchPoller poller;

	uint8_t res_pressed;
	uint8_t res_1;
	uint8_t res_2;
	uint8_t res_3;

	poller.Poll(idle, idleMs);
	poller.Poll(pressed, pressMs);
	poller.Poll(pressed, stableMs);
	res_pressed = poller.Take(0);
	res_1 = poller.Take(1);
	res_2 = poller.Take(2);
	res_3 = poller.Take(3);

	EXPECT_EQ(exp_pressed, res_pressed);
	EXPECT_EQ(exp_others, res_1);
	EXPECT_EQ(exp_others, res_2);
	EXPECT_EQ(exp_others, res_3);
}

TEST(TouchPollerTest, DebouncedPressOnButton1CountsOnlyForButton1) {
	const uint32_t idleMs = 0;
	const uint32_t pressMs = 10;
	const uint32_t stableMs = pressMs + TOUCH_DEBOUNCE_MS;

	const uint8_t exp_pressed = 1;
	const uint8_t exp_others = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pressed[TOUCH_BUTTONS_NUM] = {false, true, false, false};

	HAQuDA_TouchPoller poller;

	uint8_t res_0;
	uint8_t res_pressed;
	uint8_t res_2;
	uint8_t res_3;

	poller.Poll(idle, idleMs);
	poller.Poll(pressed, pressMs);
	poller.Poll(pressed, stableMs);
	res_0 = poller.Take(0);
	res_pressed = poller.Take(1);
	res_2 = poller.Take(2);
	res_3 = poller.Take(3);

	EXPECT_EQ(exp_others, res_0);
	EXPECT_EQ(exp_pressed, res_pressed);
	EXPECT_EQ(exp_others, res_2);
	EXPECT_EQ(exp_others, res_3);
}

TEST(TouchPollerTest, DebouncedPressOnButton2CountsOnlyForButton2) {
	const uint32_t idleMs = 0;
	const uint32_t pressMs = 10;
	const uint32_t stableMs = pressMs + TOUCH_DEBOUNCE_MS;

	const uint8_t exp_pressed = 1;
	const uint8_t exp_others = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pressed[TOUCH_BUTTONS_NUM] = {false, false, true, false};

	HAQuDA_TouchPoller poller;

	uint8_t res_0;
	uint8_t res_1;
	uint8_t res_pressed;
	uint8_t res_3;

	poller.Poll(idle, idleMs);
	poller.Poll(pressed, pressMs);
	poller.Poll(pressed, stableMs);
	res_0 = poller.Take(0);
	res_1 = poller.Take(1);
	res_pressed = poller.Take(2);
	res_3 = poller.Take(3);

	EXPECT_EQ(exp_others, res_0);
	EXPECT_EQ(exp_others, res_1);
	EXPECT_EQ(exp_pressed, res_pressed);
	EXPECT_EQ(exp_others, res_3);
}

TEST(TouchPollerTest, DebouncedPressOnButton3CountsOnlyForButton3) {
	const uint32_t idleMs = 0;
	const uint32_t pressMs = 10;
	const uint32_t stableMs = pressMs + TOUCH_DEBOUNCE_MS;

	const uint8_t exp_pressed = 1;
	const uint8_t exp_others = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pressed[TOUCH_BUTTONS_NUM] = {false, false, false, true};

	HAQuDA_TouchPoller poller;

	uint8_t res_0;
	uint8_t res_1;
	uint8_t res_2;
	uint8_t res_pressed;

	poller.Poll(idle, idleMs);
	poller.Poll(pressed, pressMs);
	poller.Poll(pressed, stableMs);
	res_0 = poller.Take(0);
	res_1 = poller.Take(1);
	res_2 = poller.Take(2);
	res_pressed = poller.Take(3);

	EXPECT_EQ(exp_others, res_0);
	EXPECT_EQ(exp_others, res_1);
	EXPECT_EQ(exp_others, res_2);
	EXPECT_EQ(exp_pressed, res_pressed);
}

TEST(TouchPollerTest, LevelShorterThanDebounceDoesNotCount) {
	const uint32_t idleMs = 0;
	const uint32_t pressMs = 10;
	const uint32_t releaseMs = pressMs + TOUCH_DEBOUNCE_MS - 1;
	const uint32_t laterMs = releaseMs + TOUCH_DEBOUNCE_MS;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pressed[TOUCH_BUTTONS_NUM] = {true, false, false, false};

	HAQuDA_TouchPoller poller;

	uint8_t res;

	poller.Poll(idle, idleMs);
	poller.Poll(pressed, pressMs);
	poller.Poll(idle, releaseMs);
	poller.Poll(idle, laterMs);
	res = poller.Take(0);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchPollerTest, TwoButtonsPressedTogetherBothCount) {
	const uint32_t idleMs = 0;
	const uint32_t pressMs = 10;
	const uint32_t stableMs = pressMs + TOUCH_DEBOUNCE_MS;

	const uint8_t exp_pressed = 1;
	const uint8_t exp_others = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pressed[TOUCH_BUTTONS_NUM] = {true, false, true, false};

	HAQuDA_TouchPoller poller;

	uint8_t res_0;
	uint8_t res_1;
	uint8_t res_2;
	uint8_t res_3;

	poller.Poll(idle, idleMs);
	poller.Poll(pressed, pressMs);
	poller.Poll(pressed, stableMs);
	res_0 = poller.Take(0);
	res_1 = poller.Take(1);
	res_2 = poller.Take(2);
	res_3 = poller.Take(3);

	EXPECT_EQ(exp_pressed, res_0);
	EXPECT_EQ(exp_others, res_1);
	EXPECT_EQ(exp_pressed, res_2);
	EXPECT_EQ(exp_others, res_3);
}

TEST(TouchPollerTest, LevelActiveAtBootIsNotCountedWhileHeld) {
	const uint32_t bootMs = 0;
	const uint32_t stableMs = bootMs + TOUCH_DEBOUNCE_MS;

	const uint8_t exp_count = 0;

	const bool held[TOUCH_BUTTONS_NUM] = {true, false, false, false};

	HAQuDA_TouchPoller poller;

	uint8_t res;

	poller.Poll(held, bootMs);
	poller.Poll(held, stableMs);
	res = poller.Take(0);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchPollerTest, PressAfterStableReleaseOfBootLevelCounts) {
	const uint32_t bootMs = 0;
	const uint32_t heldStableMs = bootMs + TOUCH_DEBOUNCE_MS;
	const uint32_t releaseMs = heldStableMs + 10;
	const uint32_t releaseStableMs = releaseMs + TOUCH_DEBOUNCE_MS;
	const uint32_t pressMs = releaseStableMs + 10;
	const uint32_t pressStableMs = pressMs + TOUCH_DEBOUNCE_MS;

	const uint8_t exp_count = 1;

	const bool held[TOUCH_BUTTONS_NUM] = {true, false, false, false};
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};

	HAQuDA_TouchPoller poller;

	uint8_t res;

	poller.Poll(held, bootMs);
	poller.Poll(held, heldStableMs);
	poller.Poll(idle, releaseMs);
	poller.Poll(idle, releaseStableMs);
	poller.Poll(held, pressMs);
	poller.Poll(held, pressStableMs);
	res = poller.Take(0);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchPollerTest, TakeResetsTheCounter) {
	const uint32_t idleMs = 0;
	const uint32_t pressMs = 10;
	const uint32_t stableMs = pressMs + TOUCH_DEBOUNCE_MS;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pressed[TOUCH_BUTTONS_NUM] = {true, false, false, false};

	HAQuDA_TouchPoller poller;

	uint8_t res;

	poller.Poll(idle, idleMs);
	poller.Poll(pressed, pressMs);
	poller.Poll(pressed, stableMs);
	poller.Take(0);
	res = poller.Take(0);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchPollerTest, TakeAtButtonsNumReturnsZeroAndKeepsCounters) {
	const uint32_t idleMs = 0;
	const uint32_t pressMs = 10;
	const uint32_t stableMs = pressMs + TOUCH_DEBOUNCE_MS;

	const uint8_t exp_out_of_range = 0;
	const uint8_t exp_pressed = 1;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pressed[TOUCH_BUTTONS_NUM] = {false, false, false, true};

	HAQuDA_TouchPoller poller;

	uint8_t res_out_of_range;
	uint8_t res_pressed;

	poller.Poll(idle, idleMs);
	poller.Poll(pressed, pressMs);
	poller.Poll(pressed, stableMs);
	res_out_of_range = poller.Take(TOUCH_BUTTONS_NUM);
	res_pressed = poller.Take(3);

	EXPECT_EQ(exp_out_of_range, res_out_of_range);
	EXPECT_EQ(exp_pressed, res_pressed);
}

TEST(TouchPollerTest, TakeAtMaxIndexReturnsZeroAndKeepsCounters) {
	const uint32_t idleMs = 0;
	const uint32_t pressMs = 10;
	const uint32_t stableMs = pressMs + TOUCH_DEBOUNCE_MS;

	const uint8_t maxIndex = UINT8_MAX;

	const uint8_t exp_out_of_range = 0;
	const uint8_t exp_pressed = 1;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pressed[TOUCH_BUTTONS_NUM] = {true, false, false, false};

	HAQuDA_TouchPoller poller;

	uint8_t res_out_of_range;
	uint8_t res_pressed;

	poller.Poll(idle, idleMs);
	poller.Poll(pressed, pressMs);
	poller.Poll(pressed, stableMs);
	res_out_of_range = poller.Take(maxIndex);
	res_pressed = poller.Take(0);

	EXPECT_EQ(exp_out_of_range, res_out_of_range);
	EXPECT_EQ(exp_pressed, res_pressed);
}
