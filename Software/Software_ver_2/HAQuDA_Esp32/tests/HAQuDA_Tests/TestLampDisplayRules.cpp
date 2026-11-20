#include <gtest/gtest.h>

#include "HAQuDA_LampDisplayRules.h"

TEST(LampDisplayRulesTest, ToMeasTypeTotalIsTotal) {
	const meas_type exp_type = total;

	meas_type res;

	res = HAQuDA_ToMeasType(lamp_param::total);

	EXPECT_EQ(exp_type, res);
}

TEST(LampDisplayRulesTest, ToMeasTypeTempIsTemp) {
	const meas_type exp_type = temp;

	meas_type res;

	res = HAQuDA_ToMeasType(lamp_param::temp);

	EXPECT_EQ(exp_type, res);
}

TEST(LampDisplayRulesTest, ToMeasTypeHumidIsHumid) {
	const meas_type exp_type = humid;

	meas_type res;

	res = HAQuDA_ToMeasType(lamp_param::humid);

	EXPECT_EQ(exp_type, res);
}

TEST(LampDisplayRulesTest, ToMeasTypeEco2IsECO2) {
	const meas_type exp_type = eCO2;

	meas_type res;

	res = HAQuDA_ToMeasType(lamp_param::eco2);

	EXPECT_EQ(exp_type, res);
}

TEST(LampDisplayRulesTest, ToMeasTypeTvocIsTVOC) {
	const meas_type exp_type = TVOC;

	meas_type res;

	res = HAQuDA_ToMeasType(lamp_param::tvoc);

	EXPECT_EQ(exp_type, res);
}

TEST(LampDisplayRulesTest, ToMeasTypePm25IsPM25) {
	const meas_type exp_type = PM2_5;

	meas_type res;

	res = HAQuDA_ToMeasType(lamp_param::pm25);

	EXPECT_EQ(exp_type, res);
}

TEST(LampDisplayRulesTest, ToMeasTypeOutOfRangeValueIsTotal) {
	const meas_type exp_type = total;
	const lamp_param out_of_range = static_cast<lamp_param>(200);

	meas_type res;

	res = HAQuDA_ToMeasType(out_of_range);

	EXPECT_EQ(exp_type, res);
}

TEST(LampDisplayRulesTest, ErrorShownErrorModeWithAnErrorIsTrue) {
	const bool exp_shown = true;

	bool res;

	res = HAQuDA_ErrorShown(error, failedToConnectToWiFi);

	EXPECT_EQ(exp_shown, res);
}

TEST(LampDisplayRulesTest, ErrorShownErrorModeWithLastErrorTypeIsTrue) {
	const bool exp_shown = true;

	bool res;

	res = HAQuDA_ErrorShown(error, failedToUpdateNTP);

	EXPECT_EQ(exp_shown, res);
}

TEST(LampDisplayRulesTest, ErrorShownErrorModeWithoutErrorIsFalse) {
	const bool exp_shown = false;

	bool res;

	res = HAQuDA_ErrorShown(error, noneError);

	EXPECT_EQ(exp_shown, res);
}

TEST(LampDisplayRulesTest, ErrorShownMeasModeWithAnErrorIsFalse) {
	const bool exp_shown = false;

	bool res;

	res = HAQuDA_ErrorShown(meas, failedToStartSensors);

	EXPECT_EQ(exp_shown, res);
}

TEST(LampDisplayRulesTest, ErrorShownEffectModeWithAnErrorIsFalse) {
	const bool exp_shown = false;

	bool res;

	res = HAQuDA_ErrorShown(effect, failedToStartSensors);

	EXPECT_EQ(exp_shown, res);
}

TEST(LampDisplayRulesTest, ErrorShownEventModeWithAnErrorIsFalse) {
	const bool exp_shown = false;

	bool res;

	res = HAQuDA_ErrorShown(event, failedToStartSensors);

	EXPECT_EQ(exp_shown, res);
}

TEST(LampDisplayRulesTest, ErrorShownNoModeWithAnErrorIsFalse) {
	const bool exp_shown = false;

	bool res;

	res = HAQuDA_ErrorShown(noneMode, failedToStartSensors);

	EXPECT_EQ(exp_shown, res);
}

TEST(LampDisplayRulesTest, ErrorShownMeasModeWithoutErrorIsFalse) {
	const bool exp_shown = false;

	bool res;

	res = HAQuDA_ErrorShown(meas, noneError);

	EXPECT_EQ(exp_shown, res);
}
