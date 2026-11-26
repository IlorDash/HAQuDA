#include <gtest/gtest.h>

#include <cstdint>

#include "HAQuDA_TouchGestures.h"

TEST(TouchGesturesTest, TapOnReleaseJustBeforeTapLimit) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = 1;
	const touch_gesture_kind exp_kind = touch_gesture_kind::tap;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(idle, startMs + TOUCH_TAP_MAX_MS - 1, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, NoTapOnReleaseAtTapLimit) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(idle, startMs + TOUCH_TAP_MAX_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, RepeatableTapOnReleaseBeforeRepeatStart) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = 1;
	const touch_gesture_kind exp_kind = touch_gesture_kind::tap;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + 590, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(idle, startMs + TOUCH_TAP_MAX_MS - 1, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, RepeatableNoRepeatJustBeforeRepeatStart) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6, startMs + TOUCH_REPEAT_START_MS - 1, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, RepeatableFirstRepeatAtRepeatStart) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = 1;
	const touch_gesture_kind exp_kind = touch_gesture_kind::repeat;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6, startMs + TOUCH_REPEAT_START_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, NoRepeatJustBeforeSecondRepeat) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + TOUCH_REPEAT_START_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6, startMs + TOUCH_REPEAT_START_MS + TOUCH_REPEAT_PERIOD_MS - 1, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, SecondRepeatOnePeriodAfterFirst) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = 1;
	const touch_gesture_kind exp_kind = touch_gesture_kind::repeat;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + TOUCH_REPEAT_START_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6, startMs + TOUCH_REPEAT_START_MS + TOUCH_REPEAT_PERIOD_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, ThirdRepeatTwoPeriodsAfterFirst) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = 1;
	const touch_gesture_kind exp_kind = touch_gesture_kind::repeat;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + TOUCH_REPEAT_START_MS, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + TOUCH_REPEAT_START_MS + TOUCH_REPEAT_PERIOD_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6, startMs + TOUCH_REPEAT_START_MS + 2 * TOUCH_REPEAT_PERIOD_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, NoDoubleRepeatRightAfterBoundary) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + TOUCH_REPEAT_START_MS, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + TOUCH_REPEAT_START_MS + TOUCH_REPEAT_PERIOD_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6, startMs + TOUCH_REPEAT_START_MS + TOUCH_REPEAT_PERIOD_MS + 1, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, LateFirstTickEmitsOneRepeat) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = 1;
	const touch_gesture_kind exp_kind = touch_gesture_kind::repeat;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6, startMs + 1000, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, NextRepeatAfterLateTickIsOnGrid) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = 1;
	const touch_gesture_kind exp_kind = touch_gesture_kind::repeat;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + 1000, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6, startMs + 1200, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, NoRepeatBeforeGridAfterLateTick) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + 1000, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6, startMs + 1199, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, NoRepeatRightAfterLateTickOnExactGrid) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + 1000, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6, startMs + 1001, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, ReleaseDuringRepeatEmitsRepeatEnd) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = 1;
	const touch_gesture_kind exp_kind = touch_gesture_kind::repeat_end;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + TOUCH_REPEAT_START_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(idle, startMs + TOUCH_REPEAT_START_MS + 50, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, ReleaseBetweenRepeatStartAndFirstTickGivesNothing) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(idle, startMs + TOUCH_REPEAT_START_MS + 50, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, NonRepeatableHoldGivesNoEvent) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad8[TOUCH_BUTTONS_NUM] = {false, false, true, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad8, startMs, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad8, startMs + 2000, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, NonRepeatableLongHoldReleaseGivesNoEvent) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad8[TOUCH_BUTTONS_NUM] = {false, false, true, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad8, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad8, startMs + 2000, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(idle, startMs + 2100, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, SecondButtonDuringRepeatEmitsRepeatEnd) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = 1;
	const touch_gesture_kind exp_kind = touch_gesture_kind::repeat_end;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	const bool pad6and9[TOUCH_BUTTONS_NUM] = {false, true, false, true};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + TOUCH_REPEAT_START_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6and9, startMs + TOUCH_REPEAT_START_MS + 100, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, NoRepeatWhileSecondButtonHeldAfterRepeatEnd) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	const bool pad6and9[TOUCH_BUTTONS_NUM] = {false, true, false, true};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + TOUCH_REPEAT_START_MS, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and9, startMs + TOUCH_REPEAT_START_MS + 100, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6and9, startMs + TOUCH_REPEAT_START_MS + 400, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, ReleaseOfOneButtonAfterRepeatEndGivesNothing) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	const bool pad9[TOUCH_BUTTONS_NUM] = {false, false, false, true};
	const bool pad6and9[TOUCH_BUTTONS_NUM] = {false, true, false, true};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + TOUCH_REPEAT_START_MS, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and9, startMs + TOUCH_REPEAT_START_MS + 100, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad9, startMs + TOUCH_REPEAT_START_MS + 200, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, FullReleaseAfterRepeatEndGivesNothing) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	const bool pad9[TOUCH_BUTTONS_NUM] = {false, false, false, true};
	const bool pad6and9[TOUCH_BUTTONS_NUM] = {false, true, false, true};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + TOUCH_REPEAT_START_MS, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and9, startMs + TOUCH_REPEAT_START_MS + 100, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad9, startMs + TOUCH_REPEAT_START_MS + 200, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(idle, startMs + TOUCH_REPEAT_START_MS + 300, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, ReleaseOfOneOfTwoButtonsGivesNoTap) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad2[TOUCH_BUTTONS_NUM] = {true, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	const bool pad2and6[TOUCH_BUTTONS_NUM] = {true, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad2, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad2and6, startMs + 100, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6, startMs + 200, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, ReleaseOfBothButtonsGivesNoTap) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad2[TOUCH_BUTTONS_NUM] = {true, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	const bool pad2and6[TOUCH_BUTTONS_NUM] = {true, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad2, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad2and6, startMs + 100, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + 200, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(idle, startMs + 300, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, TapWorksAgainAfterTwoButtonsReleased) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = 2;
	const touch_gesture_kind exp_kind = touch_gesture_kind::tap;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad8[TOUCH_BUTTONS_NUM] = {false, false, true, false};
	const bool pad2and6[TOUCH_BUTTONS_NUM] = {true, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad2and6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(idle, startMs + 100, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad8, startMs + 200, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(idle, startMs + 300, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, SoloToSoloWhileRepeatingEndsRepeatOfFirst) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = 1;
	const touch_gesture_kind exp_kind = touch_gesture_kind::repeat_end;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	const bool pad8[TOUCH_BUTTONS_NUM] = {false, false, true, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.SetRepeatable(2, true);
	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + TOUCH_REPEAT_START_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad8, startMs + TOUCH_REPEAT_START_MS + 50, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, SoloToSoloSecondButtonNoRepeatBeforeOwnStart) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	const bool pad8[TOUCH_BUTTONS_NUM] = {false, false, true, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.SetRepeatable(2, true);
	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + TOUCH_REPEAT_START_MS, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad8, startMs + TOUCH_REPEAT_START_MS + 50, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad8, startMs + TOUCH_REPEAT_START_MS + 50 + TOUCH_REPEAT_START_MS - 1, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, SoloToSoloSecondButtonRepeatsAtOwnStart) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = 2;
	const touch_gesture_kind exp_kind = touch_gesture_kind::repeat;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	const bool pad8[TOUCH_BUTTONS_NUM] = {false, false, true, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.SetRepeatable(2, true);
	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + TOUCH_REPEAT_START_MS, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad8, startMs + TOUCH_REPEAT_START_MS + 50, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad8, startMs + TOUCH_REPEAT_START_MS + 50 + TOUCH_REPEAT_START_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, SoloToSoloAfterShortHoldEmitsTapOfFirst) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = 1;
	const touch_gesture_kind exp_kind = touch_gesture_kind::tap;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	const bool pad8[TOUCH_BUTTONS_NUM] = {false, false, true, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad8, startMs + 100, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, SoloToSoloAfterShortHoldSecondReleaseIsOwnTap) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = 2;
	const touch_gesture_kind exp_kind = touch_gesture_kind::tap;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	const bool pad8[TOUCH_BUTTONS_NUM] = {false, false, true, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad8, startMs + 100, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(idle, startMs + 200, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, SoloToSoloAfterLongHoldWithoutRepeatGivesNothing) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	const bool pad8[TOUCH_BUTTONS_NUM] = {false, false, true, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad8, startMs + TOUCH_TAP_MAX_MS + 100, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, NoLockChordJustBeforeLockHold) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	const bool pad6and8[TOUCH_BUTTONS_NUM] = {false, true, true, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and8, startMs + 100, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6and8, startMs + 100 + TOUCH_LOCK_HOLD_MS - 1, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, LockChordAtLockHold) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = TOUCH_NO_BUTTON;
	const touch_gesture_kind exp_kind = touch_gesture_kind::lock_chord;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	const bool pad6and8[TOUCH_BUTTONS_NUM] = {false, true, true, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and8, startMs + 100, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6and8, startMs + 100 + TOUCH_LOCK_HOLD_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, LockChordDoesNotFireTwiceInOneHold) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6and8[TOUCH_BUTTONS_NUM] = {false, true, true, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and8, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and8, startMs + TOUCH_LOCK_HOLD_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6and8, startMs + 2 * TOUCH_LOCK_HOLD_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, LockChordFiresAgainOnNextHold) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = TOUCH_NO_BUTTON;
	const touch_gesture_kind exp_kind = touch_gesture_kind::lock_chord;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6and8[TOUCH_BUTTONS_NUM] = {false, true, true, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and8, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and8, startMs + TOUCH_LOCK_HOLD_MS, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(idle, startMs + TOUCH_LOCK_HOLD_MS + 100, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and8, startMs + 4000, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6and8, startMs + 4000 + TOUCH_LOCK_HOLD_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, NoApChordJustBeforeApHold) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad2[TOUCH_BUTTONS_NUM] = {true, false, false, false};
	const bool pad2and9[TOUCH_BUTTONS_NUM] = {true, false, false, true};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad2, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad2and9, startMs + 50, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad2and9, startMs + 50 + TOUCH_AP_HOLD_MS - 1, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, ApChordAtApHold) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = TOUCH_NO_BUTTON;
	const touch_gesture_kind exp_kind = touch_gesture_kind::ap_chord;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad2[TOUCH_BUTTONS_NUM] = {true, false, false, false};
	const bool pad2and9[TOUCH_BUTTONS_NUM] = {true, false, false, true};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad2, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad2and9, startMs + 50, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad2and9, startMs + 50 + TOUCH_AP_HOLD_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, ApChordDoesNotFireTwiceInOneHold) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad2and9[TOUCH_BUTTONS_NUM] = {true, false, false, true};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad2and9, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad2and9, startMs + TOUCH_AP_HOLD_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad2and9, startMs + 2 * TOUCH_AP_HOLD_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, WrongPairGivesNoChord) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad2and6[TOUCH_BUTTONS_NUM] = {true, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad2and6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad2and6, startMs + 2 * TOUCH_AP_HOLD_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, ThirdButtonCancelsLockChord) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6and8[TOUCH_BUTTONS_NUM] = {false, true, true, false};
	const bool pad2and6and8[TOUCH_BUTTONS_NUM] = {true, true, true, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and8, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad2and6and8, startMs + 1000, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad2and6and8, startMs + 2 * TOUCH_AP_HOLD_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, NoLockChordBeforeTimerRestartedByThirdButtonLeaving) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6and8[TOUCH_BUTTONS_NUM] = {false, true, true, false};
	const bool pad2and6and8[TOUCH_BUTTONS_NUM] = {true, true, true, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad2and6and8, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and8, startMs + 1000, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6and8, startMs + 1000 + TOUCH_LOCK_HOLD_MS - 1, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, LockChordCountsFromThirdButtonLeaving) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = TOUCH_NO_BUTTON;
	const touch_gesture_kind exp_kind = touch_gesture_kind::lock_chord;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6and8[TOUCH_BUTTONS_NUM] = {false, true, true, false};
	const bool pad2and6and8[TOUCH_BUTTONS_NUM] = {true, true, true, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad2and6and8, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and8, startMs + 1000, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6and8, startMs + 1000 + TOUCH_LOCK_HOLD_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, NoLockChordAtOriginalDeadlineAfterThirdButtonCame) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6and8[TOUCH_BUTTONS_NUM] = {false, true, true, false};
	const bool pad2and6and8[TOUCH_BUTTONS_NUM] = {true, true, true, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and8, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad2and6and8, startMs + 2000, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and8, startMs + 2500, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6and8, startMs + TOUCH_LOCK_HOLD_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, LockChordCountsFromDropToLockPair) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = TOUCH_NO_BUTTON;
	const touch_gesture_kind exp_kind = touch_gesture_kind::lock_chord;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6and8[TOUCH_BUTTONS_NUM] = {false, true, true, false};
	const bool pad2and6and8[TOUCH_BUTTONS_NUM] = {true, true, true, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and8, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad2and6and8, startMs + 2000, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and8, startMs + 2500, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6and8, startMs + 2500 + TOUCH_LOCK_HOLD_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, ReleaseOneButtonOfFiredChordGivesNoEvent) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	const bool pad6and8[TOUCH_BUTTONS_NUM] = {false, true, true, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and8, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and8, startMs + TOUCH_LOCK_HOLD_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6, startMs + TOUCH_LOCK_HOLD_MS + 100, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, ReleaseSecondButtonOfFiredChordGivesNoTap) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	const bool pad6and8[TOUCH_BUTTONS_NUM] = {false, true, true, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and8, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and8, startMs + TOUCH_LOCK_HOLD_MS, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + TOUCH_LOCK_HOLD_MS + 100, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(idle, startMs + TOUCH_LOCK_HOLD_MS + 200, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, NoChordRefireAfterReleasingOneButtonOfFiredChord) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	const bool pad6and8[TOUCH_BUTTONS_NUM] = {false, true, true, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and8, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and8, startMs + TOUCH_LOCK_HOLD_MS, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + TOUCH_LOCK_HOLD_MS + 100, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6, startMs + 4 * TOUCH_LOCK_HOLD_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, BootHeldButtonGivesNoEventOnFirstCall) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	res = g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, BootHeldRepeatableButtonDoesNotRepeat) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6, startMs + 2000, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, BootHeldButtonReleaseGivesNoTap) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + 2000, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(idle, startMs + 2100, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, BootHeldButtonTapsAfterReleaseAndPress) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = 1;
	const touch_gesture_kind exp_kind = touch_gesture_kind::tap;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(idle, startMs + 100, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + 200, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(idle, startMs + 300, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, BootHeldButtonSwitchToOtherButtonStartsItWithoutEvent) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;

	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	const bool pad8[TOUCH_BUTTONS_NUM] = {false, false, true, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad8, startMs + 100, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, BootHeldButtonSwitchToOtherButtonReleaseIsItsTap) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = 2;
	const touch_gesture_kind exp_kind = touch_gesture_kind::tap;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	const bool pad8[TOUCH_BUTTONS_NUM] = {false, false, true, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad8, startMs + 100, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(idle, startMs + 200, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, TapAcrossMillisWrap) {
	const uint32_t startMs = 0xFFFFFFFFu - 300;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = 1;
	const touch_gesture_kind exp_kind = touch_gesture_kind::tap;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(idle, startMs + TOUCH_TAP_MAX_MS - 1, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, FirstRepeatAcrossMillisWrap) {
	const uint32_t startMs = 0xFFFFFFFFu - 300;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = 1;
	const touch_gesture_kind exp_kind = touch_gesture_kind::repeat;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6, startMs + TOUCH_REPEAT_START_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, SecondRepeatAcrossMillisWrap) {
	const uint32_t startMs = 0xFFFFFFFFu - 300;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = 1;
	const touch_gesture_kind exp_kind = touch_gesture_kind::repeat;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + TOUCH_REPEAT_START_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6, startMs + TOUCH_REPEAT_START_MS + TOUCH_REPEAT_PERIOD_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, LateRepeatTickAcrossMillisWrap) {
	const uint32_t startMs = 0xFFFFFFFFu - 300;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = 1;
	const touch_gesture_kind exp_kind = touch_gesture_kind::repeat;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6, startMs + 1000, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, RepeatEndAcrossMillisWrap) {
	const uint32_t startMs = 0xFFFFFFFFu - 300;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = 1;
	const touch_gesture_kind exp_kind = touch_gesture_kind::repeat_end;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.SetRepeatable(1, true);
	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6, startMs + TOUCH_REPEAT_START_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(idle, startMs + TOUCH_REPEAT_START_MS + 50, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, LockChordAcrossMillisWrap) {
	const uint32_t startMs = 0xFFFFFFFFu - 1000;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = TOUCH_NO_BUTTON;
	const touch_gesture_kind exp_kind = touch_gesture_kind::lock_chord;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6and8[TOUCH_BUTTONS_NUM] = {false, true, true, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and8, startMs, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6and8, startMs + TOUCH_LOCK_HOLD_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, ApChordAcrossMillisWrap) {
	const uint32_t startMs = 0xFFFFFFFFu - 5000;

	const uint8_t exp_count = 1;
	const uint8_t exp_button = TOUCH_NO_BUTTON;
	const touch_gesture_kind exp_kind = touch_gesture_kind::ap_chord;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad2and9[TOUCH_BUTTONS_NUM] = {true, false, false, true};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad2and9, startMs, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad2and9, startMs + TOUCH_AP_HOLD_MS, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(exp_button, events[0].button);
	EXPECT_EQ(exp_kind, events[0].kind);
}

TEST(TouchGesturesTest, NoLockChordJustBeforeLockHoldAcrossMillisWrap) {
	const uint32_t startMs = 0xFFFFFFFFu - 1000;

	const uint8_t exp_count = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6and8[TOUCH_BUTTONS_NUM] = {false, true, true, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[TOUCH_GESTURE_MAX_EVENTS];

	uint8_t res;

	g.Update(idle, startMs - 10, events, TOUCH_GESTURE_MAX_EVENTS);
	g.Update(pad6and8, startMs, events, TOUCH_GESTURE_MAX_EVENTS);

	res = g.Update(pad6and8, startMs + TOUCH_LOCK_HOLD_MS - 1, events, TOUCH_GESTURE_MAX_EVENTS);

	EXPECT_EQ(exp_count, res);
}

TEST(TouchGesturesTest, ZeroCapacityEmitsNothingAndWritesNothing) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 0;
	const uint8_t sentinel = 0xAB;
	const uint8_t capacity = 0;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[1];
	events[0].button = sentinel;

	uint8_t res;

	g.Update(idle, startMs - 10, events, capacity);
	g.Update(pad6, startMs, events, capacity);

	res = g.Update(idle, startMs + 100, events, capacity);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(sentinel, events[0].button);
}

TEST(TouchGesturesTest, SingleCapacityReturnsOneEventAndWritesNoFurther) {
	const uint32_t startMs = 1000;

	const uint8_t exp_count = 1;
	const uint8_t sentinel = 0xAB;
	const uint8_t capacity = 1;

	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad6[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	HAQuDA_TouchGestures g;
	touch_gesture_event events[2];
	events[1].button = sentinel;

	uint8_t res;

	g.Update(idle, startMs - 10, events, capacity);
	g.Update(pad6, startMs, events, capacity);

	res = g.Update(idle, startMs + 100, events, capacity);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(sentinel, events[1].button);
}
