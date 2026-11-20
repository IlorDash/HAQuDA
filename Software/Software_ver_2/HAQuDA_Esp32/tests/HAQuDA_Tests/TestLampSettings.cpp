#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <string>

#include "HAQuDA_LEDsParams.h"
#include "HAQuDA_LampSettings.h"
#include "HAQuDA_NightProgress.h"

TEST(LampSettingsTest, DefaultsAreValid) {
	const bool exp_valid = true;

	lamp_settings settings = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_LampSettingsValid(settings);

	EXPECT_EQ(exp_valid, res);
}

TEST(LampSettingsTest, DefaultsMatchTheDocumentedLamp) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint16_t exp_from = 21 * 60;
	const uint16_t exp_to = 9 * 60;

	lamp_settings res;

	res = HAQuDA_LampDefaults();

	EXPECT_TRUE(res.on);
	EXPECT_EQ(exp_bright, res.brightPercent);
	EXPECT_EQ(lamp_mode::multi, res.mode);
	EXPECT_EQ(lamp_param::pm25, res.multi[0]);
	EXPECT_EQ(lamp_param::eco2, res.multi[1]);
	EXPECT_EQ(lamp_param::temp, res.multi[2]);
	EXPECT_EQ(exp_from, res.nightFromMin);
	EXPECT_EQ(exp_to, res.nightToMin);
	EXPECT_EQ(0, res.color);
}

TEST(LampSettingsTest, ValidRejectsBrightnessOffTheStep) {
	const bool exp_valid = false;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.brightPercent = 52;

	bool res;

	res = HAQuDA_LampSettingsValid(settings);

	EXPECT_EQ(exp_valid, res);
}

TEST(LampSettingsTest, ValidRejectsBrightnessBelowMinimum) {
	const bool exp_valid = false;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.brightPercent = 0;

	bool res;

	res = HAQuDA_LampSettingsValid(settings);

	EXPECT_EQ(exp_valid, res);
}

TEST(LampSettingsTest, ValidRejectsBrightnessAboveMaximum) {
	const bool exp_valid = false;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.brightPercent = 105;

	bool res;

	res = HAQuDA_LampSettingsValid(settings);

	EXPECT_EQ(exp_valid, res);
}

TEST(LampSettingsTest, ValidRejectsTotalInMulti) {
	const bool exp_valid = false;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.multi[1] = lamp_param::total;

	bool res;

	res = HAQuDA_LampSettingsValid(settings);

	EXPECT_EQ(exp_valid, res);
}

TEST(LampSettingsTest, ValidRejectsTotalForNight) {
	const bool exp_valid = false;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.nightParam = lamp_param::total;

	bool res;

	res = HAQuDA_LampSettingsValid(settings);

	EXPECT_EQ(exp_valid, res);
}

TEST(LampSettingsTest, ValidRejectsEqualNightTimes) {
	const bool exp_valid = false;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.nightFromMin = 300;
	settings.nightToMin = 300;

	bool res;

	res = HAQuDA_LampSettingsValid(settings);

	EXPECT_EQ(exp_valid, res);
}

TEST(LampSettingsTest, ValidRejectsNightTimeAboveADay) {
	const bool exp_valid = false;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.nightToMin = 24 * 60;

	bool res;

	res = HAQuDA_LampSettingsValid(settings);

	EXPECT_EQ(exp_valid, res);
}

TEST(LampSettingsTest, ValidRejectsColorIndexOutOfRange) {
	const bool exp_valid = false;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.color = 6;

	bool res;

	res = HAQuDA_LampSettingsValid(settings);

	EXPECT_EQ(exp_valid, res);
}

TEST(LampSettingsTest, ValidRejectsModeOutOfRange) {
	const bool exp_valid = false;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.mode = static_cast<lamp_mode>(4);

	bool res;

	res = HAQuDA_LampSettingsValid(settings);

	EXPECT_EQ(exp_valid, res);
}

TEST(LampSettingsTest, ValidAcceptsMinimumBrightness) {
	const bool exp_valid = true;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.brightPercent = LAMP_BRIGHT_MIN;

	bool res;

	res = HAQuDA_LampSettingsValid(settings);

	EXPECT_EQ(exp_valid, res);
}

TEST(LampSettingsTest, ValidAcceptsMaximumBrightness) {
	const bool exp_valid = true;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.brightPercent = LAMP_BRIGHT_MAX;

	bool res;

	res = HAQuDA_LampSettingsValid(settings);

	EXPECT_EQ(exp_valid, res);
}

TEST(LampSettingsTest, ValidAcceptsLastMinuteOfTheDay) {
	const bool exp_valid = true;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.nightFromMin = MINUTES_PER_DAY - 1;

	bool res;

	res = HAQuDA_LampSettingsValid(settings);

	EXPECT_EQ(exp_valid, res);
}

TEST(LampSettingsTest, ValidAcceptsLastColorIndex) {
	const bool exp_valid = true;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.color = LAMP_COLORS_NUM - 1;

	bool res;

	res = HAQuDA_LampSettingsValid(settings);

	EXPECT_EQ(exp_valid, res);
}

TEST(LampSettingsTest, ValidRejectsStandardParamOutOfRange) {
	const bool exp_valid = false;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.standardParam = static_cast<lamp_param>(6);

	bool res;

	res = HAQuDA_LampSettingsValid(settings);

	EXPECT_EQ(exp_valid, res);
}

TEST(LampSettingsTest, ValidRejectsNightParamOutOfRange) {
	const bool exp_valid = false;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.nightParam = static_cast<lamp_param>(6);

	bool res;

	res = HAQuDA_LampSettingsValid(settings);

	EXPECT_EQ(exp_valid, res);
}

TEST(LampSettingsTest, ValidRejectsMultiParamOutOfRange) {
	const bool exp_valid = false;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.multi[2] = static_cast<lamp_param>(6);

	bool res;

	res = HAQuDA_LampSettingsValid(settings);

	EXPECT_EQ(exp_valid, res);
}

TEST(LampSettingsTest, ValidRejectsNightFromAboveADay) {
	const bool exp_valid = false;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.nightFromMin = MINUTES_PER_DAY;

	bool res;

	res = HAQuDA_LampSettingsValid(settings);

	EXPECT_EQ(exp_valid, res);
}

TEST(LampSettingsTest, UpdatePowerOff) {
	const lamp_update_result exp_result = lamp_update_result::ok;

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.on = "0";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_FALSE(settings.on);
}

TEST(LampSettingsTest, UpdatePowerOn) {
	const lamp_update_result exp_result = lamp_update_result::ok;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.on = false;
	lamp_request request = {};
	request.on = "1";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_TRUE(settings.on);
}

TEST(LampSettingsTest, UpdatePowerWordIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_on;

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.on = "true";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
}

TEST(LampSettingsTest, UpdateBrightnessValidStep) {
	const lamp_update_result exp_result = lamp_update_result::ok;
	const uint8_t exp_bright = 65;

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.bright = "65";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_bright, settings.brightPercent);
}

TEST(LampSettingsTest, UpdateUnknownModeIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_mode;

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.mode = "disco";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
}

TEST(LampSettingsTest, UpdateNightWindowWithoutTimeIsAllowed) {
	const lamp_update_result exp_result = lamp_update_result::ok;
	const uint16_t exp_from = 22 * 60 + 30;

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightFrom = "22:30";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, false);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_from, settings.nightFromMin);
}

TEST(LampSettingsTest, UpdateStandardParam) {
	const lamp_update_result exp_result = lamp_update_result::ok;

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.standardParam = "tvoc";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(lamp_param::tvoc, settings.standardParam);
}

TEST(LampSettingsTest, UpdateStandardParamAllowsTotal) {
	const lamp_update_result exp_result = lamp_update_result::ok;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.standardParam = lamp_param::pm25;
	lamp_request request = {};
	request.standardParam = "total";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(lamp_param::total, settings.standardParam);
}

TEST(LampSettingsTest, UpdateStandardParamUnknownKeyIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_standard;

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.standardParam = "pm10";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
}

TEST(LampSettingsTest, UpdateMultiThreeKeysTopToBottom) {
	const lamp_update_result exp_result = lamp_update_result::ok;

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.multi = "temp,humid,tvoc";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(lamp_param::temp, settings.multi[0]);
	EXPECT_EQ(lamp_param::humid, settings.multi[1]);
	EXPECT_EQ(lamp_param::tvoc, settings.multi[2]);
}

TEST(LampSettingsTest, UpdateNightParam) {
	const lamp_update_result exp_result = lamp_update_result::ok;

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightParam = "humid";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(lamp_param::humid, settings.nightParam);
}

TEST(LampSettingsTest, UpdateNightParamTotalIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_night_param;

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightParam = "total";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
}

TEST(LampSettingsTest, UpdateNightWindowCrossingMidnight) {
	const lamp_update_result exp_result = lamp_update_result::ok;
	const uint16_t exp_from = 23 * 60 + 59;
	const uint16_t exp_to = 0;

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightFrom = "23:59";
	request.nightTo = "00:00";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_from, settings.nightFromMin);
	EXPECT_EQ(exp_to, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateColorIndex) {
	const lamp_update_result exp_result = lamp_update_result::ok;
	const uint8_t exp_color = 5;

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.color = "5";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_color, settings.color);
}

TEST(LampSettingsTest, UpdateKeepsTheFieldsNotInTheRequest) {
	const lamp_update_result exp_result = lamp_update_result::ok;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.color = 3;
	lamp_request request = {};
	request.bright = "20";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(3, settings.color);
	EXPECT_EQ(lamp_mode::multi, settings.mode);
	EXPECT_TRUE(settings.on);
}

TEST(LampSettingsTest, UpdateFieldLongerThan32CharactersIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_multi;
	const std::string longText(33, 'a');

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.multi = longText.c_str();

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
}

TEST(LampSettingsTest, UpdateLongTimeFieldReportsBadTime) {
	const lamp_update_result exp_result = lamp_update_result::bad_time;
	const std::string longText(100, '1');

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightFrom = longText.c_str();

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
}

TEST(LampSettingsTest, UpdateEmptyRequestIsNothing) {
	const lamp_update_result exp_result = lamp_update_result::nothing;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.on, settings.on);
	EXPECT_EQ(exp_settings.brightPercent, settings.brightPercent);
	EXPECT_EQ(exp_settings.mode, settings.mode);
	EXPECT_EQ(exp_settings.standardParam, settings.standardParam);
	EXPECT_EQ(exp_settings.multi[0], settings.multi[0]);
	EXPECT_EQ(exp_settings.multi[1], settings.multi[1]);
	EXPECT_EQ(exp_settings.multi[2], settings.multi[2]);
	EXPECT_EQ(exp_settings.nightParam, settings.nightParam);
	EXPECT_EQ(exp_settings.nightFromMin, settings.nightFromMin);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
	EXPECT_EQ(exp_settings.color, settings.color);
}

TEST(LampSettingsTest, UpdatePowerEmptyIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_on;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.on = "";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.on, settings.on);
}

TEST(LampSettingsTest, UpdatePowerTwoIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_on;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.on = "2";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.on, settings.on);
}

TEST(LampSettingsTest, UpdatePowerLeadingZeroIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_on;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.on = "01";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.on, settings.on);
}

TEST(LampSettingsTest, UpdatePowerWordOnIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_on;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.on = "on";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.on, settings.on);
}

TEST(LampSettingsTest, UpdatePowerTrailingSpaceIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_on;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.on = "1 ";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.on, settings.on);
}

TEST(LampSettingsTest, UpdateBrightnessMinimumIsAccepted) {
	const lamp_update_result exp_result = lamp_update_result::ok;
	const uint8_t exp_bright = LAMP_BRIGHT_MIN;

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.bright = "5";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_bright, settings.brightPercent);
}

TEST(LampSettingsTest, UpdateBrightnessMaximumIsAccepted) {
	const lamp_update_result exp_result = lamp_update_result::ok;
	const uint8_t exp_bright = LAMP_BRIGHT_MAX;

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.bright = "100";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_bright, settings.brightPercent);
}

TEST(LampSettingsTest, UpdateBrightnessLeadingZerosAreAccepted) {
	const lamp_update_result exp_result = lamp_update_result::ok;
	const uint8_t exp_bright = LAMP_BRIGHT_MIN;

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.bright = "005";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_bright, settings.brightPercent);
}

TEST(LampSettingsTest, UpdateBrightnessZeroIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_bright;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.bright = "0";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.brightPercent, settings.brightPercent);
}

TEST(LampSettingsTest, UpdateBrightnessFourIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_bright;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.bright = "4";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.brightPercent, settings.brightPercent);
}

TEST(LampSettingsTest, UpdateBrightnessSevenOffTheStepIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_bright;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.bright = "7";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.brightPercent, settings.brightPercent);
}

TEST(LampSettingsTest, UpdateBrightnessNinetyNineIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_bright;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.bright = "99";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.brightPercent, settings.brightPercent);
}

TEST(LampSettingsTest, UpdateBrightnessOneHundredOneIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_bright;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.bright = "101";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.brightPercent, settings.brightPercent);
}

TEST(LampSettingsTest, UpdateBrightnessOneHundredFiveIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_bright;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.bright = "105";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.brightPercent, settings.brightPercent);
}

TEST(LampSettingsTest, UpdateBrightnessFourDigitsIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_bright;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.bright = "1000";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.brightPercent, settings.brightPercent);
}

TEST(LampSettingsTest, UpdateBrightnessFourDigitsWithLeadingZeroIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_bright;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.bright = "0100";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.brightPercent, settings.brightPercent);
}

TEST(LampSettingsTest, UpdateBrightnessEmptyIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_bright;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.bright = "";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.brightPercent, settings.brightPercent);
}

TEST(LampSettingsTest, UpdateBrightnessLettersAreBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_bright;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.bright = "abc";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.brightPercent, settings.brightPercent);
}

TEST(LampSettingsTest, UpdateBrightnessNegativeIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_bright;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.bright = "-5";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.brightPercent, settings.brightPercent);
}

TEST(LampSettingsTest, UpdateBrightnessTrailingSpaceIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_bright;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.bright = "5 ";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.brightPercent, settings.brightPercent);
}

TEST(LampSettingsTest, UpdateBrightnessDecimalIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_bright;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.bright = "6.5";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.brightPercent, settings.brightPercent);
}

TEST(LampSettingsTest, UpdateBrightnessPlusSignIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_bright;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.bright = "+5";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.brightPercent, settings.brightPercent);
}

TEST(LampSettingsTest, UpdateModeStandardIsAccepted) {
	const lamp_update_result exp_result = lamp_update_result::ok;
	const lamp_mode exp_mode = lamp_mode::standard;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.mode = lamp_mode::multi;
	lamp_request request = {};
	request.mode = "standard";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_mode, settings.mode);
}

TEST(LampSettingsTest, UpdateModeMultiIsAccepted) {
	const lamp_update_result exp_result = lamp_update_result::ok;
	const lamp_mode exp_mode = lamp_mode::multi;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.mode = lamp_mode::light;
	lamp_request request = {};
	request.mode = "multi";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_mode, settings.mode);
}

TEST(LampSettingsTest, UpdateModeNightIsAccepted) {
	const lamp_update_result exp_result = lamp_update_result::ok;
	const lamp_mode exp_mode = lamp_mode::night;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.mode = lamp_mode::multi;
	lamp_request request = {};
	request.mode = "night";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_mode, settings.mode);
}

TEST(LampSettingsTest, UpdateModeLightIsAccepted) {
	const lamp_update_result exp_result = lamp_update_result::ok;
	const lamp_mode exp_mode = lamp_mode::light;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.mode = lamp_mode::multi;
	lamp_request request = {};
	request.mode = "light";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_mode, settings.mode);
}

TEST(LampSettingsTest, UpdateModeEmptyIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_mode;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.mode = "";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.mode, settings.mode);
}

TEST(LampSettingsTest, UpdateModeUpperCaseIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_mode;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.mode = "Night";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.mode, settings.mode);
}

TEST(LampSettingsTest, UpdateModeTrailingSpaceIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_mode;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.mode = "night ";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.mode, settings.mode);
}

TEST(LampSettingsTest, UpdateNightModeWithoutTimeIsRefused) {
	const lamp_update_result exp_result = lamp_update_result::night_needs_time;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.mode = "night";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, false);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.mode, settings.mode);
}

TEST(LampSettingsTest, UpdateNightModeWithoutTimeRefusesTheWholeRequest) {
	const lamp_update_result exp_result = lamp_update_result::night_needs_time;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.bright = "80";
	request.mode = "night";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, false);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.brightPercent, settings.brightPercent);
	EXPECT_EQ(exp_settings.mode, settings.mode);
}

TEST(LampSettingsTest, UpdateLightModeWithoutTimeIsAccepted) {
	const lamp_update_result exp_result = lamp_update_result::ok;
	const lamp_mode exp_mode = lamp_mode::light;

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.mode = "light";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, false);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_mode, settings.mode);
}

TEST(LampSettingsTest, UpdateBrightnessWhileInNightModeWithoutTimeIsAccepted) {
	const lamp_update_result exp_result = lamp_update_result::ok;
	const uint8_t exp_bright = 20;
	const lamp_mode exp_mode = lamp_mode::night;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.mode = lamp_mode::night;
	lamp_request request = {};
	request.bright = "20";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, false);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_bright, settings.brightPercent);
	EXPECT_EQ(exp_mode, settings.mode);
}

TEST(LampSettingsTest, UpdateStandardParamEmptyIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_standard;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.standardParam = "";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.standardParam, settings.standardParam);
}

TEST(LampSettingsTest, UpdateStandardParamUpperCaseIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_standard;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.standardParam = "TOTAL";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.standardParam, settings.standardParam);
}

TEST(LampSettingsTest, UpdateNightParamEmptyIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_night_param;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightParam = "";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightParam, settings.nightParam);
}

TEST(LampSettingsTest, UpdateNightParamUnknownKeyIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_night_param;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightParam = "pm10";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightParam, settings.nightParam);
}

TEST(LampSettingsTest, UpdateNightParamUpperCaseIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_night_param;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightParam = "HUMID";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightParam, settings.nightParam);
}

TEST(LampSettingsTest, UpdateMultiDuplicatesAreAccepted) {
	const lamp_update_result exp_result = lamp_update_result::ok;
	const lamp_param exp_param = lamp_param::temp;

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.multi = "temp,temp,temp";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_param, settings.multi[0]);
	EXPECT_EQ(exp_param, settings.multi[1]);
	EXPECT_EQ(exp_param, settings.multi[2]);
}

TEST(LampSettingsTest, UpdateMultiEmptyIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_multi;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.multi = "";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.multi[0], settings.multi[0]);
	EXPECT_EQ(exp_settings.multi[1], settings.multi[1]);
	EXPECT_EQ(exp_settings.multi[2], settings.multi[2]);
}

TEST(LampSettingsTest, UpdateMultiOneKeyIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_multi;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.multi = "temp";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.multi[0], settings.multi[0]);
	EXPECT_EQ(exp_settings.multi[1], settings.multi[1]);
	EXPECT_EQ(exp_settings.multi[2], settings.multi[2]);
}

TEST(LampSettingsTest, UpdateMultiTwoKeysIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_multi;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.multi = "temp,humid";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.multi[0], settings.multi[0]);
	EXPECT_EQ(exp_settings.multi[1], settings.multi[1]);
	EXPECT_EQ(exp_settings.multi[2], settings.multi[2]);
}

TEST(LampSettingsTest, UpdateMultiFourKeysIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_multi;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.multi = "temp,humid,tvoc,pm25";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.multi[0], settings.multi[0]);
	EXPECT_EQ(exp_settings.multi[1], settings.multi[1]);
	EXPECT_EQ(exp_settings.multi[2], settings.multi[2]);
}

TEST(LampSettingsTest, UpdateMultiTotalFirstIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_multi;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.multi = "total,temp,humid";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.multi[0], settings.multi[0]);
	EXPECT_EQ(exp_settings.multi[1], settings.multi[1]);
	EXPECT_EQ(exp_settings.multi[2], settings.multi[2]);
}

TEST(LampSettingsTest, UpdateMultiTotalLastIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_multi;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.multi = "temp,humid,total";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.multi[0], settings.multi[0]);
	EXPECT_EQ(exp_settings.multi[1], settings.multi[1]);
	EXPECT_EQ(exp_settings.multi[2], settings.multi[2]);
}

TEST(LampSettingsTest, UpdateMultiEmptyMiddleKeyIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_multi;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.multi = "temp,,humid";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.multi[0], settings.multi[0]);
	EXPECT_EQ(exp_settings.multi[1], settings.multi[1]);
	EXPECT_EQ(exp_settings.multi[2], settings.multi[2]);
}

TEST(LampSettingsTest, UpdateMultiTrailingCommaIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_multi;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.multi = "temp,humid,";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.multi[0], settings.multi[0]);
	EXPECT_EQ(exp_settings.multi[1], settings.multi[1]);
	EXPECT_EQ(exp_settings.multi[2], settings.multi[2]);
}

TEST(LampSettingsTest, UpdateMultiLeadingCommaIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_multi;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.multi = ",temp,humid";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.multi[0], settings.multi[0]);
	EXPECT_EQ(exp_settings.multi[1], settings.multi[1]);
	EXPECT_EQ(exp_settings.multi[2], settings.multi[2]);
}

TEST(LampSettingsTest, UpdateMultiUnknownLastKeyIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_multi;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.multi = "temp,humid,pm10";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.multi[0], settings.multi[0]);
	EXPECT_EQ(exp_settings.multi[1], settings.multi[1]);
	EXPECT_EQ(exp_settings.multi[2], settings.multi[2]);
}

TEST(LampSettingsTest, UpdateMultiSpacesInsteadOfCommasIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_multi;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.multi = "temp humid tvoc";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.multi[0], settings.multi[0]);
	EXPECT_EQ(exp_settings.multi[1], settings.multi[1]);
	EXPECT_EQ(exp_settings.multi[2], settings.multi[2]);
}

TEST(LampSettingsTest, UpdateMultiSpaceAfterCommaIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_multi;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.multi = "temp, humid,tvoc";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.multi[0], settings.multi[0]);
	EXPECT_EQ(exp_settings.multi[1], settings.multi[1]);
	EXPECT_EQ(exp_settings.multi[2], settings.multi[2]);
}

TEST(LampSettingsTest, UpdateMultiUpperCaseKeyIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_multi;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.multi = "TEMP,humid,tvoc";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.multi[0], settings.multi[0]);
	EXPECT_EQ(exp_settings.multi[1], settings.multi[1]);
	EXPECT_EQ(exp_settings.multi[2], settings.multi[2]);
}

TEST(LampSettingsTest, UpdateMultiOnlyCommasIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_multi;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.multi = ",,";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.multi[0], settings.multi[0]);
	EXPECT_EQ(exp_settings.multi[1], settings.multi[1]);
	EXPECT_EQ(exp_settings.multi[2], settings.multi[2]);
}

TEST(LampSettingsTest, UpdateNightToMidnightIsAccepted) {
	const lamp_update_result exp_result = lamp_update_result::ok;
	const uint16_t exp_to = 0;

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightTo = "00:00";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_to, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateNightToLastMinuteIsAccepted) {
	const lamp_update_result exp_result = lamp_update_result::ok;
	const uint16_t exp_to = MINUTES_PER_DAY - 1;

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightTo = "23:59";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_to, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateNightWindowWithinOneDayIsAccepted) {
	const lamp_update_result exp_result = lamp_update_result::ok;
	const uint16_t exp_from = 1 * 60;
	const uint16_t exp_to = 6 * 60;

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightFrom = "01:00";
	request.nightTo = "06:00";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_from, settings.nightFromMin);
	EXPECT_EQ(exp_to, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateNightToEmptyIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_time;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightTo = "";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateNightToHour24IsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_time;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightTo = "24:00";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateNightToMinute60AtLastHourIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_time;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightTo = "23:60";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateNightToMinute60IsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_time;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightTo = "12:60";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateNightToHour24Minute59IsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_time;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightTo = "24:59";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateNightToNinetyNineIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_time;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightTo = "99:99";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateNightToSingleDigitHourIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_time;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightTo = "9:00";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateNightToSingleDigitMinuteIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_time;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightTo = "09:0";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateNightToLettersIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_time;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightTo = "ab:cd";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateNightToDashSeparatorIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_time;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightTo = "09-00";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateNightToNoSeparatorIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_time;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightTo = "0900";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateNightToWithSecondsIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_time;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightTo = "09:00:00";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateNightToPlusSignIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_time;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightTo = "+9:00";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateNightToLeadingSpaceIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_time;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightTo = " 9:00";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateNightToTrailingSpaceIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_time;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightTo = "09:00 ";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateNightToNegativeHourIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_time;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightTo = "-1:00";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateNightFromHour24IsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_time;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightFrom = "24:00";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightFromMin, settings.nightFromMin);
}

TEST(LampSettingsTest, UpdateNightBadFromKeepsTheValidTo) {
	const lamp_update_result exp_result = lamp_update_result::bad_time;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightFrom = "25:00";
	request.nightTo = "10:00";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightFromMin, settings.nightFromMin);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateNightBadToKeepsTheValidFrom) {
	const lamp_update_result exp_result = lamp_update_result::bad_time;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightFrom = "22:00";
	request.nightTo = "24:00";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightFromMin, settings.nightFromMin);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateNightFromEqualToStoredToIsRefused) {
	const lamp_update_result exp_result = lamp_update_result::night_window_empty;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightFrom = "09:00";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightFromMin, settings.nightFromMin);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateNightToEqualToStoredFromIsRefused) {
	const lamp_update_result exp_result = lamp_update_result::night_window_empty;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightTo = "21:00";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightFromMin, settings.nightFromMin);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateNightEqualTimesInOneRequestAreRefused) {
	const lamp_update_result exp_result = lamp_update_result::night_window_empty;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightFrom = "10:00";
	request.nightTo = "10:00";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightFromMin, settings.nightFromMin);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateColorFirstIndexIsAccepted) {
	const lamp_update_result exp_result = lamp_update_result::ok;
	const uint8_t exp_color = 0;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.color = 3;
	lamp_request request = {};
	request.color = "0";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_color, settings.color);
}

TEST(LampSettingsTest, UpdateColorLeadingZeroIsAccepted) {
	const lamp_update_result exp_result = lamp_update_result::ok;
	const uint8_t exp_color = 3;

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.color = "03";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_color, settings.color);
}

TEST(LampSettingsTest, UpdateColorSixIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_color;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.color = "6";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.color, settings.color);
}

TEST(LampSettingsTest, UpdateColorNegativeIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_color;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.color = "-1";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.color, settings.color);
}

TEST(LampSettingsTest, UpdateColorEmptyIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_color;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.color = "";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.color, settings.color);
}

TEST(LampSettingsTest, UpdateColorLetterIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_color;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.color = "x";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.color, settings.color);
}

TEST(LampSettingsTest, UpdateColorTenIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_color;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.color = "10";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.color, settings.color);
}

TEST(LampSettingsTest, UpdateColorHexIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_color;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.color = "0x1";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.color, settings.color);
}

TEST(LampSettingsTest, UpdateColorFourDigitsIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_color;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.color = "0001";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.color, settings.color);
}

TEST(LampSettingsTest, UpdateColorPlusSignIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_color;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.color = "+1";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.color, settings.color);
}

TEST(LampSettingsTest, UpdateColorLeadingSpaceIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_color;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.color = " 1";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.color, settings.color);
}

TEST(LampSettingsTest, UpdateColorDecimalIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_color;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.color = "1.0";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.color, settings.color);
}

TEST(LampSettingsTest, UpdateColorTwoFiftyFiveIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_color;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.color = "255";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.color, settings.color);
}

TEST(LampSettingsTest, UpdateColorOverflowingNumberIsBad) {
	const lamp_update_result exp_result = lamp_update_result::bad_color;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.color = "4294967297";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.color, settings.color);
}

TEST(LampSettingsTest, UpdatePowerLongerThan32CharactersIsBad) {
	const std::string longText(33, '1');

	const lamp_update_result exp_result = lamp_update_result::bad_on;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.on = longText.c_str();

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.on, settings.on);
}

TEST(LampSettingsTest, UpdateBrightnessLongerThan32CharactersIsBad) {
	const std::string longText(33, '1');

	const lamp_update_result exp_result = lamp_update_result::bad_bright;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.bright = longText.c_str();

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.brightPercent, settings.brightPercent);
}

TEST(LampSettingsTest, UpdateModeLongerThan32CharactersIsBad) {
	const std::string longText(33, '1');

	const lamp_update_result exp_result = lamp_update_result::bad_mode;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.mode = longText.c_str();

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.mode, settings.mode);
}

TEST(LampSettingsTest, UpdateStandardParamLongerThan32CharactersIsBad) {
	const std::string longText(33, '1');

	const lamp_update_result exp_result = lamp_update_result::bad_standard;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.standardParam = longText.c_str();

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.standardParam, settings.standardParam);
}

TEST(LampSettingsTest, UpdateNightParamLongerThan32CharactersIsBad) {
	const std::string longText(33, '1');

	const lamp_update_result exp_result = lamp_update_result::bad_night_param;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightParam = longText.c_str();

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightParam, settings.nightParam);
}

TEST(LampSettingsTest, UpdateNightToLongerThan32CharactersIsBad) {
	const std::string longText(33, '1');

	const lamp_update_result exp_result = lamp_update_result::bad_time;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.nightTo = longText.c_str();

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
}

TEST(LampSettingsTest, UpdateColorLongerThan32CharactersIsBad) {
	const std::string longText(33, '1');

	const lamp_update_result exp_result = lamp_update_result::bad_color;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.color = longText.c_str();

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.color, settings.color);
}

TEST(LampSettingsTest, UpdateLongFieldRefusesTheWholeRequest) {
	const std::string longText(33, 'a');

	const lamp_update_result exp_result = lamp_update_result::bad_multi;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.bright = "80";
	request.multi = longText.c_str();

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.brightPercent, settings.brightPercent);
	EXPECT_EQ(exp_settings.multi[0], settings.multi[0]);
	EXPECT_EQ(exp_settings.multi[1], settings.multi[1]);
	EXPECT_EQ(exp_settings.multi[2], settings.multi[2]);
}

TEST(LampSettingsTest, UpdateBadColorKeepsEveryOtherField) {
	const lamp_update_result exp_result = lamp_update_result::bad_color;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.on = "0";
	request.bright = "80";
	request.mode = "light";
	request.color = "9";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.on, settings.on);
	EXPECT_EQ(exp_settings.brightPercent, settings.brightPercent);
	EXPECT_EQ(exp_settings.mode, settings.mode);
	EXPECT_EQ(exp_settings.standardParam, settings.standardParam);
	EXPECT_EQ(exp_settings.multi[0], settings.multi[0]);
	EXPECT_EQ(exp_settings.multi[1], settings.multi[1]);
	EXPECT_EQ(exp_settings.multi[2], settings.multi[2]);
	EXPECT_EQ(exp_settings.nightParam, settings.nightParam);
	EXPECT_EQ(exp_settings.nightFromMin, settings.nightFromMin);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
	EXPECT_EQ(exp_settings.color, settings.color);
}

TEST(LampSettingsTest, UpdateBadMultiInTheMiddleKeepsTheLaterFields) {
	const lamp_update_result exp_result = lamp_update_result::bad_multi;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.bright = "80";
	request.multi = "temp,humid";
	request.color = "3";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.on, settings.on);
	EXPECT_EQ(exp_settings.brightPercent, settings.brightPercent);
	EXPECT_EQ(exp_settings.mode, settings.mode);
	EXPECT_EQ(exp_settings.standardParam, settings.standardParam);
	EXPECT_EQ(exp_settings.multi[0], settings.multi[0]);
	EXPECT_EQ(exp_settings.multi[1], settings.multi[1]);
	EXPECT_EQ(exp_settings.multi[2], settings.multi[2]);
	EXPECT_EQ(exp_settings.nightParam, settings.nightParam);
	EXPECT_EQ(exp_settings.nightFromMin, settings.nightFromMin);
	EXPECT_EQ(exp_settings.nightToMin, settings.nightToMin);
	EXPECT_EQ(exp_settings.color, settings.color);
}

TEST(LampSettingsTest, UpdateAllFieldsAtOnceAreApplied) {
	const lamp_update_result exp_result = lamp_update_result::ok;
	const uint8_t exp_bright = 35;
	const uint16_t exp_from = 22 * 60 + 15;
	const uint16_t exp_to = 6 * 60 + 45;
	const uint8_t exp_color = 4;

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	request.on = "0";
	request.bright = "35";
	request.mode = "night";
	request.standardParam = "pm25";
	request.multi = "humid,tvoc,temp";
	request.nightParam = "tvoc";
	request.nightFrom = "22:15";
	request.nightTo = "06:45";
	request.color = "4";

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_FALSE(settings.on);
	EXPECT_EQ(exp_bright, settings.brightPercent);
	EXPECT_EQ(lamp_mode::night, settings.mode);
	EXPECT_EQ(lamp_param::pm25, settings.standardParam);
	EXPECT_EQ(lamp_param::humid, settings.multi[0]);
	EXPECT_EQ(lamp_param::tvoc, settings.multi[1]);
	EXPECT_EQ(lamp_param::temp, settings.multi[2]);
	EXPECT_EQ(lamp_param::tvoc, settings.nightParam);
	EXPECT_EQ(exp_from, settings.nightFromMin);
	EXPECT_EQ(exp_to, settings.nightToMin);
	EXPECT_EQ(exp_color, settings.color);
}

TEST(LampSettingsTest, UpdateTextNothingIsTheDocumentedMessage) {
	const char *exp_text = "Nothing to change.";

	const lamp_update_result result = lamp_update_result::nothing;

	const char *res;

	res = HAQuDA_LampUpdateText(result);

	EXPECT_STREQ(exp_text, res);
}

TEST(LampSettingsTest, UpdateTextBadOnIsTheDocumentedMessage) {
	const char *exp_text = "Power must be 1 or 0.";

	const lamp_update_result result = lamp_update_result::bad_on;

	const char *res;

	res = HAQuDA_LampUpdateText(result);

	EXPECT_STREQ(exp_text, res);
}

TEST(LampSettingsTest, UpdateTextBadBrightIsTheDocumentedMessage) {
	const char *exp_text = "Brightness must be 5 to 100 in steps of 5.";

	const lamp_update_result result = lamp_update_result::bad_bright;

	const char *res;

	res = HAQuDA_LampUpdateText(result);

	EXPECT_STREQ(exp_text, res);
}

TEST(LampSettingsTest, UpdateTextBadModeIsTheDocumentedMessage) {
	const char *exp_text = "Unknown mode.";

	const lamp_update_result result = lamp_update_result::bad_mode;

	const char *res;

	res = HAQuDA_LampUpdateText(result);

	EXPECT_STREQ(exp_text, res);
}

TEST(LampSettingsTest, UpdateTextBadStandardIsTheDocumentedMessage) {
	const char *exp_text = "Unknown reading for the standard mode.";

	const lamp_update_result result = lamp_update_result::bad_standard;

	const char *res;

	res = HAQuDA_LampUpdateText(result);

	EXPECT_STREQ(exp_text, res);
}

TEST(LampSettingsTest, UpdateTextBadMultiIsTheDocumentedMessage) {
	const char *exp_text = "The multi mode needs three readings, top to bottom, separated by commas.";

	const lamp_update_result result = lamp_update_result::bad_multi;

	const char *res;

	res = HAQuDA_LampUpdateText(result);

	EXPECT_STREQ(exp_text, res);
}

TEST(LampSettingsTest, UpdateTextBadNightParamIsTheDocumentedMessage) {
	const char *exp_text = "Unknown reading for the night mode.";

	const lamp_update_result result = lamp_update_result::bad_night_param;

	const char *res;

	res = HAQuDA_LampUpdateText(result);

	EXPECT_STREQ(exp_text, res);
}

TEST(LampSettingsTest, UpdateTextBadTimeIsTheDocumentedMessage) {
	const char *exp_text = "The night time must look like 21:30.";

	const lamp_update_result result = lamp_update_result::bad_time;

	const char *res;

	res = HAQuDA_LampUpdateText(result);

	EXPECT_STREQ(exp_text, res);
}

TEST(LampSettingsTest, UpdateTextBadColorIsTheDocumentedMessage) {
	const char *exp_text = "The colour must be 0 to 5.";

	const lamp_update_result result = lamp_update_result::bad_color;

	const char *res;

	res = HAQuDA_LampUpdateText(result);

	EXPECT_STREQ(exp_text, res);
}

TEST(LampSettingsTest, UpdateTextNightWindowEmptyIsTheDocumentedMessage) {
	const char *exp_text = "The night light must start and end at different times.";

	const lamp_update_result result = lamp_update_result::night_window_empty;

	const char *res;

	res = HAQuDA_LampUpdateText(result);

	EXPECT_STREQ(exp_text, res);
}

TEST(LampSettingsTest, UpdateTextNightNeedsTimeIsTheDocumentedMessage) {
	const char *exp_text = "The night mode needs the time from the internet.";

	const lamp_update_result result = lamp_update_result::night_needs_time;

	const char *res;

	res = HAQuDA_LampUpdateText(result);

	EXPECT_STREQ(exp_text, res);
}

TEST(LampSettingsTest, UpdateTextOkIsTheDocumentedMessage) {
	const char *exp_text = "";

	const lamp_update_result result = lamp_update_result::ok;

	const char *res;

	res = HAQuDA_LampUpdateText(result);

	EXPECT_STREQ(exp_text, res);
}

TEST(LampSettingsTest, JsonOfDefaultsWithoutTime) {
	const char *exp_json = "{\"on\":true,\"bright\":35,\"mode\":\"multi\",\"multi\":[\"pm25\",\"eco2\",\"temp\"],\"std\":\"total\","
						   "\"night\":{\"param\":\"eco2\",\"from\":\"21:00\",\"to\":\"09:00\"},\"color\":0,\"time\":false}";
	const size_t exp_len = std::strlen(exp_json);

	const lamp_settings settings = HAQuDA_LampDefaults();
	char buf[LAMP_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildLampJson(settings, false, 0, buf, sizeof(buf));

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(LampSettingsTest, JsonWithTimeCarriesTheClock) {
	const char *exp_json = "{\"on\":false,\"bright\":5,\"mode\":\"night\",\"multi\":[\"temp\",\"humid\",\"tvoc\"],\"std\":\"pm25\","
						   "\"night\":{\"param\":\"humid\",\"from\":\"07:05\",\"to\":\"23:59\"},\"color\":4,\"time\":true,\"now\":\"22:15\"}";
	const size_t exp_len = std::strlen(exp_json);

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.on = false;
	settings.brightPercent = 5;
	settings.mode = lamp_mode::night;
	settings.multi[0] = lamp_param::temp;
	settings.multi[1] = lamp_param::humid;
	settings.multi[2] = lamp_param::tvoc;
	settings.standardParam = lamp_param::pm25;
	settings.nightParam = lamp_param::humid;
	settings.nightFromMin = 7 * 60 + 5;
	settings.nightToMin = 23 * 60 + 59;
	settings.color = 4;
	char buf[LAMP_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildLampJson(settings, true, 22 * 60 + 15, buf, sizeof(buf));

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(LampSettingsTest, JsonLongestFitsTheBuffer) {
	lamp_settings settings = HAQuDA_LampDefaults();
	settings.brightPercent = 100;
	settings.mode = lamp_mode::standard;
	settings.multi[0] = lamp_param::humid;
	settings.multi[1] = lamp_param::humid;
	settings.multi[2] = lamp_param::humid;
	settings.nightParam = lamp_param::humid;
	settings.nightFromMin = 23 * 60 + 59;
	settings.nightToMin = 23 * 60 + 58;
	settings.color = 5;
	char buf[LAMP_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildLampJson(settings, true, 23 * 60 + 59, buf, sizeof(buf));

	EXPECT_GT(res, 0u);
	EXPECT_EQ(std::strlen(buf), res);
}

TEST(LampSettingsTest, JsonBufferTooSmallGivesZeroAndEmptyText) {
	const size_t exp_len = 0;

	const lamp_settings settings = HAQuDA_LampDefaults();
	char buf[40];
	std::memset(buf, 'x', sizeof(buf));

	size_t res;

	res = HAQuDA_BuildLampJson(settings, true, 100, buf, sizeof(buf));

	EXPECT_EQ(exp_len, res);
	EXPECT_EQ('\0', buf[0]);
}

TEST(LampSettingsTest, JsonNullBufferGivesZero) {
	const size_t exp_len = 0;

	const lamp_settings settings = HAQuDA_LampDefaults();

	size_t res;

	res = HAQuDA_BuildLampJson(settings, false, 0, nullptr, 100);

	EXPECT_EQ(exp_len, res);
}

TEST(LampSettingsTest, JsonWithoutTimeHasNoClockWhateverTheMinute) {
	const char *exp_json = "{\"on\":true,\"bright\":35,\"mode\":\"multi\",\"multi\":[\"pm25\",\"eco2\",\"temp\"],\"std\":\"total\",\"night\":{\"param\":"
						   "\"eco2\",\"from\":\"21:00\",\"to\":\"09:00\"},\"color\":0,\"time\":false}";
	const size_t exp_len = std::strlen(exp_json);

	lamp_settings settings = HAQuDA_LampDefaults();
	char buf[LAMP_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildLampJson(settings, false, 600, buf, sizeof(buf));

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(LampSettingsTest, JsonClockAtMidnightIsZeroZero) {
	const char *exp_json = "{\"on\":true,\"bright\":35,\"mode\":\"multi\",\"multi\":[\"pm25\",\"eco2\",\"temp\"],\"std\":\"total\",\"night\":{\"param\":"
						   "\"eco2\",\"from\":\"21:00\",\"to\":\"09:00\"},\"color\":0,\"time\":true,\"now\":\"00:00\"}";
	const size_t exp_len = std::strlen(exp_json);

	lamp_settings settings = HAQuDA_LampDefaults();
	char buf[LAMP_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildLampJson(settings, true, 0, buf, sizeof(buf));

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(LampSettingsTest, JsonClockAtLastMinuteOfTheDay) {
	const char *exp_json = "{\"on\":true,\"bright\":35,\"mode\":\"multi\",\"multi\":[\"pm25\",\"eco2\",\"temp\"],\"std\":\"total\",\"night\":{\"param\":"
						   "\"eco2\",\"from\":\"21:00\",\"to\":\"09:00\"},\"color\":0,\"time\":true,\"now\":\"23:59\"}";
	const size_t exp_len = std::strlen(exp_json);

	lamp_settings settings = HAQuDA_LampDefaults();
	char buf[LAMP_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildLampJson(settings, true, 1439, buf, sizeof(buf));

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(LampSettingsTest, JsonClockMinuteAboveADayWrapsTheHour) {
	// 25:00 is not a time of day; the hour wraps instead of printing 25

	const char *exp_json = "{\"on\":true,\"bright\":35,\"mode\":\"multi\",\"multi\":[\"pm25\",\"eco2\",\"temp\"],\"std\":\"total\",\"night\":{\"param\":"
						   "\"eco2\",\"from\":\"21:00\",\"to\":\"09:00\"},\"color\":0,\"time\":true,\"now\":\"01:00\"}";
	const size_t exp_len = std::strlen(exp_json);

	lamp_settings settings = HAQuDA_LampDefaults();
	char buf[LAMP_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildLampJson(settings, true, 1500, buf, sizeof(buf));

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(LampSettingsTest, JsonModeLightWithLastColor) {
	const char *exp_json = "{\"on\":true,\"bright\":35,\"mode\":\"light\",\"multi\":[\"pm25\",\"eco2\",\"temp\"],\"std\":\"total\",\"night\":{\"param\":"
						   "\"eco2\",\"from\":\"21:00\",\"to\":\"09:00\"},\"color\":5,\"time\":false}";
	const size_t exp_len = std::strlen(exp_json);

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.mode = lamp_mode::light;
	settings.color = LAMP_COLORS_NUM - 1;
	char buf[LAMP_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildLampJson(settings, false, 0, buf, sizeof(buf));

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(LampSettingsTest, JsonModeStandardWithTvocReading) {
	const char *exp_json = "{\"on\":true,\"bright\":35,\"mode\":\"standard\",\"multi\":[\"pm25\",\"eco2\",\"temp\"],\"std\":\"tvoc\",\"night\":{\"param\":"
						   "\"eco2\",\"from\":\"21:00\",\"to\":\"09:00\"},\"color\":0,\"time\":false}";
	const size_t exp_len = std::strlen(exp_json);

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.mode = lamp_mode::standard;
	settings.standardParam = lamp_param::tvoc;
	char buf[LAMP_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildLampJson(settings, false, 0, buf, sizeof(buf));

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(LampSettingsTest, JsonPowerOffAtFullBrightness) {
	const char *exp_json = "{\"on\":false,\"bright\":100,\"mode\":\"multi\",\"multi\":[\"pm25\",\"eco2\",\"temp\"],\"std\":\"total\",\"night\":{\"param\":"
						   "\"eco2\",\"from\":\"21:00\",\"to\":\"09:00\"},\"color\":0,\"time\":false}";
	const size_t exp_len = std::strlen(exp_json);

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.on = false;
	settings.brightPercent = LAMP_BRIGHT_MAX;
	char buf[LAMP_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildLampJson(settings, false, 0, buf, sizeof(buf));

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(LampSettingsTest, JsonModeOutOfRangeFallsBackToStandard) {
	const char *exp_json = "{\"on\":true,\"bright\":35,\"mode\":\"standard\",\"multi\":[\"pm25\",\"eco2\",\"temp\"],\"std\":\"total\",\"night\":{\"param\":"
						   "\"eco2\",\"from\":\"21:00\",\"to\":\"09:00\"},\"color\":0,\"time\":false}";
	const size_t exp_len = std::strlen(exp_json);

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.mode = static_cast<lamp_mode>(9);
	char buf[LAMP_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildLampJson(settings, false, 0, buf, sizeof(buf));

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(LampSettingsTest, JsonParamOutOfRangeFallsBackToTotal) {
	const char *exp_json = "{\"on\":true,\"bright\":35,\"mode\":\"multi\",\"multi\":[\"pm25\",\"eco2\",\"temp\"],\"std\":\"total\",\"night\":{\"param\":"
						   "\"total\",\"from\":\"21:00\",\"to\":\"09:00\"},\"color\":0,\"time\":false}";
	const size_t exp_len = std::strlen(exp_json);

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.nightParam = static_cast<lamp_param>(9);
	char buf[LAMP_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildLampJson(settings, false, 0, buf, sizeof(buf));

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(LampSettingsTest, JsonBufferOfExactlyTheTextPlusTerminatorFits) {
	const char *exp_json = "{\"on\":true,\"bright\":35,\"mode\":\"multi\",\"multi\":[\"pm25\",\"eco2\",\"temp\"],\"std\":\"total\",\"night\":{\"param\":"
						   "\"eco2\",\"from\":\"21:00\",\"to\":\"09:00\"},\"color\":0,\"time\":true,\"now\":\"00:00\"}";
	const size_t exp_len = std::strlen(exp_json);

	const lamp_settings settings = HAQuDA_LampDefaults();
	char buf[LAMP_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildLampJson(settings, true, 0, buf, exp_len + 1);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(LampSettingsTest, JsonBufferWithoutRoomForTheTerminatorGivesZero) {
	// the text is 172 characters; a buffer of that size has no room for the NUL

	const char *exp_full = "{\"on\":true,\"bright\":35,\"mode\":\"multi\",\"multi\":[\"pm25\",\"eco2\",\"temp\"],\"std\":\"total\",\"night\":{\"param\":"
						   "\"eco2\",\"from\":\"21:00\",\"to\":\"09:00\"},\"color\":0,\"time\":true,\"now\":\"00:00\"}";
	const size_t exp_len = 0;

	const size_t fullLen = std::strlen(exp_full);
	const lamp_settings settings = HAQuDA_LampDefaults();
	char buf[LAMP_JSON_BUF_LEN];
	std::memset(buf, 'x', sizeof(buf));

	size_t res;

	res = HAQuDA_BuildLampJson(settings, true, 0, buf, fullLen);

	EXPECT_EQ(exp_len, res);
	EXPECT_EQ('\0', buf[0]);
}

TEST(LampSettingsTest, JsonBufferCutInsideTheClockGivesZero) {
	// the cut lands in ","now":"00:00"}", after the whole head

	const char *exp_full = "{\"on\":true,\"bright\":35,\"mode\":\"multi\",\"multi\":[\"pm25\",\"eco2\",\"temp\"],\"std\":\"total\",\"night\":{\"param\":"
						   "\"eco2\",\"from\":\"21:00\",\"to\":\"09:00\"},\"color\":0,\"time\":true,\"now\":\"00:00\"}";
	const size_t exp_len = 0;

	const size_t fullLen = std::strlen(exp_full);
	const size_t cut = 5;
	const lamp_settings settings = HAQuDA_LampDefaults();
	char buf[LAMP_JSON_BUF_LEN];
	std::memset(buf, 'x', sizeof(buf));

	size_t res;

	res = HAQuDA_BuildLampJson(settings, true, 0, buf, fullLen - cut);

	EXPECT_EQ(exp_len, res);
	EXPECT_EQ('\0', buf[0]);
}

TEST(LampSettingsTest, JsonBufferEndingRightAfterTheHeadGivesZero) {
	// no clock: the head ends with true and only the closing brace is missing

	const char *exp_full = "{\"on\":true,\"bright\":35,\"mode\":\"multi\",\"multi\":[\"pm25\",\"eco2\",\"temp\"],\"std\":\"total\",\"night\":{\"param\":"
						   "\"eco2\",\"from\":\"21:00\",\"to\":\"09:00\"},\"color\":0,\"time\":false}";
	const size_t exp_len = 0;

	const size_t fullLen = std::strlen(exp_full);
	const lamp_settings settings = HAQuDA_LampDefaults();
	char buf[LAMP_JSON_BUF_LEN];
	std::memset(buf, 'x', sizeof(buf));

	size_t res;

	res = HAQuDA_BuildLampJson(settings, false, 0, buf, fullLen);

	EXPECT_EQ(exp_len, res);
	EXPECT_EQ('\0', buf[0]);
}

TEST(LampSettingsTest, JsonBufferOfOneByteGivesZeroAndEmptyText) {
	const size_t exp_len = 0;

	const lamp_settings settings = HAQuDA_LampDefaults();
	char buf[1];
	buf[0] = 'x';

	size_t res;

	res = HAQuDA_BuildLampJson(settings, true, 0, buf, sizeof(buf));

	EXPECT_EQ(exp_len, res);
	EXPECT_EQ('\0', buf[0]);
}

TEST(LampSettingsTest, JsonBufferOfSizeZeroGivesZeroAndIsNotTouched) {
	const size_t exp_len = 0;
	const char exp_first = 'x';

	const lamp_settings settings = HAQuDA_LampDefaults();
	char buf[LAMP_JSON_BUF_LEN];
	buf[0] = 'x';

	size_t res;

	res = HAQuDA_BuildLampJson(settings, true, 0, buf, 0);

	EXPECT_EQ(exp_len, res);
	EXPECT_EQ(exp_first, buf[0]);
}

TEST(LampSettingsTest, JsonBufferShorterThanTheFirstFieldGivesZero) {

	const size_t exp_len = 0;

	const lamp_settings settings = HAQuDA_LampDefaults();
	const size_t size = 10;
	char buf[LAMP_JSON_BUF_LEN];
	std::memset(buf, 'x', sizeof(buf));

	size_t res;

	res = HAQuDA_BuildLampJson(settings, true, 0, buf, size);

	EXPECT_EQ(exp_len, res);
	EXPECT_EQ('\0', buf[0]);
}

TEST(LampSettingsTest, JsonBufferOfSizeZeroAndNullGivesZero) {
	const size_t exp_len = 0;

	const lamp_settings settings = HAQuDA_LampDefaults();

	size_t res;

	res = HAQuDA_BuildLampJson(settings, true, 0, nullptr, 0);

	EXPECT_EQ(exp_len, res);
}

TEST(LampSettingsTest, ColorOutOfRangeIsZero) {
	const uint32_t exp_color = 0;

	uint32_t res;

	res = HAQuDA_LampColor(6);

	EXPECT_EQ(exp_color, res);
}

TEST(LampSettingsTest, ColorIndex0IsWarm) {
	const uint32_t exp_color = 0xFFCF8F;

	const uint8_t index = 0;

	uint32_t res;

	res = HAQuDA_LampColor(index);

	EXPECT_EQ(exp_color, res);
}

TEST(LampSettingsTest, ColorIndex1IsNeutral) {
	const uint32_t exp_color = 0xFFF1DC;

	const uint8_t index = 1;

	uint32_t res;

	res = HAQuDA_LampColor(index);

	EXPECT_EQ(exp_color, res);
}

TEST(LampSettingsTest, ColorIndex2IsCool) {
	const uint32_t exp_color = 0xDCE9FF;

	const uint8_t index = 2;

	uint32_t res;

	res = HAQuDA_LampColor(index);

	EXPECT_EQ(exp_color, res);
}

TEST(LampSettingsTest, ColorIndex3IsAmber) {
	const uint32_t exp_color = 0xFF9F43;

	const uint8_t index = 3;

	uint32_t res;

	res = HAQuDA_LampColor(index);

	EXPECT_EQ(exp_color, res);
}

TEST(LampSettingsTest, ColorIndex4IsLavender) {
	const uint32_t exp_color = 0x8F8CFF;

	const uint8_t index = 4;

	uint32_t res;

	res = HAQuDA_LampColor(index);

	EXPECT_EQ(exp_color, res);
}

TEST(LampSettingsTest, ColorIndex5IsTeal) {
	const uint32_t exp_color = 0x36C9B8;

	const uint8_t index = 5;

	uint32_t res;

	res = HAQuDA_LampColor(index);

	EXPECT_EQ(exp_color, res);
}

TEST(LampSettingsTest, ColorIndexTwoFiftyFiveIsZero) {
	const uint32_t exp_color = 0;

	const uint8_t index = 255;

	uint32_t res;

	res = HAQuDA_LampColor(index);

	EXPECT_EQ(exp_color, res);
}

TEST(LampSettingsTest, EncodeInvalidSettingsFails) {
	const bool exp_encoded = false;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.brightPercent = 3;
	uint8_t record[LAMP_RECORD_LEN];

	bool res;

	res = HAQuDA_EncodeLampRecord(settings, record, sizeof(record));

	EXPECT_EQ(exp_encoded, res);
}

TEST(LampSettingsTest, EncodeIntoSmallBufferFails) {
	const bool exp_encoded = false;

	const lamp_settings settings = HAQuDA_LampDefaults();
	uint8_t record[LAMP_RECORD_LEN - 1];

	bool res;

	res = HAQuDA_EncodeLampRecord(settings, record, sizeof(record));

	EXPECT_EQ(exp_encoded, res);
}

TEST(LampSettingsTest, DecodeNullFails) {
	const bool exp_decoded = false;

	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(nullptr, LAMP_RECORD_LEN, out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodeErasedFlashFails) {
	const bool exp_decoded = false;

	uint8_t record[LAMP_RECORD_LEN];
	std::memset(record, 0xFF, sizeof(record));
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodeAllZeroFails) {
	const bool exp_decoded = false;

	uint8_t record[LAMP_RECORD_LEN] = {0};
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, EncodeDefaultsMatchesTheStoredLayout) {
	// magic 'H' 'L' 'M' 'P', version 1, then the fields, then the Fletcher-16 sum; times are little endian (1260 = 0x04EC, 540 = 0x021C)

	const uint8_t exp_record[LAMP_RECORD_LEN]
		= {0x48, 0x4C, 0x4D, 0x50, 0x01, 0x01, 0x23, 0x01, 0x00, 0x05, 0x03, 0x01, 0x03, 0x00, 0xEC, 0x04, 0x1C, 0x02, 0x73, 0xD3};

	const lamp_settings settings = HAQuDA_LampDefaults();
	uint8_t record[LAMP_RECORD_LEN];
	std::memset(record, 0, sizeof(record));

	bool res;

	res = HAQuDA_EncodeLampRecord(settings, record, sizeof(record));

	EXPECT_TRUE(res);
	EXPECT_EQ(0, std::memcmp(exp_record, record, LAMP_RECORD_LEN));
}

TEST(LampSettingsTest, EncodeLargerBufferLeavesTheBytesAfterTheRecordUntouched) {
	const uint8_t exp_filler = 0xAA;
	const size_t extra = 4;

	const lamp_settings settings = HAQuDA_LampDefaults();
	uint8_t record[LAMP_RECORD_LEN + 4];
	std::memset(record, 0xAA, sizeof(record));

	bool res;

	res = HAQuDA_EncodeLampRecord(settings, record, sizeof(record));

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_filler, record[LAMP_RECORD_LEN]);
	EXPECT_EQ(exp_filler, record[LAMP_RECORD_LEN + extra - 1]);
}

TEST(LampSettingsTest, EncodeInvalidSettingsLeavesTheBufferUntouched) {
	const uint8_t exp_filler = 0xAA;

	lamp_settings settings = HAQuDA_LampDefaults();
	settings.color = LAMP_COLORS_NUM;
	uint8_t record[LAMP_RECORD_LEN];
	std::memset(record, 0xAA, sizeof(record));

	bool res;

	res = HAQuDA_EncodeLampRecord(settings, record, sizeof(record));

	EXPECT_FALSE(res);
	EXPECT_EQ(exp_filler, record[0]);
	EXPECT_EQ(exp_filler, record[LAMP_RECORD_LEN - 1]);
}

TEST(LampSettingsTest, EncodeNullBufferFails) {
	const bool exp_encoded = false;

	const lamp_settings settings = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_EncodeLampRecord(settings, nullptr, LAMP_RECORD_LEN);

	EXPECT_EQ(exp_encoded, res);
}

TEST(LampSettingsTest, RecordRoundTripKeepsEveryField) {
	const lamp_settings exp_settings = {
		false, 35, lamp_mode::light, lamp_param::tvoc, {lamp_param::humid, lamp_param::temp, lamp_param::pm25}, lamp_param::tvoc, 1 * 60 + 5, 22 * 60 + 40, 4};

	uint8_t record[LAMP_RECORD_LEN];
	const bool encoded = HAQuDA_EncodeLampRecord(exp_settings, record, sizeof(record));
	lamp_settings decoded = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), decoded);

	EXPECT_TRUE(encoded);
	EXPECT_TRUE(res);
	EXPECT_EQ(exp_settings.on, decoded.on);
	EXPECT_EQ(exp_settings.brightPercent, decoded.brightPercent);
	EXPECT_EQ(exp_settings.mode, decoded.mode);
	EXPECT_EQ(exp_settings.standardParam, decoded.standardParam);
	EXPECT_EQ(exp_settings.multi[0], decoded.multi[0]);
	EXPECT_EQ(exp_settings.multi[1], decoded.multi[1]);
	EXPECT_EQ(exp_settings.multi[2], decoded.multi[2]);
	EXPECT_EQ(exp_settings.nightParam, decoded.nightParam);
	EXPECT_EQ(exp_settings.nightFromMin, decoded.nightFromMin);
	EXPECT_EQ(exp_settings.nightToMin, decoded.nightToMin);
	EXPECT_EQ(exp_settings.color, decoded.color);
}

TEST(LampSettingsTest, RecordRoundTripKeepsTheBoundaryValues) {

	const lamp_settings exp_settings
		= {true, 100, lamp_mode::night, lamp_param::pm25, {lamp_param::temp, lamp_param::temp, lamp_param::temp}, lamp_param::temp, 23 * 60 + 59, 0, 5};

	uint8_t record[LAMP_RECORD_LEN];
	const bool encoded = HAQuDA_EncodeLampRecord(exp_settings, record, sizeof(record));
	lamp_settings decoded = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), decoded);

	EXPECT_TRUE(encoded);
	EXPECT_TRUE(res);
	EXPECT_EQ(exp_settings.on, decoded.on);
	EXPECT_EQ(exp_settings.brightPercent, decoded.brightPercent);
	EXPECT_EQ(exp_settings.mode, decoded.mode);
	EXPECT_EQ(exp_settings.standardParam, decoded.standardParam);
	EXPECT_EQ(exp_settings.multi[0], decoded.multi[0]);
	EXPECT_EQ(exp_settings.multi[1], decoded.multi[1]);
	EXPECT_EQ(exp_settings.multi[2], decoded.multi[2]);
	EXPECT_EQ(exp_settings.nightParam, decoded.nightParam);
	EXPECT_EQ(exp_settings.nightFromMin, decoded.nightFromMin);
	EXPECT_EQ(exp_settings.nightToMin, decoded.nightToMin);
	EXPECT_EQ(exp_settings.color, decoded.color);
}

TEST(LampSettingsTest, DecodeStoredLayoutGivesTheDefaults) {
	// the bytes EncodeDefaultsMatchesTheStoredLayout expects

	const lamp_settings exp_settings = HAQuDA_LampDefaults();

	const uint8_t record[LAMP_RECORD_LEN]
		= {0x48, 0x4C, 0x4D, 0x50, 0x01, 0x01, 0x23, 0x01, 0x00, 0x05, 0x03, 0x01, 0x03, 0x00, 0xEC, 0x04, 0x1C, 0x02, 0x73, 0xD3};
	lamp_settings decoded = {};

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), decoded);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_settings.on, decoded.on);
	EXPECT_EQ(exp_settings.brightPercent, decoded.brightPercent);
	EXPECT_EQ(exp_settings.mode, decoded.mode);
	EXPECT_EQ(exp_settings.standardParam, decoded.standardParam);
	EXPECT_EQ(exp_settings.multi[0], decoded.multi[0]);
	EXPECT_EQ(exp_settings.multi[1], decoded.multi[1]);
	EXPECT_EQ(exp_settings.multi[2], decoded.multi[2]);
	EXPECT_EQ(exp_settings.nightParam, decoded.nightParam);
	EXPECT_EQ(exp_settings.nightFromMin, decoded.nightFromMin);
	EXPECT_EQ(exp_settings.nightToMin, decoded.nightToMin);
	EXPECT_EQ(exp_settings.color, decoded.color);
}

TEST(LampSettingsTest, DecodeWrongMagicFails) {
	// first magic byte 'I' instead of 'H', checksum recomputed

	const bool exp_decoded = false;

	const uint8_t record[LAMP_RECORD_LEN]
		= {0x49, 0x4C, 0x4D, 0x50, 0x01, 0x01, 0x32, 0x01, 0x00, 0x05, 0x03, 0x01, 0x03, 0x00, 0xEC, 0x04, 0x1C, 0x02, 0x83, 0x9A};
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodeVersionZeroFails) {
	// version 0, checksum recomputed

	const bool exp_decoded = false;

	const uint8_t record[LAMP_RECORD_LEN]
		= {0x48, 0x4C, 0x4D, 0x50, 0x00, 0x01, 0x32, 0x01, 0x00, 0x05, 0x03, 0x01, 0x03, 0x00, 0xEC, 0x04, 0x1C, 0x02, 0x81, 0x7A};
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodeNextVersionFails) {
	// version 2, checksum recomputed

	const bool exp_decoded = false;

	const uint8_t record[LAMP_RECORD_LEN]
		= {0x48, 0x4C, 0x4D, 0x50, 0x02, 0x01, 0x32, 0x01, 0x00, 0x05, 0x03, 0x01, 0x03, 0x00, 0xEC, 0x04, 0x1C, 0x02, 0x83, 0x96};
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodeWrongChecksumLowByteFails) {
	// low checksum byte flipped

	const bool exp_decoded = false;

	const uint8_t record[LAMP_RECORD_LEN]
		= {0x48, 0x4C, 0x4D, 0x50, 0x01, 0x01, 0x32, 0x01, 0x00, 0x05, 0x03, 0x01, 0x03, 0x00, 0xEC, 0x04, 0x1C, 0x02, 0x83, 0x88};
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodeWrongChecksumHighByteFails) {
	// high checksum byte flipped

	const bool exp_decoded = false;

	const uint8_t record[LAMP_RECORD_LEN]
		= {0x48, 0x4C, 0x4D, 0x50, 0x01, 0x01, 0x32, 0x01, 0x00, 0x05, 0x03, 0x01, 0x03, 0x00, 0xEC, 0x04, 0x1C, 0x02, 0x82, 0x89};
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodeFieldChangedWithoutNewChecksumFails) {
	// colour changed to a valid 2 but the old checksum kept

	const bool exp_decoded = false;

	const uint8_t record[LAMP_RECORD_LEN]
		= {0x48, 0x4C, 0x4D, 0x50, 0x01, 0x01, 0x32, 0x01, 0x00, 0x05, 0x03, 0x01, 0x03, 0x02, 0xEC, 0x04, 0x1C, 0x02, 0x82, 0x88};
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodePowerByteTwoFails) {
	// power byte 2, checksum recomputed

	const bool exp_decoded = false;

	const uint8_t record[LAMP_RECORD_LEN]
		= {0x48, 0x4C, 0x4D, 0x50, 0x01, 0x02, 0x32, 0x01, 0x00, 0x05, 0x03, 0x01, 0x03, 0x00, 0xEC, 0x04, 0x1C, 0x02, 0x83, 0x95};
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodeBrightnessOffTheStepFails) {
	// brightness 7, checksum recomputed

	const bool exp_decoded = false;

	const uint8_t record[LAMP_RECORD_LEN]
		= {0x48, 0x4C, 0x4D, 0x50, 0x01, 0x01, 0x07, 0x01, 0x00, 0x05, 0x03, 0x01, 0x03, 0x00, 0xEC, 0x04, 0x1C, 0x02, 0x57, 0x82};
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodeBrightnessAboveMaximumFails) {
	// brightness 105, checksum recomputed

	const bool exp_decoded = false;

	const uint8_t record[LAMP_RECORD_LEN]
		= {0x48, 0x4C, 0x4D, 0x50, 0x01, 0x01, 0x69, 0x01, 0x00, 0x05, 0x03, 0x01, 0x03, 0x00, 0xEC, 0x04, 0x1C, 0x02, 0xB9, 0x1F};
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodeBrightnessZeroFails) {
	// brightness 0, checksum recomputed

	const bool exp_decoded = false;

	const uint8_t record[LAMP_RECORD_LEN]
		= {0x48, 0x4C, 0x4D, 0x50, 0x01, 0x01, 0x00, 0x01, 0x00, 0x05, 0x03, 0x01, 0x03, 0x00, 0xEC, 0x04, 0x1C, 0x02, 0x50, 0x2E};
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodeModeOutOfRangeFails) {
	// mode 4, checksum recomputed

	const bool exp_decoded = false;

	const uint8_t record[LAMP_RECORD_LEN]
		= {0x48, 0x4C, 0x4D, 0x50, 0x01, 0x01, 0x32, 0x04, 0x00, 0x05, 0x03, 0x01, 0x03, 0x00, 0xEC, 0x04, 0x1C, 0x02, 0x85, 0xA9};
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodeStandardParamOutOfRangeFails) {
	// standard reading 6, checksum recomputed

	const bool exp_decoded = false;

	const uint8_t record[LAMP_RECORD_LEN]
		= {0x48, 0x4C, 0x4D, 0x50, 0x01, 0x01, 0x32, 0x01, 0x06, 0x05, 0x03, 0x01, 0x03, 0x00, 0xEC, 0x04, 0x1C, 0x02, 0x88, 0xC4};
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodeMultiTotalFails) {
	// total in the top multi row, checksum recomputed

	const bool exp_decoded = false;

	const uint8_t record[LAMP_RECORD_LEN]
		= {0x48, 0x4C, 0x4D, 0x50, 0x01, 0x01, 0x32, 0x01, 0x00, 0x00, 0x03, 0x01, 0x03, 0x00, 0xEC, 0x04, 0x1C, 0x02, 0x7D, 0x5B};
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodeMultiOutOfRangeFails) {
	// multi reading 6, checksum recomputed

	const bool exp_decoded = false;

	const uint8_t record[LAMP_RECORD_LEN]
		= {0x48, 0x4C, 0x4D, 0x50, 0x01, 0x01, 0x32, 0x01, 0x00, 0x05, 0x06, 0x01, 0x03, 0x00, 0xEC, 0x04, 0x1C, 0x02, 0x85, 0xA0};
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodeNightParamTotalFails) {
	// total as the night reading, checksum recomputed

	const bool exp_decoded = false;

	const uint8_t record[LAMP_RECORD_LEN]
		= {0x48, 0x4C, 0x4D, 0x50, 0x01, 0x01, 0x32, 0x01, 0x00, 0x05, 0x03, 0x01, 0x00, 0x00, 0xEC, 0x04, 0x1C, 0x02, 0x7F, 0x76};
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodeNightParamOutOfRangeFails) {
	// night reading 6, checksum recomputed

	const bool exp_decoded = false;

	const uint8_t record[LAMP_RECORD_LEN]
		= {0x48, 0x4C, 0x4D, 0x50, 0x01, 0x01, 0x32, 0x01, 0x00, 0x05, 0x03, 0x01, 0x06, 0x00, 0xEC, 0x04, 0x1C, 0x02, 0x85, 0x9A};
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodeColorOutOfRangeFails) {
	// colour index 6, checksum recomputed

	const bool exp_decoded = false;

	const uint8_t record[LAMP_RECORD_LEN]
		= {0x48, 0x4C, 0x4D, 0x50, 0x01, 0x01, 0x32, 0x01, 0x00, 0x05, 0x03, 0x01, 0x03, 0x06, 0xEC, 0x04, 0x1C, 0x02, 0x88, 0xA6};
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodeNightFromAboveADayFails) {
	// night start 1440 (0x05A0), checksum recomputed

	const bool exp_decoded = false;

	const uint8_t record[LAMP_RECORD_LEN]
		= {0x48, 0x4C, 0x4D, 0x50, 0x01, 0x01, 0x32, 0x01, 0x00, 0x05, 0x03, 0x01, 0x03, 0x00, 0xA0, 0x05, 0x1C, 0x02, 0x37, 0x5A};
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodeNightToAboveADayFails) {
	// night end 1440 (0x05A0), checksum recomputed

	const bool exp_decoded = false;

	const uint8_t record[LAMP_RECORD_LEN]
		= {0x48, 0x4C, 0x4D, 0x50, 0x01, 0x01, 0x32, 0x01, 0x00, 0x05, 0x03, 0x01, 0x03, 0x00, 0xEC, 0x04, 0xA0, 0x05, 0x0A, 0x94};
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodeEqualNightTimesFails) {
	// night end equal to the start (1260), checksum recomputed

	const bool exp_decoded = false;

	const uint8_t record[LAMP_RECORD_LEN]
		= {0x48, 0x4C, 0x4D, 0x50, 0x01, 0x01, 0x32, 0x01, 0x00, 0x05, 0x03, 0x01, 0x03, 0x00, 0xEC, 0x04, 0xEC, 0x04, 0x55, 0x2C};
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodeOneByteShortFails) {
	// the right bytes but one fewer reported

	const bool exp_decoded = false;

	const uint8_t record[LAMP_RECORD_LEN]
		= {0x48, 0x4C, 0x4D, 0x50, 0x01, 0x01, 0x32, 0x01, 0x00, 0x05, 0x03, 0x01, 0x03, 0x00, 0xEC, 0x04, 0x1C, 0x02, 0x82, 0x88};
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, LAMP_RECORD_LEN - 1, out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodeOneByteLongFails) {
	// the right record followed by one more byte

	const bool exp_decoded = false;

	const uint8_t record[LAMP_RECORD_LEN + 1]
		= {0x48, 0x4C, 0x4D, 0x50, 0x01, 0x01, 0x32, 0x01, 0x00, 0x05, 0x03, 0x01, 0x03, 0x00, 0xEC, 0x04, 0x1C, 0x02, 0x82, 0x88, 0x00};
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodeZeroLengthFails) {
	const bool exp_decoded = false;

	const uint8_t record[LAMP_RECORD_LEN]
		= {0x48, 0x4C, 0x4D, 0x50, 0x01, 0x01, 0x32, 0x01, 0x00, 0x05, 0x03, 0x01, 0x03, 0x00, 0xEC, 0x04, 0x1C, 0x02, 0x82, 0x88};
	lamp_settings out = HAQuDA_LampDefaults();

	bool res;

	res = HAQuDA_DecodeLampRecord(record, 0, out);

	EXPECT_EQ(exp_decoded, res);
}

TEST(LampSettingsTest, DecodeFailureLeavesOutUntouched) {
	// wrong magic; `out` holds non-default values to see that none is overwritten

	const lamp_settings exp_settings = {
		false, 85, lamp_mode::light, lamp_param::tvoc, {lamp_param::humid, lamp_param::temp, lamp_param::pm25}, lamp_param::tvoc, 1 * 60 + 5, 22 * 60 + 40, 3};

	const uint8_t record[LAMP_RECORD_LEN]
		= {0x49, 0x4C, 0x4D, 0x50, 0x01, 0x01, 0x32, 0x01, 0x00, 0x05, 0x03, 0x01, 0x03, 0x00, 0xEC, 0x04, 0x1C, 0x02, 0x83, 0x9A};
	lamp_settings decoded = {
		false, 85, lamp_mode::light, lamp_param::tvoc, {lamp_param::humid, lamp_param::temp, lamp_param::pm25}, lamp_param::tvoc, 1 * 60 + 5, 22 * 60 + 40, 3};

	bool res;

	res = HAQuDA_DecodeLampRecord(record, sizeof(record), decoded);

	EXPECT_FALSE(res);
	EXPECT_EQ(exp_settings.on, decoded.on);
	EXPECT_EQ(exp_settings.brightPercent, decoded.brightPercent);
	EXPECT_EQ(exp_settings.mode, decoded.mode);
	EXPECT_EQ(exp_settings.standardParam, decoded.standardParam);
	EXPECT_EQ(exp_settings.multi[0], decoded.multi[0]);
	EXPECT_EQ(exp_settings.multi[1], decoded.multi[1]);
	EXPECT_EQ(exp_settings.multi[2], decoded.multi[2]);
	EXPECT_EQ(exp_settings.nightParam, decoded.nightParam);
	EXPECT_EQ(exp_settings.nightFromMin, decoded.nightFromMin);
	EXPECT_EQ(exp_settings.nightToMin, decoded.nightToMin);
	EXPECT_EQ(exp_settings.color, decoded.color);
}
