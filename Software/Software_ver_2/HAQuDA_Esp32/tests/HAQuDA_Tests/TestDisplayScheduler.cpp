#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "HAQuDA_DisplayScheduler.h"

namespace {
	const display_item BOOT = HAQuDA_ItemOf(display_event::bootSnake);
	const display_item JOINED = HAQuDA_ItemOf(display_event::wifiJoined);
	const display_item RECONNECTING = HAQuDA_ItemOf(display_event::wifiReconnecting);
	const display_item AP = HAQuDA_ItemOf(display_event::apRunning);
	const display_item CONNECT_FAILED = HAQuDA_ItemOf(display_error::wifiConnectFailed);
	const display_item LOST = HAQuDA_ItemOf(display_error::wifiLost);
	const display_item AP_FAILED = HAQuDA_ItemOf(display_error::apFailed);
	const display_item SENSORS = HAQuDA_ItemOf(display_error::sensorsFailed);
	const display_item NTP = HAQuDA_ItemOf(display_error::ntpFailed);

	constexpr uint32_t SECOND = 1000;

	std::vector<display_item> drain(HAQuDA_DisplayScheduler &scheduler, uint32_t nowMs, size_t limit = 30) {
		std::vector<display_item> items;
		for (size_t i = 0; i < limit; i++) {
			const display_pick pick = scheduler.Next(nowMs);
			if (pick.changed && (pick.item != DISPLAY_ITEM_NONE)) {
				items.push_back(pick.item);
			}
			if (pick.nextMs == DISPLAY_NO_DEADLINE) {
				break;
			}
			nowMs += pick.nextMs;
		}
		return items;
	}
} // namespace

TEST(DisplaySchedulerTest, NothingPostedShowsNothing) {
	HAQuDA_DisplayScheduler scheduler;

	const display_pick pick = scheduler.Next(0);

	EXPECT_FALSE(pick.changed);
	EXPECT_EQ(display_source::none, pick.source);
	EXPECT_EQ(anim_id::none, pick.anim);
	EXPECT_EQ(DISPLAY_ITEM_NONE, pick.item);
	EXPECT_EQ(DISPLAY_NO_DEADLINE, pick.nextMs);
}

TEST(DisplaySchedulerTest, PostedEventIsShownAtOnceAndReportedOnce) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_event::bootSnake);

	const display_pick first = scheduler.Next(100);
	const display_pick second = scheduler.Next(200);

	EXPECT_TRUE(first.changed);
	EXPECT_EQ(display_source::event, first.source);
	EXPECT_EQ(BOOT, first.item);
	EXPECT_EQ(anim_id::snake, first.anim);
	EXPECT_FALSE(second.changed);
	EXPECT_EQ(BOOT, second.item);
}

TEST(DisplaySchedulerTest, PostedErrorIsReportedAsError) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_error::sensorsFailed);

	const display_pick pick = scheduler.Next(0);

	EXPECT_EQ(display_source::error, pick.source);
	EXPECT_EQ(SENSORS, pick.item);
	EXPECT_EQ(anim_id::randomPixels, pick.anim);
}

TEST(DisplaySchedulerTest, InvalidIdsAreRefused) {
	HAQuDA_DisplayScheduler scheduler;

	EXPECT_FALSE(scheduler.Post(display_event::count));
	EXPECT_FALSE(scheduler.Post(display_error::count));
	EXPECT_FALSE(scheduler.Next(0).changed);
}

TEST(DisplaySchedulerTest, TimedEventPlaysItsMinimumTimeThenLeaves) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_event::wifiJoined);
	scheduler.Next(0);

	const display_pick before = scheduler.Next(5 * SECOND - 1);
	const display_pick after = scheduler.Next(5 * SECOND);

	EXPECT_EQ(JOINED, before.item);
	EXPECT_FALSE(before.changed);
	EXPECT_EQ(display_source::none, after.source);
	EXPECT_TRUE(after.changed);
}

TEST(DisplaySchedulerTest, TimedErrorPlaysTenSecondsAndTheBootConnectErrorFive) {
	HAQuDA_DisplayScheduler lost;
	lost.Post(display_error::wifiLost);
	lost.Next(0);
	HAQuDA_DisplayScheduler connectFailed;
	connectFailed.Post(display_error::wifiConnectFailed);
	connectFailed.Next(0);

	EXPECT_EQ(LOST, lost.Next(10 * SECOND - 1).item);
	EXPECT_EQ(display_source::none, lost.Next(10 * SECOND).source);
	EXPECT_EQ(CONNECT_FAILED, connectFailed.Next(5 * SECOND - 1).item);
	EXPECT_EQ(display_source::none, connectFailed.Next(5 * SECOND).source);
}

TEST(DisplaySchedulerTest, UntilEventItemIsNeverShorterThanItsMinimumTime) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_event::bootSnake);
	scheduler.Next(0);

	scheduler.End(display_end::bootResult);

	EXPECT_EQ(BOOT, scheduler.Next(1 * SECOND).item);
	EXPECT_EQ(BOOT, scheduler.Next(5 * SECOND - 1).item);
	EXPECT_EQ(display_source::none, scheduler.Next(5 * SECOND).source);
}

TEST(DisplaySchedulerTest, UntilEventItemWithoutAnEndSignalKeepsPlayingAlone) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_event::bootSnake);
	scheduler.Next(0);

	const display_pick later = scheduler.Next(3600 * SECOND);

	EXPECT_EQ(BOOT, later.item);
	EXPECT_FALSE(later.changed);
	EXPECT_EQ(DISPLAY_NO_DEADLINE, later.nextMs);
}

TEST(DisplaySchedulerTest, UntilEventItemPastItsMinimumYieldsAtOnceToAnItemPostedLater) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_event::bootSnake);
	scheduler.Next(0);

	// frequent calls while the item plays alone past its minimum time must not restart that time
	for (uint32_t nowMs = 6 * SECOND; nowMs < 20 * SECOND; nowMs += SECOND) {
		EXPECT_EQ(BOOT, scheduler.Next(nowMs).item);
	}
	scheduler.Post(display_event::wifiJoined);

	const display_pick pick = scheduler.Next(20 * SECOND);

	EXPECT_EQ(JOINED, pick.item);
	EXPECT_TRUE(pick.changed);
}

TEST(DisplaySchedulerTest, NextMsCountsDownToTheEndOfTheMinimumTime) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_event::wifiJoined);

	EXPECT_EQ(5 * SECOND, scheduler.Next(1000).nextMs);
	EXPECT_EQ(3 * SECOND, scheduler.Next(3000).nextMs);
}

TEST(DisplaySchedulerTest, EventDoesNotInterruptAnEvent) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_event::wifiJoined);
	scheduler.Next(0);
	scheduler.Post(display_event::wifiReconnecting);

	EXPECT_EQ(JOINED, scheduler.Next(2 * SECOND).item);
	EXPECT_EQ(RECONNECTING, scheduler.Next(5 * SECOND).item);
}

TEST(DisplaySchedulerTest, ErrorInterruptsAnEventAtOnce) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_event::wifiJoined);
	scheduler.Next(0);
	scheduler.Post(display_error::sensorsFailed);

	const display_pick pick = scheduler.Next(1 * SECOND);

	EXPECT_EQ(SENSORS, pick.item);
	EXPECT_TRUE(pick.changed);
}

TEST(DisplaySchedulerTest, InterruptedEventGoesBackToTheHeadWithItsFullTime) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_event::wifiJoined);
	scheduler.Next(0);
	scheduler.Post(display_error::wifiConnectFailed);
	// the error shows from 2 s for its 5 s, the event had 2 s of its 5 s behind it
	EXPECT_EQ(CONNECT_FAILED, scheduler.Next(2 * SECOND).item);

	const display_pick resumed = scheduler.Next(7 * SECOND);

	EXPECT_EQ(JOINED, resumed.item);
	EXPECT_TRUE(resumed.changed);
	EXPECT_EQ(JOINED, scheduler.Next(12 * SECOND - 1).item);
	EXPECT_EQ(display_source::none, scheduler.Next(12 * SECOND).source);
}

TEST(DisplaySchedulerTest, ErrorNeverInterruptsAnotherErrorInsideItsMinimumTime) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_error::wifiConnectFailed);
	scheduler.Next(0);
	scheduler.Post(display_error::wifiLost);

	EXPECT_EQ(CONNECT_FAILED, scheduler.Next(1 * SECOND).item);
	EXPECT_EQ(CONNECT_FAILED, scheduler.Next(5 * SECOND - 1).item);
	EXPECT_EQ(LOST, scheduler.Next(5 * SECOND).item);
}

TEST(DisplaySchedulerTest, ErrorsPlayInTheOrderTheyArrived) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_error::wifiConnectFailed);
	scheduler.Next(0);
	scheduler.Post(display_error::ntpFailed);
	scheduler.Post(display_error::sensorsFailed);

	EXPECT_EQ(NTP, scheduler.Next(5 * SECOND).item);
	EXPECT_EQ(SENSORS, scheduler.Next(15 * SECOND).item);
}

TEST(DisplaySchedulerTest, ErrorsGoBeforeEventsThatWereQueuedEarlier) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_error::wifiConnectFailed);
	scheduler.Next(0);
	scheduler.Post(display_event::wifiJoined);
	scheduler.Post(display_error::ntpFailed);

	EXPECT_EQ(NTP, scheduler.Next(5 * SECOND).item);
}

TEST(DisplaySchedulerTest, DeadStopInterruptsAnErrorInsideItsMinimumTime) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_error::wifiLost);
	scheduler.Next(0);
	scheduler.Post(display_error::apFailed);

	const display_pick pick = scheduler.Next(1 * SECOND);

	EXPECT_EQ(AP_FAILED, pick.item);
	EXPECT_TRUE(pick.changed);
}

TEST(DisplaySchedulerTest, DeadStopInterruptsAnEvent) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_event::bootSnake);
	scheduler.Next(0);
	scheduler.Post(display_error::fsFailed);

	EXPECT_EQ(HAQuDA_ItemOf(display_error::fsFailed), scheduler.Next(0).item);
}

TEST(DisplaySchedulerTest, DeadStopNeverLeavesAndNothingTakesItsPlace) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_event::bootSnake);
	scheduler.Next(0);
	scheduler.Post(display_error::apFailed);
	scheduler.Next(0);
	scheduler.Post(display_error::sensorsFailed);

	const display_pick later = scheduler.Next(3600 * SECOND);

	EXPECT_EQ(AP_FAILED, later.item);
	EXPECT_FALSE(later.changed);
	EXPECT_EQ(DISPLAY_NO_DEADLINE, later.nextMs);
}

TEST(DisplaySchedulerTest, ErrorPostedBeforeTheEventStartedDoesNotRequeueIt) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_event::wifiJoined);
	scheduler.Post(display_error::wifiConnectFailed);

	EXPECT_EQ(CONNECT_FAILED, scheduler.Next(0).item);
	EXPECT_EQ(JOINED, scheduler.Next(5 * SECOND).item);
}

TEST(DisplaySchedulerTest, ActiveUntilEventErrorAndEventRotateEachForItsClassTime) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_event::apRunning);
	scheduler.Next(0);
	scheduler.Post(display_error::sensorsFailed);

	// the error interrupts at 1 s and plays 10 s, the aqua event resumes for 5 s, then both alternate
	EXPECT_EQ(SENSORS, scheduler.Next(1 * SECOND).item);
	EXPECT_EQ(SENSORS, scheduler.Next(11 * SECOND - 1).item);
	EXPECT_EQ(AP, scheduler.Next(11 * SECOND).item);
	EXPECT_EQ(AP, scheduler.Next(16 * SECOND - 1).item);
	EXPECT_EQ(SENSORS, scheduler.Next(16 * SECOND).item);
	EXPECT_EQ(AP, scheduler.Next(26 * SECOND).item);
	EXPECT_EQ(SENSORS, scheduler.Next(31 * SECOND).item);
}

TEST(DisplaySchedulerTest, RotationIsNotStarvedByTheErrorsPriority) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_error::sensorsFailed);
	scheduler.Post(display_event::wifiReconnecting);
	scheduler.Next(0);

	const std::vector<display_item> shown = drain(scheduler, 10 * SECOND, 6);

	EXPECT_EQ((std::vector<display_item>{RECONNECTING, SENSORS, RECONNECTING, SENSORS, RECONNECTING, SENSORS}), shown);
}

TEST(DisplaySchedulerTest, EndedItemDropsOutOfTheRotation) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_event::apRunning);
	scheduler.Next(0);
	scheduler.Post(display_error::sensorsFailed);
	scheduler.Next(1 * SECOND);
	scheduler.MeasReady(true);

	EXPECT_EQ(SENSORS, scheduler.Next(11 * SECOND).item);
	EXPECT_EQ(SENSORS, scheduler.Next(100 * SECOND).item);
	EXPECT_FALSE(scheduler.Next(200 * SECOND).changed);
}

TEST(DisplaySchedulerTest, NewTimedEventPlaysBeforeTheRotationContinues) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_error::sensorsFailed);
	scheduler.Post(display_event::wifiReconnecting);
	scheduler.Next(0);
	scheduler.Next(10 * SECOND);
	scheduler.Post(display_event::wifiJoined);

	EXPECT_EQ(JOINED, scheduler.Next(15 * SECOND).item);
}

TEST(DisplaySchedulerTest, PostingAResultEndsTheBootSnake) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_event::bootSnake);
	scheduler.Next(0);
	scheduler.Post(display_event::wifiJoined);

	EXPECT_EQ(BOOT, scheduler.Next(4 * SECOND).item);
	EXPECT_EQ(JOINED, scheduler.Next(5 * SECOND).item);
	EXPECT_EQ(display_source::none, scheduler.Next(10 * SECOND).source);
}

TEST(DisplaySchedulerTest, AnyErrorEndsTheBootSnakeToo) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_event::bootSnake);
	scheduler.Next(0);
	scheduler.Post(display_error::sensorsFailed);

	EXPECT_EQ(SENSORS, scheduler.Next(1 * SECOND).item);
	EXPECT_EQ(SENSORS, scheduler.Next(11 * SECOND).item);
	EXPECT_FALSE(scheduler.Next(11 * SECOND).changed);
	EXPECT_EQ(0, scheduler.QueuedCount());
}

TEST(DisplaySchedulerTest, ApRunningEndsTheBootSnake) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_event::bootSnake);
	scheduler.Next(0);
	scheduler.Post(display_event::apRunning);

	EXPECT_EQ(AP, scheduler.Next(5 * SECOND).item);
	EXPECT_EQ(AP, scheduler.Next(60 * SECOND).item);
}

TEST(DisplaySchedulerTest, EndSignalRemovesAWaitingItemThatNeverPlayed) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_error::wifiConnectFailed);
	scheduler.Next(0);
	scheduler.Post(display_event::bootSnake);
	ASSERT_EQ(1, scheduler.QueuedCount());

	scheduler.End(display_end::bootResult);

	EXPECT_EQ(0, scheduler.QueuedCount());
	EXPECT_EQ(display_source::none, scheduler.Next(5 * SECOND).source);
}

TEST(DisplaySchedulerTest, WifiLostIsNotCutShortByTheReconnectSnake) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_error::wifiLost);
	scheduler.Next(0);

	scheduler.Post(display_event::wifiReconnecting);

	EXPECT_EQ(LOST, scheduler.Next(9 * SECOND).item);
	EXPECT_EQ(RECONNECTING, scheduler.Next(10 * SECOND).item);
}

TEST(DisplaySchedulerTest, WifiLostPostedWithTheReconnectSnakeInOneBatchPlaysFirstThenTheSnake) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_error::wifiLost);
	scheduler.Post(display_event::wifiReconnecting);

	const display_pick first = scheduler.Next(0);
	const display_pick during = scheduler.Next(10 * SECOND - 1);
	const display_pick after = scheduler.Next(10 * SECOND);

	EXPECT_EQ(LOST, first.item);
	EXPECT_EQ(LOST, during.item);
	EXPECT_FALSE(during.changed);
	EXPECT_EQ(RECONNECTING, after.item);
	EXPECT_TRUE(after.changed);
}

TEST(DisplaySchedulerTest, WifiLostPostedWithTheReconnectSnakeAndTheJoinInOneBatchNeverShowsTheSnake) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_error::wifiLost);
	scheduler.Post(display_event::wifiReconnecting);
	scheduler.Post(display_event::wifiJoined);

	const std::vector<display_item> shown = drain(scheduler, 0);

	ASSERT_EQ(2u, shown.size());
	EXPECT_EQ(LOST, shown[0]);
	EXPECT_EQ(JOINED, shown[1]);
}

TEST(DisplaySchedulerTest, WifiLostIsFollowedByTheJoinedWifiAfterItsMinimumTime) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_error::wifiLost);
	scheduler.Next(0);

	scheduler.Post(display_event::wifiJoined);

	EXPECT_EQ(LOST, scheduler.Next(9 * SECOND).item);
	EXPECT_EQ(JOINED, scheduler.Next(10 * SECOND).item);
}

TEST(DisplaySchedulerTest, ReconnectSnakeEndsWhenTheWifiIsJoined) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_event::wifiReconnecting);
	scheduler.Next(0);

	scheduler.End(display_end::wifiJoined);

	EXPECT_EQ(RECONNECTING, scheduler.Next(4 * SECOND).item);
	EXPECT_EQ(display_source::none, scheduler.Next(5 * SECOND).source);
}

TEST(DisplaySchedulerTest, SensorsErrorEndsOnRecoveryAfterItsMinimumTime) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_error::sensorsFailed);
	scheduler.Next(0);

	scheduler.End(display_end::sensorsRecovered);

	EXPECT_EQ(SENSORS, scheduler.Next(9 * SECOND).item);
	EXPECT_EQ(display_source::none, scheduler.Next(10 * SECOND).source);
}

TEST(DisplaySchedulerTest, NtpErrorPostedWhileActiveIsIgnored) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_error::ntpFailed);
	scheduler.Next(0);

	EXPECT_TRUE(scheduler.Post(display_error::ntpFailed));
	EXPECT_TRUE(scheduler.Post(display_error::ntpFailed));

	EXPECT_EQ(0, scheduler.QueuedCount());
	EXPECT_EQ(NTP, scheduler.Next(11 * SECOND).item);
	EXPECT_FALSE(scheduler.Next(11 * SECOND).changed);
}

TEST(DisplaySchedulerTest, ActiveUntilEventItemIsNotQueuedTwiceEvenWithOthersBetween) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_error::wifiConnectFailed);
	scheduler.Next(0);
	scheduler.Post(display_error::sensorsFailed);
	scheduler.Post(display_error::wifiConnectFailed);
	scheduler.Post(display_error::sensorsFailed);

	EXPECT_EQ(2, scheduler.QueuedCount());
}

TEST(DisplaySchedulerTest, ErrorCanBePostedAgainOnceItEnded) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_error::sensorsFailed);
	scheduler.Next(0);
	scheduler.End(display_end::sensorsRecovered);

	scheduler.Post(display_error::sensorsFailed);

	// the new occurrence plays after the minimum time of the ended one; the same item again still counts as a change
	EXPECT_EQ(1, scheduler.QueuedCount());
	const display_pick again = scheduler.Next(10 * SECOND);
	EXPECT_EQ(SENSORS, again.item);
	EXPECT_TRUE(again.changed);
}

TEST(DisplaySchedulerTest, ConsecutiveDuplicatesAreCoalesced) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_error::wifiConnectFailed);
	scheduler.Next(0);

	scheduler.Post(display_event::wifiJoined);
	scheduler.Post(display_event::wifiJoined);
	scheduler.Post(display_event::wifiJoined);

	EXPECT_EQ(1, scheduler.QueuedCount());
}

TEST(DisplaySchedulerTest, NonConsecutiveDuplicatesAreKept) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_error::apFailed);
	scheduler.Next(0);

	scheduler.Post(display_event::wifiJoined);
	scheduler.Post(display_error::wifiConnectFailed);
	scheduler.Post(display_event::wifiJoined);

	EXPECT_EQ(3, scheduler.QueuedCount());
}

TEST(DisplaySchedulerTest, PostEqualToThePlayingTimedItemIsCoalesced) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_event::wifiJoined);
	scheduler.Next(0);

	scheduler.Post(display_event::wifiJoined);

	EXPECT_EQ(0, scheduler.QueuedCount());
	EXPECT_EQ(display_source::none, scheduler.Next(5 * SECOND).source);
}

TEST(DisplaySchedulerTest, OverflowDropsTheOldestTimedEventOnly) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_error::apFailed);
	scheduler.Next(0);
	for (int i = 0; i < 4; i++) {
		scheduler.Post(display_event::wifiJoined);
		scheduler.Post(display_error::wifiConnectFailed);
	}
	ASSERT_EQ(DISPLAY_QUEUE_CAPACITY, scheduler.QueuedCount());
	ASSERT_EQ(0, scheduler.DroppedCount());

	EXPECT_TRUE(scheduler.Post(display_event::wifiJoined));

	EXPECT_EQ(DISPLAY_QUEUE_CAPACITY, scheduler.QueuedCount());
	EXPECT_EQ(1, scheduler.DroppedCount());
}

TEST(DisplaySchedulerTest, DroppedEventIsTheOldestOne) {
	HAQuDA_DisplayScheduler scheduler;
	// the error plays and the queue fills behind it: joined, connect-failed, joined, ... starting with the oldest joined
	scheduler.Post(display_error::wifiLost);
	scheduler.Next(0);
	for (int i = 0; i < 4; i++) {
		scheduler.Post(display_event::wifiJoined);
		scheduler.Post(display_error::wifiConnectFailed);
	}
	scheduler.Post(display_event::wifiJoined);

	const std::vector<display_item> shown = drain(scheduler, 10 * SECOND);

	size_t joinedNum = 0;
	size_t failedNum = 0;
	for (display_item item : shown) {
		joinedNum += (item == JOINED) ? 1 : 0;
		failedNum += (item == CONNECT_FAILED) ? 1 : 0;
	}
	// five joined were posted and one was dropped, all four errors stayed
	EXPECT_EQ(4u, joinedNum);
	EXPECT_EQ(4u, failedNum);
}

TEST(DisplaySchedulerTest, QueueFullOfItemsThatMustNotBeDroppedRefusesTheNewOne) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_error::apFailed);
	scheduler.Next(0);
	scheduler.Post(display_error::wifiConnectFailed);
	scheduler.Post(display_error::ntpFailed);
	scheduler.Post(display_error::wifiConnectFailed);
	scheduler.Post(display_error::sensorsFailed);
	scheduler.Post(display_error::wifiConnectFailed);
	scheduler.Post(display_error::wifiLost);
	scheduler.Post(display_error::wifiConnectFailed);
	scheduler.Post(display_event::wifiReconnecting);
	ASSERT_EQ(DISPLAY_QUEUE_CAPACITY, scheduler.QueuedCount());

	// the boot snake raises no signal that would end a queued item, so the queue stays full
	EXPECT_FALSE(scheduler.Post(display_event::bootSnake));

	EXPECT_EQ(DISPLAY_QUEUE_CAPACITY, scheduler.QueuedCount());
	EXPECT_EQ(0, scheduler.DroppedCount());
}

TEST(DisplaySchedulerTest, WaitingItemPlaysWhenNothingElseIsActive) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.SetBackground(anim_id::measMulti, HAQuDA_ItemOf(display_event::bootSnake));

	const display_pick pick = scheduler.Next(0);

	EXPECT_TRUE(pick.changed);
	EXPECT_EQ(display_source::waiting, pick.source);
	EXPECT_EQ(BOOT, pick.item);
	EXPECT_EQ(anim_id::snake, pick.anim);
	EXPECT_EQ(DISPLAY_NO_DEADLINE, pick.nextMs);
}

TEST(DisplaySchedulerTest, MeasurementsReplaceTheWaitingItemOnlyWhenReady) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.SetBackground(anim_id::measMulti, BOOT);
	scheduler.Next(0);

	scheduler.MeasReady(true);
	const display_pick ready = scheduler.Next(1000);
	scheduler.MeasReady(false);
	const display_pick notReady = scheduler.Next(2000);

	EXPECT_TRUE(ready.changed);
	EXPECT_EQ(display_source::meas, ready.source);
	EXPECT_EQ(anim_id::measMulti, ready.anim);
	EXPECT_EQ(DISPLAY_ITEM_NONE, ready.item);
	EXPECT_TRUE(notReady.changed);
	EXPECT_EQ(display_source::waiting, notReady.source);
}

TEST(DisplaySchedulerTest, StationBootShowsGreenThenTheWaitingSnakeThenTheMeasurements) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_event::bootSnake);
	scheduler.Next(0);
	scheduler.SetBackground(anim_id::measMulti, BOOT);
	scheduler.Post(display_event::wifiJoined);

	const display_pick green = scheduler.Next(5 * SECOND);
	const display_pick waiting = scheduler.Next(10 * SECOND);
	const display_pick stillWaiting = scheduler.Next(18 * SECOND);
	scheduler.MeasReady(true);
	const display_pick meas = scheduler.Next(18 * SECOND + 100);

	EXPECT_EQ(display_source::event, green.source);
	EXPECT_EQ(anim_id::staticColor, green.anim);
	EXPECT_EQ(display_source::waiting, waiting.source);
	EXPECT_EQ(BOOT, waiting.item);
	EXPECT_FALSE(stillWaiting.changed);
	EXPECT_EQ(display_source::meas, meas.source);
}

TEST(DisplaySchedulerTest, ApWaitingPictureStaysTheFadingUntilTheMeasurementsAreReady) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_event::apRunning);
	scheduler.SetBackground(anim_id::measMulti, AP);
	scheduler.Next(0);

	const display_pick later = scheduler.Next(60 * SECOND);
	scheduler.MeasReady(true);
	const display_pick ready = scheduler.Next(60 * SECOND + 100);

	EXPECT_EQ(AP, later.item);
	EXPECT_EQ(anim_id::fading, later.anim);
	EXPECT_EQ(display_source::meas, ready.source);
}

TEST(DisplaySchedulerTest, ReadyWithoutAMeasurementAnimationKeepsTheWaitingItem) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.SetBackground(anim_id::none, BOOT);
	scheduler.MeasReady(true);

	EXPECT_EQ(display_source::waiting, scheduler.Next(0).source);
}

TEST(DisplaySchedulerTest, NothingWaitingAndNotReadyShowsNothing) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.SetBackground(anim_id::measStandard, DISPLAY_ITEM_NONE);

	EXPECT_EQ(display_source::none, scheduler.Next(0).source);
}

TEST(DisplaySchedulerTest, InvalidWaitingItemIsIgnored) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.SetBackground(anim_id::measStandard, 200);

	EXPECT_EQ(display_source::none, scheduler.Next(0).source);
}

TEST(DisplaySchedulerTest, EventEndsAndTheMeasurementsComeBack) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.SetBackground(anim_id::measNight, DISPLAY_ITEM_NONE);
	scheduler.MeasReady(true);
	scheduler.Next(0);
	scheduler.Post(display_event::wifiJoined);

	const display_pick during = scheduler.Next(1000);
	const display_pick after = scheduler.Next(6000);

	EXPECT_EQ(display_source::event, during.source);
	EXPECT_EQ(display_source::meas, after.source);
	EXPECT_EQ(anim_id::measNight, after.anim);
	EXPECT_TRUE(after.changed);
}

TEST(DisplaySchedulerTest, ErrorInterruptsTheMeasurementsAndTheyReturn) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.SetBackground(anim_id::measStandard, DISPLAY_ITEM_NONE);
	scheduler.MeasReady(true);
	scheduler.Next(0);
	scheduler.Post(display_error::wifiLost);

	EXPECT_EQ(display_source::error, scheduler.Next(100).source);
	EXPECT_EQ(display_source::meas, scheduler.Next(10 * SECOND + 100).source);
}

TEST(DisplaySchedulerTest, MeasurementsReadyEndTheApPictureAfterItsMinimumTime) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.SetBackground(anim_id::measMulti, DISPLAY_ITEM_NONE);
	scheduler.Post(display_event::apRunning);
	scheduler.Next(0);

	scheduler.MeasReady(true);

	EXPECT_EQ(AP, scheduler.Next(4 * SECOND).item);
	EXPECT_EQ(display_source::meas, scheduler.Next(5 * SECOND).source);
}

TEST(DisplaySchedulerTest, ApPictureWaitsForTheMeasurementsAndFadesUntilThen) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.SetBackground(anim_id::measMulti, DISPLAY_ITEM_NONE);
	scheduler.Post(display_event::apRunning);
	scheduler.Next(0);

	const display_pick later = scheduler.Next(120 * SECOND);

	EXPECT_EQ(AP, later.item);
	EXPECT_FALSE(later.changed);
}

TEST(DisplaySchedulerTest, ApPictureIsNotQueuedWhenTheMeasurementsAreAlreadyReady) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.SetBackground(anim_id::measMulti, DISPLAY_ITEM_NONE);
	scheduler.MeasReady(true);

	EXPECT_TRUE(scheduler.Post(display_event::apRunning));

	EXPECT_EQ(display_source::meas, scheduler.Next(0).source);
}

TEST(DisplaySchedulerTest, SensorsRecoveredGoesStraightToTheMeasurements) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.SetBackground(anim_id::measMulti, DISPLAY_ITEM_NONE);
	scheduler.MeasReady(true);
	scheduler.Post(display_error::sensorsFailed);
	scheduler.Next(0);

	scheduler.End(display_end::sensorsRecovered);

	EXPECT_EQ(display_source::meas, scheduler.Next(10 * SECOND).source);
}

TEST(DisplaySchedulerTest, MinimumTimeSurvivesTheWrapOfTheClock) {
	HAQuDA_DisplayScheduler scheduler;
	const uint32_t start = 0xFFFFFFFFu - 2000;
	scheduler.Post(display_event::wifiJoined);
	scheduler.Next(start);

	// 4999 ms later the counter has wrapped
	const display_pick before = scheduler.Next(start + 4999);
	const display_pick after = scheduler.Next(start + 5000);

	EXPECT_EQ(JOINED, before.item);
	EXPECT_EQ(display_source::none, after.source);
	EXPECT_LT(start + 5000, start);
}

TEST(DisplaySchedulerTest, NextMsIsRightAcrossTheWrap) {
	HAQuDA_DisplayScheduler scheduler;
	const uint32_t start = 0xFFFFFFFFu - 1000;
	scheduler.Post(display_event::wifiJoined);
	scheduler.Next(start);

	EXPECT_EQ(3 * SECOND, scheduler.Next(start + 2 * SECOND).nextMs);
}

TEST(DisplaySchedulerTest, RotationKeepsItsTimesAcrossTheWrap) {
	HAQuDA_DisplayScheduler scheduler;
	const uint32_t start = 0xFFFFFFFFu - 3000;
	scheduler.Post(display_error::sensorsFailed);
	scheduler.Post(display_event::wifiReconnecting);
	scheduler.Next(start);

	EXPECT_EQ(SENSORS, scheduler.Next(start + 9999).item);
	EXPECT_EQ(RECONNECTING, scheduler.Next(start + 10000).item);
	EXPECT_EQ(RECONNECTING, scheduler.Next(start + 14999).item);
	EXPECT_EQ(SENSORS, scheduler.Next(start + 15000).item);
}

TEST(DisplaySchedulerTest, LampLightIsTheBackgroundAndEventsShowOverIt) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.SetSolid(false);

	const display_pick light = scheduler.Next(0);
	scheduler.Post(display_event::wifiJoined);
	const display_pick during = scheduler.Next(1000);
	const display_pick after = scheduler.Next(6000);

	EXPECT_EQ(display_source::solid, light.source);
	EXPECT_EQ(anim_id::staticColor, light.anim);
	EXPECT_EQ(DISPLAY_NO_DEADLINE, light.nextMs);
	EXPECT_EQ(display_source::event, during.source);
	EXPECT_EQ(display_source::solid, after.source);
	EXPECT_TRUE(after.changed);
}

TEST(DisplaySchedulerTest, LampLightWinsOverReadyMeasurements) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.SetBackground(anim_id::measMulti, DISPLAY_ITEM_NONE);
	scheduler.MeasReady(true);
	scheduler.Next(0);

	scheduler.SetSolid(false);

	const display_pick pick = scheduler.Next(100);
	EXPECT_EQ(display_source::solid, pick.source);
	EXPECT_TRUE(pick.changed);
}

TEST(DisplaySchedulerTest, ANewMeasurementBackgroundEndsTheLampPicture) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.SetSolid(true);
	scheduler.Next(0);

	scheduler.SetBackground(anim_id::measMulti, DISPLAY_ITEM_NONE);
	scheduler.MeasReady(true);

	EXPECT_EQ(display_source::meas, scheduler.Next(100).source);
	EXPECT_TRUE(scheduler.Post(display_event::wifiJoined));
	EXPECT_EQ(display_source::event, scheduler.Next(200).source);
}

TEST(DisplaySchedulerTest, LampOffIgnoresEventsButShowsErrors) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.SetSolid(true);
	scheduler.Next(0);

	EXPECT_TRUE(scheduler.Post(display_event::wifiJoined));
	EXPECT_EQ(display_source::solid, scheduler.Next(100).source);

	scheduler.Post(display_error::wifiLost);
	const display_pick error = scheduler.Next(200);
	EXPECT_EQ(display_source::error, error.source);
	EXPECT_EQ(LOST, error.item);

	EXPECT_EQ(display_source::solid, scheduler.Next(10 * SECOND + 200).source);
}

TEST(DisplaySchedulerTest, LampOffDropsThePlayingAndTheQueuedEvents) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_event::wifiJoined);
	scheduler.Post(display_event::wifiReconnecting);
	ASSERT_EQ(display_source::event, scheduler.Next(0).source);

	scheduler.SetSolid(true);

	EXPECT_EQ(display_source::solid, scheduler.Next(100).source);
	EXPECT_EQ(0, scheduler.QueuedCount());
}

TEST(DisplaySchedulerTest, LampOffKeepsAnErrorThatIsPlaying) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_error::sensorsFailed);
	ASSERT_EQ(display_source::error, scheduler.Next(0).source);

	scheduler.SetSolid(true);

	EXPECT_EQ(SENSORS, scheduler.Next(100).item);
}

TEST(DisplaySchedulerTest, LampLightEndsTheApPictureAfterItsMinimumTime) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.Post(display_event::apRunning);
	scheduler.Next(0);

	scheduler.SetSolid(false);

	EXPECT_EQ(AP, scheduler.Next(4 * SECOND).item);
	EXPECT_EQ(display_source::solid, scheduler.Next(5 * SECOND).source);
}

TEST(DisplaySchedulerTest, ApPictureIsNotQueuedOverTheLampLight) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.SetSolid(false);

	EXPECT_TRUE(scheduler.Post(display_event::apRunning));

	EXPECT_EQ(display_source::solid, scheduler.Next(0).source);
}

TEST(DisplaySchedulerTest, ANewLampColourRestartsTheSolidPictureButNotAnEventOverIt) {
	HAQuDA_DisplayScheduler scheduler;
	scheduler.SetSolid(false);
	scheduler.Next(0);

	scheduler.SetSolid(false);
	EXPECT_TRUE(scheduler.Next(100).changed);
	EXPECT_FALSE(scheduler.Next(200).changed);

	scheduler.Post(display_event::wifiReconnecting);
	scheduler.Next(300);
	scheduler.SetSolid(false);
	const display_pick pick = scheduler.Next(400);
	EXPECT_EQ(display_source::event, pick.source);
	EXPECT_FALSE(pick.changed);
}
