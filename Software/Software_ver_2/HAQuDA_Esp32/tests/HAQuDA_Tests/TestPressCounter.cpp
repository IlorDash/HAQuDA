#include <gtest/gtest.h>

#include <cstdint>

#include "HAQuDA_PressCounter.h"

TEST(PressCounterTest, TakeOnNewCounterIsZero) {
	const uint8_t exp_count = 0;

	HAQuDA_PressCounter counter;

	uint8_t res;

	res = counter.Take();

	EXPECT_EQ(exp_count, res);
}

TEST(PressCounterTest, TakeAfterOneAddIsOne) {
	const uint8_t exp_count = 1;

	HAQuDA_PressCounter counter;

	uint8_t res;

	counter.Add();
	res = counter.Take();

	EXPECT_EQ(exp_count, res);
}

TEST(PressCounterTest, TakeAfterSeveralAddsReturnsTheirNumber) {
	const uint8_t adds = 5;

	const uint8_t exp_count = adds;

	HAQuDA_PressCounter counter;

	uint8_t res;

	for (uint8_t i = 0; i < adds; i++) {
		counter.Add();
	}
	res = counter.Take();

	EXPECT_EQ(exp_count, res);
}

TEST(PressCounterTest, SecondTakeIsZero) {
	const uint8_t exp_count = 0;

	HAQuDA_PressCounter counter;

	uint8_t res;

	counter.Add();
	counter.Take();
	res = counter.Take();

	EXPECT_EQ(exp_count, res);
}

TEST(PressCounterTest, TakeAtMaxAddsReturnsMax) {
	const uint8_t exp_count = UINT8_MAX;

	HAQuDA_PressCounter counter;

	uint8_t res;

	for (uint8_t i = 0; i < UINT8_MAX; i++) {
		counter.Add();
	}
	res = counter.Take();

	EXPECT_EQ(exp_count, res);
}

TEST(PressCounterTest, AddPastMaxStaysAtMax) {
	const uint16_t adds = UINT8_MAX + 1;

	const uint8_t exp_count = UINT8_MAX;

	HAQuDA_PressCounter counter;

	uint8_t res;

	for (uint16_t i = 0; i < adds; i++) {
		counter.Add();
	}
	res = counter.Take();

	EXPECT_EQ(exp_count, res);
}

TEST(PressCounterTest, ManyAddsPastMaxDoNotWrapToZero) {
	const uint16_t adds = 300;

	const uint8_t exp_count = UINT8_MAX;

	HAQuDA_PressCounter counter;

	uint8_t res;

	for (uint16_t i = 0; i < adds; i++) {
		counter.Add();
	}
	res = counter.Take();

	EXPECT_EQ(exp_count, res);
}

TEST(PressCounterTest, AddAfterSaturatedTakeCountsFromZero) {
	const uint16_t saturating_adds = 300;

	const uint8_t exp_count = 1;

	HAQuDA_PressCounter counter;

	uint8_t res;

	for (uint16_t i = 0; i < saturating_adds; i++) {
		counter.Add();
	}
	counter.Take();
	counter.Add();
	res = counter.Take();

	EXPECT_EQ(exp_count, res);
}

TEST(PressCounterTest, AddAfterTakeCountsFromZero) {
	const uint8_t exp_count = 1;

	HAQuDA_PressCounter counter;

	uint8_t res;

	counter.Add();
	counter.Add();
	counter.Take();
	counter.Add();
	res = counter.Take();

	EXPECT_EQ(exp_count, res);
}

TEST(PressCounterTest, InstancesCountIndependently) {
	const uint8_t exp_first = 2;
	const uint8_t exp_second = 1;

	HAQuDA_PressCounter first;
	HAQuDA_PressCounter second;

	uint8_t res_first;
	uint8_t res_second;

	first.Add();
	first.Add();
	second.Add();
	res_first = first.Take();
	res_second = second.Take();

	EXPECT_EQ(exp_first, res_first);
	EXPECT_EQ(exp_second, res_second);
}
