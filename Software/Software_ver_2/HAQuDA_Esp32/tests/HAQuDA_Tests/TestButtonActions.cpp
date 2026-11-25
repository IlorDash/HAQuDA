#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>

#include "HAQuDA_ButtonActions.h"
#include "MockApRequest.h"
#include "MockFileStorage.h"
#include "MockLampActions.h"
#include "MockLogger.h"

using ::testing::InSequence;
using ::testing::Return;
using ::testing::StrEq;
using ::testing::StrictMock;

TEST(ButtonActionsTest, RepeatableMaskOfDefaultsIsUpAndDown) {
	const uint8_t btnUp = 1;
	const uint8_t btnDown = 3;

	const uint8_t exp_mask = (1 << btnUp) | (1 << btnDown);

	const buttons_settings settings = HAQuDA_ButtonsDefaults();

	uint8_t res;

	res = HAQuDA_RepeatableMask(settings);

	EXPECT_EQ(exp_mask, res);
}

TEST(ButtonActionsTest, RepeatableMaskFollowsTheBindings) {
	const uint8_t btnPower = 0;
	const uint8_t btnUp = 1;
	const uint8_t btnDown = 3;

	const uint8_t exp_mask = (1 << btnPower);

	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.buttons[btnPower] = {button_action::down, lamp_mode::standard, lamp_param::total};
	settings.buttons[btnUp] = {button_action::none, lamp_mode::standard, lamp_param::total};
	settings.buttons[btnDown] = {button_action::show, lamp_mode::standard, lamp_param::eco2};

	uint8_t res;

	res = HAQuDA_RepeatableMask(settings);

	EXPECT_EQ(exp_mask, res);
}

TEST(ButtonActionsTest, RepeatableMaskWithNoBrightnessButtonIsZero) {
	const uint8_t btnUp = 1;
	const uint8_t btnDown = 3;

	const uint8_t exp_mask = 0;

	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.buttons[btnUp] = {button_action::none, lamp_mode::standard, lamp_param::total};
	settings.buttons[btnDown] = {button_action::next, lamp_mode::standard, lamp_param::total};

	uint8_t res;

	res = HAQuDA_RepeatableMask(settings);

	EXPECT_EQ(exp_mask, res);
}

TEST(ButtonActionsTest, RepeatableMaskOfFourBrightnessButtonsIsAllBits) {
	const uint8_t btnPower = 0;
	const uint8_t btnNext = 2;

	const uint8_t exp_mask = 0x0F;

	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.buttons[btnPower] = {button_action::up, lamp_mode::standard, lamp_param::total};
	settings.buttons[btnNext] = {button_action::down, lamp_mode::standard, lamp_param::total};

	uint8_t res;

	res = HAQuDA_RepeatableMask(settings);

	EXPECT_EQ(exp_mask, res);
}

TEST(ButtonActionsTest, TapPowerTogglesPower) {
	const uint8_t btnPower = 0;

	const touch_gesture_event events[] = {{btnPower, touch_gesture_kind::tap}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, TogglePower());
	EXPECT_CALL(logger, LogInfo(StrEq("Touch 2: power")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, TapUpStepsUpThenCommitsThenLogs) {
	const uint8_t btnUp = 1;

	const touch_gesture_event events[] = {{btnUp, touch_gesture_kind::tap}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, StepBrightness(true)).WillOnce(Return(true));
	EXPECT_CALL(lamp, CommitBrightness());
	EXPECT_CALL(logger, LogInfo(StrEq("Touch 6: brighter")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, TapUpAtTheLimitStillCommits) {
	const uint8_t btnUp = 1;

	const touch_gesture_event events[] = {{btnUp, touch_gesture_kind::tap}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, StepBrightness(true)).WillOnce(Return(false));
	EXPECT_CALL(lamp, CommitBrightness());
	EXPECT_CALL(logger, LogInfo(StrEq("Touch 6: brighter")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, TapDownStepsDownThenCommitsThenLogs) {
	const uint8_t btnDown = 3;

	const touch_gesture_event events[] = {{btnDown, touch_gesture_kind::tap}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, StepBrightness(false)).WillOnce(Return(true));
	EXPECT_CALL(lamp, CommitBrightness());
	EXPECT_CALL(logger, LogInfo(StrEq("Touch 9: dimmer")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, TapDownAtTheLimitStillCommits) {
	const uint8_t btnDown = 3;

	const touch_gesture_event events[] = {{btnDown, touch_gesture_kind::tap}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, StepBrightness(false)).WillOnce(Return(false));
	EXPECT_CALL(lamp, CommitBrightness());
	EXPECT_CALL(logger, LogInfo(StrEq("Touch 9: dimmer")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, TapNextCallsNextMode) {
	const uint8_t btnNext = 2;

	const touch_gesture_event events[] = {{btnNext, touch_gesture_kind::tap}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, NextMode());
	EXPECT_CALL(logger, LogInfo(StrEq("Touch 8: next mode")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, TapModeSetsTheBoundMode) {
	const uint8_t btnNext = 2;

	const touch_gesture_event events[] = {{btnNext, touch_gesture_kind::tap}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.buttons[btnNext] = {button_action::mode, lamp_mode::night, lamp_param::total};

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, SetMode(lamp_mode::night)).WillOnce(Return(true));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch 8: mode")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, TapModeMultiSetsTheBoundMode) {
	const uint8_t btnPower = 0;

	const touch_gesture_event events[] = {{btnPower, touch_gesture_kind::tap}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.buttons[btnPower] = {button_action::mode, lamp_mode::multi, lamp_param::total};

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, SetMode(lamp_mode::multi)).WillOnce(Return(true));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch 2: mode")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, TapModeRefusedFlashesRefusedThenLogs) {
	const uint8_t btnNext = 2;

	const touch_gesture_event events[] = {{btnNext, touch_gesture_kind::tap}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.buttons[btnNext] = {button_action::mode, lamp_mode::night, lamp_param::total};

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, SetMode(lamp_mode::night)).WillOnce(Return(false));
	EXPECT_CALL(lamp, Flash(lamp_flash::refused));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch 8: mode refused")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, TapShowShowsTheBoundParameter) {
	const uint8_t btnPower = 0;

	const touch_gesture_event events[] = {{btnPower, touch_gesture_kind::tap}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.buttons[btnPower] = {button_action::show, lamp_mode::standard, lamp_param::eco2};

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, ShowReading(lamp_param::eco2)).WillOnce(Return(true));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch 2: show reading")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, TapShowRefusedFlashesRefusedThenLogs) {
	const uint8_t btnPower = 0;

	const touch_gesture_event events[] = {{btnPower, touch_gesture_kind::tap}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.buttons[btnPower] = {button_action::show, lamp_mode::standard, lamp_param::tvoc};

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, ShowReading(lamp_param::tvoc)).WillOnce(Return(false));
	EXPECT_CALL(lamp, Flash(lamp_flash::refused));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch 2: show reading refused")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, TapNoneDoesNothingAndLogsNothing) {
	const uint8_t btnPower = 0;

	const touch_gesture_event events[] = {{btnPower, touch_gesture_kind::tap}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.buttons[btnPower] = {button_action::none, lamp_mode::standard, lamp_param::total};

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, TapOnTheLastButtonUsesItsOwnBinding) {
	const uint8_t btnDown = 3;

	const touch_gesture_event events[] = {{btnDown, touch_gesture_kind::tap}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.buttons[btnDown] = {button_action::power, lamp_mode::standard, lamp_param::total};

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, TogglePower());
	EXPECT_CALL(logger, LogInfo(StrEq("Touch 9: power")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, RepeatUpStepsUpWithoutCommitAndLog) {
	const uint8_t btnUp = 1;

	const touch_gesture_event events[] = {{btnUp, touch_gesture_kind::repeat}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	EXPECT_CALL(lamp, StepBrightness(true)).WillOnce(Return(true));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, RepeatDownStepsDownWithoutCommitAndLog) {
	const uint8_t btnDown = 3;

	const touch_gesture_event events[] = {{btnDown, touch_gesture_kind::repeat}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	EXPECT_CALL(lamp, StepBrightness(false)).WillOnce(Return(true));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, RepeatOnANonBrightnessButtonDoesNothing) {
	const uint8_t btnNext = 2;

	const touch_gesture_event events[] = {{btnNext, touch_gesture_kind::repeat}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, RepeatOnAReboundShowButtonDoesNothing) {
	const uint8_t btnUp = 1;

	const touch_gesture_event events[] = {{btnUp, touch_gesture_kind::repeat}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.buttons[btnUp] = {button_action::show, lamp_mode::standard, lamp_param::pm25};

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, RepeatEndCommitsThenLogsHeld) {
	const uint8_t btnUp = 1;

	const touch_gesture_event events[] = {{btnUp, touch_gesture_kind::repeat_end}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, CommitBrightness());
	EXPECT_CALL(logger, LogInfo(StrEq("Touch 6: brighter, held")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, RepeatEndOfDownLogsDimmerHeld) {
	const uint8_t btnDown = 3;

	const touch_gesture_event events[] = {{btnDown, touch_gesture_kind::repeat_end}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, CommitBrightness());
	EXPECT_CALL(logger, LogInfo(StrEq("Touch 9: dimmer, held")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, RepeatEndOnAReboundButtonCommitsWithoutLog) {
	const uint8_t btnPower = 0;

	const touch_gesture_event events[] = {{btnPower, touch_gesture_kind::repeat_end}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	EXPECT_CALL(lamp, CommitBrightness());

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, RepeatEndWhenLockedCommitsWithoutLog) {
	const uint8_t btnUp = 1;

	const touch_gesture_event events[] = {{btnUp, touch_gesture_kind::repeat_end}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.locked = true;

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	EXPECT_CALL(lamp, CommitBrightness());

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, RepeatEndOutOfRangeCommitsWithoutLog) {
	const touch_gesture_event events[] = {{TOUCH_BUTTONS_NUM, touch_gesture_kind::repeat_end}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	EXPECT_CALL(lamp, CommitBrightness());

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, RunOfRepeatsOfOneButtonDoesOneStep) {
	const uint8_t btnUp = 1;

	const touch_gesture_event events[] = {{btnUp, touch_gesture_kind::repeat}, {btnUp, touch_gesture_kind::repeat}, {btnUp, touch_gesture_kind::repeat}};
	const uint8_t eventCount = 3;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	EXPECT_CALL(lamp, StepBrightness(true)).Times(1).WillOnce(Return(true));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, RunOfRepeatsThenRepeatEndStepsOnceCommitsThenLogs) {
	const uint8_t btnDown = 3;

	const touch_gesture_event events[]
		= {{btnDown, touch_gesture_kind::repeat}, {btnDown, touch_gesture_kind::repeat}, {btnDown, touch_gesture_kind::repeat_end}};
	const uint8_t eventCount = 3;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, StepBrightness(false)).WillOnce(Return(true));
	EXPECT_CALL(lamp, CommitBrightness());
	EXPECT_CALL(logger, LogInfo(StrEq("Touch 9: dimmer, held")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, RepeatsOfDifferentButtonsAreNotCollapsed) {
	const uint8_t btnUp = 1;
	const uint8_t btnDown = 3;

	const touch_gesture_event events[] = {{btnUp, touch_gesture_kind::repeat}, {btnDown, touch_gesture_kind::repeat}};
	const uint8_t eventCount = 2;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, StepBrightness(true)).WillOnce(Return(true));
	EXPECT_CALL(lamp, StepBrightness(false)).WillOnce(Return(true));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, RepeatsSeparatedByATapAreNotCollapsed) {
	const uint8_t btnPower = 0;
	const uint8_t btnUp = 1;

	const touch_gesture_event events[] = {{btnUp, touch_gesture_kind::repeat}, {btnPower, touch_gesture_kind::tap}, {btnUp, touch_gesture_kind::repeat}};
	const uint8_t eventCount = 3;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, StepBrightness(true)).WillOnce(Return(true));
	EXPECT_CALL(lamp, TogglePower());
	EXPECT_CALL(logger, LogInfo(StrEq("Touch 2: power")));
	EXPECT_CALL(lamp, StepBrightness(true)).WillOnce(Return(true));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, RepeatsSeparatedByRepeatEndAreNotCollapsed) {
	const uint8_t btnUp = 1;

	const touch_gesture_event events[] = {{btnUp, touch_gesture_kind::repeat}, {btnUp, touch_gesture_kind::repeat_end}, {btnUp, touch_gesture_kind::repeat}};
	const uint8_t eventCount = 3;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, StepBrightness(true)).WillOnce(Return(true));
	EXPECT_CALL(lamp, CommitBrightness());
	EXPECT_CALL(logger, LogInfo(StrEq("Touch 6: brighter, held")));
	EXPECT_CALL(lamp, StepBrightness(true)).WillOnce(Return(true));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, RunsOfTwoButtonsAreCollapsedEach) {
	const uint8_t btnUp = 1;
	const uint8_t btnDown = 3;

	const touch_gesture_event events[] = {
		{btnUp, touch_gesture_kind::repeat}, {btnUp, touch_gesture_kind::repeat}, {btnDown, touch_gesture_kind::repeat}, {btnDown, touch_gesture_kind::repeat}};
	const uint8_t eventCount = 4;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, StepBrightness(true)).WillOnce(Return(true));
	EXPECT_CALL(lamp, StepBrightness(false)).WillOnce(Return(true));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, RunOfRepeatsAfterATapOfTheSameButtonStepsTwiceInTotal) {
	const uint8_t btnUp = 1;

	const touch_gesture_event events[] = {{btnUp, touch_gesture_kind::tap}, {btnUp, touch_gesture_kind::repeat}, {btnUp, touch_gesture_kind::repeat}};
	const uint8_t eventCount = 3;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, StepBrightness(true)).WillOnce(Return(true));
	EXPECT_CALL(lamp, CommitBrightness());
	EXPECT_CALL(logger, LogInfo(StrEq("Touch 6: brighter")));
	EXPECT_CALL(lamp, StepBrightness(true)).WillOnce(Return(true));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, LockedIgnoresTap) {
	const uint8_t btnPower = 0;

	const touch_gesture_event events[] = {{btnPower, touch_gesture_kind::tap}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.locked = true;

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, LockedIgnoresRepeat) {
	const uint8_t btnUp = 1;

	const touch_gesture_event events[] = {{btnUp, touch_gesture_kind::repeat}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.locked = true;

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, LockedIgnoresModeAndShowTaps) {
	const uint8_t btnPower = 0;
	const uint8_t btnNext = 2;

	const touch_gesture_event events[] = {{btnNext, touch_gesture_kind::tap}, {btnPower, touch_gesture_kind::tap}};
	const uint8_t eventCount = 2;
	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.locked = true;
	settings.buttons[btnNext] = {button_action::mode, lamp_mode::night, lamp_param::total};
	settings.buttons[btnPower] = {button_action::show, lamp_mode::standard, lamp_param::eco2};

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, LockChordLocksFlashesLockedThenLogs) {
	const touch_gesture_event events[] = {{TOUCH_NO_BUTTON, touch_gesture_kind::lock_chord}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, Flash(lamp_flash::locked));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch buttons locked")));

	actions.Handle(events, eventCount, settings);

	EXPECT_TRUE(control.CopySettings().locked);
}

TEST(ButtonActionsTest, LockChordWhenLockedUnlocksFlashesUnlockedThenLogs) {
	const touch_gesture_event events[] = {{TOUCH_NO_BUTTON, touch_gesture_kind::lock_chord}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.locked = true;

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);
	control.SetLocked(true);

	InSequence seq;
	EXPECT_CALL(lamp, Flash(lamp_flash::unlocked));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch buttons unlocked")));

	actions.Handle(events, eventCount, settings);

	EXPECT_FALSE(control.CopySettings().locked);
}

TEST(ButtonActionsTest, LockChordTwiceInOneBatchLocksThenUnlocks) {
	const touch_gesture_event events[] = {{TOUCH_NO_BUTTON, touch_gesture_kind::lock_chord}, {TOUCH_NO_BUTTON, touch_gesture_kind::lock_chord}};
	const uint8_t eventCount = 2;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, Flash(lamp_flash::locked));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch buttons locked")));
	EXPECT_CALL(lamp, Flash(lamp_flash::unlocked));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch buttons unlocked")));

	actions.Handle(events, eventCount, settings);

	EXPECT_FALSE(control.CopySettings().locked);
}

TEST(ButtonActionsTest, TapAfterTheLockChordInTheSameBatchIsIgnored) {
	const uint8_t btnPower = 0;

	const touch_gesture_event events[] = {{TOUCH_NO_BUTTON, touch_gesture_kind::lock_chord}, {btnPower, touch_gesture_kind::tap}};
	const uint8_t eventCount = 2;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, Flash(lamp_flash::locked));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch buttons locked")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, RepeatAfterTheLockChordInTheSameBatchIsIgnored) {
	const uint8_t btnUp = 1;

	const touch_gesture_event events[] = {{TOUCH_NO_BUTTON, touch_gesture_kind::lock_chord}, {btnUp, touch_gesture_kind::repeat}};
	const uint8_t eventCount = 2;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, Flash(lamp_flash::locked));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch buttons locked")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, RepeatEndAfterTheLockChordInTheSameBatchCommitsWithoutHeldLog) {
	const uint8_t btnUp = 1;

	const touch_gesture_event events[] = {{TOUCH_NO_BUTTON, touch_gesture_kind::lock_chord}, {btnUp, touch_gesture_kind::repeat_end}};
	const uint8_t eventCount = 2;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, Flash(lamp_flash::locked));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch buttons locked")));
	EXPECT_CALL(lamp, CommitBrightness());

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, TapAfterTheUnlockChordInTheSameBatchWorks) {
	const uint8_t btnPower = 0;

	const touch_gesture_event events[] = {{TOUCH_NO_BUTTON, touch_gesture_kind::lock_chord}, {btnPower, touch_gesture_kind::tap}};
	const uint8_t eventCount = 2;
	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.locked = true;

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);
	control.SetLocked(true);

	InSequence seq;
	EXPECT_CALL(lamp, Flash(lamp_flash::unlocked));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch buttons unlocked")));
	EXPECT_CALL(lamp, TogglePower());
	EXPECT_CALL(logger, LogInfo(StrEq("Touch 2: power")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, TapBeforeTheLockChordInTheSameBatchWorks) {
	const uint8_t btnPower = 0;

	const touch_gesture_event events[] = {{btnPower, touch_gesture_kind::tap}, {TOUCH_NO_BUTTON, touch_gesture_kind::lock_chord}};
	const uint8_t eventCount = 2;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, TogglePower());
	EXPECT_CALL(logger, LogInfo(StrEq("Touch 2: power")));
	EXPECT_CALL(lamp, Flash(lamp_flash::locked));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch buttons locked")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, ApChordRequestsTheAccessPointThenLogs) {
	const touch_gesture_event events[] = {{TOUCH_NO_BUTTON, touch_gesture_kind::ap_chord}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(apRequest, RequestAccessPoint()).WillOnce(Return(true));
	EXPECT_CALL(lamp, Flash(lamp_flash::access_point));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch buttons: restarting into the access point")));

	actions.Handle(events, eventCount, settings);

	EXPECT_FALSE(control.CopySettings().locked);
}

TEST(ButtonActionsTest, ApChordRefusedFlashesRefusedThenLogs) {
	const touch_gesture_event events[] = {{TOUCH_NO_BUTTON, touch_gesture_kind::ap_chord}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(apRequest, RequestAccessPoint()).WillOnce(Return(false));
	EXPECT_CALL(lamp, Flash(lamp_flash::refused));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch buttons: access point refused")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, ApChordWhenLockedStillRequestsTheAccessPoint) {
	const touch_gesture_event events[] = {{TOUCH_NO_BUTTON, touch_gesture_kind::ap_chord}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.locked = true;

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(apRequest, RequestAccessPoint()).WillOnce(Return(true));
	EXPECT_CALL(lamp, Flash(lamp_flash::access_point));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch buttons: restarting into the access point")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, ApChordWhenLockedRefusedFlashesRefused) {
	const touch_gesture_event events[] = {{TOUCH_NO_BUTTON, touch_gesture_kind::ap_chord}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.locked = true;

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(apRequest, RequestAccessPoint()).WillOnce(Return(false));
	EXPECT_CALL(lamp, Flash(lamp_flash::refused));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch buttons: access point refused")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, TapBeforeTheApChordInTheSameBatchWorks) {
	const uint8_t btnPower = 0;

	const touch_gesture_event events[] = {{btnPower, touch_gesture_kind::tap}, {TOUCH_NO_BUTTON, touch_gesture_kind::ap_chord}};
	const uint8_t eventCount = 2;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, TogglePower());
	EXPECT_CALL(logger, LogInfo(StrEq("Touch 2: power")));
	EXPECT_CALL(apRequest, RequestAccessPoint()).WillOnce(Return(true));
	EXPECT_CALL(lamp, Flash(lamp_flash::access_point));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch buttons: restarting into the access point")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, TapAfterTheRefusedApChordInTheSameBatchWorks) {
	const uint8_t btnPower = 0;

	const touch_gesture_event events[] = {{TOUCH_NO_BUTTON, touch_gesture_kind::ap_chord}, {btnPower, touch_gesture_kind::tap}};
	const uint8_t eventCount = 2;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(apRequest, RequestAccessPoint()).WillOnce(Return(false));
	EXPECT_CALL(lamp, Flash(lamp_flash::refused));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch buttons: access point refused")));
	EXPECT_CALL(lamp, TogglePower());
	EXPECT_CALL(logger, LogInfo(StrEq("Touch 2: power")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, LockChordAfterTheApChordInTheSameBatchStillLocks) {
	const touch_gesture_event events[] = {{TOUCH_NO_BUTTON, touch_gesture_kind::ap_chord}, {TOUCH_NO_BUTTON, touch_gesture_kind::lock_chord}};
	const uint8_t eventCount = 2;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(apRequest, RequestAccessPoint()).WillOnce(Return(true));
	EXPECT_CALL(lamp, Flash(lamp_flash::access_point));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch buttons: restarting into the access point")));
	EXPECT_CALL(lamp, Flash(lamp_flash::locked));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch buttons locked")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, TapAfterTheLockChordAndTheApChordInTheSameBatchIsIgnored) {
	const uint8_t btnPower = 0;

	const touch_gesture_event events[]
		= {{TOUCH_NO_BUTTON, touch_gesture_kind::lock_chord}, {TOUCH_NO_BUTTON, touch_gesture_kind::ap_chord}, {btnPower, touch_gesture_kind::tap}};
	const uint8_t eventCount = 3;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, Flash(lamp_flash::locked));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch buttons locked")));
	EXPECT_CALL(apRequest, RequestAccessPoint()).WillOnce(Return(true));
	EXPECT_CALL(lamp, Flash(lamp_flash::access_point));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch buttons: restarting into the access point")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, TwoApChordsInOneBatchAskTwiceAndSecondRefusedFlashes) {
	const touch_gesture_event events[] = {{TOUCH_NO_BUTTON, touch_gesture_kind::ap_chord}, {TOUCH_NO_BUTTON, touch_gesture_kind::ap_chord}};
	const uint8_t eventCount = 2;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(apRequest, RequestAccessPoint()).WillOnce(Return(true));
	EXPECT_CALL(lamp, Flash(lamp_flash::access_point));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch buttons: restarting into the access point")));
	EXPECT_CALL(apRequest, RequestAccessPoint()).WillOnce(Return(false));
	EXPECT_CALL(lamp, Flash(lamp_flash::refused));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch buttons: access point refused")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, RepeatsSeparatedByTheApChordAreNotCollapsed) {
	const uint8_t btnUp = 1;

	const touch_gesture_event events[]
		= {{btnUp, touch_gesture_kind::repeat}, {TOUCH_NO_BUTTON, touch_gesture_kind::ap_chord}, {btnUp, touch_gesture_kind::repeat}};
	const uint8_t eventCount = 3;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, StepBrightness(true)).WillOnce(Return(true));
	EXPECT_CALL(apRequest, RequestAccessPoint()).WillOnce(Return(false));
	EXPECT_CALL(lamp, Flash(lamp_flash::refused));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch buttons: access point refused")));
	EXPECT_CALL(lamp, StepBrightness(true)).WillOnce(Return(true));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, RepeatEndAfterTheApChordStillCommitsAndLogsHeld) {
	const uint8_t btnUp = 1;

	const touch_gesture_event events[]
		= {{btnUp, touch_gesture_kind::repeat}, {TOUCH_NO_BUTTON, touch_gesture_kind::ap_chord}, {btnUp, touch_gesture_kind::repeat_end}};
	const uint8_t eventCount = 3;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, StepBrightness(true)).WillOnce(Return(true));
	EXPECT_CALL(apRequest, RequestAccessPoint()).WillOnce(Return(true));
	EXPECT_CALL(lamp, Flash(lamp_flash::access_point));
	EXPECT_CALL(logger, LogInfo(StrEq("Touch buttons: restarting into the access point")));
	EXPECT_CALL(lamp, CommitBrightness());
	EXPECT_CALL(logger, LogInfo(StrEq("Touch 6: brighter, held")));

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, TapOutOfRangeIsIgnored) {
	const touch_gesture_event events[] = {{TOUCH_BUTTONS_NUM, touch_gesture_kind::tap}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, RepeatOfNoButtonIsIgnored) {
	const touch_gesture_event events[] = {{TOUCH_NO_BUTTON, touch_gesture_kind::repeat}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, NullEventsAreIgnored) {
	const uint8_t eventCount = 3;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	actions.Handle(nullptr, eventCount, settings);
}

TEST(ButtonActionsTest, CountZeroHandlesNothing) {
	const uint8_t btnPower = 0;

	const touch_gesture_event events[] = {{btnPower, touch_gesture_kind::tap}};
	const uint8_t eventCount = 0;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	actions.Handle(events, eventCount, settings);
}

TEST(ButtonActionsTest, CountLimitsTheEventsHandled) {
	const uint8_t btnPower = 0;
	const uint8_t btnNext = 2;

	const touch_gesture_event events[] = {{btnPower, touch_gesture_kind::tap}, {btnNext, touch_gesture_kind::tap}};
	const uint8_t eventCount = 1;
	buttons_settings settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockLampActions> lamp;
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	StrictMock<MockApRequest> apRequest;
	HAQuDA_ButtonsControl control(files, logger);
	HAQuDA_ButtonActions actions(lamp, control, logger, apRequest);

	InSequence seq;
	EXPECT_CALL(lamp, TogglePower());
	EXPECT_CALL(logger, LogInfo(StrEq("Touch 2: power")));

	actions.Handle(events, eventCount, settings);
}
