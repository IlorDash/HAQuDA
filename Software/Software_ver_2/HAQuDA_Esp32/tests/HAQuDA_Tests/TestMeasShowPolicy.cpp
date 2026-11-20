#include <gtest/gtest.h>

#include "HAQuDA_MeasShowPolicy.h"

TEST(MeasShowPolicyTest, DrawnRestartsAndDropsTheRequest) {
	const bool exp_restart = true;
	const bool exp_keep = false;

	meas_show_outcome res;

	res = HAQuDA_DecideShowOutcome(true, false);

	EXPECT_EQ(exp_restart, res.restartPeriod);
	EXPECT_EQ(exp_keep, res.keepRequest);
}

TEST(MeasShowPolicyTest, DrawnAfterThePeriodRestartsAndDropsTheRequest) {
	const bool exp_restart = true;
	const bool exp_keep = false;

	meas_show_outcome res;

	res = HAQuDA_DecideShowOutcome(true, true);

	EXPECT_EQ(exp_restart, res.restartPeriod);
	EXPECT_EQ(exp_keep, res.keepRequest);
}

TEST(MeasShowPolicyTest, NotDrawnInsideThePeriodKeepsEverything) {
	const bool exp_restart = false;
	const bool exp_keep = true;

	meas_show_outcome res;

	res = HAQuDA_DecideShowOutcome(false, false);

	EXPECT_EQ(exp_restart, res.restartPeriod);
	EXPECT_EQ(exp_keep, res.keepRequest);
}

TEST(MeasShowPolicyTest, NotDrawnAfterThePeriodClearsTheSumsButKeepsTheRequest) {
	const bool exp_restart = true;
	const bool exp_keep = true;

	meas_show_outcome res;

	res = HAQuDA_DecideShowOutcome(false, true);

	EXPECT_EQ(exp_restart, res.restartPeriod);
	EXPECT_EQ(exp_keep, res.keepRequest);
}
