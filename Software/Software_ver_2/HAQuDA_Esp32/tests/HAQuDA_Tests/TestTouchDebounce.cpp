#include <gtest/gtest.h>

#include <cstdint>

#include "HAQuDA_TouchDebounce.h"

TEST(TouchDebounceTest, ActiveIsFalseBeforeTheDebounceTime) {
	const uint32_t idleAtMs = 1000;
	const uint32_t touchAtMs = 1100;

	const bool exp_active = false;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, touchAtMs);
	deb.Update(true, touchAtMs + TOUCH_DEBOUNCE_MS - 1);

	bool res;

	res = deb.Active();

	EXPECT_EQ(exp_active, res);
}

TEST(TouchDebounceTest, ActiveAtExactlyTheDebounceTime) {
	const uint32_t idleAtMs = 1000;
	const uint32_t touchAtMs = 1100;

	const bool exp_active = true;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, touchAtMs);
	deb.Update(true, touchAtMs + TOUCH_DEBOUNCE_MS);

	bool res;

	res = deb.Active();

	EXPECT_EQ(exp_active, res);
}

TEST(TouchDebounceTest, ActiveStaysTrueWhileHeldAfterTheDebounceTime) {
	const uint32_t idleAtMs = 1000;
	const uint32_t touchAtMs = 1100;

	const bool exp_active = true;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, touchAtMs);
	deb.Update(true, touchAtMs + TOUCH_DEBOUNCE_MS);
	deb.Update(true, touchAtMs + 5 * TOUCH_DEBOUNCE_MS);

	bool res;

	res = deb.Active();

	EXPECT_EQ(exp_active, res);
}

TEST(TouchDebounceTest, ActiveIsFalseBeforeAnyUpdate) {
	const bool exp_active = false;

	HAQuDA_TouchDebounce deb(true);

	bool res;

	res = deb.Active();

	EXPECT_EQ(exp_active, res);
}

TEST(TouchDebounceTest, ActiveIsFalseAfterTheFirstUpdateOnTheActiveLevel) {
	const uint32_t bootMs = 1000;

	const bool exp_active = false;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(true, bootMs);

	bool res;

	res = deb.Active();

	EXPECT_EQ(exp_active, res);
}

TEST(TouchDebounceTest, ActiveBecomesTrueForALevelHeldFromBoot) {
	const uint32_t bootMs = 1000;

	const bool exp_active = true;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(true, bootMs);
	deb.Update(true, bootMs + TOUCH_DEBOUNCE_MS);

	bool res;

	res = deb.Active();

	EXPECT_EQ(exp_active, res);
}

TEST(TouchDebounceTest, ActiveStaysTrueOneMillisecondBeforeTheReleaseDebounceTime) {
	const uint32_t bootMs = 1000;
	const uint32_t releaseMs = 2000;

	const bool exp_active = true;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(true, bootMs);
	deb.Update(true, bootMs + TOUCH_DEBOUNCE_MS);
	deb.Update(false, releaseMs);
	deb.Update(false, releaseMs + TOUCH_DEBOUNCE_MS - 1);

	bool res;

	res = deb.Active();

	EXPECT_EQ(exp_active, res);
}

TEST(TouchDebounceTest, ActiveEndsAtExactlyTheReleaseDebounceTime) {
	const uint32_t bootMs = 1000;
	const uint32_t releaseMs = 2000;

	const bool exp_active = false;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(true, bootMs);
	deb.Update(true, bootMs + TOUCH_DEBOUNCE_MS);
	deb.Update(false, releaseMs);
	deb.Update(false, releaseMs + TOUCH_DEBOUNCE_MS);

	bool res;

	res = deb.Active();

	EXPECT_EQ(exp_active, res);
}

TEST(TouchDebounceTest, SparseCallsStillBecomeActive) {
	const uint32_t idleAtMs = 1000;
	const uint32_t touchAtMs = 1100;
	const uint32_t lateCallAtMs = 5000;

	const bool exp_active = true;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, touchAtMs);
	deb.Update(true, lateCallAtMs);

	bool res;

	res = deb.Active();

	EXPECT_EQ(exp_active, res);
}

TEST(TouchDebounceTest, SpikeShorterThanDebounceStaysInactive) {
	const uint32_t idleAtMs = 1000;
	const uint32_t spikeAtMs = 1100;
	const uint32_t spikeLengthMs = 20;

	const bool exp_active = false;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, spikeAtMs);
	deb.Update(false, spikeAtMs + spikeLengthMs);
	deb.Update(false, spikeAtMs + spikeLengthMs + TOUCH_DEBOUNCE_MS);

	bool res;

	res = deb.Active();

	EXPECT_EQ(exp_active, res);
}

TEST(TouchDebounceTest, BounceOnTouchEdgeRestartsDebounceTime) {
	const uint32_t idleAtMs = 1000;
	const uint32_t firstTouchAtMs = 1100;
	const uint32_t bounceAtMs = 1110;
	const uint32_t secondTouchAtMs = 1120;

	const bool exp_active = false;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, firstTouchAtMs);
	deb.Update(false, bounceAtMs);
	deb.Update(true, secondTouchAtMs);
	deb.Update(true, secondTouchAtMs + TOUCH_DEBOUNCE_MS - 1);

	bool res;

	res = deb.Active();

	EXPECT_EQ(exp_active, res);
}

TEST(TouchDebounceTest, BounceOnTouchEdgeBecomesActiveAfterFullDebounceFromLastEdge) {
	const uint32_t idleAtMs = 1000;
	const uint32_t firstTouchAtMs = 1100;
	const uint32_t bounceAtMs = 1110;
	const uint32_t secondTouchAtMs = 1120;

	const bool exp_active = true;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, firstTouchAtMs);
	deb.Update(false, bounceAtMs);
	deb.Update(true, secondTouchAtMs);
	deb.Update(true, secondTouchAtMs + TOUCH_DEBOUNCE_MS);

	bool res;

	res = deb.Active();

	EXPECT_EQ(exp_active, res);
}

TEST(TouchDebounceTest, ActiveAgainAfterStableReleaseAndSecondTouch) {
	const uint32_t idleAtMs = 1000;
	const uint32_t touchAtMs = 1100;
	const uint32_t releaseAtMs = 1200;
	const uint32_t secondTouchAtMs = 1300;

	const bool exp_active = true;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, touchAtMs);
	deb.Update(true, touchAtMs + TOUCH_DEBOUNCE_MS);
	deb.Update(false, releaseAtMs);
	deb.Update(false, releaseAtMs + TOUCH_DEBOUNCE_MS);
	deb.Update(true, secondTouchAtMs);
	deb.Update(true, secondTouchAtMs + TOUCH_DEBOUNCE_MS);

	bool res;

	res = deb.Active();

	EXPECT_EQ(exp_active, res);
}

TEST(TouchDebounceTest, ReleaseShorterThanDebounceKeepsActive) {
	const uint32_t idleAtMs = 1000;
	const uint32_t touchAtMs = 1100;
	const uint32_t releaseAtMs = 1200;
	const uint32_t releaseLengthMs = TOUCH_DEBOUNCE_MS - 1;
	const uint32_t laterCallAtMs = 1500;

	const bool exp_active = true;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, touchAtMs);
	deb.Update(true, touchAtMs + TOUCH_DEBOUNCE_MS);
	deb.Update(false, releaseAtMs);
	deb.Update(false, releaseAtMs + releaseLengthMs);
	deb.Update(true, releaseAtMs + releaseLengthMs + 1);
	deb.Update(true, laterCallAtMs);

	bool res;

	res = deb.Active();

	EXPECT_EQ(exp_active, res);
}

TEST(TouchDebounceTest, SpikeDuringStableReleaseStaysInactive) {
	const uint32_t idleAtMs = 1000;
	const uint32_t touchAtMs = 1100;
	const uint32_t releaseAtMs = 1200;
	const uint32_t spikeAtMs = 1300;
	const uint32_t spikeLengthMs = 10;

	const bool exp_active = false;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, touchAtMs);
	deb.Update(true, touchAtMs + TOUCH_DEBOUNCE_MS);
	deb.Update(false, releaseAtMs);
	deb.Update(false, releaseAtMs + TOUCH_DEBOUNCE_MS);
	deb.Update(true, spikeAtMs);
	deb.Update(false, spikeAtMs + spikeLengthMs);
	deb.Update(false, spikeAtMs + spikeLengthMs + TOUCH_DEBOUNCE_MS);

	bool res;

	res = deb.Active();

	EXPECT_EQ(exp_active, res);
}

TEST(TouchDebounceTest, ActiveLowInputBecomesActiveOnLowLevel) {
	const uint32_t idleAtMs = 1000;
	const uint32_t touchAtMs = 1100;

	const bool exp_active = true;

	HAQuDA_TouchDebounce deb(false);
	deb.Update(true, idleAtMs);
	deb.Update(false, touchAtMs);
	deb.Update(false, touchAtMs + TOUCH_DEBOUNCE_MS);

	bool res;

	res = deb.Active();

	EXPECT_EQ(exp_active, res);
}

TEST(TouchDebounceTest, ActiveLowInputStaysInactiveOnHighLevel) {
	const uint32_t idleAtMs = 1000;
	const uint32_t laterCallAtMs = 2000;

	const bool exp_active = false;

	HAQuDA_TouchDebounce deb(false);
	deb.Update(true, idleAtMs);
	deb.Update(true, laterCallAtMs);

	bool res;

	res = deb.Active();

	EXPECT_EQ(exp_active, res);
}

TEST(TouchDebounceTest, TouchAcrossMillisWrapBecomesActive) {
	const uint32_t touchAtMs = 0xFFFFFFFFu - 10;
	const uint32_t idleAtMs = touchAtMs - 100;

	const bool exp_active = true;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, touchAtMs);
	deb.Update(true, touchAtMs + TOUCH_DEBOUNCE_MS);

	bool res;

	res = deb.Active();

	EXPECT_EQ(exp_active, res);
}

TEST(TouchDebounceTest, TouchAcrossMillisWrapStaysInactiveBeforeDebounceTime) {
	const uint32_t touchAtMs = 0xFFFFFFFFu - 10;
	const uint32_t idleAtMs = touchAtMs - 100;

	const bool exp_active = false;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, touchAtMs);
	deb.Update(true, touchAtMs + TOUCH_DEBOUNCE_MS - 1);

	bool res;

	res = deb.Active();

	EXPECT_EQ(exp_active, res);
}
