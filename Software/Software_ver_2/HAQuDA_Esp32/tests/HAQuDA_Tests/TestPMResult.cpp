#include <gtest/gtest.h>

#include "HAQuDA_PMResult.h"

TEST(PMResultTest, ShouldReportPMFailureFailedIsTrue) {
	const pm_meas_result input = pm_meas_result::failed;

	const bool exp_report = true;

	bool res;

	res = HAQuDA_ShouldReportPMFailure(input);

	EXPECT_EQ(exp_report, res);
}

TEST(PMResultTest, ShouldReportPMFailureOkIsFalse) {
	const pm_meas_result input = pm_meas_result::ok;

	const bool exp_report = false;

	bool res;

	res = HAQuDA_ShouldReportPMFailure(input);

	EXPECT_EQ(exp_report, res);
}

TEST(PMResultTest, ShouldReportPMFailureWarmingUpIsFalse) {
	const pm_meas_result input = pm_meas_result::warming_up;

	const bool exp_report = false;

	bool res;

	res = HAQuDA_ShouldReportPMFailure(input);

	EXPECT_EQ(exp_report, res);
}
