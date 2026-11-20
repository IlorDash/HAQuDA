#include <gtest/gtest.h>

#include <cstring>
#include <string>

#include "HAQuDA_LampSettings.h"

TEST(LampFormTest, SetFieldStoresOnField) {
	const char *const name = "on";
	const char *const value = "1";

	lamp_request request = {};

	bool res;

	res = HAQuDA_SetLampField(request, name, value);

	EXPECT_TRUE(res);
	EXPECT_EQ(value, request.on);
}

TEST(LampFormTest, SetFieldStoresBrightField) {
	const char *const name = "bright";
	const char *const value = "60";

	lamp_request request = {};

	bool res;

	res = HAQuDA_SetLampField(request, name, value);

	EXPECT_TRUE(res);
	EXPECT_EQ(value, request.bright);
}

TEST(LampFormTest, SetFieldStoresModeField) {
	const char *const name = "mode";
	const char *const value = "night";

	lamp_request request = {};

	bool res;

	res = HAQuDA_SetLampField(request, name, value);

	EXPECT_TRUE(res);
	EXPECT_EQ(value, request.mode);
}

TEST(LampFormTest, SetFieldStoresStdField) {
	const char *const name = "std";
	const char *const value = "temp";

	lamp_request request = {};

	bool res;

	res = HAQuDA_SetLampField(request, name, value);

	EXPECT_TRUE(res);
	EXPECT_EQ(value, request.standardParam);
}

TEST(LampFormTest, SetFieldStoresMultiField) {
	const char *const name = "multi";
	const char *const value = "pm25,eco2,temp";

	lamp_request request = {};

	bool res;

	res = HAQuDA_SetLampField(request, name, value);

	EXPECT_TRUE(res);
	EXPECT_EQ(value, request.multi);
}

TEST(LampFormTest, SetFieldStoresNightParamField) {
	const char *const name = "night_param";
	const char *const value = "eco2";

	lamp_request request = {};

	bool res;

	res = HAQuDA_SetLampField(request, name, value);

	EXPECT_TRUE(res);
	EXPECT_EQ(value, request.nightParam);
}

TEST(LampFormTest, SetFieldStoresNightFromField) {
	const char *const name = "night_from";
	const char *const value = "21:30";

	lamp_request request = {};

	bool res;

	res = HAQuDA_SetLampField(request, name, value);

	EXPECT_TRUE(res);
	EXPECT_EQ(value, request.nightFrom);
}

TEST(LampFormTest, SetFieldStoresNightToField) {
	const char *const name = "night_to";
	const char *const value = "09:00";

	lamp_request request = {};

	bool res;

	res = HAQuDA_SetLampField(request, name, value);

	EXPECT_TRUE(res);
	EXPECT_EQ(value, request.nightTo);
}

TEST(LampFormTest, SetFieldStoresColorField) {
	const char *const name = "color";
	const char *const value = "3";

	lamp_request request = {};

	bool res;

	res = HAQuDA_SetLampField(request, name, value);

	EXPECT_TRUE(res);
	EXPECT_EQ(value, request.color);
}

TEST(LampFormTest, SetFieldLeavesTheOtherFieldsEmpty) {
	const char *const name = "bright";
	const char *const value = "60";

	lamp_request request = {};

	bool res;

	res = HAQuDA_SetLampField(request, name, value);

	EXPECT_TRUE(res);
	EXPECT_EQ(nullptr, request.on);
	EXPECT_EQ(nullptr, request.mode);
	EXPECT_EQ(nullptr, request.standardParam);
	EXPECT_EQ(nullptr, request.multi);
	EXPECT_EQ(nullptr, request.nightParam);
	EXPECT_EQ(nullptr, request.nightFrom);
	EXPECT_EQ(nullptr, request.nightTo);
	EXPECT_EQ(nullptr, request.color);
}

TEST(LampFormTest, SetFieldUnknownKeyIsRefusedAndStoresNothing) {
	const lamp_request exp_request = {};
	const char *const name = "volume";
	const char *const value = "9";

	lamp_request request = {};

	bool res;

	res = HAQuDA_SetLampField(request, name, value);

	EXPECT_FALSE(res);
	EXPECT_EQ(0, memcmp(&exp_request, &request, sizeof(request)));
}

TEST(LampFormTest, SetFieldKeyIsCaseSensitive) {
	const lamp_request exp_request = {};
	const char *const name = "Bright";
	const char *const value = "60";

	lamp_request request = {};

	bool res;

	res = HAQuDA_SetLampField(request, name, value);

	EXPECT_FALSE(res);
	EXPECT_EQ(0, memcmp(&exp_request, &request, sizeof(request)));
}

TEST(LampFormTest, SetFieldKeyPrefixOfAFieldIsRefused) {
	const lamp_request exp_request = {};
	const char *const name = "bri";
	const char *const value = "60";

	lamp_request request = {};

	bool res;

	res = HAQuDA_SetLampField(request, name, value);

	EXPECT_FALSE(res);
	EXPECT_EQ(0, memcmp(&exp_request, &request, sizeof(request)));
}

TEST(LampFormTest, SetFieldKeyLongerThanAFieldIsRefused) {
	const lamp_request exp_request = {};
	const char *const name = "brightness";
	const char *const value = "60";

	lamp_request request = {};

	bool res;

	res = HAQuDA_SetLampField(request, name, value);

	EXPECT_FALSE(res);
	EXPECT_EQ(0, memcmp(&exp_request, &request, sizeof(request)));
}

TEST(LampFormTest, SetFieldEmptyKeyIsRefused) {
	const lamp_request exp_request = {};
	const char *const name = "";
	const char *const value = "60";

	lamp_request request = {};

	bool res;

	res = HAQuDA_SetLampField(request, name, value);

	EXPECT_FALSE(res);
	EXPECT_EQ(0, memcmp(&exp_request, &request, sizeof(request)));
}

TEST(LampFormTest, SetFieldRepeatedFieldIsRefused) {
	const char *const name = "bright";
	const char *const first_value = "60";
	const char *const second_value = "80";

	lamp_request request = {};
	HAQuDA_SetLampField(request, name, first_value);

	bool res;

	res = HAQuDA_SetLampField(request, name, second_value);

	EXPECT_FALSE(res);
}

TEST(LampFormTest, SetFieldRepeatedFieldKeepsTheFirstValue) {
	const char *const name = "bright";
	const char *const first_value = "60";
	const char *const second_value = "80";

	lamp_request request = {};
	HAQuDA_SetLampField(request, name, first_value);

	HAQuDA_SetLampField(request, name, second_value);

	EXPECT_EQ(first_value, request.bright);
}

TEST(LampFormTest, SetFieldRepeatedEmptyFirstValueStillWins) {
	const char *const name = "mode";
	const char *const first_value = "";
	const char *const second_value = "night";

	lamp_request request = {};
	HAQuDA_SetLampField(request, name, first_value);

	bool res;

	res = HAQuDA_SetLampField(request, name, second_value);

	EXPECT_FALSE(res);
	EXPECT_EQ(first_value, request.mode);
}

TEST(LampFormTest, SetFieldDifferentFieldsAreBothStored) {
	const char *const first_name = "on";
	const char *const second_name = "color";
	const char *const first_value = "0";
	const char *const second_value = "4";

	lamp_request request = {};
	HAQuDA_SetLampField(request, first_name, first_value);

	bool res;

	res = HAQuDA_SetLampField(request, second_name, second_value);

	EXPECT_TRUE(res);
	EXPECT_EQ(first_value, request.on);
	EXPECT_EQ(second_value, request.color);
}

TEST(LampFormTest, SetFieldEmptyValueIsStored) {
	const char *const name = "bright";
	const char *const value = "";

	lamp_request request = {};

	bool res;

	res = HAQuDA_SetLampField(request, name, value);

	EXPECT_TRUE(res);
	EXPECT_EQ(value, request.bright);
}

TEST(LampFormTest, SetFieldOverLongValueIsStored) {
	const size_t too_long = 33;
	const std::string value(too_long, '5');
	const char *const name = "bright";

	lamp_request request = {};

	bool res;

	res = HAQuDA_SetLampField(request, name, value.c_str());

	EXPECT_TRUE(res);
	EXPECT_EQ(value.c_str(), request.bright);
}

TEST(LampFormTest, SetFieldNullNameIsRefusedAndStoresNothing) {
	const lamp_request exp_request = {};
	const char *const value = "60";

	lamp_request request = {};

	bool res;

	res = HAQuDA_SetLampField(request, nullptr, value);

	EXPECT_FALSE(res);
	EXPECT_EQ(0, memcmp(&exp_request, &request, sizeof(request)));
}

TEST(LampFormTest, SetFieldNullValueIsRefusedAndStoresNothing) {
	const lamp_request exp_request = {};
	const char *const name = "bright";

	lamp_request request = {};

	bool res;

	res = HAQuDA_SetLampField(request, name, nullptr);

	EXPECT_FALSE(res);
	EXPECT_EQ(0, memcmp(&exp_request, &request, sizeof(request)));
}

TEST(LampFormTest, SetFieldNullValueDoesNotUseUpTheField) {
	const char *const name = "bright";
	const char *const value = "60";

	lamp_request request = {};
	HAQuDA_SetLampField(request, name, nullptr);

	bool res;

	res = HAQuDA_SetLampField(request, name, value);

	EXPECT_TRUE(res);
	EXPECT_EQ(value, request.bright);
}

TEST(LampFormTest, SetFieldOverLongValueIsRefusedByLampUpdate) {
	const lamp_update_result exp_result = lamp_update_result::bad_bright;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();
	const size_t too_long = 33;
	const std::string value(too_long, '5');
	const char *const name = "bright";

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	HAQuDA_SetLampField(request, name, value.c_str());

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_settings.brightPercent, settings.brightPercent);
}

TEST(LampFormTest, SetFieldEmptyValueIsRefusedByLampUpdate) {
	const lamp_update_result exp_result = lamp_update_result::bad_bright;
	const char *const name = "bright";
	const char *const value = "";

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	HAQuDA_SetLampField(request, name, value);

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
}

TEST(LampFormTest, SetFieldFilledRequestIsAcceptedByLampUpdate) {
	const lamp_update_result exp_result = lamp_update_result::ok;
	const uint8_t exp_bright = 60;
	const char *const bright_value = "60";
	const char *const mode_value = "standard";
	const char *const std_value = "temp";

	lamp_settings settings = HAQuDA_LampDefaults();
	lamp_request request = {};
	HAQuDA_SetLampField(request, "bright", bright_value);
	HAQuDA_SetLampField(request, "mode", mode_value);
	HAQuDA_SetLampField(request, "std", std_value);

	lamp_update_result res;

	res = HAQuDA_LampUpdate(settings, request, true);

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_bright, settings.brightPercent);
	EXPECT_EQ(lamp_mode::standard, settings.mode);
	EXPECT_EQ(lamp_param::temp, settings.standardParam);
}
