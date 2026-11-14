#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>
#include <limits>

#include "HAQuDA_MeasSnapshot.h"

TEST(MeasSnapshotTest, GetBeforePublishReturnsFalse) {
	const bool exp_ok = false;

	const HAQuDA_MeasSnapshot snapshot;
	float value = 1.0f;
	uint32_t rgb = 1;

	bool res;

	res = snapshot.Get(snapshot_param::temp, value, rgb);

	EXPECT_EQ(exp_ok, res);
}

TEST(MeasSnapshotTest, GetBeforePublishValueIsNaN) {
	const HAQuDA_MeasSnapshot snapshot;
	float value = 1.0f;
	uint32_t rgb = 1;

	bool res;

	res = snapshot.Get(snapshot_param::humid, value, rgb);

	EXPECT_FALSE(res);
	EXPECT_TRUE(std::isnan(value));
}

TEST(MeasSnapshotTest, GetBeforePublishColorIsZero) {
	const uint32_t exp_rgb = 0;

	const HAQuDA_MeasSnapshot snapshot;
	float value = 1.0f;
	uint32_t rgb = 0xFFFFFF;

	bool res;

	res = snapshot.Get(snapshot_param::eco2, value, rgb);

	EXPECT_FALSE(res);
	EXPECT_EQ(exp_rgb, rgb);
}

TEST(MeasSnapshotTest, GetAfterPublishReturnsTrue) {
	const bool exp_ok = true;

	const float publishedValue = 23.5f;
	const uint32_t publishedRgb = 0x00FF00;
	HAQuDA_MeasSnapshot snapshot;
	float value = 0.0f;
	uint32_t rgb = 0;

	bool res;

	snapshot.Publish(snapshot_param::temp, publishedValue, publishedRgb);
	res = snapshot.Get(snapshot_param::temp, value, rgb);

	EXPECT_EQ(exp_ok, res);
}

TEST(MeasSnapshotTest, GetAfterPublishReturnsValueAndColor) {
	const float exp_value = 23.5f;
	const uint32_t exp_rgb = 0x12AB34;

	HAQuDA_MeasSnapshot snapshot;
	float value = 0.0f;
	uint32_t rgb = 0;

	bool res;

	snapshot.Publish(snapshot_param::temp, exp_value, exp_rgb);
	res = snapshot.Get(snapshot_param::temp, value, rgb);

	EXPECT_TRUE(res);
	EXPECT_FLOAT_EQ(exp_value, value);
	EXPECT_EQ(exp_rgb, rgb);
}

TEST(MeasSnapshotTest, GetLastSlotReturnsPublishedPm25) {
	const float exp_value = 12.0f;
	const uint32_t exp_rgb = 0xFF0000;

	HAQuDA_MeasSnapshot snapshot;
	float value = 0.0f;
	uint32_t rgb = 0;

	bool res;

	snapshot.Publish(snapshot_param::pm25, exp_value, exp_rgb);
	res = snapshot.Get(snapshot_param::pm25, value, rgb);

	EXPECT_TRUE(res);
	EXPECT_FLOAT_EQ(exp_value, value);
	EXPECT_EQ(exp_rgb, rgb);
}

TEST(MeasSnapshotTest, GetAfterPublishNegativeValueKeepsSign) {
	const float exp_value = -5.5f;
	const uint32_t exp_rgb = 0x0000FF;

	HAQuDA_MeasSnapshot snapshot;
	float value = 0.0f;
	uint32_t rgb = 0;

	bool res;

	snapshot.Publish(snapshot_param::temp, exp_value, exp_rgb);
	res = snapshot.Get(snapshot_param::temp, value, rgb);

	EXPECT_TRUE(res);
	EXPECT_FLOAT_EQ(exp_value, value);
}

TEST(MeasSnapshotTest, GetAfterPublishZeroValueReturnsTrue) {
	const bool exp_ok = true;

	const float zeroValue = 0.0f;
	const uint32_t publishedRgb = 0x0000FF;
	HAQuDA_MeasSnapshot snapshot;
	float value = 1.0f;
	uint32_t rgb = 0;

	bool res;

	snapshot.Publish(snapshot_param::tvoc, zeroValue, publishedRgb);
	res = snapshot.Get(snapshot_param::tvoc, value, rgb);

	EXPECT_EQ(exp_ok, res);
}

TEST(MeasSnapshotTest, GetAfterPublishNaNReturnsFalse) {
	const bool exp_ok = false;

	const float failedPoll = std::numeric_limits<float>::quiet_NaN();
	const uint32_t publishedRgb = 0x00FF00;
	HAQuDA_MeasSnapshot snapshot;
	float value = 0.0f;
	uint32_t rgb = 0;

	bool res;

	snapshot.Publish(snapshot_param::eco2, failedPoll, publishedRgb);
	res = snapshot.Get(snapshot_param::eco2, value, rgb);

	EXPECT_EQ(exp_ok, res);
	EXPECT_TRUE(std::isnan(value));
}

TEST(MeasSnapshotTest, PublishNaNAfterValueMarksParameterFailed) {
	const bool exp_ok = false;

	const float goodValue = 400.0f;
	const float failedPoll = std::numeric_limits<float>::quiet_NaN();
	const uint32_t publishedRgb = 0x00FF00;
	HAQuDA_MeasSnapshot snapshot;
	float value = 0.0f;
	uint32_t rgb = 0;

	bool res;

	snapshot.Publish(snapshot_param::eco2, goodValue, publishedRgb);
	snapshot.Publish(snapshot_param::eco2, failedPoll, publishedRgb);
	res = snapshot.Get(snapshot_param::eco2, value, rgb);

	EXPECT_EQ(exp_ok, res);
}

TEST(MeasSnapshotTest, PublishTwiceKeepsLatestValueAndColor) {
	const float exp_value = 60.0f;
	const uint32_t exp_rgb = 0xFFFF00;

	const float firstValue = 40.0f;
	const uint32_t firstRgb = 0x00FF00;
	HAQuDA_MeasSnapshot snapshot;
	float value = 0.0f;
	uint32_t rgb = 0;

	bool res;

	snapshot.Publish(snapshot_param::humid, firstValue, firstRgb);
	snapshot.Publish(snapshot_param::humid, exp_value, exp_rgb);
	res = snapshot.Get(snapshot_param::humid, value, rgb);

	EXPECT_TRUE(res);
	EXPECT_FLOAT_EQ(exp_value, value);
	EXPECT_EQ(exp_rgb, rgb);
}

TEST(MeasSnapshotTest, PublishDoesNotChangeOtherSlot) {
	const bool exp_ok = false;

	const float publishedValue = 23.5f;
	const uint32_t publishedRgb = 0x00FF00;
	HAQuDA_MeasSnapshot snapshot;
	float value = 0.0f;
	uint32_t rgb = 0;

	bool res;

	snapshot.Publish(snapshot_param::temp, publishedValue, publishedRgb);
	res = snapshot.Get(snapshot_param::humid, value, rgb);

	EXPECT_EQ(exp_ok, res);
}

TEST(MeasSnapshotTest, PublishOutOfRangeParamLeavesSlotsUntouched) {
	const bool exp_ok = false;

	const float publishedValue = 99.0f;
	const uint32_t publishedRgb = 0xFF0000;
	HAQuDA_MeasSnapshot snapshot;
	float value = 0.0f;
	uint32_t rgb = 0;

	bool res;

	snapshot.Publish(snapshot_param::count, publishedValue, publishedRgb);
	res = snapshot.Get(snapshot_param::pm25, value, rgb);

	EXPECT_EQ(exp_ok, res);
}

TEST(MeasSnapshotTest, GetOutOfRangeParamReturnsFalse) {
	const bool exp_ok = false;

	const HAQuDA_MeasSnapshot snapshot;
	float value = 1.0f;
	uint32_t rgb = 1;

	bool res;

	res = snapshot.Get(snapshot_param::count, value, rgb);

	EXPECT_EQ(exp_ok, res);
}

TEST(MeasSnapshotTest, GetOutOfRangeParamResetsOutputs) {
	const uint32_t exp_rgb = 0;

	const HAQuDA_MeasSnapshot snapshot;
	float value = 1.0f;
	uint32_t rgb = 0xFFFFFF;

	bool res;

	res = snapshot.Get(snapshot_param::count, value, rgb);

	EXPECT_FALSE(res);
	EXPECT_TRUE(std::isnan(value));
	EXPECT_EQ(exp_rgb, rgb);
}

TEST(MeasSnapshotTest, GetAfterPublishNaNReturnsPublishedColor) {
	const uint32_t exp_rgb = 0x808080;

	const float failedPoll = std::numeric_limits<float>::quiet_NaN();
	HAQuDA_MeasSnapshot snapshot;
	float value = 0.0f;
	uint32_t rgb = 0;

	bool res;

	snapshot.Publish(snapshot_param::tvoc, failedPoll, exp_rgb);
	res = snapshot.Get(snapshot_param::tvoc, value, rgb);

	EXPECT_FALSE(res);
	EXPECT_EQ(exp_rgb, rgb);
}
