#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "HAQuDA_DisplayEngine.h"
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
