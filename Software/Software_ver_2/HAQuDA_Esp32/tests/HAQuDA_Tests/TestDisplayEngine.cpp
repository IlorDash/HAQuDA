#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "HAQuDA_DisplayEngine.h"
#include "HAQuDA_LEDsParams.h"
#include "MockLEDs.h"

namespace {

	class FakeMeasSource : public HAQuDA_IMeasSource {
	  public:
		FakeMeasSource() {
			for (uint8_t i = 0; i < 3; i++) {
				data.values[i] = 22;
				data.dots[i] = div_dots{20, 26, 30};
			}
		}

		bool ReadAverages(anim_id &anim, meas_anim_data &out) override {
			readCalls++;
			if (!ready) {
				return false;
			}
			anim = nextAnim;
			out = data;
			return true;
		}

		void RestartAveraging() override {
			restartCalls++;
		}

		bool ready = false;
		anim_id nextAnim = anim_id::measMulti;
		meas_anim_data data = {};
		int readCalls = 0;
		int restartCalls = 0;
	};

	display_message eventMsg(display_event event) {
		return display_message{display_msg::event, static_cast<uint8_t>(event), 0};
	}

	display_message errorMsg(display_error error) {
		return display_message{display_msg::error, static_cast<uint8_t>(error), 0};
	}

	display_message endMsg(display_end end) {
		return display_message{display_msg::end, static_cast<uint8_t>(end), 0};
	}

	display_message backgroundMsg(anim_id anim, display_item waiting = DISPLAY_ITEM_NONE) {
		return display_message{display_msg::background, static_cast<uint8_t>(anim), waiting};
	}

	display_message measEnabledMsg(bool enabled) {
		return display_message{display_msg::measEnabled, static_cast<uint8_t>(enabled ? 1 : 0), 0};
	}

	constexpr display_item BOOT_ITEM = HAQuDA_ItemOf(display_event::bootSnake);
	constexpr display_item AP_ITEM = HAQuDA_ItemOf(display_event::apRunning);

	display_message brightnessMsg(uint8_t percent) {
		return display_message{display_msg::brightness, percent, 0};
	}

	constexpr uint32_t START_BRIGHT_PERCENT = 50;
	constexpr uint32_t SECOND = 1000;

	class DisplayEngineTest : public testing::Test {
	  protected:
		DisplayEngineTest() {
			ON_CALL(leds, SetBright(testing::_)).WillByDefault(testing::Invoke([this](uint8_t percent) { brightCalls.push_back(percent); }));
			ON_CALL(leds, Show(testing::_)).WillByDefault(testing::Invoke([this](uint16_t) { showCalls++; }));
		}

		void startMeasurements(uint32_t atMs) {
			engine.Apply(eventMsg(display_event::wifiJoined));
			engine.Run(atMs);
			engine.Apply(backgroundMsg(anim_id::measMulti));
			engine.Apply(measEnabledMsg(true));
		}

		testing::NiceMock<MockLEDs> leds;
		FakeMeasSource source;
		HAQuDA_DisplayEngine engine{leds, source, START_BRIGHT_PERCENT};
		std::vector<uint8_t> brightCalls;
		int showCalls = 0;
	};

} // namespace

TEST_F(DisplayEngineTest, NothingPostedDrawsNothing) {
	EXPECT_CALL(leds, Show(testing::_)).Times(0);

	const uint32_t sleepMs = engine.Run(0);

	EXPECT_EQ(anim_id::none, engine.ActiveAnim());
	EXPECT_EQ(ANIM_IDLE_MS, sleepMs);
}

TEST_F(DisplayEngineTest, BeginSetsTheStartBrightness) {
	engine.Begin();

	EXPECT_EQ(std::vector<uint8_t>{50}, brightCalls);
}

TEST_F(DisplayEngineTest, StartBrightnessAboveFullIsClamped) {
	testing::NiceMock<MockLEDs> otherLeds;
	HAQuDA_DisplayEngine other(otherLeds, source, 250);

	EXPECT_EQ(100, other.BrightPercent());
}

TEST_F(DisplayEngineTest, PostedEventStartsItsAnimationInTheSameRun) {
	engine.Apply(eventMsg(display_event::bootSnake));

	const uint32_t sleepMs = engine.Run(0);

	EXPECT_EQ(anim_id::snake, engine.ActiveAnim());
	EXPECT_EQ(ANIM_LEDS_SHOW_MS, sleepMs);
}

TEST_F(DisplayEngineTest, AnAnimationStepsOnlyWhenItsStepIsDue) {
	engine.Apply(eventMsg(display_event::bootSnake));
	engine.Run(0);
	ASSERT_EQ(1, showCalls);

	const uint32_t earlySleepMs = engine.Run(50);
	EXPECT_EQ(1, showCalls);
	EXPECT_EQ(150u, earlySleepMs);

	engine.Run(ANIM_LEDS_SHOW_MS);
	EXPECT_EQ(2, showCalls);
}

TEST_F(DisplayEngineTest, BootSnakePlaysOnWithoutAResult) {
	engine.Apply(eventMsg(display_event::bootSnake));
	engine.Run(0);

	engine.Run(60 * SECOND);

	EXPECT_EQ(anim_id::snake, engine.ActiveAnim());
}

TEST_F(DisplayEngineTest, BootResultEndsTheSnakeOnlyAfterItsMinimumTime) {
	engine.Apply(eventMsg(display_event::bootSnake));
	engine.Run(0);
	engine.Apply(eventMsg(display_event::wifiJoined));

	engine.Run(1 * SECOND);
	EXPECT_EQ(anim_id::snake, engine.ActiveAnim());

	engine.Run(5 * SECOND);
	EXPECT_EQ(anim_id::staticColor, engine.ActiveAnim());
}

TEST_F(DisplayEngineTest, ErrorInterruptsARunningEventAtOnce) {
	engine.Apply(eventMsg(display_event::bootSnake));
	engine.Run(0);
	engine.Apply(errorMsg(display_error::sensorsFailed));

	engine.Run(300);

	EXPECT_EQ(anim_id::randomPixels, engine.ActiveAnim());
}

TEST_F(DisplayEngineTest, EndMessageEndsAnUntilEventErrorAfterItsMinimumTime) {
	engine.Apply(errorMsg(display_error::sensorsFailed));
	engine.Run(0);
	engine.Apply(endMsg(display_end::sensorsRecovered));

	engine.Run(5 * SECOND);
	EXPECT_EQ(anim_id::randomPixels, engine.ActiveAnim());

	engine.Run(10 * SECOND);
	EXPECT_EQ(anim_id::none, engine.ActiveAnim());
}

TEST_F(DisplayEngineTest, SleepIsCappedByTheEndOfTheShownItem) {
	engine.Apply(eventMsg(display_event::wifiJoined));
	engine.Run(0);

	const uint32_t sleepMs = engine.Run(4900);

	EXPECT_EQ(100u, sleepMs);
}

TEST_F(DisplayEngineTest, SleepIsNeverZero) {
	engine.Apply(eventMsg(display_event::wifiJoined));
	engine.Apply(errorMsg(display_error::wifiConnectFailed));

	for (uint32_t nowMs = 0; nowMs < 20 * SECOND; nowMs += 7) {
		EXPECT_GE(engine.Run(nowMs), 1u);
	}
}

TEST_F(DisplayEngineTest, InvalidMessagesAreIgnored) {
	engine.Apply(display_message{display_msg::event, 200, 0});
	engine.Apply(display_message{display_msg::error, 200, 0});
	engine.Apply(display_message{display_msg::end, 200, 0});
	engine.Apply(display_message{display_msg::background, 200, 0});

	engine.Run(0);

	EXPECT_EQ(anim_id::none, engine.ActiveAnim());
}

TEST_F(DisplayEngineTest, StationBootPlaysTheSnakeAgainAfterTheGreenUntilTheFirstSample) {
	engine.Apply(eventMsg(display_event::bootSnake));
	engine.Run(0);
	engine.Apply(backgroundMsg(anim_id::measMulti, BOOT_ITEM));
	engine.Apply(measEnabledMsg(true));
	engine.Apply(eventMsg(display_event::wifiJoined));

	engine.Run(1 * SECOND);
	EXPECT_EQ(anim_id::snake, engine.ActiveAnim());
	engine.Run(5 * SECOND);
	EXPECT_EQ(anim_id::staticColor, engine.ActiveAnim());
	engine.Run(10 * SECOND);
	EXPECT_EQ(anim_id::snake, engine.ActiveAnim());
	engine.Run(18 * SECOND);
	EXPECT_EQ(anim_id::snake, engine.ActiveAnim());
	EXPECT_EQ(0, source.restartCalls);

	source.ready = true;
	engine.Run(18 * SECOND + 100);

	EXPECT_EQ(anim_id::measMulti, engine.ActiveAnim());
	EXPECT_EQ(1, source.restartCalls);
}

TEST_F(DisplayEngineTest, ReconnectWithDataGoesFromTheGreenStraightToTheColours) {
	source.ready = true;
	engine.Apply(eventMsg(display_event::wifiJoined));
	engine.Run(0);
	engine.Apply(backgroundMsg(anim_id::measMulti, BOOT_ITEM));
	engine.Apply(measEnabledMsg(true));

	engine.Run(1 * SECOND);
	EXPECT_EQ(anim_id::staticColor, engine.ActiveAnim());
	engine.Run(5 * SECOND);

	EXPECT_EQ(anim_id::measMulti, engine.ActiveAnim());
}

TEST_F(DisplayEngineTest, AccessPointKeepsFadingUntilTheFirstSample) {
	engine.Apply(eventMsg(display_event::apRunning));
	engine.Run(0);
	engine.Apply(backgroundMsg(anim_id::measMulti, AP_ITEM));
	engine.Apply(measEnabledMsg(true));

	engine.Run(30 * SECOND);
	EXPECT_EQ(anim_id::fading, engine.ActiveAnim());
	engine.Run(120 * SECOND);
	EXPECT_EQ(anim_id::fading, engine.ActiveAnim());

	source.ready = true;
	engine.Run(120 * SECOND + 100);

	EXPECT_EQ(anim_id::measMulti, engine.ActiveAnim());
}

TEST_F(DisplayEngineTest, ARejectedMeasurementStartKeepsTheRunningPicture) {
	engine.Apply(eventMsg(display_event::bootSnake));
	engine.Run(0);
	engine.Apply(backgroundMsg(anim_id::measMulti, BOOT_ITEM));
	engine.Apply(measEnabledMsg(true));
	source.ready = true;
	source.data.dots[1] = div_dots{0, 0, 0};

	engine.Run(1 * SECOND);
	engine.Run(2 * SECOND);

	EXPECT_EQ(anim_id::snake, engine.ActiveAnim());
	EXPECT_EQ(0, source.restartCalls);
}

TEST_F(DisplayEngineTest, TheWaitingPictureShowsNothingWithoutAWaitingItem) {
	startMeasurements(0);

	engine.Run(5 * SECOND);

	EXPECT_EQ(anim_id::none, engine.ActiveAnim());
	EXPECT_EQ(0, source.restartCalls);
}

TEST_F(DisplayEngineTest, MeasurementsStartAsSoonAsASampleArrives) {
	startMeasurements(0);
	engine.Run(5 * SECOND);
	source.ready = true;

	engine.Run(5 * SECOND + 100);

	EXPECT_EQ(anim_id::measMulti, engine.ActiveAnim());
	EXPECT_EQ(1, source.restartCalls);
}

TEST_F(DisplayEngineTest, MeasurementsRestartWithFreshDataEveryPeriod) {
	startMeasurements(0);
	source.ready = true;
	engine.Run(5 * SECOND);
	const int readsAtStart = source.readCalls;

	engine.Run(5 * SECOND + DISPLAY_MEAS_PERIOD_MS - 1);
	EXPECT_EQ(readsAtStart, source.readCalls);
	EXPECT_EQ(1, source.restartCalls);

	engine.Run(5 * SECOND + DISPLAY_MEAS_PERIOD_MS);
	EXPECT_EQ(readsAtStart + 1, source.readCalls);
	EXPECT_EQ(2, source.restartCalls);
	EXPECT_EQ(anim_id::measMulti, engine.ActiveAnim());
}

TEST_F(DisplayEngineTest, MeasurementPeriodSurvivesTheMillisWrap) {
	const uint32_t startMs = 0xFFFFFFF0u;
	startMeasurements(startMs);
	source.ready = true;
	engine.Run(startMs + 5 * SECOND);

	engine.Run(startMs + 5 * SECOND + DISPLAY_MEAS_PERIOD_MS);

	EXPECT_EQ(2, source.restartCalls);
}

TEST_F(DisplayEngineTest, AMissingSampleAtTheEndOfAPeriodKeepsThePictureAndRestartsTheAveraging) {
	startMeasurements(0);
	source.ready = true;
	engine.Run(5 * SECOND);
	source.ready = false;

	engine.Run(5 * SECOND + DISPLAY_MEAS_PERIOD_MS);

	EXPECT_EQ(anim_id::measMulti, engine.ActiveAnim());
	EXPECT_EQ(2, source.restartCalls);
	const int readsAfterPeriod = source.readCalls;
	engine.Run(5 * SECOND + DISPLAY_MEAS_PERIOD_MS + 1000);
	EXPECT_EQ(readsAfterPeriod, source.readCalls);
}

TEST_F(DisplayEngineTest, TheRefreshAtTheEndOfAPeriodRedrawsInPlaceWithoutADarkFrame) {
	source.nextAnim = anim_id::measStandard;
	startMeasurements(0);
	source.ready = true;
	engine.Run(5 * SECOND);
	EXPECT_CALL(leds, FillColor(testing::Matcher<uint32_t>(0u))).Times(0);
	EXPECT_CALL(leds, FillColor(testing::Matcher<uint8_t>(testing::_), testing::_, testing::_)).Times(1);

	engine.Run(5 * SECOND + DISPLAY_MEAS_PERIOD_MS);

	EXPECT_EQ(anim_id::measStandard, engine.ActiveAnim());
}

TEST_F(DisplayEngineTest, TwoQuickBrightnessChangesKeepTheMeasurementPicture) {
	startMeasurements(0);
	source.ready = true;
	engine.Run(5 * SECOND);
	source.ready = false;
	const int restartsBefore = source.restartCalls;

	engine.Apply(brightnessMsg(60));
	engine.Run(5 * SECOND + 10);
	engine.Apply(brightnessMsg(70));
	engine.Run(5 * SECOND + 20);

	EXPECT_EQ(anim_id::measMulti, engine.ActiveAnim());
	EXPECT_EQ(restartsBefore, source.restartCalls);
	EXPECT_EQ((std::vector<uint8_t>{60, 70}), brightCalls);
}

TEST_F(DisplayEngineTest, TheMeasurementPictureIsRedrawnAtTheNewBrightnessOnItsNextStep) {
	startMeasurements(0);
	source.ready = true;
	engine.Run(5 * SECOND);
	const int showsBefore = showCalls;
	engine.Apply(brightnessMsg(60));

	engine.Run(5 * SECOND + ANIM_LEDS_SHOW_MS + ANIM_PASS_PAUSE_MS);

	EXPECT_EQ(showsBefore + 1, showCalls);
}

TEST_F(DisplayEngineTest, ANewMeasurementAnimationReplacesTheOldOneAtTheNextPeriod) {
	startMeasurements(0);
	source.ready = true;
	engine.Run(5 * SECOND);
	source.nextAnim = anim_id::measStandard;

	engine.Run(5 * SECOND + DISPLAY_MEAS_PERIOD_MS);

	EXPECT_EQ(anim_id::measStandard, engine.ActiveAnim());
}

TEST_F(DisplayEngineTest, SleepDuringMeasurementsStopsAtTheEndOfThePeriod) {
	startMeasurements(0);
	source.ready = true;
	engine.Run(5 * SECOND);

	const uint32_t sleepMs = engine.Run(5 * SECOND + DISPLAY_MEAS_PERIOD_MS - 50);

	EXPECT_EQ(50u, sleepMs);
}

TEST_F(DisplayEngineTest, ErrorInterruptsMeasurements) {
	startMeasurements(0);
	source.ready = true;
	engine.Run(5 * SECOND);
	engine.Apply(errorMsg(display_error::wifiLost));

	engine.Run(6 * SECOND);

	EXPECT_EQ(anim_id::staticColor, engine.ActiveAnim());
}

TEST_F(DisplayEngineTest, BrightnessMessageSetsTheLeds) {
	engine.Apply(brightnessMsg(80));

	EXPECT_EQ(80, engine.BrightPercent());
	EXPECT_EQ(std::vector<uint8_t>{80}, brightCalls);
}

TEST_F(DisplayEngineTest, BrightnessAboveFullIsClamped) {
	engine.Apply(brightnessMsg(200));

	EXPECT_EQ(100, engine.BrightPercent());
	EXPECT_EQ(std::vector<uint8_t>{100}, brightCalls);
}

TEST_F(DisplayEngineTest, TheSameBrightnessTouchesNothing) {
	engine.Apply(brightnessMsg(50));

	EXPECT_TRUE(brightCalls.empty());
}

TEST_F(DisplayEngineTest, BrightnessChangeRestartsTheRunningAnimation) {
	engine.Apply(eventMsg(display_event::bootSnake));
	engine.Run(0);
	engine.Apply(brightnessMsg(80));
	EXPECT_EQ(anim_id::none, engine.ActiveAnim());

	engine.Run(100);

	EXPECT_EQ(anim_id::snake, engine.ActiveAnim());
}

TEST_F(DisplayEngineTest, FadePutsBackTheLampBrightnessWhenAnErrorInterruptsIt) {
	engine.Apply(brightnessMsg(40));
	engine.Apply(eventMsg(display_event::apRunning));
	engine.Run(0);
	ASSERT_EQ(anim_id::fading, engine.ActiveAnim());
	engine.Apply(errorMsg(display_error::sensorsFailed));

	engine.Run(100);

	EXPECT_EQ(anim_id::randomPixels, engine.ActiveAnim());
	ASSERT_FALSE(brightCalls.empty());
	EXPECT_EQ(40, brightCalls.back());
}

TEST_F(DisplayEngineTest, LampLightDrawsTheColourAsAStaticPicture) {
	engine.Apply(display_message{display_msg::lampLight, 0, 0, 0xFFB46B});
	EXPECT_CALL(leds, FillColor(testing::Matcher<uint32_t>(0xFFB46Bu))).Times(testing::AtLeast(1));

	engine.Run(0);

	EXPECT_EQ(anim_id::staticColor, engine.ActiveAnim());
}

TEST_F(DisplayEngineTest, LampOffDrawsDark) {
	engine.Apply(display_message{display_msg::lampOff, 0, 0, 0});
	EXPECT_CALL(leds, FillColor(testing::Matcher<uint32_t>(0u))).Times(testing::AtLeast(1));

	engine.Run(0);

	EXPECT_EQ(anim_id::staticColor, engine.ActiveAnim());
}

TEST_F(DisplayEngineTest, ANewLampColourReplacesTheOldOne) {
	engine.Apply(display_message{display_msg::lampLight, 0, 0, 0xFFB46B});
	engine.Run(0);
	engine.Apply(display_message{display_msg::lampLight, 0, 0, 0x112233});
	EXPECT_CALL(leds, FillColor(testing::Matcher<uint32_t>(0x112233u))).Times(testing::AtLeast(1));

	engine.Run(100);
}

TEST_F(DisplayEngineTest, LampOffDoesNotShowAnEventButShowsAnError) {
	engine.Apply(display_message{display_msg::lampOff, 0, 0, 0});
	engine.Run(0);

	engine.Apply(eventMsg(display_event::wifiReconnecting));
	engine.Run(100);
	EXPECT_EQ(anim_id::staticColor, engine.ActiveAnim());

	engine.Apply(errorMsg(display_error::sensorsFailed));
	engine.Run(200);
	EXPECT_EQ(anim_id::randomPixels, engine.ActiveAnim());
}

TEST_F(DisplayEngineTest, MeasurementsReplaceTheLampLightWhenTheyAreSelected) {
	source.ready = true;
	engine.Apply(display_message{display_msg::lampLight, 0, 0, 0xFFB46B});
	engine.Run(0);

	engine.Apply(backgroundMsg(anim_id::measMulti));
	engine.Apply(measEnabledMsg(true));
	engine.Run(100);
	engine.Run(200);

	EXPECT_EQ(anim_id::measMulti, engine.ActiveAnim());
}

TEST_F(DisplayEngineTest, WifiLostAppliedWithTheReconnectSnakeInOneBatchShowsRedThenTheSnake) {
	EXPECT_CALL(leds, FillColor(testing::Matcher<uint32_t>(ANIM_COLOR_RED))).Times(testing::AtLeast(1));
	EXPECT_CALL(leds, FillColor(testing::Matcher<uint32_t>(0u))).Times(testing::AtLeast(1));
	engine.Apply(errorMsg(display_error::wifiLost));
	engine.Apply(eventMsg(display_event::wifiReconnecting));

	engine.Run(0);
	EXPECT_EQ(anim_id::staticColor, engine.ActiveAnim());
	engine.Run(10 * SECOND - 1);
	EXPECT_EQ(anim_id::staticColor, engine.ActiveAnim());
	engine.Run(10 * SECOND);

	EXPECT_EQ(anim_id::snake, engine.ActiveAnim());
}

TEST_F(DisplayEngineTest, WifiLostAppliedWithTheReconnectSnakeAndTheJoinInOneBatchShowsRedThenGreenNeverTheSnake) {
	EXPECT_CALL(leds, FillColor(testing::Matcher<uint32_t>(ANIM_COLOR_RED))).Times(testing::AtLeast(1));
	EXPECT_CALL(leds, FillColor(testing::Matcher<uint32_t>(ANIM_COLOR_GREEN))).Times(testing::AtLeast(1));
	engine.Apply(errorMsg(display_error::wifiLost));
	engine.Apply(eventMsg(display_event::wifiReconnecting));
	engine.Apply(eventMsg(display_event::wifiJoined));

	engine.Run(0);
	EXPECT_EQ(anim_id::staticColor, engine.ActiveAnim());
	engine.Run(10 * SECOND - 1);
	EXPECT_EQ(anim_id::staticColor, engine.ActiveAnim());
	engine.Run(10 * SECOND);
	EXPECT_EQ(anim_id::staticColor, engine.ActiveAnim());
	engine.Run(15 * SECOND);

	EXPECT_EQ(anim_id::none, engine.ActiveAnim());
}

TEST_F(DisplayEngineTest, UntilEventErrorAppliedWithItsOwnEndSignalInOneBatchIsRemovedUnseen) {
	EXPECT_CALL(leds, Show(testing::_)).Times(0);
	engine.Apply(errorMsg(display_error::sensorsFailed));
	engine.Apply(endMsg(display_end::sensorsRecovered));

	engine.Run(0);

	EXPECT_EQ(anim_id::none, engine.ActiveAnim());
}

TEST(SensorsErrorActiveTest, ThePostMakesItActiveBeforeAnythingIsShown) {
	const bool exp_active = true;

	bool res;

	res = HAQuDA_SensorsErrorActiveAfter(false, errorMsg(display_error::sensorsFailed));

	EXPECT_EQ(exp_active, res);
}

TEST(SensorsErrorActiveTest, TheEndSignalMakesItInactive) {
	const bool exp_active = false;

	bool res;

	res = HAQuDA_SensorsErrorActiveAfter(true, endMsg(display_end::sensorsRecovered));

	EXPECT_EQ(exp_active, res);
}

TEST(SensorsErrorActiveTest, OtherErrorsEventsAndEndsLeaveItAsItWas) {
	const display_message others[] = {errorMsg(display_error::ntpFailed), errorMsg(display_error::wifiLost), eventMsg(display_event::wifiJoined),
									  endMsg(display_end::ntpSynced), endMsg(display_end::wifiJoined)};
	for (const display_message &message : others) {
		EXPECT_TRUE(HAQuDA_SensorsErrorActiveAfter(true, message));
		EXPECT_FALSE(HAQuDA_SensorsErrorActiveAfter(false, message));
	}
}

TEST_F(DisplayEngineTest, TheSensorsErrorStaysActiveWhileAnotherErrorOrEventHasTheMatrix) {
	const display_message posts[] = {errorMsg(display_error::sensorsFailed), errorMsg(display_error::ntpFailed), eventMsg(display_event::apRunning)};
	bool active = false;
	for (const display_message &message : posts) {
		engine.Apply(message);
		active = HAQuDA_SensorsErrorActiveAfter(active, message);
	}

	engine.Run(0);
	engine.Run(10 * SECOND);
	EXPECT_EQ(HAQuDA_ItemOf(display_error::ntpFailed), engine.Shown().item);
	EXPECT_TRUE(active);

	engine.Apply(endMsg(display_end::sensorsRecovered));
	active = HAQuDA_SensorsErrorActiveAfter(active, endMsg(display_end::sensorsRecovered));
	engine.Run(20 * SECOND);

	EXPECT_FALSE(active);
	EXPECT_NE(HAQuDA_ItemOf(display_error::sensorsFailed), engine.Shown().item);
}

TEST_F(DisplayEngineTest, NothingShownYetHasASilentStartingPicture) {
	engine.Run(0);

	EXPECT_EQ(0u, engine.Shown().seq);
	EXPECT_EQ(display_source::none, engine.Shown().source);
	EXPECT_EQ(DISPLAY_ITEM_NONE, engine.Shown().item);
}

TEST_F(DisplayEngineTest, AnItemStartRaisesTheCounterAndRecordsWhatPlays) {
	engine.Apply(eventMsg(display_event::wifiReconnecting));

	engine.Run(100);

	const display_shown &shown = engine.Shown();
	EXPECT_EQ(1u, shown.seq);
	EXPECT_EQ(100u, shown.startMs);
	EXPECT_EQ(display_source::event, shown.source);
	EXPECT_EQ(HAQuDA_ItemOf(display_event::wifiReconnecting), shown.item);
	EXPECT_EQ(anim_id::snake, shown.anim.anim);
	EXPECT_EQ(ANIM_COLOR_LIME, shown.anim.color);
}

TEST_F(DisplayEngineTest, StepsOfTheShownItemKeepTheCounter) {
	engine.Apply(eventMsg(display_event::wifiReconnecting));
	engine.Run(0);

	engine.Run(ANIM_LEDS_SHOW_MS);
	engine.Run(2 * ANIM_LEDS_SHOW_MS);

	EXPECT_EQ(1u, engine.Shown().seq);
}

TEST_F(DisplayEngineTest, ARotationBetweenTwoErrorsRaisesTheCounterAtEverySwitch) {
	engine.Apply(errorMsg(display_error::sensorsFailed));
	engine.Apply(errorMsg(display_error::ntpFailed));
	engine.Run(0);
	ASSERT_EQ(HAQuDA_ItemOf(display_error::sensorsFailed), engine.Shown().item);
	const uint32_t firstSeq = engine.Shown().seq;

	engine.Run(10 * SECOND - 1);
	EXPECT_EQ(firstSeq, engine.Shown().seq);
	engine.Run(10 * SECOND);

	EXPECT_EQ(firstSeq + 1, engine.Shown().seq);
	EXPECT_EQ(display_source::error, engine.Shown().source);
	EXPECT_EQ(HAQuDA_ItemOf(display_error::ntpFailed), engine.Shown().item);
	EXPECT_EQ(10 * SECOND, engine.Shown().startMs);

	engine.Run(20 * SECOND);

	EXPECT_EQ(firstSeq + 2, engine.Shown().seq);
	EXPECT_EQ(HAQuDA_ItemOf(display_error::sensorsFailed), engine.Shown().item);
}

TEST_F(DisplayEngineTest, AnInterruptionAndTheReturnOfTheEventRaiseTheCounter) {
	engine.Apply(eventMsg(display_event::wifiReconnecting));
	engine.Run(0);
	engine.Apply(errorMsg(display_error::wifiLost));

	engine.Run(100);

	EXPECT_EQ(2u, engine.Shown().seq);
	EXPECT_EQ(display_source::error, engine.Shown().source);
	EXPECT_EQ(HAQuDA_ItemOf(display_error::wifiLost), engine.Shown().item);
	EXPECT_EQ(ANIM_COLOR_RED, engine.Shown().anim.color);

	engine.Run(10 * SECOND + 100);

	EXPECT_EQ(3u, engine.Shown().seq);
	EXPECT_EQ(display_source::event, engine.Shown().source);
	EXPECT_EQ(HAQuDA_ItemOf(display_event::wifiReconnecting), engine.Shown().item);
}

TEST_F(DisplayEngineTest, WhenNothingIsLeftToShowTheCounterRisesOnceWithSourceNone) {
	engine.Apply(eventMsg(display_event::wifiJoined));
	engine.Run(0);

	engine.Run(5 * SECOND);

	EXPECT_EQ(2u, engine.Shown().seq);
	EXPECT_EQ(display_source::none, engine.Shown().source);
	EXPECT_EQ(DISPLAY_ITEM_NONE, engine.Shown().item);

	engine.Run(6 * SECOND);

	EXPECT_EQ(2u, engine.Shown().seq);
}

TEST_F(DisplayEngineTest, TheWaitingPictureIsRecordedAsWaiting) {
	engine.Apply(backgroundMsg(anim_id::measMulti, AP_ITEM));
	engine.Apply(measEnabledMsg(true));

	engine.Run(0);

	EXPECT_EQ(1u, engine.Shown().seq);
	EXPECT_EQ(display_source::waiting, engine.Shown().source);
	EXPECT_EQ(AP_ITEM, engine.Shown().item);
	EXPECT_EQ(anim_id::fading, engine.Shown().anim.anim);
}

TEST_F(DisplayEngineTest, TheFirstSampleRaisesTheCounterWithTheMeasurementPicture) {
	engine.Apply(backgroundMsg(anim_id::measMulti, AP_ITEM));
	engine.Apply(measEnabledMsg(true));
	engine.Run(0);
	const uint32_t waitingSeq = engine.Shown().seq;
	source.ready = true;

	engine.Run(100);

	const display_shown &shown = engine.Shown();
	EXPECT_GT(shown.seq, waitingSeq);
	EXPECT_EQ(display_source::meas, shown.source);
	EXPECT_EQ(anim_id::measMulti, shown.anim.anim);
	EXPECT_EQ(DISPLAY_ITEM_NONE, shown.item);
	EXPECT_TRUE(shown.measDrawn);
}

TEST_F(DisplayEngineTest, LampLightAndOffAreSolidPicturesThatRaiseTheCounter) {
	engine.Apply(display_message{display_msg::lampLight, 0, 0, 0xFFB46B});
	engine.Run(0);

	EXPECT_EQ(1u, engine.Shown().seq);
	EXPECT_EQ(display_source::solid, engine.Shown().source);
	EXPECT_EQ(anim_id::staticColor, engine.Shown().anim.anim);
	EXPECT_EQ(0xFFB46Bu, engine.Shown().anim.color);

	engine.Apply(display_message{display_msg::lampLight, 0, 0, 0x112233});
	engine.Run(100);

	EXPECT_EQ(2u, engine.Shown().seq);
	EXPECT_EQ(0x112233u, engine.Shown().anim.color);

	engine.Apply(display_message{display_msg::lampOff, 0, 0, 0});
	engine.Run(200);

	EXPECT_EQ(3u, engine.Shown().seq);
	EXPECT_EQ(display_source::solid, engine.Shown().source);
	EXPECT_EQ(0u, engine.Shown().anim.color);
}

TEST_F(DisplayEngineTest, TheSameLampLightStepsKeepTheCounter) {
	engine.Apply(display_message{display_msg::lampLight, 0, 0, 0xFFB46B});
	engine.Run(0);

	engine.Run(ANIM_LEDS_SHOW_MS + ANIM_PASS_PAUSE_MS);
	engine.Run(2 * (ANIM_LEDS_SHOW_MS + ANIM_PASS_PAUSE_MS));

	EXPECT_EQ(1u, engine.Shown().seq);
}

TEST_F(DisplayEngineTest, ALampLightOverTheMeasurementsAndBackShowsTheMeasurementColoursAgain) {
	source.ready = true;
	engine.Apply(display_message{display_msg::lampLight, 0, 0, 0xFFB46B});
	engine.Run(0);
	engine.Apply(backgroundMsg(anim_id::measMulti));
	engine.Apply(measEnabledMsg(true));

	engine.Run(100);

	rgb_color expected = {0, 0, 0};
	HAQuDA_GetMeasRGB(22, div_dots{20, 26, 30}, expected);
	const display_shown &shown = engine.Shown();
	EXPECT_EQ(display_source::meas, shown.source);
	EXPECT_TRUE(shown.measDrawn);
	EXPECT_EQ(3, shown.colorCount);
	for (uint8_t i = 0; i < 3; i++) {
		EXPECT_EQ(expected.r, shown.colors[i].r);
		EXPECT_EQ(expected.g, shown.colors[i].g);
		EXPECT_EQ(expected.b, shown.colors[i].b);
	}
}

TEST_F(DisplayEngineTest, TheMeasurementKeysAreRecordedWithTheColours) {
	source.data.keys[0] = temp;
	source.data.keys[1] = eCO2;
	source.data.keys[2] = PM2_5;
	startMeasurements(0);
	source.ready = true;

	engine.Run(5 * SECOND);

	EXPECT_EQ(temp, engine.Shown().keys[0]);
	EXPECT_EQ(eCO2, engine.Shown().keys[1]);
	EXPECT_EQ(PM2_5, engine.Shown().keys[2]);
}

TEST_F(DisplayEngineTest, TheStandardPictureHasOneColourAndTheTotalOneToo) {
	source.nextAnim = anim_id::measStandard;
	startMeasurements(0);
	source.ready = true;
	engine.Run(5 * SECOND);
	EXPECT_EQ(1, engine.Shown().colorCount);

	source.nextAnim = anim_id::measTotal;
	for (uint8_t i = 0; i < MEAS_ANIM_MAX_PARAMS; i++) {
		source.data.values[i] = 22;
		source.data.dots[i] = div_dots{20, 26, 30};
	}
	engine.Run(5 * SECOND + DISPLAY_MEAS_PERIOD_MS);

	EXPECT_EQ(anim_id::measTotal, engine.Shown().anim.anim);
	EXPECT_EQ(1, engine.Shown().colorCount);
}

TEST_F(DisplayEngineTest, ARedrawWithNewColoursRaisesTheCounter) {
	startMeasurements(0);
	source.ready = true;
	engine.Run(5 * SECOND);
	const uint32_t seqBefore = engine.Shown().seq;
	const rgb_color before = engine.Shown().colors[0];
	for (uint8_t i = 0; i < 3; i++) {
		source.data.values[i] = 33;
	}

	engine.Run(5 * SECOND + DISPLAY_MEAS_PERIOD_MS);

	EXPECT_EQ(seqBefore + 1, engine.Shown().seq);
	EXPECT_EQ(5 * SECOND + DISPLAY_MEAS_PERIOD_MS, engine.Shown().startMs);
	EXPECT_FALSE((before.r == engine.Shown().colors[0].r) && (before.g == engine.Shown().colors[0].g) && (before.b == engine.Shown().colors[0].b));
}

TEST_F(DisplayEngineTest, ARedrawWithTheSameColoursKeepsTheCounter) {
	startMeasurements(0);
	source.ready = true;
	engine.Run(5 * SECOND);
	const uint32_t seqBefore = engine.Shown().seq;

	engine.Run(5 * SECOND + DISPLAY_MEAS_PERIOD_MS);
	engine.Run(5 * SECOND + DISPLAY_MEAS_PERIOD_MS + ANIM_LEDS_SHOW_MS + ANIM_PASS_PAUSE_MS);
	engine.Run(5 * SECOND + 2 * DISPLAY_MEAS_PERIOD_MS);

	EXPECT_EQ(seqBefore, engine.Shown().seq);
}

TEST_F(DisplayEngineTest, AChangedKeyWithTheSameColoursRaisesTheCounter) {
	startMeasurements(0);
	source.ready = true;
	engine.Run(5 * SECOND);
	const uint32_t seqBefore = engine.Shown().seq;
	source.data.keys[0] = PM2_5;

	engine.Run(5 * SECOND + DISPLAY_MEAS_PERIOD_MS);

	EXPECT_EQ(seqBefore + 1, engine.Shown().seq);
	EXPECT_EQ(PM2_5, engine.Shown().keys[0]);
}

TEST_F(DisplayEngineTest, TheNightPictureRecordsTheLitCellsAndTheGreyOfTheBar) {
	source.nextAnim = anim_id::measNight;
	source.data.nightLitCells = 3;
	startMeasurements(0);
	source.ready = true;
	engine.Run(5 * SECOND);
	const display_shown first = engine.Shown();
	EXPECT_EQ(3, first.nightLitCells);
	const uint8_t grey = HAQuDA_NightGrey();
	EXPECT_EQ(grey, first.white.r);
	EXPECT_EQ(grey, first.white.g);
	EXPECT_EQ(grey, first.white.b);
	source.data.nightLitCells = 4;

	engine.Run(5 * SECOND + DISPLAY_MEAS_PERIOD_MS);

	EXPECT_EQ(first.seq + 1, engine.Shown().seq);
	EXPECT_EQ(4, engine.Shown().nightLitCells);
}

TEST_F(DisplayEngineTest, AMeasurementPictureThatIsNotDrawnYetIsMarkedSo) {
	engine.Apply(backgroundMsg(anim_id::measMulti, BOOT_ITEM));
	engine.Apply(measEnabledMsg(true));
	engine.Run(0);

	EXPECT_FALSE(engine.Shown().measDrawn);
	EXPECT_EQ(display_source::waiting, engine.Shown().source);
}

TEST_F(DisplayEngineTest, ABrightnessChangeRestartsTheItemAndRaisesTheCounter) {
	engine.Apply(eventMsg(display_event::bootSnake));
	engine.Run(0);
	engine.Apply(brightnessMsg(80));

	engine.Run(100);

	EXPECT_EQ(2u, engine.Shown().seq);
	EXPECT_EQ(100u, engine.Shown().startMs);
}

TEST_F(DisplayEngineTest, ABrightnessChangeOverTheMeasurementsKeepsTheCounter) {
	startMeasurements(0);
	source.ready = true;
	engine.Run(5 * SECOND);
	const uint32_t seqBefore = engine.Shown().seq;

	engine.Apply(brightnessMsg(60));
	engine.Run(5 * SECOND + 10);

	EXPECT_EQ(seqBefore, engine.Shown().seq);
}

TEST_F(DisplayEngineTest, ASettingChangeRedrawsTheMeasurementAtOnceWithTheCurrentAverages) {
	startMeasurements(0);
	source.ready = true;
	engine.Run(5 * SECOND);
	const uint32_t seqBefore = engine.Shown().seq;
	const int restartsBefore = source.restartCalls;
	source.data.keys[0] = PM2_5;
	for (uint8_t i = 0; i < 3; i++) {
		source.data.values[i] = 33;
	}

	engine.Apply(backgroundMsg(anim_id::measMulti));
	engine.Apply(measEnabledMsg(true));
	engine.Run(5 * SECOND + 100);

	EXPECT_EQ(seqBefore + 1, engine.Shown().seq);
	EXPECT_EQ(PM2_5, engine.Shown().keys[0]);
	EXPECT_EQ(150, engine.Shown().colors[0].r);
	EXPECT_EQ(restartsBefore + 1, source.restartCalls);
	EXPECT_EQ(anim_id::measMulti, engine.ActiveAnim());
}

TEST_F(DisplayEngineTest, ASettingChangeWithoutASampleKeepsTheRunningPictureUntilOneArrives) {
	startMeasurements(0);
	source.ready = true;
	engine.Run(5 * SECOND);
	const uint32_t seqBefore = engine.Shown().seq;
	source.ready = false;
	source.data.keys[0] = PM2_5;

	engine.Apply(backgroundMsg(anim_id::measMulti));
	engine.Apply(measEnabledMsg(true));
	engine.Run(5 * SECOND + 100);

	EXPECT_EQ(anim_id::measMulti, engine.ActiveAnim());
	EXPECT_EQ(display_source::meas, engine.Shown().source);
	EXPECT_EQ(seqBefore, engine.Shown().seq);

	source.ready = true;
	engine.Run(5 * SECOND + 200);

	EXPECT_EQ(seqBefore + 1, engine.Shown().seq);
	EXPECT_EQ(PM2_5, engine.Shown().keys[0]);
}

TEST_F(DisplayEngineTest, EnablingTheMeasurementsAgainDoesNotSendTheShownPictureBackToTheWaitingOne) {
	engine.Apply(backgroundMsg(anim_id::measMulti, BOOT_ITEM));
	engine.Apply(measEnabledMsg(true));
	source.ready = true;
	engine.Run(0);
	source.ready = false;

	engine.Apply(measEnabledMsg(true));
	engine.Run(100);

	EXPECT_EQ(display_source::meas, engine.Shown().source);
}

TEST_F(DisplayEngineTest, EveryStartOfTheRandomPixelsGetsItsOwnSeed) {
	engine.Apply(errorMsg(display_error::sensorsFailed));
	engine.Run(0);
	const uint32_t firstSeed = engine.Shown().seed;
	EXPECT_NE(0u, firstSeed);
	engine.Apply(brightnessMsg(80));

	engine.Run(100);

	EXPECT_NE(0u, engine.Shown().seed);
	EXPECT_NE(firstSeed, engine.Shown().seed);
}
