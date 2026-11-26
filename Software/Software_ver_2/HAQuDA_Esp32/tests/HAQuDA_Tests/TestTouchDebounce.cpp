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

TEST(TouchDebounceTest, ActiveFollowsABootLevelAndItsRelease) {
	const uint32_t bootMs = 1000;
	const uint32_t releaseMs = 2000;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(true, bootMs);
	deb.Update(true, bootMs + TOUCH_DEBOUNCE_MS);
	const bool res_held = deb.Active();
	deb.Update(false, releaseMs);
	deb.Update(false, releaseMs + TOUCH_DEBOUNCE_MS);
	const bool res_released = deb.Active();

	EXPECT_TRUE(res_held);
	EXPECT_FALSE(res_released);
}

TEST(TouchDebounceTest, PressFiresAtExactlyDebounceTime) {
	const uint32_t idleAtMs = 1000;
	const uint32_t touchAtMs = 1100;

	const bool exp_pressed = true;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, touchAtMs);

	bool res;

	res = deb.Update(true, touchAtMs + TOUCH_DEBOUNCE_MS);

	EXPECT_EQ(exp_pressed, res);
}

TEST(TouchDebounceTest, PressOneMsBeforeDebounceTimeDoesNotFire) {
	const uint32_t idleAtMs = 1000;
	const uint32_t touchAtMs = 1100;

	const bool exp_pressed = false;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, touchAtMs);

	bool res;

	res = deb.Update(true, touchAtMs + TOUCH_DEBOUNCE_MS - 1);

	EXPECT_EQ(exp_pressed, res);
}

TEST(TouchDebounceTest, CallAfterFiredPressReturnsFalse) {
	const uint32_t idleAtMs = 1000;
	const uint32_t touchAtMs = 1100;
	const uint32_t nextCallDelayMs = 10;

	const bool exp_pressed = false;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, touchAtMs);
	deb.Update(true, touchAtMs + TOUCH_DEBOUNCE_MS);

	bool res;

	res = deb.Update(true, touchAtMs + TOUCH_DEBOUNCE_MS + nextCallDelayMs);

	EXPECT_EQ(exp_pressed, res);
}

TEST(TouchDebounceTest, LongHoldDoesNotRepeatPress) {
	const uint32_t idleAtMs = 1000;
	const uint32_t touchAtMs = 1100;
	const uint32_t holdMs = 60 * 1000;

	const bool exp_pressed = false;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, touchAtMs);
	deb.Update(true, touchAtMs + TOUCH_DEBOUNCE_MS);

	bool res;

	res = deb.Update(true, touchAtMs + holdMs);

	EXPECT_EQ(exp_pressed, res);
}

TEST(TouchDebounceTest, SparseCallsStillFirePress) {
	const uint32_t idleAtMs = 1000;
	const uint32_t touchAtMs = 1100;
	const uint32_t lateCallAtMs = 5000;

	const bool exp_pressed = true;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, touchAtMs);

	bool res;

	res = deb.Update(true, lateCallAtMs);

	EXPECT_EQ(exp_pressed, res);
}

TEST(TouchDebounceTest, SpikeShorterThanDebounceDoesNotFire) {
	const uint32_t idleAtMs = 1000;
	const uint32_t spikeAtMs = 1100;
	const uint32_t spikeLengthMs = 20;

	const bool exp_pressed = false;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, spikeAtMs);
	deb.Update(false, spikeAtMs + spikeLengthMs);

	bool res;

	res = deb.Update(false, spikeAtMs + spikeLengthMs + TOUCH_DEBOUNCE_MS);

	EXPECT_EQ(exp_pressed, res);
}

TEST(TouchDebounceTest, BounceOnPressEdgeRestartsDebounceTime) {
	const uint32_t idleAtMs = 1000;
	const uint32_t firstTouchAtMs = 1100;
	const uint32_t bounceAtMs = 1110;
	const uint32_t secondTouchAtMs = 1120;

	const bool exp_pressed = false;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, firstTouchAtMs);
	deb.Update(false, bounceAtMs);
	deb.Update(true, secondTouchAtMs);

	bool res;

	res = deb.Update(true, secondTouchAtMs + TOUCH_DEBOUNCE_MS - 1);

	EXPECT_EQ(exp_pressed, res);
}

TEST(TouchDebounceTest, BounceOnPressEdgeFiresAfterFullDebounceFromLastEdge) {
	const uint32_t idleAtMs = 1000;
	const uint32_t firstTouchAtMs = 1100;
	const uint32_t bounceAtMs = 1110;
	const uint32_t secondTouchAtMs = 1120;

	const bool exp_pressed = true;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, firstTouchAtMs);
	deb.Update(false, bounceAtMs);
	deb.Update(true, secondTouchAtMs);

	bool res;

	res = deb.Update(true, secondTouchAtMs + TOUCH_DEBOUNCE_MS);

	EXPECT_EQ(exp_pressed, res);
}

TEST(TouchDebounceTest, SecondPressFiresAfterStableRelease) {
	const uint32_t idleAtMs = 1000;
	const uint32_t touchAtMs = 1100;
	const uint32_t releaseAtMs = 1200;
	const uint32_t secondTouchAtMs = 1300;

	const bool exp_pressed = true;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, touchAtMs);
	deb.Update(true, touchAtMs + TOUCH_DEBOUNCE_MS);
	deb.Update(false, releaseAtMs);
	deb.Update(false, releaseAtMs + TOUCH_DEBOUNCE_MS);
	deb.Update(true, secondTouchAtMs);

	bool res;

	res = deb.Update(true, secondTouchAtMs + TOUCH_DEBOUNCE_MS);

	EXPECT_EQ(exp_pressed, res);
}

TEST(TouchDebounceTest, ReleaseShorterThanDebounceDoesNotAllowSecondPress) {
	const uint32_t idleAtMs = 1000;
	const uint32_t touchAtMs = 1100;
	const uint32_t releaseAtMs = 1200;
	const uint32_t releaseLengthMs = 20;
	const uint32_t laterCallAtMs = 1500;

	const bool exp_pressed = false;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, touchAtMs);
	deb.Update(true, touchAtMs + TOUCH_DEBOUNCE_MS);
	deb.Update(false, releaseAtMs);
	deb.Update(true, releaseAtMs + releaseLengthMs);

	bool res;

	res = deb.Update(true, laterCallAtMs);

	EXPECT_EQ(exp_pressed, res);
}

TEST(TouchDebounceTest, ReleaseOneMsBeforeDebounceTimeDoesNotAllowSecondPress) {
	const uint32_t idleAtMs = 1000;
	const uint32_t touchAtMs = 1100;
	const uint32_t releaseAtMs = 1200;
	const uint32_t laterCallAtMs = 1500;

	const bool exp_pressed = false;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, touchAtMs);
	deb.Update(true, touchAtMs + TOUCH_DEBOUNCE_MS);
	deb.Update(false, releaseAtMs);
	deb.Update(false, releaseAtMs + TOUCH_DEBOUNCE_MS - 1);
	deb.Update(true, releaseAtMs + TOUCH_DEBOUNCE_MS);

	bool res;

	res = deb.Update(true, laterCallAtMs);

	EXPECT_EQ(exp_pressed, res);
}

TEST(TouchDebounceTest, SpikeDuringStableReleaseThenRealPressFires) {
	const uint32_t idleAtMs = 1000;
	const uint32_t touchAtMs = 1100;
	const uint32_t releaseAtMs = 1200;
	const uint32_t spikeAtMs = 1300;
	const uint32_t spikeLengthMs = 10;
	const uint32_t secondTouchAtMs = 1400;

	const bool exp_pressed = true;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, touchAtMs);
	deb.Update(true, touchAtMs + TOUCH_DEBOUNCE_MS);
	deb.Update(false, releaseAtMs);
	deb.Update(false, releaseAtMs + TOUCH_DEBOUNCE_MS);
	deb.Update(true, spikeAtMs);
	deb.Update(false, spikeAtMs + spikeLengthMs);
	deb.Update(true, secondTouchAtMs);

	bool res;

	res = deb.Update(true, secondTouchAtMs + TOUCH_DEBOUNCE_MS);

	EXPECT_EQ(exp_pressed, res);
}

TEST(TouchDebounceTest, SpikeDuringStableReleaseDoesNotFire) {
	const uint32_t idleAtMs = 1000;
	const uint32_t touchAtMs = 1100;
	const uint32_t releaseAtMs = 1200;
	const uint32_t spikeAtMs = 1300;
	const uint32_t spikeLengthMs = 10;

	const bool exp_pressed = false;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, touchAtMs);
	deb.Update(true, touchAtMs + TOUCH_DEBOUNCE_MS);
	deb.Update(false, releaseAtMs);
	deb.Update(false, releaseAtMs + TOUCH_DEBOUNCE_MS);
	deb.Update(true, spikeAtMs);
	deb.Update(false, spikeAtMs + spikeLengthMs);

	bool res;

	res = deb.Update(false, spikeAtMs + spikeLengthMs + TOUCH_DEBOUNCE_MS);

	EXPECT_EQ(exp_pressed, res);
}

TEST(TouchDebounceTest, ActiveLowPressFires) {
	const uint32_t idleAtMs = 1000;
	const uint32_t touchAtMs = 1100;

	const bool exp_pressed = true;

	HAQuDA_TouchDebounce deb(false);
	deb.Update(true, idleAtMs);
	deb.Update(false, touchAtMs);

	bool res;

	res = deb.Update(false, touchAtMs + TOUCH_DEBOUNCE_MS);

	EXPECT_EQ(exp_pressed, res);
}

TEST(TouchDebounceTest, ActiveLowIdleHighIsNotAPress) {
	const uint32_t idleAtMs = 1000;
	const uint32_t laterCallAtMs = 2000;

	const bool exp_pressed = false;

	HAQuDA_TouchDebounce deb(false);
	deb.Update(true, idleAtMs);

	bool res;

	res = deb.Update(true, laterCallAtMs);

	EXPECT_EQ(exp_pressed, res);
}

TEST(TouchDebounceTest, PressAcrossMillisWrapFires) {
	const uint32_t touchAtMs = 0xFFFFFFFFu - 10;
	const uint32_t idleAtMs = touchAtMs - 100;

	const bool exp_pressed = true;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, touchAtMs);

	bool res;

	res = deb.Update(true, touchAtMs + TOUCH_DEBOUNCE_MS);

	EXPECT_EQ(exp_pressed, res);
}

TEST(TouchDebounceTest, SpikeAcrossMillisWrapDoesNotFire) {
	const uint32_t touchAtMs = 0xFFFFFFFFu - 10;
	const uint32_t idleAtMs = touchAtMs - 100;

	const bool exp_pressed = false;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(false, idleAtMs);
	deb.Update(true, touchAtMs);

	bool res;

	res = deb.Update(true, touchAtMs + TOUCH_DEBOUNCE_MS - 1);

	EXPECT_EQ(exp_pressed, res);
}

TEST(TouchDebounceTest, ActiveAtPowerUpDoesNotFire) {
	const uint32_t powerUpAtMs = 0;
	const uint32_t laterCallAtMs = 1000;

	const bool exp_pressed = false;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(true, powerUpAtMs);

	bool res;

	res = deb.Update(true, laterCallAtMs);

	EXPECT_EQ(exp_pressed, res);
}

TEST(TouchDebounceTest, ActiveAtPowerUpThenStableReleaseAndPressFires) {
	const uint32_t powerUpAtMs = 0;
	const uint32_t stableAtMs = 1000;
	const uint32_t releaseAtMs = 1100;
	const uint32_t touchAtMs = 1200;

	const bool exp_pressed = true;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(true, powerUpAtMs);
	deb.Update(true, stableAtMs);
	deb.Update(false, releaseAtMs);
	deb.Update(false, releaseAtMs + TOUCH_DEBOUNCE_MS);
	deb.Update(true, touchAtMs);

	bool res;

	res = deb.Update(true, touchAtMs + TOUCH_DEBOUNCE_MS);

	EXPECT_EQ(exp_pressed, res);
}

TEST(TouchDebounceTest, ActiveAtPowerUpThenUnstableReleaseDoesNotFire) {
	const uint32_t powerUpAtMs = 0;
	const uint32_t stableAtMs = 1000;
	const uint32_t releaseAtMs = 1100;
	const uint32_t releaseLengthMs = 10;
	const uint32_t laterCallAtMs = 1500;

	const bool exp_pressed = false;

	HAQuDA_TouchDebounce deb(true);
	deb.Update(true, powerUpAtMs);
	deb.Update(true, stableAtMs);
	deb.Update(false, releaseAtMs);
	deb.Update(true, releaseAtMs + releaseLengthMs);

	bool res;

	res = deb.Update(true, laterCallAtMs);

	EXPECT_EQ(exp_pressed, res);
}
