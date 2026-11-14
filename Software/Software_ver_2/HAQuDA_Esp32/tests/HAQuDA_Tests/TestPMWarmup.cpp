#include <gtest/gtest.h>

#include <cstdint>

#include "HAQuDA_PMWarmup.h"

TEST(PMWarmupTest, IsWarmingUpBeforePowerOnIsFalse) {
	const uint32_t nowMs = 1000;

	const bool exp_warming = false;

	HAQuDA_PMWarmup warmup;

	bool res;

	res = warmup.IsWarmingUp(nowMs);

	EXPECT_EQ(exp_warming, res);
}

TEST(PMWarmupTest, IsWarmingUpJustAfterPowerOnIsTrue) {
	const uint32_t powerOnMs = 1000;

	const bool exp_warming = true;

	HAQuDA_PMWarmup warmup;

	bool res;

	warmup.PowerOn(powerOnMs);

	res = warmup.IsWarmingUp(powerOnMs);

	EXPECT_EQ(exp_warming, res);
}

TEST(PMWarmupTest, IsWarmingUpInsideWindowIsTrue) {
	const uint32_t powerOnMs = 1000;
	const uint32_t elapsedMs = 5000;

	const bool exp_warming = true;

	HAQuDA_PMWarmup warmup;

	bool res;

	warmup.PowerOn(powerOnMs);

	res = warmup.IsWarmingUp(powerOnMs + elapsedMs);

	EXPECT_EQ(exp_warming, res);
}

TEST(PMWarmupTest, IsWarmingUpOneMsBeforeWindowEndIsTrue) {
	const uint32_t powerOnMs = 1000;

	const bool exp_warming = true;

	HAQuDA_PMWarmup warmup;

	bool res;

	warmup.PowerOn(powerOnMs);

	res = warmup.IsWarmingUp(powerOnMs + PM_WARMUP_MS - 1);

	EXPECT_EQ(exp_warming, res);
}

TEST(PMWarmupTest, IsWarmingUpAtWindowEndIsFalse) {
	const uint32_t powerOnMs = 1000;

	const bool exp_warming = false;

	HAQuDA_PMWarmup warmup;

	bool res;

	warmup.PowerOn(powerOnMs);

	res = warmup.IsWarmingUp(powerOnMs + PM_WARMUP_MS);

	EXPECT_EQ(exp_warming, res);
}

TEST(PMWarmupTest, IsWarmingUpLongAfterWindowIsFalse) {
	const uint32_t powerOnMs = 1000;
	const uint32_t elapsedMs = 600000;

	const bool exp_warming = false;

	HAQuDA_PMWarmup warmup;

	bool res;

	warmup.PowerOn(powerOnMs);

	res = warmup.IsWarmingUp(powerOnMs + elapsedMs);

	EXPECT_EQ(exp_warming, res);
}

TEST(PMWarmupTest, IsWarmingUpStaysFalseWhenMillisWrapsBackIntoWindow) {
	// 2^32 ms (49.7 days) after the power-on the unsigned difference is small again;
	// powerOn + 2^32 + 100 is the same uint32_t value as powerOn + 100.
	const uint32_t powerOnMs = 1000;
	const uint32_t smallDiffMs = 100;

	const bool exp_warming = false;

	HAQuDA_PMWarmup warmup;

	bool res;

	warmup.PowerOn(powerOnMs);
	warmup.IsWarmingUp(powerOnMs + PM_WARMUP_MS); // the window has passed and latches off

	res = warmup.IsWarmingUp(powerOnMs + smallDiffMs);

	EXPECT_EQ(exp_warming, res);
}

TEST(PMWarmupTest, IsWarmingUpPowerOnNearMaxStillWarmingAfterWrap) {
	// powerOn = UINT32_MAX - 1000; 5 s later millis() has wrapped to 3999 (= powerOn + 5000 modulo 2^32)
	const uint32_t powerOnMs = UINT32_MAX - 1000;
	const uint32_t wrappedNowMs = 3999;

	const bool exp_warming = true;

	HAQuDA_PMWarmup warmup;

	bool res;

	warmup.PowerOn(powerOnMs);

	res = warmup.IsWarmingUp(wrappedNowMs);

	EXPECT_EQ(exp_warming, res);
}

TEST(PMWarmupTest, IsWarmingUpPowerOnNearMaxLastMsInsideAfterWrap) {
	// powerOn + PM_WARMUP_MS - 1 modulo 2^32 = 10998
	const uint32_t powerOnMs = UINT32_MAX - 1000;
	const uint32_t wrappedNowMs = 10998;

	const bool exp_warming = true;

	HAQuDA_PMWarmup warmup;

	bool res;

	warmup.PowerOn(powerOnMs);

	res = warmup.IsWarmingUp(wrappedNowMs);

	EXPECT_EQ(exp_warming, res);
}

TEST(PMWarmupTest, IsWarmingUpPowerOnNearMaxWindowEndIsFalseAfterWrap) {
	// powerOn + PM_WARMUP_MS modulo 2^32 = 10999
	const uint32_t powerOnMs = UINT32_MAX - 1000;
	const uint32_t wrappedNowMs = 10999;

	const bool exp_warming = false;

	HAQuDA_PMWarmup warmup;

	bool res;

	warmup.PowerOn(powerOnMs);

	res = warmup.IsWarmingUp(wrappedNowMs);

	EXPECT_EQ(exp_warming, res);
}

TEST(PMWarmupTest, PowerOnAfterLatchOpensNewWindow) {
	const uint32_t firstPowerOnMs = 1000;
	const uint32_t secondPowerOnMs = 50000;
	const uint32_t elapsedMs = 100;

	const bool exp_warming = true;

	HAQuDA_PMWarmup warmup;

	bool res;

	warmup.PowerOn(firstPowerOnMs);
	warmup.IsWarmingUp(firstPowerOnMs + PM_WARMUP_MS); // latched off
	warmup.PowerOn(secondPowerOnMs);

	res = warmup.IsWarmingUp(secondPowerOnMs + elapsedMs);

	EXPECT_EQ(exp_warming, res);
}

TEST(PMWarmupTest, PowerOnWhileWarmingRestartsWindowFromNewTime) {
	// Measured from the first power-on the window would be over at firstPowerOn + PM_WARMUP_MS
	const uint32_t firstPowerOnMs = 1000;
	const uint32_t secondPowerOnMs = 8000;

	const bool exp_warming = true;

	HAQuDA_PMWarmup warmup;

	bool res;

	warmup.PowerOn(firstPowerOnMs);
	warmup.PowerOn(secondPowerOnMs);

	res = warmup.IsWarmingUp(secondPowerOnMs + PM_WARMUP_MS - 1);

	EXPECT_EQ(exp_warming, res);
}
