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
