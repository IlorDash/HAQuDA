#include <gtest/gtest.h>

#include "HAQuDA_BrightnessApply.h"

TEST(BrightnessApplyTest, DecidePendingOnlySetsBrightWithoutClear) {
	const bool pending = true;
	const bool restoreRequested = false;

	const bool exp_clear = false;
	const bool exp_setBright = true;

	brightness_apply res;

	res = HAQuDA_DecideBrightnessApply(pending, restoreRequested);

	EXPECT_EQ(exp_clear, res.clear);
	EXPECT_EQ(exp_setBright, res.setBright);
}

TEST(BrightnessApplyTest, DecideRestoreOnlyClearsAndSetsBright) {
	const bool pending = false;
	const bool restoreRequested = true;

	const bool exp_clear = true;
	const bool exp_setBright = true;

	brightness_apply res;

	res = HAQuDA_DecideBrightnessApply(pending, restoreRequested);

	EXPECT_EQ(exp_clear, res.clear);
	EXPECT_EQ(exp_setBright, res.setBright);
}

TEST(BrightnessApplyTest, DecideRestoreAndPendingClearsAndSetsBrightOnce) {
	// One setBright flag covers both requests: the caller makes a single SetBright() with the newest percent
	const bool pending = true;
	const bool restoreRequested = true;

	const bool exp_clear = true;
	const bool exp_setBright = true;

	brightness_apply res;

	res = HAQuDA_DecideBrightnessApply(pending, restoreRequested);

	EXPECT_EQ(exp_clear, res.clear);
	EXPECT_EQ(exp_setBright, res.setBright);
}

TEST(BrightnessApplyTest, DecideNoRequestDoesNothing) {
	const bool pending = false;
	const bool restoreRequested = false;

	const bool exp_clear = false;
	const bool exp_setBright = false;

	brightness_apply res;

	res = HAQuDA_DecideBrightnessApply(pending, restoreRequested);

	EXPECT_EQ(exp_clear, res.clear);
	EXPECT_EQ(exp_setBright, res.setBright);
}
