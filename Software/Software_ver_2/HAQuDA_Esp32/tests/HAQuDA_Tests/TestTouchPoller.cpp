#include <gtest/gtest.h>

#include <cstdint>

#include "HAQuDA_TouchPoller.h"

TEST(TouchPollerTest, IdleLevelsGiveNoEvents) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};

	const uint8_t exp_quiet = 0;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	quiet += poller.Poll(idle, 2000, scratch, 1);

	EXPECT_EQ(exp_quiet, quiet);
}

TEST(TouchPollerTest, ShortTouchOnButton0GivesTapOfButton0) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad0[TOUCH_BUTTONS_NUM] = {true, false, false, false};

	const uint8_t exp_quiet = 0;
	const uint8_t exp_count_tap = 1;
	const touch_gesture_kind exp_kind_tap = touch_gesture_kind::tap;
	const uint8_t exp_button_tap = 0;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;
	touch_gesture_event out_tap[1];
	uint8_t res_tap;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	quiet += poller.Poll(pad0, 1100, scratch, 1);
	quiet += poller.Poll(pad0, 1130, scratch, 1);
	quiet += poller.Poll(idle, 1200, scratch, 1);
	res_tap = poller.Poll(idle, 1230, out_tap, 1);

	EXPECT_EQ(exp_quiet, quiet);
	EXPECT_EQ(exp_count_tap, res_tap);
	EXPECT_EQ(exp_kind_tap, out_tap[0].kind);
	EXPECT_EQ(exp_button_tap, out_tap[0].button);
}

TEST(TouchPollerTest, ShortTouchOnButton1GivesTapOfButton1) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad1[TOUCH_BUTTONS_NUM] = {false, true, false, false};

	const uint8_t exp_quiet = 0;
	const uint8_t exp_count_tap = 1;
	const touch_gesture_kind exp_kind_tap = touch_gesture_kind::tap;
	const uint8_t exp_button_tap = 1;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;
	touch_gesture_event out_tap[1];
	uint8_t res_tap;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	quiet += poller.Poll(pad1, 1100, scratch, 1);
	quiet += poller.Poll(pad1, 1130, scratch, 1);
	quiet += poller.Poll(idle, 1200, scratch, 1);
	res_tap = poller.Poll(idle, 1230, out_tap, 1);

	EXPECT_EQ(exp_quiet, quiet);
	EXPECT_EQ(exp_count_tap, res_tap);
	EXPECT_EQ(exp_kind_tap, out_tap[0].kind);
	EXPECT_EQ(exp_button_tap, out_tap[0].button);
}

TEST(TouchPollerTest, ShortTouchOnButton2GivesTapOfButton2) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad2[TOUCH_BUTTONS_NUM] = {false, false, true, false};

	const uint8_t exp_quiet = 0;
	const uint8_t exp_count_tap = 1;
	const touch_gesture_kind exp_kind_tap = touch_gesture_kind::tap;
	const uint8_t exp_button_tap = 2;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;
	touch_gesture_event out_tap[1];
	uint8_t res_tap;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	quiet += poller.Poll(pad2, 1100, scratch, 1);
	quiet += poller.Poll(pad2, 1130, scratch, 1);
	quiet += poller.Poll(idle, 1200, scratch, 1);
	res_tap = poller.Poll(idle, 1230, out_tap, 1);

	EXPECT_EQ(exp_quiet, quiet);
	EXPECT_EQ(exp_count_tap, res_tap);
	EXPECT_EQ(exp_kind_tap, out_tap[0].kind);
	EXPECT_EQ(exp_button_tap, out_tap[0].button);
}

TEST(TouchPollerTest, ShortTouchOnButton3GivesTapOfButton3) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad3[TOUCH_BUTTONS_NUM] = {false, false, false, true};

	const uint8_t exp_quiet = 0;
	const uint8_t exp_count_tap = 1;
	const touch_gesture_kind exp_kind_tap = touch_gesture_kind::tap;
	const uint8_t exp_button_tap = 3;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;
	touch_gesture_event out_tap[1];
	uint8_t res_tap;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	quiet += poller.Poll(pad3, 1100, scratch, 1);
	quiet += poller.Poll(pad3, 1130, scratch, 1);
	quiet += poller.Poll(idle, 1200, scratch, 1);
	res_tap = poller.Poll(idle, 1230, out_tap, 1);

	EXPECT_EQ(exp_quiet, quiet);
	EXPECT_EQ(exp_count_tap, res_tap);
	EXPECT_EQ(exp_kind_tap, out_tap[0].kind);
	EXPECT_EQ(exp_button_tap, out_tap[0].button);
}

TEST(TouchPollerTest, TouchShorterThanDebounceGivesNoEvent) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad0[TOUCH_BUTTONS_NUM] = {true, false, false, false};

	const uint8_t exp_quiet = 0;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	quiet += poller.Poll(pad0, 1100, scratch, 1);
	quiet += poller.Poll(idle, 1120, scratch, 1);
	quiet += poller.Poll(idle, 1150, scratch, 1);
	quiet += poller.Poll(idle, 1180, scratch, 1);

	EXPECT_EQ(exp_quiet, quiet);
}

TEST(TouchPollerTest, TapReleasedJustBeforeSixHundredMsIsATap) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad0[TOUCH_BUTTONS_NUM] = {true, false, false, false};

	const uint8_t exp_quiet = 0;
	const uint8_t exp_count_tap = 1;
	const touch_gesture_kind exp_kind_tap = touch_gesture_kind::tap;
	const uint8_t exp_button_tap = 0;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;
	touch_gesture_event out_tap[1];
	uint8_t res_tap;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	quiet += poller.Poll(pad0, 1100, scratch, 1);
	quiet += poller.Poll(pad0, 1130, scratch, 1);
	quiet += poller.Poll(idle, 1699, scratch, 1);
	res_tap = poller.Poll(idle, 1729, out_tap, 1);

	EXPECT_EQ(exp_quiet, quiet);
	EXPECT_EQ(exp_count_tap, res_tap);
	EXPECT_EQ(exp_kind_tap, out_tap[0].kind);
	EXPECT_EQ(exp_button_tap, out_tap[0].button);
}

TEST(TouchPollerTest, TouchReleasedAtSixHundredMsIsNotATap) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad0[TOUCH_BUTTONS_NUM] = {true, false, false, false};

	const uint8_t exp_quiet = 0;
	const uint8_t exp_count_tap = 0;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;
	touch_gesture_event out_tap[1];
	uint8_t res_tap;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	quiet += poller.Poll(pad0, 1100, scratch, 1);
	quiet += poller.Poll(pad0, 1130, scratch, 1);
	quiet += poller.Poll(idle, 1700, scratch, 1);
	res_tap = poller.Poll(idle, 1730, out_tap, 1);

	EXPECT_EQ(exp_quiet, quiet);
	EXPECT_EQ(exp_count_tap, res_tap);
}

TEST(TouchPollerTest, TwoButtonsTouchedBriefGiveNoTap) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad02[TOUCH_BUTTONS_NUM] = {true, false, true, false};

	const uint8_t exp_quiet = 0;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	quiet += poller.Poll(pad02, 1100, scratch, 1);
	quiet += poller.Poll(pad02, 1130, scratch, 1);
	quiet += poller.Poll(idle, 1200, scratch, 1);
	quiet += poller.Poll(idle, 1230, scratch, 1);

	EXPECT_EQ(exp_quiet, quiet);
}

TEST(TouchPollerTest, SecondTouchAfterATapGivesASecondTap) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad2[TOUCH_BUTTONS_NUM] = {false, false, true, false};

	const uint8_t exp_quiet = 1;
	const uint8_t exp_count_tap = 1;
	const touch_gesture_kind exp_kind_tap = touch_gesture_kind::tap;
	const uint8_t exp_button_tap = 2;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;
	touch_gesture_event out_tap[1];
	uint8_t res_tap;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	quiet += poller.Poll(pad2, 1100, scratch, 1);
	quiet += poller.Poll(pad2, 1130, scratch, 1);
	quiet += poller.Poll(idle, 1200, scratch, 1);
	quiet += poller.Poll(idle, 1230, scratch, 1);
	quiet += poller.Poll(pad2, 1300, scratch, 1);
	quiet += poller.Poll(pad2, 1330, scratch, 1);
	quiet += poller.Poll(idle, 1400, scratch, 1);
	res_tap = poller.Poll(idle, 1430, out_tap, 1);

	EXPECT_EQ(exp_quiet, quiet);
	EXPECT_EQ(exp_count_tap, res_tap);
	EXPECT_EQ(exp_kind_tap, out_tap[0].kind);
	EXPECT_EQ(exp_button_tap, out_tap[0].button);
}

TEST(TouchPollerTest, HeldRepeatableButtonGivesTheFirstRepeatAtSixHundredMs) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad1[TOUCH_BUTTONS_NUM] = {false, true, false, false};

	const uint8_t exp_quiet = 0;
	const uint8_t exp_count_repeat = 1;
	const touch_gesture_kind exp_kind_repeat = touch_gesture_kind::repeat;
	const uint8_t exp_button_repeat = 1;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;
	touch_gesture_event out_repeat[1];
	uint8_t res_repeat;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	poller.SetRepeatable(1, true);
	quiet += poller.Poll(pad1, 1100, scratch, 1);
	quiet += poller.Poll(pad1, 1130, scratch, 1);
	quiet += poller.Poll(pad1, 1700, scratch, 1);
	res_repeat = poller.Poll(pad1, 1730, out_repeat, 1);

	EXPECT_EQ(exp_quiet, quiet);
	EXPECT_EQ(exp_count_repeat, res_repeat);
	EXPECT_EQ(exp_kind_repeat, out_repeat[0].kind);
	EXPECT_EQ(exp_button_repeat, out_repeat[0].button);
}

TEST(TouchPollerTest, HeldRepeatableButtonGivesTheSecondRepeatTwoHundredMsLater) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad1[TOUCH_BUTTONS_NUM] = {false, true, false, false};

	const uint8_t exp_quiet = 1;
	const uint8_t exp_count_repeat = 1;
	const touch_gesture_kind exp_kind_repeat = touch_gesture_kind::repeat;
	const uint8_t exp_button_repeat = 1;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;
	touch_gesture_event out_repeat[1];
	uint8_t res_repeat;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	poller.SetRepeatable(1, true);
	quiet += poller.Poll(pad1, 1100, scratch, 1);
	quiet += poller.Poll(pad1, 1130, scratch, 1);
	quiet += poller.Poll(pad1, 1730, scratch, 1);
	quiet += poller.Poll(pad1, 1900, scratch, 1);
	res_repeat = poller.Poll(pad1, 1930, out_repeat, 1);

	EXPECT_EQ(exp_quiet, quiet);
	EXPECT_EQ(exp_count_repeat, res_repeat);
	EXPECT_EQ(exp_kind_repeat, out_repeat[0].kind);
	EXPECT_EQ(exp_button_repeat, out_repeat[0].button);
}

TEST(TouchPollerTest, ReleaseAfterRepeatsGivesRepeatEndOfTheButton) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad1[TOUCH_BUTTONS_NUM] = {false, true, false, false};

	const uint8_t exp_quiet = 2;
	const uint8_t exp_count_end = 1;
	const touch_gesture_kind exp_kind_end = touch_gesture_kind::repeat_end;
	const uint8_t exp_button_end = 1;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;
	touch_gesture_event out_end[1];
	uint8_t res_end;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	poller.SetRepeatable(1, true);
	quiet += poller.Poll(pad1, 1100, scratch, 1);
	quiet += poller.Poll(pad1, 1130, scratch, 1);
	quiet += poller.Poll(pad1, 1730, scratch, 1);
	quiet += poller.Poll(pad1, 1930, scratch, 1);
	quiet += poller.Poll(idle, 2000, scratch, 1);
	res_end = poller.Poll(idle, 2030, out_end, 1);

	EXPECT_EQ(exp_quiet, quiet);
	EXPECT_EQ(exp_count_end, res_end);
	EXPECT_EQ(exp_kind_end, out_end[0].kind);
	EXPECT_EQ(exp_button_end, out_end[0].button);
}

TEST(TouchPollerTest, HeldButtonWithoutRepeatGivesNoEvent) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad1[TOUCH_BUTTONS_NUM] = {false, true, false, false};

	const uint8_t exp_quiet = 0;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	quiet += poller.Poll(pad1, 1100, scratch, 1);
	quiet += poller.Poll(pad1, 1130, scratch, 1);
	quiet += poller.Poll(pad1, 1730, scratch, 1);
	quiet += poller.Poll(pad1, 3000, scratch, 1);
	quiet += poller.Poll(idle, 3100, scratch, 1);
	quiet += poller.Poll(idle, 3130, scratch, 1);

	EXPECT_EQ(exp_quiet, quiet);
}

TEST(TouchPollerTest, RepeatableFlagSetMidHoldRepeatsAtTheNextPoll) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad1[TOUCH_BUTTONS_NUM] = {false, true, false, false};

	const uint8_t exp_quiet = 0;
	const uint8_t exp_count_repeat = 1;
	const touch_gesture_kind exp_kind_repeat = touch_gesture_kind::repeat;
	const uint8_t exp_button_repeat = 1;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;
	touch_gesture_event out_repeat[1];
	uint8_t res_repeat;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	quiet += poller.Poll(pad1, 1100, scratch, 1);
	quiet += poller.Poll(pad1, 1130, scratch, 1);
	quiet += poller.Poll(pad1, 1900, scratch, 1);
	poller.SetRepeatable(1, true);
	res_repeat = poller.Poll(pad1, 1930, out_repeat, 1);

	EXPECT_EQ(exp_quiet, quiet);
	EXPECT_EQ(exp_count_repeat, res_repeat);
	EXPECT_EQ(exp_kind_repeat, out_repeat[0].kind);
	EXPECT_EQ(exp_button_repeat, out_repeat[0].button);
}

TEST(TouchPollerTest, RepeatableFlagClearedMidHoldStopsRepeatsButStillEnds) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad1[TOUCH_BUTTONS_NUM] = {false, true, false, false};

	const uint8_t exp_quiet = 1;
	const uint8_t exp_count_end = 1;
	const touch_gesture_kind exp_kind_end = touch_gesture_kind::repeat_end;
	const uint8_t exp_button_end = 1;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;
	touch_gesture_event out_end[1];
	uint8_t res_end;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	poller.SetRepeatable(1, true);
	quiet += poller.Poll(pad1, 1100, scratch, 1);
	quiet += poller.Poll(pad1, 1130, scratch, 1);
	quiet += poller.Poll(pad1, 1730, scratch, 1);
	poller.SetRepeatable(1, false);
	quiet += poller.Poll(pad1, 1930, scratch, 1);
	quiet += poller.Poll(pad1, 2500, scratch, 1);
	quiet += poller.Poll(idle, 2600, scratch, 1);
	res_end = poller.Poll(idle, 2630, out_end, 1);

	EXPECT_EQ(exp_quiet, quiet);
	EXPECT_EQ(exp_count_end, res_end);
	EXPECT_EQ(exp_kind_end, out_end[0].kind);
	EXPECT_EQ(exp_button_end, out_end[0].button);
}

TEST(TouchPollerTest, RepeatableFlagSetAfterAHoldAppliesToTheNextHold) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad3[TOUCH_BUTTONS_NUM] = {false, false, false, true};

	const uint8_t exp_quiet = 0;
	const uint8_t exp_count_repeat = 1;
	const touch_gesture_kind exp_kind_repeat = touch_gesture_kind::repeat;
	const uint8_t exp_button_repeat = 3;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;
	touch_gesture_event out_repeat[1];
	uint8_t res_repeat;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	quiet += poller.Poll(pad3, 1100, scratch, 1);
	quiet += poller.Poll(pad3, 1130, scratch, 1);
	quiet += poller.Poll(pad3, 1900, scratch, 1);
	quiet += poller.Poll(idle, 2000, scratch, 1);
	quiet += poller.Poll(idle, 2030, scratch, 1);
	poller.SetRepeatable(3, true);
	quiet += poller.Poll(pad3, 3000, scratch, 1);
	quiet += poller.Poll(pad3, 3030, scratch, 1);
	res_repeat = poller.Poll(pad3, 3630, out_repeat, 1);

	EXPECT_EQ(exp_quiet, quiet);
	EXPECT_EQ(exp_count_repeat, res_repeat);
	EXPECT_EQ(exp_kind_repeat, out_repeat[0].kind);
	EXPECT_EQ(exp_button_repeat, out_repeat[0].button);
}

TEST(TouchPollerTest, RepeatableFlagOfAnotherButtonDoesNotRepeatTheHeldOne) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad1[TOUCH_BUTTONS_NUM] = {false, true, false, false};

	const uint8_t exp_quiet = 0;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	poller.SetRepeatable(2, true);
	quiet += poller.Poll(pad1, 1100, scratch, 1);
	quiet += poller.Poll(pad1, 1130, scratch, 1);
	quiet += poller.Poll(pad1, 1730, scratch, 1);
	quiet += poller.Poll(pad1, 3000, scratch, 1);

	EXPECT_EQ(exp_quiet, quiet);
}

TEST(TouchPollerTest, RepeatableFlagOutOfRangeIsIgnored) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad0[TOUCH_BUTTONS_NUM] = {true, false, false, false};

	const uint8_t exp_quiet = 0;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	poller.SetRepeatable(TOUCH_BUTTONS_NUM, true);
	quiet += poller.Poll(pad0, 1100, scratch, 1);
	quiet += poller.Poll(pad0, 1130, scratch, 1);
	quiet += poller.Poll(pad0, 1730, scratch, 1);
	quiet += poller.Poll(pad0, 3000, scratch, 1);

	EXPECT_EQ(exp_quiet, quiet);
}

TEST(TouchPollerTest, SecondButtonDuringRepeatGivesRepeatEnd) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad1[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	const bool pad01[TOUCH_BUTTONS_NUM] = {true, true, false, false};

	const uint8_t exp_quiet = 1;
	const uint8_t exp_count_end = 1;
	const touch_gesture_kind exp_kind_end = touch_gesture_kind::repeat_end;
	const uint8_t exp_button_end = 1;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;
	touch_gesture_event out_end[1];
	uint8_t res_end;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	poller.SetRepeatable(1, true);
	quiet += poller.Poll(pad1, 1100, scratch, 1);
	quiet += poller.Poll(pad1, 1130, scratch, 1);
	quiet += poller.Poll(pad1, 1730, scratch, 1);
	quiet += poller.Poll(pad01, 1800, scratch, 1);
	res_end = poller.Poll(pad01, 1830, out_end, 1);

	EXPECT_EQ(exp_quiet, quiet);
	EXPECT_EQ(exp_count_end, res_end);
	EXPECT_EQ(exp_kind_end, out_end[0].kind);
	EXPECT_EQ(exp_button_end, out_end[0].button);
}

TEST(TouchPollerTest, LockPairHeldJustShortOfThreeSecondsGivesNothing) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool lockPair[TOUCH_BUTTONS_NUM] = {false, true, true, false};

	const uint8_t exp_quiet = 0;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	quiet += poller.Poll(lockPair, 1100, scratch, 1);
	quiet += poller.Poll(lockPair, 1130, scratch, 1);
	quiet += poller.Poll(lockPair, 4120, scratch, 1);

	EXPECT_EQ(exp_quiet, quiet);
}

TEST(TouchPollerTest, LockPairHeldThreeSecondsGivesLockChord) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool lockPair[TOUCH_BUTTONS_NUM] = {false, true, true, false};

	const uint8_t exp_quiet = 0;
	const uint8_t exp_count_chord = 1;
	const touch_gesture_kind exp_kind_chord = touch_gesture_kind::lock_chord;
	const uint8_t exp_button_chord = TOUCH_NO_BUTTON;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;
	touch_gesture_event out_chord[1];
	uint8_t res_chord;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	quiet += poller.Poll(lockPair, 1100, scratch, 1);
	quiet += poller.Poll(lockPair, 1130, scratch, 1);
	res_chord = poller.Poll(lockPair, 4130, out_chord, 1);

	EXPECT_EQ(exp_quiet, quiet);
	EXPECT_EQ(exp_count_chord, res_chord);
	EXPECT_EQ(exp_kind_chord, out_chord[0].kind);
	EXPECT_EQ(exp_button_chord, out_chord[0].button);
}

TEST(TouchPollerTest, LockChordFiresOnlyOnce) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool lockPair[TOUCH_BUTTONS_NUM] = {false, true, true, false};

	const uint8_t exp_quiet = 1;
	const uint8_t exp_count_again = 0;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;
	touch_gesture_event out_again[1];
	uint8_t res_again;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	quiet += poller.Poll(lockPair, 1100, scratch, 1);
	quiet += poller.Poll(lockPair, 1130, scratch, 1);
	quiet += poller.Poll(lockPair, 4130, scratch, 1);
	res_again = poller.Poll(lockPair, 8000, out_again, 1);

	EXPECT_EQ(exp_quiet, quiet);
	EXPECT_EQ(exp_count_again, res_again);
}

TEST(TouchPollerTest, ReleaseAfterLockChordGivesNoEvent) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool lockPair[TOUCH_BUTTONS_NUM] = {false, true, true, false};

	const uint8_t exp_quiet = 1;
	const uint8_t exp_count_end = 0;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;
	touch_gesture_event out_end[1];
	uint8_t res_end;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	quiet += poller.Poll(lockPair, 1100, scratch, 1);
	quiet += poller.Poll(lockPair, 1130, scratch, 1);
	quiet += poller.Poll(lockPair, 4130, scratch, 1);
	quiet += poller.Poll(idle, 4200, scratch, 1);
	res_end = poller.Poll(idle, 4230, out_end, 1);

	EXPECT_EQ(exp_quiet, quiet);
	EXPECT_EQ(exp_count_end, res_end);
}

TEST(TouchPollerTest, ApPairHeldThreeSecondsGivesNothing) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool apPair[TOUCH_BUTTONS_NUM] = {true, false, true, false};

	const uint8_t exp_quiet = 0;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	quiet += poller.Poll(apPair, 1100, scratch, 1);
	quiet += poller.Poll(apPair, 1130, scratch, 1);
	quiet += poller.Poll(apPair, 4130, scratch, 1);

	EXPECT_EQ(exp_quiet, quiet);
}

TEST(TouchPollerTest, ApPairHeldJustShortOfTenSecondsGivesNothing) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool apPair[TOUCH_BUTTONS_NUM] = {true, false, true, false};

	const uint8_t exp_quiet = 0;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	quiet += poller.Poll(apPair, 1100, scratch, 1);
	quiet += poller.Poll(apPair, 1130, scratch, 1);
	quiet += poller.Poll(apPair, 11120, scratch, 1);

	EXPECT_EQ(exp_quiet, quiet);
}

TEST(TouchPollerTest, ApPairHeldTenSecondsGivesApChord) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool apPair[TOUCH_BUTTONS_NUM] = {true, false, true, false};

	const uint8_t exp_quiet = 0;
	const uint8_t exp_count_chord = 1;
	const touch_gesture_kind exp_kind_chord = touch_gesture_kind::ap_chord;
	const uint8_t exp_button_chord = TOUCH_NO_BUTTON;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;
	touch_gesture_event out_chord[1];
	uint8_t res_chord;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	quiet += poller.Poll(apPair, 1100, scratch, 1);
	quiet += poller.Poll(apPair, 1130, scratch, 1);
	res_chord = poller.Poll(apPair, 11130, out_chord, 1);

	EXPECT_EQ(exp_quiet, quiet);
	EXPECT_EQ(exp_count_chord, res_chord);
	EXPECT_EQ(exp_kind_chord, out_chord[0].kind);
	EXPECT_EQ(exp_button_chord, out_chord[0].button);
}

TEST(TouchPollerTest, FormerApPairOfPads2And9HeldTwelveSecondsGivesNothing) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad2and9[TOUCH_BUTTONS_NUM] = {true, false, false, true};

	const uint8_t exp_quiet = 0;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1030, scratch, 1);
	quiet += poller.Poll(pad2and9, 1100, scratch, 1);
	quiet += poller.Poll(pad2and9, 1130, scratch, 1);
	quiet += poller.Poll(pad2and9, 13130, scratch, 1);

	EXPECT_EQ(exp_quiet, quiet);
}

TEST(TouchPollerTest, SwitchFromLockPairToApPairGivesApChordTenSecondsAfterTheSwitch) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool lockPair[TOUCH_BUTTONS_NUM] = {false, true, true, false};
	const bool apPair[TOUCH_BUTTONS_NUM] = {true, false, true, false};

	const uint8_t exp_count_chord = 1;
	const touch_gesture_kind exp_kind_chord = touch_gesture_kind::ap_chord;
	const uint8_t exp_button_chord = TOUCH_NO_BUTTON;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	touch_gesture_event out_chord[1];
	uint8_t res_chord;

	poller.Poll(idle, 1000, scratch, 1);
	poller.Poll(idle, 1030, scratch, 1);
	poller.Poll(lockPair, 1100, scratch, 1);
	poller.Poll(lockPair, 1130, scratch, 1);
	poller.Poll(lockPair, 4130, scratch, 1);
	poller.Poll(apPair, 4200, scratch, 1);
	poller.Poll(apPair, 4230, scratch, 1);

	res_chord = poller.Poll(apPair, 14230, out_chord, 1);

	EXPECT_EQ(exp_count_chord, res_chord);
	EXPECT_EQ(exp_kind_chord, out_chord[0].kind);
	EXPECT_EQ(exp_button_chord, out_chord[0].button);
}

TEST(TouchPollerTest, PadHeldAtBootGivesNoEventWhileHeldOrOnRelease) {
	const bool pad0[TOUCH_BUTTONS_NUM] = {true, false, false, false};
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};

	const uint8_t exp_quiet = 0;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;

	quiet += poller.Poll(pad0, 1000, scratch, 1);
	quiet += poller.Poll(pad0, 1030, scratch, 1);
	quiet += poller.Poll(pad0, 1100, scratch, 1);
	quiet += poller.Poll(pad0, 2000, scratch, 1);
	quiet += poller.Poll(idle, 2100, scratch, 1);
	quiet += poller.Poll(idle, 2130, scratch, 1);

	EXPECT_EQ(exp_quiet, quiet);
}

TEST(TouchPollerTest, TapAfterReleaseOfBootHeldPadIsReported) {
	const bool pad0[TOUCH_BUTTONS_NUM] = {true, false, false, false};
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};

	const uint8_t exp_quiet = 0;
	const uint8_t exp_count_tap = 1;
	const touch_gesture_kind exp_kind_tap = touch_gesture_kind::tap;
	const uint8_t exp_button_tap = 0;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;
	touch_gesture_event out_tap[1];
	uint8_t res_tap;

	quiet += poller.Poll(pad0, 1000, scratch, 1);
	quiet += poller.Poll(pad0, 1030, scratch, 1);
	quiet += poller.Poll(idle, 1200, scratch, 1);
	quiet += poller.Poll(idle, 1230, scratch, 1);
	quiet += poller.Poll(pad0, 1300, scratch, 1);
	quiet += poller.Poll(pad0, 1330, scratch, 1);
	quiet += poller.Poll(idle, 1400, scratch, 1);
	res_tap = poller.Poll(idle, 1430, out_tap, 1);

	EXPECT_EQ(exp_quiet, quiet);
	EXPECT_EQ(exp_count_tap, res_tap);
	EXPECT_EQ(exp_kind_tap, out_tap[0].kind);
	EXPECT_EQ(exp_button_tap, out_tap[0].button);
}

TEST(TouchPollerTest, PressInsideTheBootGateIsATap) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad0[TOUCH_BUTTONS_NUM] = {true, false, false, false};

	const uint8_t exp_quiet = 0;
	const uint8_t exp_count_tap = 1;
	const touch_gesture_kind exp_kind_tap = touch_gesture_kind::tap;
	const uint8_t exp_button_tap = 0;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;
	touch_gesture_event out_tap[1];
	uint8_t res_tap;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(pad0, 1010, scratch, 1);
	quiet += poller.Poll(pad0, 1030, scratch, 1);
	quiet += poller.Poll(pad0, 1040, scratch, 1);
	quiet += poller.Poll(idle, 1100, scratch, 1);
	res_tap = poller.Poll(idle, 1130, out_tap, 1);

	EXPECT_EQ(exp_quiet, quiet);
	EXPECT_EQ(exp_count_tap, res_tap);
	EXPECT_EQ(exp_kind_tap, out_tap[0].kind);
	EXPECT_EQ(exp_button_tap, out_tap[0].button);
}

TEST(TouchPollerTest, LockPairHeldAtBootGivesNoChord) {
	const bool lockPair[TOUCH_BUTTONS_NUM] = {false, true, true, false};

	const uint8_t exp_quiet = 0;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;

	quiet += poller.Poll(lockPair, 1000, scratch, 1);
	quiet += poller.Poll(lockPair, 1030, scratch, 1);
	quiet += poller.Poll(lockPair, 4100, scratch, 1);
	quiet += poller.Poll(lockPair, 5000, scratch, 1);

	EXPECT_EQ(exp_quiet, quiet);
}

TEST(TouchPollerTest, ApPairHeldAtBootGivesNoChord) {
	const bool apPair[TOUCH_BUTTONS_NUM] = {true, false, true, false};

	const uint8_t exp_quiet = 0;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;

	quiet += poller.Poll(apPair, 1000, scratch, 1);
	quiet += poller.Poll(apPair, 1030, scratch, 1);
	quiet += poller.Poll(apPair, 12000, scratch, 1);

	EXPECT_EQ(exp_quiet, quiet);
}

TEST(TouchPollerTest, OtherPadPressedWhileAPadIsBootHeldIsATap) {
	const bool pad0[TOUCH_BUTTONS_NUM] = {true, false, false, false};
	const bool pad01[TOUCH_BUTTONS_NUM] = {true, true, false, false};

	const uint8_t exp_quiet = 0;
	const uint8_t exp_count_tap = 1;
	const touch_gesture_kind exp_kind_tap = touch_gesture_kind::tap;
	const uint8_t exp_button_tap = 1;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;
	touch_gesture_event out_tap[1];
	uint8_t res_tap;

	quiet += poller.Poll(pad0, 1000, scratch, 1);
	quiet += poller.Poll(pad0, 1030, scratch, 1);
	quiet += poller.Poll(pad01, 1100, scratch, 1);
	quiet += poller.Poll(pad01, 1130, scratch, 1);
	quiet += poller.Poll(pad0, 1200, scratch, 1);
	res_tap = poller.Poll(pad0, 1230, out_tap, 1);

	EXPECT_EQ(exp_quiet, quiet);
	EXPECT_EQ(exp_count_tap, res_tap);
	EXPECT_EQ(exp_kind_tap, out_tap[0].kind);
	EXPECT_EQ(exp_button_tap, out_tap[0].button);
}

TEST(TouchPollerTest, PadHeldAtBootThenPairAfterReleaseGivesLockChord) {
	const bool pad1[TOUCH_BUTTONS_NUM] = {false, true, false, false};
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool lockPair[TOUCH_BUTTONS_NUM] = {false, true, true, false};

	const uint8_t exp_quiet = 0;
	const uint8_t exp_count_chord = 1;
	const touch_gesture_kind exp_kind_chord = touch_gesture_kind::lock_chord;
	const uint8_t exp_button_chord = TOUCH_NO_BUTTON;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;
	touch_gesture_event out_chord[1];
	uint8_t res_chord;

	quiet += poller.Poll(pad1, 1000, scratch, 1);
	quiet += poller.Poll(pad1, 1030, scratch, 1);
	quiet += poller.Poll(idle, 1100, scratch, 1);
	quiet += poller.Poll(idle, 1130, scratch, 1);
	quiet += poller.Poll(lockPair, 1200, scratch, 1);
	quiet += poller.Poll(lockPair, 1230, scratch, 1);
	res_chord = poller.Poll(lockPair, 4230, out_chord, 1);

	EXPECT_EQ(exp_quiet, quiet);
	EXPECT_EQ(exp_count_chord, res_chord);
	EXPECT_EQ(exp_kind_chord, out_chord[0].kind);
	EXPECT_EQ(exp_button_chord, out_chord[0].button);
}

TEST(TouchPollerTest, FirstPollsInsideTheBootGateGiveNoEvent) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};

	const uint8_t exp_quiet = 0;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;

	quiet += poller.Poll(idle, 1000, scratch, 1);
	quiet += poller.Poll(idle, 1010, scratch, 1);
	quiet += poller.Poll(idle, 1029, scratch, 1);

	EXPECT_EQ(exp_quiet, quiet);
}

TEST(TouchPollerTest, TapAcrossTheMillisWrapIsReported) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad2[TOUCH_BUTTONS_NUM] = {false, false, true, false};
	const uint32_t start = UINT32_MAX - 49;

	const uint8_t exp_quiet = 0;
	const uint8_t exp_count_tap = 1;
	const touch_gesture_kind exp_kind_tap = touch_gesture_kind::tap;
	const uint8_t exp_button_tap = 2;

	HAQuDA_TouchPoller poller;
	touch_gesture_event scratch[1];
	uint8_t quiet = 0;
	touch_gesture_event out_tap[1];
	uint8_t res_tap;

	quiet += poller.Poll(idle, start, scratch, 1);
	quiet += poller.Poll(idle, start + 30, scratch, 1);
	quiet += poller.Poll(pad2, start + 100, scratch, 1);
	quiet += poller.Poll(pad2, start + 130, scratch, 1);
	quiet += poller.Poll(idle, start + 200, scratch, 1);
	res_tap = poller.Poll(idle, start + 230, out_tap, 1);

	EXPECT_EQ(exp_quiet, quiet);
	EXPECT_EQ(exp_count_tap, res_tap);
	EXPECT_EQ(exp_kind_tap, out_tap[0].kind);
	EXPECT_EQ(exp_button_tap, out_tap[0].button);
}

TEST(TouchPollerTest, CapacityZeroWritesNothing) {
	const bool idle[TOUCH_BUTTONS_NUM] = {false, false, false, false};
	const bool pad0[TOUCH_BUTTONS_NUM] = {true, false, false, false};

	const uint8_t exp_count = 0;

	HAQuDA_TouchPoller poller;
	touch_gesture_event out[1];
	uint8_t res = 0;

	res += poller.Poll(idle, 1000, out, 0);
	res += poller.Poll(idle, 1030, out, 0);
	res += poller.Poll(pad0, 1100, out, 0);
	res += poller.Poll(pad0, 1130, out, 0);
	res += poller.Poll(idle, 1200, out, 0);
	res += poller.Poll(idle, 1230, out, 0);

	EXPECT_EQ(exp_count, res);
}
