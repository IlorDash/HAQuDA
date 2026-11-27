#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <set>
#include <string>

#include "HAQuDA_AnimationSet.h"
#include "HAQuDA_DisplayCatalog.h"
#include "MockLEDs.h"

namespace {
	const display_entry &row(display_event event) {
		return *HAQuDA_GetDisplayEntry(HAQuDA_ItemOf(event));
	}

	const display_entry &row(display_error error) {
		return *HAQuDA_GetDisplayEntry(HAQuDA_ItemOf(error));
	}

	void expectParams(const anim_params &exp, const anim_params &res) {
		EXPECT_EQ(exp.anim, res.anim);
		EXPECT_EQ(exp.color, res.color);
		EXPECT_EQ(exp.pauseMs, res.pauseMs);
		EXPECT_EQ(exp.endPauseMs, res.endPauseMs);
		EXPECT_EQ(exp.tailLen, res.tailLen);
		EXPECT_EQ(exp.fromPercent, res.fromPercent);
		EXPECT_EQ(exp.toPercent, res.toPercent);
		EXPECT_EQ(exp.stepPercent, res.stepPercent);
	}
} // namespace

TEST(DisplayCatalogTest, EveryItemHasARowThatKnowsItsNumber) {
	for (uint8_t item = 0; item < DISPLAY_ITEM_COUNT; item++) {
		const display_entry *entry = HAQuDA_GetDisplayEntry(item);
		ASSERT_NE(nullptr, entry) << "item " << static_cast<int>(item);
		EXPECT_EQ(item, entry->id);
	}
}

TEST(DisplayCatalogTest, NumbersOutsideTheTableHaveNoRow) {
	EXPECT_EQ(nullptr, HAQuDA_GetDisplayEntry(DISPLAY_ITEM_COUNT));
	EXPECT_EQ(nullptr, HAQuDA_GetDisplayEntry(DISPLAY_ITEM_NONE));
	EXPECT_STREQ("?", HAQuDA_DisplayItemName(DISPLAY_ITEM_NONE));
}

TEST(DisplayCatalogTest, ItemNumbersListEventsThenErrors) {
	EXPECT_EQ(0, HAQuDA_ItemOf(display_event::bootSnake));
	EXPECT_EQ(DISPLAY_EVENT_COUNT - 1, HAQuDA_ItemOf(display_event::apRunning));
	EXPECT_EQ(DISPLAY_EVENT_COUNT, HAQuDA_ItemOf(display_error::wifiConnectFailed));
	EXPECT_EQ(DISPLAY_ITEM_COUNT - 1, HAQuDA_ItemOf(display_error::ntpFailed));
	EXPECT_EQ(10, DISPLAY_ITEM_COUNT);
}

TEST(DisplayCatalogTest, NamesAreUniqueAndNotEmpty) {
	std::set<std::string> names;
	for (uint8_t item = 0; item < DISPLAY_ITEM_COUNT; item++) {
		const std::string name = HAQuDA_DisplayItemName(item);
		EXPECT_FALSE(name.empty());
		EXPECT_NE("?", name);
		names.insert(name);
	}
	EXPECT_EQ(static_cast<size_t>(DISPLAY_ITEM_COUNT), names.size());
}

TEST(DisplayCatalogTest, EveryRowPlaysAnAnimationThatExistsAndAcceptsItsParams) {
	testing::NiceMock<MockLEDs> leds;
	HAQuDA_AnimationSet set(leds);

	for (uint8_t item = 0; item < DISPLAY_ITEM_COUNT; item++) {
		const display_entry *entry = HAQuDA_GetDisplayEntry(item);
		HAQuDA_Animation *animation = set.Get(entry->anim.anim);
		ASSERT_NE(nullptr, animation) << entry->name;
		anim_context context = {};
		context.lampBrightPercent = 50;
		EXPECT_TRUE(animation->Begin(entry->anim, context)) << entry->name;
		animation->Stop();
	}
}

TEST(DisplayCatalogTest, RowsPlayEffectsNotMeasurementPictures) {
	for (uint8_t item = 0; item < DISPLAY_ITEM_COUNT; item++) {
		const anim_id anim = HAQuDA_GetDisplayEntry(item)->anim.anim;
		EXPECT_LE(static_cast<uint8_t>(anim), static_cast<uint8_t>(anim_id::staticColor)) << HAQuDA_DisplayItemName(item);
	}
}

TEST(DisplayCatalogTest, MinimumTimesFollowTheClasses) {
	for (uint8_t item = 0; item < DISPLAY_ITEM_COUNT; item++) {
		const display_entry &entry = *HAQuDA_GetDisplayEntry(item);
		if (entry.cls == display_class::event) {
			EXPECT_EQ(5000u, entry.minMs) << entry.name;
		} else {
			// the failed WiFi connection at boot is the one error that shows 5 s
			const uint32_t expected = (item == HAQuDA_ItemOf(display_error::wifiConnectFailed)) ? 5000u : 10000u;
			EXPECT_EQ(expected, entry.minMs) << entry.name;
		}
	}
}

TEST(DisplayCatalogTest, ClassesFollowTheTable) {
	EXPECT_EQ(display_class::event, row(display_event::bootSnake).cls);
	EXPECT_EQ(display_class::event, row(display_event::wifiJoined).cls);
	EXPECT_EQ(display_class::event, row(display_event::wifiReconnecting).cls);
	EXPECT_EQ(display_class::event, row(display_event::apRunning).cls);
	EXPECT_EQ(display_class::error, row(display_error::wifiConnectFailed).cls);
	EXPECT_EQ(display_class::error, row(display_error::wifiLost).cls);
	EXPECT_EQ(display_class::fatal, row(display_error::apFailed).cls);
	EXPECT_EQ(display_class::fatal, row(display_error::fsFailed).cls);
	EXPECT_EQ(display_class::error, row(display_error::sensorsFailed).cls);
	EXPECT_EQ(display_class::error, row(display_error::ntpFailed).cls);
}

TEST(DisplayCatalogTest, OnlyTheNtpErrorYieldsToTheBackground) {
	for (uint8_t item = 0; item < DISPLAY_ITEM_COUNT; item++) {
		const display_entry &entry = *HAQuDA_GetDisplayEntry(item);
		EXPECT_EQ(item == HAQuDA_ItemOf(display_error::ntpFailed), entry.yields) << entry.name;
	}
	EXPECT_EQ(50000u, DISPLAY_YIELD_BACKGROUND_MS);
}

TEST(DisplayCatalogTest, FatalRowsNeverEnd) {
	for (display_error fatal : {display_error::apFailed, display_error::fsFailed}) {
		EXPECT_EQ(display_end_cond::untilEvent, row(fatal).end);
		EXPECT_EQ(0, row(fatal).endsOn);
	}
}

TEST(DisplayCatalogTest, TimedRowsHaveNoEndSignal) {
	for (uint8_t item = 0; item < DISPLAY_ITEM_COUNT; item++) {
		const display_entry &entry = *HAQuDA_GetDisplayEntry(item);
		if (entry.end == display_end_cond::timed) {
			EXPECT_EQ(0, entry.endsOn) << entry.name;
		}
	}
}

TEST(DisplayCatalogTest, UntilEventRowsThatCanEndHaveAnEndSignal) {
	const display_end_cond until = display_end_cond::untilEvent;

	EXPECT_EQ(until, row(display_event::bootSnake).end);
	EXPECT_EQ(HAQuDA_EndBit(display_end::bootResult), row(display_event::bootSnake).endsOn);
	EXPECT_EQ(until, row(display_event::wifiReconnecting).end);
	EXPECT_EQ(HAQuDA_EndBit(display_end::wifiJoined), row(display_event::wifiReconnecting).endsOn);
	EXPECT_EQ(until, row(display_event::apRunning).end);
	EXPECT_EQ(HAQuDA_EndBit(display_end::measReady), row(display_event::apRunning).endsOn);
	EXPECT_EQ(until, row(display_error::sensorsFailed).end);
	EXPECT_EQ(HAQuDA_EndBit(display_end::sensorsRecovered), row(display_error::sensorsFailed).endsOn);
	EXPECT_EQ(until, row(display_error::ntpFailed).end);
	EXPECT_EQ(HAQuDA_EndBit(display_end::ntpSynced), row(display_error::ntpFailed).endsOn);
}

TEST(DisplayCatalogTest, TimedRowsAreJoinedWifiAndTheWifiErrors) {
	EXPECT_EQ(display_end_cond::timed, row(display_event::wifiJoined).end);
	EXPECT_EQ(display_end_cond::timed, row(display_error::wifiConnectFailed).end);
	EXPECT_EQ(display_end_cond::timed, row(display_error::wifiLost).end);
}

TEST(DisplayCatalogTest, WifiLostIsTimedForTenSecondsAndRaisesTheBootResult) {
	const display_entry &lost = row(display_error::wifiLost);

	EXPECT_EQ(display_end_cond::timed, lost.end);
	EXPECT_EQ(0, lost.endsOn);
	EXPECT_EQ(10000u, lost.minMs);
	EXPECT_EQ(HAQuDA_EndBit(display_end::bootResult), lost.raises);
}

TEST(DisplayCatalogTest, JoinedWifiRaisesTheBootResultAndTheJoinSignal) {
	EXPECT_EQ(HAQuDA_EndBit(display_end::bootResult) | HAQuDA_EndBit(display_end::wifiJoined), row(display_event::wifiJoined).raises);
}

TEST(DisplayCatalogTest, EveryEndSignalEndsAtLeastOneRow) {
	for (uint8_t signal = 0; signal < DISPLAY_END_COUNT; signal++) {
		bool used = false;
		for (uint8_t item = 0; item < DISPLAY_ITEM_COUNT; item++) {
			used = used || ((HAQuDA_GetDisplayEntry(item)->endsOn & HAQuDA_EndBit(static_cast<display_end>(signal))) != 0);
		}
		EXPECT_TRUE(used) << HAQuDA_DisplayEndName(static_cast<display_end>(signal));
	}
}

TEST(DisplayCatalogTest, EveryEndSignalHasAName) {
	for (uint8_t signal = 0; signal < DISPLAY_END_COUNT; signal++) {
		EXPECT_STRNE("?", HAQuDA_DisplayEndName(static_cast<display_end>(signal)));
	}
	EXPECT_STREQ("?", HAQuDA_DisplayEndName(display_end::count));
}

TEST(DisplayCatalogTest, EveryErrorEndsTheBoot) {
	for (uint8_t error = 0; error < DISPLAY_ERROR_COUNT; error++) {
		EXPECT_NE(0, row(static_cast<display_error>(error)).raises & HAQuDA_EndBit(display_end::bootResult));
	}
}

TEST(DisplayCatalogTest, BootIsALimeSnake) {
	expectParams(HAQuDA_SnakeParams(ANIM_COLOR_LIME, 200, 5), row(display_event::bootSnake).anim);
}

TEST(DisplayCatalogTest, JoinedWifiIsStaticGreen) {
	expectParams(HAQuDA_StaticColorParams(ANIM_COLOR_GREEN), row(display_event::wifiJoined).anim);
}

TEST(DisplayCatalogTest, ReconnectingIsALimeSnake) {
	expectParams(HAQuDA_SnakeParams(ANIM_COLOR_LIME, 200, 5), row(display_event::wifiReconnecting).anim);
}

TEST(DisplayCatalogTest, AccessPointRunningIsAquaFadingDown) {
	expectParams(HAQuDA_FadingParams(ANIM_COLOR_AQUA, 10, 100, 0, 5), row(display_event::apRunning).anim);
}

TEST(DisplayCatalogTest, WifiConnectFailedIsRedGrowing) {
	expectParams(HAQuDA_GrowingParams(ANIM_COLOR_RED, 100), row(display_error::wifiConnectFailed).anim);
}

TEST(DisplayCatalogTest, WifiLostIsStaticRed) {
	expectParams(HAQuDA_StaticColorParams(ANIM_COLOR_RED), row(display_error::wifiLost).anim);
}

TEST(DisplayCatalogTest, AccessPointFailedIsARedSnakeWithATailOfOne) {
	expectParams(HAQuDA_SnakeParams(ANIM_COLOR_RED, 100, 1), row(display_error::apFailed).anim);
}

TEST(DisplayCatalogTest, FileSystemFailedIsRedFadingDown) {
	expectParams(HAQuDA_FadingParams(ANIM_COLOR_RED, 10, 100, 0, 5), row(display_error::fsFailed).anim);
}

TEST(DisplayCatalogTest, SensorsFailedIsRandomPixels) {
	expectParams(HAQuDA_RandomPixelsParams(50, 500), row(display_error::sensorsFailed).anim);
}

TEST(DisplayCatalogTest, NtpFailedIsYellowFadingUp) {
	const anim_params &params = row(display_error::ntpFailed).anim;

	expectParams(HAQuDA_FadingParams(ANIM_COLOR_YELLOW, 10, 0, 100, 10), params);
	EXPECT_LT(params.fromPercent, params.toPercent);
}
