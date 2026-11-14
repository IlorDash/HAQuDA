#include <gtest/gtest.h>

#include <cstdint>

#include "HAQuDA_FlashLock.h"

// The mutex handle in HAQuDA_FlashLock.cpp is a file-level static that is created once and never freed, so every
// test calls Create() itself (it is a no-op when the mutex already exists). The "never created" path cannot be
// reached in a run where another test has already created it, so it is not tested here.
// The fixture only resets the fake mutex (not held, call counters zero).
class FlashLockTest : public ::testing::Test {
  protected:
	void SetUp() override {
		FakeFreeRTOS::Reset();
	}
};

TEST_F(FlashLockTest, CreateMakesMutexExist) {
	const bool exp_exists = true;

	bool res;

	HAQuDA_FlashLock::Create();
	res = HAQuDA_FlashLock::Exists();

	EXPECT_EQ(exp_exists, res);
}

TEST_F(FlashLockTest, TakeFreeMutexReturnsTrue) {
	const uint32_t timeoutMs = 100;

	const bool exp_taken = true;

	HAQuDA_FlashLock::Create();

	bool res;

	res = HAQuDA_FlashLock::Take(timeoutMs);

	EXPECT_EQ(exp_taken, res);
}

TEST_F(FlashLockTest, TakeHeldMutexTimesOutReturnsFalse) {
	const uint32_t timeoutMs = 100;

	const bool exp_taken = false;

	HAQuDA_FlashLock::Create();
	HAQuDA_FlashLock::Take(timeoutMs); // another task holds the mutex

	bool res;

	res = HAQuDA_FlashLock::Take(timeoutMs);

	EXPECT_EQ(exp_taken, res);
}

TEST_F(FlashLockTest, TakePassesTimeoutInTicks) {
	const uint32_t timeoutMs = 500;

	const TickType_t exp_ticks = pdMS_TO_TICKS(timeoutMs);

	HAQuDA_FlashLock::Create();

	TickType_t res;

	HAQuDA_FlashLock::Take(timeoutMs);
	res = FakeFreeRTOS::mutex.lastTakeTicks;

	EXPECT_EQ(exp_ticks, res);
}

TEST_F(FlashLockTest, GiveReleasesMutexForNextTake) {
	const uint32_t timeoutMs = 100;

	const bool exp_taken = true;

	HAQuDA_FlashLock::Create();
	HAQuDA_FlashLock::Take(timeoutMs);

	bool res;

	HAQuDA_FlashLock::Give();
	res = HAQuDA_FlashLock::Take(timeoutMs);

	EXPECT_EQ(exp_taken, res);
}

TEST_F(FlashLockTest, GuardHoldsMutexWhileAlive) {
	const uint32_t timeoutMs = 100;

	const bool exp_held = true;

	HAQuDA_FlashLock::Create();

	bool res;

	{
		HAQuDA_FlashLockGuard guard(timeoutMs);
		res = FakeFreeRTOS::mutex.held;
	}

	EXPECT_EQ(exp_held, res);
}

TEST_F(FlashLockTest, GuardAcquiredIsTrueOnFreeMutex) {
	const uint32_t timeoutMs = 100;

	const bool exp_acquired = true;

	HAQuDA_FlashLock::Create();

	bool res;

	{
		HAQuDA_FlashLockGuard guard(timeoutMs);
		res = guard.Acquired();
	}

	EXPECT_EQ(exp_acquired, res);
}

TEST_F(FlashLockTest, GuardReleasesMutexOnScopeExit) {
	const uint32_t timeoutMs = 100;

	const bool exp_held = false;

	HAQuDA_FlashLock::Create();

	bool res;

	{
		HAQuDA_FlashLockGuard guard(timeoutMs);
	}
	res = FakeFreeRTOS::mutex.held;

	EXPECT_EQ(exp_held, res);
}

TEST_F(FlashLockTest, GuardGivesMutexExactlyOnce) {
	const uint32_t timeoutMs = 100;

	const int exp_gives = 1;

	HAQuDA_FlashLock::Create();

	int res;

	{
		HAQuDA_FlashLockGuard guard(timeoutMs);
	}
	res = FakeFreeRTOS::mutex.giveCalls;

	EXPECT_EQ(exp_gives, res);
}

TEST_F(FlashLockTest, GuardAcquiredIsFalseWhenMutexHeldElsewhere) {
	const uint32_t timeoutMs = 100;

	const bool exp_acquired = false;

	HAQuDA_FlashLock::Create();
	HAQuDA_FlashLock::Take(timeoutMs); // another task holds the mutex

	bool res;

	{
		HAQuDA_FlashLockGuard guard(timeoutMs);
		res = guard.Acquired();
	}

	EXPECT_EQ(exp_acquired, res);
}

TEST_F(FlashLockTest, GuardThatTimedOutDoesNotGiveOthersMutex) {
	const uint32_t timeoutMs = 100;

	const int exp_gives = 0;

	HAQuDA_FlashLock::Create();
	HAQuDA_FlashLock::Take(timeoutMs); // another task holds the mutex

	int res;

	{
		HAQuDA_FlashLockGuard guard(timeoutMs);
	}
	res = FakeFreeRTOS::mutex.giveCalls;

	EXPECT_EQ(exp_gives, res);
}
