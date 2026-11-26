#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>

#include "HAQuDA_ButtonsSettings.h"
#include "HAQuDA_LampSettings.h"
#include "HAQuDA_RecordCodec.h"

TEST(ButtonsSettingsTest, DefaultsArePowerUpNextDownUnlocked) {
	const button_action exp_first = button_action::power;
	const button_action exp_second = button_action::up;
	const button_action exp_third = button_action::next;
	const button_action exp_fourth = button_action::down;

	buttons_settings res;

	res = HAQuDA_ButtonsDefaults();

	EXPECT_EQ(exp_first, res.buttons[0].action);
	EXPECT_EQ(exp_second, res.buttons[1].action);
	EXPECT_EQ(exp_third, res.buttons[2].action);
	EXPECT_EQ(exp_fourth, res.buttons[3].action);
	EXPECT_FALSE(res.locked);
}

TEST(ButtonsSettingsTest, DefaultsHaveStandardModeAndTotalParam) {
	const lamp_mode exp_mode = lamp_mode::standard;
	const lamp_param exp_param = lamp_param::total;

	buttons_settings res;

	res = HAQuDA_ButtonsDefaults();

	EXPECT_EQ(exp_mode, res.buttons[0].mode);
	EXPECT_EQ(exp_param, res.buttons[0].param);
	EXPECT_EQ(exp_mode, res.buttons[3].mode);
	EXPECT_EQ(exp_param, res.buttons[3].param);
}

TEST(ButtonsSettingsTest, DefaultsAreValid) {
	const bool exp_valid = true;

	const buttons_settings s = HAQuDA_ButtonsDefaults();

	bool res;

	res = HAQuDA_ButtonsSettingsValid(s);

	EXPECT_EQ(exp_valid, res);
}

TEST(ButtonsSettingsTest, ValidRejectsActionOutOfRange) {
	const bool exp_valid = false;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	const uint8_t outOfRangeAction = 7;
	s.buttons[1].action = static_cast<button_action>(outOfRangeAction);

	bool res;

	res = HAQuDA_ButtonsSettingsValid(s);

	EXPECT_EQ(exp_valid, res);
}

TEST(ButtonsSettingsTest, ValidRejectsModeOutOfRange) {
	const bool exp_valid = false;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	const uint8_t outOfRangeMode = 4;
	s.buttons[3].action = button_action::mode;
	s.buttons[3].mode = static_cast<lamp_mode>(outOfRangeMode);

	bool res;

	res = HAQuDA_ButtonsSettingsValid(s);

	EXPECT_EQ(exp_valid, res);
}

TEST(ButtonsSettingsTest, ValidRejectsParamOutOfRange) {
	const bool exp_valid = false;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	const uint8_t outOfRangeParam = 6;
	s.buttons[3].action = button_action::show;
	s.buttons[3].param = static_cast<lamp_param>(outOfRangeParam);

	bool res;

	res = HAQuDA_ButtonsSettingsValid(s);

	EXPECT_EQ(exp_valid, res);
}

TEST(ButtonsSettingsTest, ValidRejectsModeArgumentOnActionWithoutOne) {
	const bool exp_valid = false;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0].mode = lamp_mode::night;

	bool res;

	res = HAQuDA_ButtonsSettingsValid(s);

	EXPECT_EQ(exp_valid, res);
}

TEST(ButtonsSettingsTest, ValidRejectsParamArgumentOnActionWithoutOne) {
	const bool exp_valid = false;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0].param = lamp_param::temp;

	bool res;

	res = HAQuDA_ButtonsSettingsValid(s);

	EXPECT_EQ(exp_valid, res);
}

TEST(ButtonsSettingsTest, ValidRejectsParamOnModeAction) {
	const bool exp_valid = false;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0] = {button_action::mode, lamp_mode::light, lamp_param::temp};

	bool res;

	res = HAQuDA_ButtonsSettingsValid(s);

	EXPECT_EQ(exp_valid, res);
}

TEST(ButtonsSettingsTest, ValidRejectsModeOnShowAction) {
	const bool exp_valid = false;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0] = {button_action::show, lamp_mode::night, lamp_param::pm25};

	bool res;

	res = HAQuDA_ButtonsSettingsValid(s);

	EXPECT_EQ(exp_valid, res);
}

TEST(ButtonsSettingsTest, ValidAcceptsModeWithArgument) {
	const bool exp_valid = true;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0] = {button_action::mode, lamp_mode::light, lamp_param::total};

	bool res;

	res = HAQuDA_ButtonsSettingsValid(s);

	EXPECT_EQ(exp_valid, res);
}

TEST(ButtonsSettingsTest, ValidAcceptsShowWithArgument) {
	const bool exp_valid = true;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0] = {button_action::show, lamp_mode::standard, lamp_param::pm25};

	bool res;

	res = HAQuDA_ButtonsSettingsValid(s);

	EXPECT_EQ(exp_valid, res);
}

TEST(ButtonsSettingsTest, RepeatableNoneIsFalse) {
	const bool exp_repeatable = false;

	const button_binding binding = {button_action::none, lamp_mode::standard, lamp_param::total};

	bool res;

	res = HAQuDA_ButtonRepeatable(binding);

	EXPECT_EQ(exp_repeatable, res);
}

TEST(ButtonsSettingsTest, RepeatablePowerIsFalse) {
	const bool exp_repeatable = false;

	const button_binding binding = {button_action::power, lamp_mode::standard, lamp_param::total};

	bool res;

	res = HAQuDA_ButtonRepeatable(binding);

	EXPECT_EQ(exp_repeatable, res);
}

TEST(ButtonsSettingsTest, RepeatableUpIsTrue) {
	const bool exp_repeatable = true;

	const button_binding binding = {button_action::up, lamp_mode::standard, lamp_param::total};

	bool res;

	res = HAQuDA_ButtonRepeatable(binding);

	EXPECT_EQ(exp_repeatable, res);
}

TEST(ButtonsSettingsTest, RepeatableDownIsTrue) {
	const bool exp_repeatable = true;

	const button_binding binding = {button_action::down, lamp_mode::standard, lamp_param::total};

	bool res;

	res = HAQuDA_ButtonRepeatable(binding);

	EXPECT_EQ(exp_repeatable, res);
}

TEST(ButtonsSettingsTest, RepeatableNextIsFalse) {
	const bool exp_repeatable = false;

	const button_binding binding = {button_action::next, lamp_mode::standard, lamp_param::total};

	bool res;

	res = HAQuDA_ButtonRepeatable(binding);

	EXPECT_EQ(exp_repeatable, res);
}

TEST(ButtonsSettingsTest, RepeatableModeIsFalse) {
	const bool exp_repeatable = false;

	const button_binding binding = {button_action::mode, lamp_mode::standard, lamp_param::total};

	bool res;

	res = HAQuDA_ButtonRepeatable(binding);

	EXPECT_EQ(exp_repeatable, res);
}

TEST(ButtonsSettingsTest, RepeatableShowIsFalse) {
	const bool exp_repeatable = false;

	const button_binding binding = {button_action::show, lamp_mode::standard, lamp_param::total};

	bool res;

	res = HAQuDA_ButtonRepeatable(binding);

	EXPECT_EQ(exp_repeatable, res);
}

TEST(ButtonsSettingsTest, SetFieldA2StoresTheValuePointer) {
	const char *const value = "x";

	buttons_request request = {};

	bool res;

	res = HAQuDA_SetButtonsField(request, "a2", value);

	EXPECT_TRUE(res);
	EXPECT_EQ(value, request.action[0]);
}

TEST(ButtonsSettingsTest, SetFieldA6StoresTheValuePointer) {
	const char *const value = "x";

	buttons_request request = {};

	bool res;

	res = HAQuDA_SetButtonsField(request, "a6", value);

	EXPECT_TRUE(res);
	EXPECT_EQ(value, request.action[1]);
}

TEST(ButtonsSettingsTest, SetFieldA8StoresTheValuePointer) {
	const char *const value = "x";

	buttons_request request = {};

	bool res;

	res = HAQuDA_SetButtonsField(request, "a8", value);

	EXPECT_TRUE(res);
	EXPECT_EQ(value, request.action[2]);
}

TEST(ButtonsSettingsTest, SetFieldA9StoresTheValuePointer) {
	const char *const value = "x";

	buttons_request request = {};

	bool res;

	res = HAQuDA_SetButtonsField(request, "a9", value);

	EXPECT_TRUE(res);
	EXPECT_EQ(value, request.action[3]);
}

TEST(ButtonsSettingsTest, SetFieldArg2StoresTheValuePointer) {
	const char *const value = "x";

	buttons_request request = {};

	bool res;

	res = HAQuDA_SetButtonsField(request, "arg2", value);

	EXPECT_TRUE(res);
	EXPECT_EQ(value, request.arg[0]);
}

TEST(ButtonsSettingsTest, SetFieldArg6StoresTheValuePointer) {
	const char *const value = "x";

	buttons_request request = {};

	bool res;

	res = HAQuDA_SetButtonsField(request, "arg6", value);

	EXPECT_TRUE(res);
	EXPECT_EQ(value, request.arg[1]);
}

TEST(ButtonsSettingsTest, SetFieldArg8StoresTheValuePointer) {
	const char *const value = "x";

	buttons_request request = {};

	bool res;

	res = HAQuDA_SetButtonsField(request, "arg8", value);

	EXPECT_TRUE(res);
	EXPECT_EQ(value, request.arg[2]);
}

TEST(ButtonsSettingsTest, SetFieldArg9StoresTheValuePointer) {
	const char *const value = "x";

	buttons_request request = {};

	bool res;

	res = HAQuDA_SetButtonsField(request, "arg9", value);

	EXPECT_TRUE(res);
	EXPECT_EQ(value, request.arg[3]);
}

TEST(ButtonsSettingsTest, SetFieldLockedStoresTheValuePointer) {
	const char *const value = "x";

	buttons_request request = {};

	bool res;

	res = HAQuDA_SetButtonsField(request, "locked", value);

	EXPECT_TRUE(res);
	EXPECT_EQ(value, request.locked);
}

TEST(ButtonsSettingsTest, SetFieldResetStoresTheValuePointer) {
	const char *const value = "x";

	buttons_request request = {};

	bool res;

	res = HAQuDA_SetButtonsField(request, "reset", value);

	EXPECT_TRUE(res);
	EXPECT_EQ(value, request.reset);
}

TEST(ButtonsSettingsTest, SetFieldKeepsFirstValueOnDuplicate) {
	const char *const first = "up";
	const char *const second = "down";

	buttons_request request = {};
	HAQuDA_SetButtonsField(request, "a6", first);

	bool res;

	res = HAQuDA_SetButtonsField(request, "a6", second);

	EXPECT_FALSE(res);
	EXPECT_EQ(first, request.action[1]);
}

TEST(ButtonsSettingsTest, SetFieldRefusesUnknownNameA3) {
	buttons_request request = {};

	bool res;

	res = HAQuDA_SetButtonsField(request, "a3", "up");

	EXPECT_FALSE(res);
}

TEST(ButtonsSettingsTest, SetFieldRefusesUnknownNameWrongCase) {
	buttons_request request = {};

	bool res;

	res = HAQuDA_SetButtonsField(request, "A2", "up");

	EXPECT_FALSE(res);
}

TEST(ButtonsSettingsTest, SetFieldRefusesUnknownNameTrailingSpace) {
	buttons_request request = {};

	bool res;

	res = HAQuDA_SetButtonsField(request, "a2 ", "up");

	EXPECT_FALSE(res);
}

TEST(ButtonsSettingsTest, SetFieldRefusesUnknownNameEmpty) {
	buttons_request request = {};

	bool res;

	res = HAQuDA_SetButtonsField(request, "", "up");

	EXPECT_FALSE(res);
}

TEST(ButtonsSettingsTest, SetFieldRefusesUnknownNameArg1) {
	buttons_request request = {};

	bool res;

	res = HAQuDA_SetButtonsField(request, "arg1", "up");

	EXPECT_FALSE(res);
}

TEST(ButtonsSettingsTest, SetFieldRefusesUnknownNameArg3) {
	buttons_request request = {};

	bool res;

	res = HAQuDA_SetButtonsField(request, "arg3", "up");

	EXPECT_FALSE(res);
}

TEST(ButtonsSettingsTest, SetFieldRefusesNullName) {
	buttons_request request = {};

	bool res;

	res = HAQuDA_SetButtonsField(request, nullptr, "up");

	EXPECT_FALSE(res);
}

TEST(ButtonsSettingsTest, SetFieldRefusesNullValueAndStoresNothing) {
	buttons_request request = {};

	bool res;

	res = HAQuDA_SetButtonsField(request, "a2", nullptr);

	EXPECT_FALSE(res);
	EXPECT_EQ(nullptr, request.action[0]);
}

TEST(ButtonsSettingsTest, SetFieldStoresEmptyValue) {
	const char *const value = "";

	buttons_request request = {};

	bool res;

	res = HAQuDA_SetButtonsField(request, "a2", value);

	EXPECT_TRUE(res);
	EXPECT_EQ(value, request.action[0]);
}

TEST(ButtonsSettingsTest, UpdateEmptyRequestIsNothing) {
	const buttons_update_result exp_res = buttons_update_result::nothing;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateChangesOneActionAndKeepsTheRest) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[3] = "none";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::none, s.buttons[3].action);
	EXPECT_EQ(button_action::power, s.buttons[0].action);
	EXPECT_EQ(button_action::up, s.buttons[1].action);
	EXPECT_EQ(button_action::next, s.buttons[2].action);
}

TEST(ButtonsSettingsTest, UpdateChangesSeveralButtonsInOneRequest) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "next";
	request.action[1] = "mode";
	request.arg[1] = "light";
	request.action[2] = "down";
	request.action[3] = "show";
	request.arg[3] = "tvoc";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::next, s.buttons[0].action);
	EXPECT_EQ(button_action::mode, s.buttons[1].action);
	EXPECT_EQ(lamp_mode::light, s.buttons[1].mode);
	EXPECT_EQ(button_action::down, s.buttons[2].action);
	EXPECT_EQ(button_action::show, s.buttons[3].action);
	EXPECT_EQ(lamp_param::tvoc, s.buttons[3].param);
}

TEST(ButtonsSettingsTest, UpdateAcceptsNoneKeyWithoutArgument) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "none";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::none, s.buttons[0].action);
}

TEST(ButtonsSettingsTest, UpdateAcceptsPowerKeyWithoutArgument) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0].action = button_action::none;
	buttons_request request = {};
	request.action[0] = "power";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::power, s.buttons[0].action);
}

TEST(ButtonsSettingsTest, UpdateAcceptsUpKeyWithoutArgument) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "up";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::up, s.buttons[0].action);
}

TEST(ButtonsSettingsTest, UpdateAcceptsDownKeyWithoutArgument) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "down";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::down, s.buttons[0].action);
}

TEST(ButtonsSettingsTest, UpdateAcceptsNextKeyWithoutArgument) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "next";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::next, s.buttons[0].action);
}

TEST(ButtonsSettingsTest, UpdateModeActionStandardArgument) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "mode";
	request.arg[0] = "standard";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::mode, s.buttons[0].action);
	EXPECT_EQ(lamp_mode::standard, s.buttons[0].mode);
}

TEST(ButtonsSettingsTest, UpdateModeActionMultiArgument) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "mode";
	request.arg[0] = "multi";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::mode, s.buttons[0].action);
	EXPECT_EQ(lamp_mode::multi, s.buttons[0].mode);
}

TEST(ButtonsSettingsTest, UpdateModeActionNightArgument) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "mode";
	request.arg[0] = "night";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::mode, s.buttons[0].action);
	EXPECT_EQ(lamp_mode::night, s.buttons[0].mode);
}

TEST(ButtonsSettingsTest, UpdateModeActionLightArgument) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "mode";
	request.arg[0] = "light";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::mode, s.buttons[0].action);
	EXPECT_EQ(lamp_mode::light, s.buttons[0].mode);
}

TEST(ButtonsSettingsTest, UpdateShowActionTotalArgument) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[1] = "show";
	request.arg[1] = "total";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::show, s.buttons[1].action);
	EXPECT_EQ(lamp_param::total, s.buttons[1].param);
}

TEST(ButtonsSettingsTest, UpdateShowActionTempArgument) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[1] = "show";
	request.arg[1] = "temp";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::show, s.buttons[1].action);
	EXPECT_EQ(lamp_param::temp, s.buttons[1].param);
}

TEST(ButtonsSettingsTest, UpdateShowActionHumidArgument) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[1] = "show";
	request.arg[1] = "humid";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::show, s.buttons[1].action);
	EXPECT_EQ(lamp_param::humid, s.buttons[1].param);
}

TEST(ButtonsSettingsTest, UpdateShowActionEco2Argument) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[1] = "show";
	request.arg[1] = "eco2";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::show, s.buttons[1].action);
	EXPECT_EQ(lamp_param::eco2, s.buttons[1].param);
}

TEST(ButtonsSettingsTest, UpdateShowActionTvocArgument) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[1] = "show";
	request.arg[1] = "tvoc";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::show, s.buttons[1].action);
	EXPECT_EQ(lamp_param::tvoc, s.buttons[1].param);
}

TEST(ButtonsSettingsTest, UpdateShowActionPm25Argument) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[1] = "show";
	request.arg[1] = "pm25";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::show, s.buttons[1].action);
	EXPECT_EQ(lamp_param::pm25, s.buttons[1].param);
}

TEST(ButtonsSettingsTest, UpdateShowActionRefusesArgumentPm1) {
	const buttons_update_result exp_res = buttons_update_result::bad_arg;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "show";
	request.arg[0] = "pm1";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateShowActionRefusesArgumentPm10) {
	const buttons_update_result exp_res = buttons_update_result::bad_arg;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "show";
	request.arg[0] = "pm10";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateShowActionRefusesArgumentHum) {
	const buttons_update_result exp_res = buttons_update_result::bad_arg;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "show";
	request.arg[0] = "hum";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateSwitchingToModeWithoutArgumentIsBadArg) {
	const buttons_update_result exp_res = buttons_update_result::bad_arg;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "mode";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateSwitchingToShowWithoutArgumentIsBadArg) {
	const buttons_update_result exp_res = buttons_update_result::bad_arg;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "show";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateModeToModeWithoutArgumentIsBadArg) {
	const buttons_update_result exp_res = buttons_update_result::bad_arg;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[3] = {button_action::mode, lamp_mode::night, lamp_param::total};
	buttons_request request = {};
	request.action[3] = "mode";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(lamp_mode::night, s.buttons[3].mode);
}

TEST(ButtonsSettingsTest, UpdateModeActionWithShowKeyIsBadArg) {
	const buttons_update_result exp_res = buttons_update_result::bad_arg;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "mode";
	request.arg[0] = "temp";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateShowActionWithModeKeyIsBadArg) {
	const buttons_update_result exp_res = buttons_update_result::bad_arg;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "show";
	request.arg[0] = "night";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateEmptyArgumentIsBadArg) {
	const buttons_update_result exp_res = buttons_update_result::bad_arg;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "mode";
	request.arg[0] = "";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateWrongCaseArgumentIsBadArg) {
	const buttons_update_result exp_res = buttons_update_result::bad_arg;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "mode";
	request.arg[0] = "Night";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateActionKeyEmptyIsBadAction) {
	const buttons_update_result exp_res = buttons_update_result::bad_action;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[2] = "";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateActionKeyWrongCaseIsBadAction) {
	const buttons_update_result exp_res = buttons_update_result::bad_action;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[2] = "Power";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateActionKeyTypoIsBadAction) {
	const buttons_update_result exp_res = buttons_update_result::bad_action;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[2] = "powr";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateActionKeyTrailingSpaceIsBadAction) {
	const buttons_update_result exp_res = buttons_update_result::bad_action;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[2] = "mode ";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateActionKeyUnknownIsBadAction) {
	const buttons_update_result exp_res = buttons_update_result::bad_action;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[2] = "toggle";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateArgumentWithActionNoneIsRefused) {
	const buttons_update_result exp_res = buttons_update_result::arg_not_allowed;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "none";
	request.arg[0] = "night";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateArgumentWithActionPowerIsRefused) {
	const buttons_update_result exp_res = buttons_update_result::arg_not_allowed;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "power";
	request.arg[0] = "night";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateArgumentWithActionUpIsRefused) {
	const buttons_update_result exp_res = buttons_update_result::arg_not_allowed;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[1] = "up";
	request.arg[1] = "night";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateArgumentWithActionDownIsRefused) {
	const buttons_update_result exp_res = buttons_update_result::arg_not_allowed;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[3] = "down";
	request.arg[3] = "night";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateArgumentWithActionNextIsRefused) {
	const buttons_update_result exp_res = buttons_update_result::arg_not_allowed;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[2] = "next";
	request.arg[2] = "night";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateArgumentAloneOnCurrentNoneIsRefused) {
	const buttons_update_result exp_res = buttons_update_result::arg_not_allowed;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0].action = button_action::none;
	buttons_request request = {};
	request.arg[0] = "night";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::none, s.buttons[0].action);
	EXPECT_EQ(lamp_mode::standard, s.buttons[0].mode);
}

TEST(ButtonsSettingsTest, UpdateArgumentAloneOnCurrentPowerIsRefused) {
	const buttons_update_result exp_res = buttons_update_result::arg_not_allowed;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.arg[0] = "night";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateArgumentAloneOnCurrentUpIsRefused) {
	const buttons_update_result exp_res = buttons_update_result::arg_not_allowed;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.arg[1] = "night";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateArgumentAloneOnCurrentDownIsRefused) {
	const buttons_update_result exp_res = buttons_update_result::arg_not_allowed;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.arg[3] = "night";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateArgumentAloneOnCurrentNextIsRefused) {
	const buttons_update_result exp_res = buttons_update_result::arg_not_allowed;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.arg[2] = "night";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateArgumentAloneChangesModeOfCurrentModeAction) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[3] = {button_action::mode, lamp_mode::multi, lamp_param::total};
	buttons_request request = {};
	request.arg[3] = "light";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::mode, s.buttons[3].action);
	EXPECT_EQ(lamp_mode::light, s.buttons[3].mode);
}

TEST(ButtonsSettingsTest, UpdateArgumentAloneChangesReadingOfCurrentShowAction) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0] = {button_action::show, lamp_mode::standard, lamp_param::temp};
	buttons_request request = {};
	request.arg[0] = "tvoc";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::show, s.buttons[0].action);
	EXPECT_EQ(lamp_param::tvoc, s.buttons[0].param);
}

TEST(ButtonsSettingsTest, UpdateArgumentAloneWithShowKeyOnModeActionIsBadArg) {
	const buttons_update_result exp_res = buttons_update_result::bad_arg;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[3] = {button_action::mode, lamp_mode::multi, lamp_param::total};
	buttons_request request = {};
	request.arg[3] = "temp";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(lamp_mode::multi, s.buttons[3].mode);
}

TEST(ButtonsSettingsTest, UpdateArgumentAloneWithModeKeyOnShowActionIsBadArg) {
	const buttons_update_result exp_res = buttons_update_result::bad_arg;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0] = {button_action::show, lamp_mode::standard, lamp_param::temp};
	buttons_request request = {};
	request.arg[0] = "night";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(lamp_param::temp, s.buttons[0].param);
}

TEST(ButtonsSettingsTest, UpdateArgumentAloneWithPm10OnShowActionIsBadArg) {
	const buttons_update_result exp_res = buttons_update_result::bad_arg;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0] = {button_action::show, lamp_mode::standard, lamp_param::temp};
	buttons_request request = {};
	request.arg[0] = "pm10";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(lamp_param::temp, s.buttons[0].param);
}

TEST(ButtonsSettingsTest, UpdateSwitchingFromModeToPowerResetsTheMode) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[3] = {button_action::mode, lamp_mode::night, lamp_param::total};
	buttons_request request = {};
	request.action[3] = "power";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::power, s.buttons[3].action);
	EXPECT_EQ(lamp_mode::standard, s.buttons[3].mode);
	EXPECT_EQ(lamp_param::total, s.buttons[3].param);
}

TEST(ButtonsSettingsTest, UpdateSwitchingFromShowToNextResetsTheParam) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0] = {button_action::show, lamp_mode::standard, lamp_param::pm25};
	buttons_request request = {};
	request.action[0] = "next";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::next, s.buttons[0].action);
	EXPECT_EQ(lamp_mode::standard, s.buttons[0].mode);
	EXPECT_EQ(lamp_param::total, s.buttons[0].param);
}

TEST(ButtonsSettingsTest, UpdateSwitchingFromModeToShowResetsTheMode) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0] = {button_action::mode, lamp_mode::night, lamp_param::total};
	buttons_request request = {};
	request.action[0] = "show";
	request.arg[0] = "pm25";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::show, s.buttons[0].action);
	EXPECT_EQ(lamp_mode::standard, s.buttons[0].mode);
	EXPECT_EQ(lamp_param::pm25, s.buttons[0].param);
}

TEST(ButtonsSettingsTest, UpdateSwitchingFromShowToModeResetsTheParam) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0] = {button_action::show, lamp_mode::standard, lamp_param::pm25};
	buttons_request request = {};
	request.action[0] = "mode";
	request.arg[0] = "light";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::mode, s.buttons[0].action);
	EXPECT_EQ(lamp_mode::light, s.buttons[0].mode);
	EXPECT_EQ(lamp_param::total, s.buttons[0].param);
}

TEST(ButtonsSettingsTest, UpdateLockedOneLocks) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.locked = "1";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_TRUE(s.locked);
}

TEST(ButtonsSettingsTest, UpdateLockedZeroUnlocks) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.locked = true;
	buttons_request request = {};
	request.locked = "0";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_FALSE(s.locked);
}

TEST(ButtonsSettingsTest, UpdateLockedOneKeepsTheBindings) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.locked = "1";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::power, s.buttons[0].action);
	EXPECT_EQ(button_action::down, s.buttons[3].action);
}

TEST(ButtonsSettingsTest, UpdateLockedEmptyIsBadLocked) {
	const buttons_update_result exp_res = buttons_update_result::bad_locked;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.locked = "";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_FALSE(s.locked);
}

TEST(ButtonsSettingsTest, UpdateLockedTwoIsBadLocked) {
	const buttons_update_result exp_res = buttons_update_result::bad_locked;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.locked = "2";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_FALSE(s.locked);
}

TEST(ButtonsSettingsTest, UpdateLockedTrueIsBadLocked) {
	const buttons_update_result exp_res = buttons_update_result::bad_locked;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.locked = "true";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_FALSE(s.locked);
}

TEST(ButtonsSettingsTest, UpdateLockedLeadingZeroIsBadLocked) {
	const buttons_update_result exp_res = buttons_update_result::bad_locked;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.locked = "01";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_FALSE(s.locked);
}

TEST(ButtonsSettingsTest, UpdateLockedMinusIsBadLocked) {
	const buttons_update_result exp_res = buttons_update_result::bad_locked;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.locked = "-1";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_FALSE(s.locked);
}

TEST(ButtonsSettingsTest, UpdateLockedAndActionInOneRequest) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.locked = "1";
	request.action[0] = "none";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_TRUE(s.locked);
	EXPECT_EQ(button_action::none, s.buttons[0].action);
}

TEST(ButtonsSettingsTest, UpdateIsAllOrNothingWhenALaterActionIsBad) {
	const buttons_update_result exp_res = buttons_update_result::bad_action;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.locked = "1";
	request.action[0] = "none";
	request.action[1] = "down";
	request.action[3] = "bogus";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateIsAllOrNothingWhenLockedIsBad) {
	const buttons_update_result exp_res = buttons_update_result::bad_locked;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "none";
	request.locked = "maybe";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateIsAllOrNothingWhenALaterArgumentIsRefused) {
	const buttons_update_result exp_res = buttons_update_result::arg_not_allowed;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "none";
	request.arg[1] = "night";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateActionOfSixteenCharactersIsBadAction) {
	const buttons_update_result exp_res = buttons_update_result::bad_action;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "0123456789abcdef";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateActionOfSeventeenCharactersIsTooLong) {
	const buttons_update_result exp_res = buttons_update_result::field_too_long;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "0123456789abcdefg";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateArgumentOfSixteenCharactersIsBadArg) {
	const buttons_update_result exp_res = buttons_update_result::bad_arg;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "mode";
	request.arg[0] = "0123456789abcdef";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateArgumentOfSeventeenCharactersIsTooLong) {
	const buttons_update_result exp_res = buttons_update_result::field_too_long;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "none";
	request.arg[3] = "0123456789abcdefg";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateLockedOfSixteenCharactersIsBadLocked) {
	const buttons_update_result exp_res = buttons_update_result::bad_locked;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.locked = "0123456789abcdef";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_FALSE(s.locked);
}

TEST(ButtonsSettingsTest, UpdateLockedOfSeventeenCharactersIsTooLong) {
	const buttons_update_result exp_res = buttons_update_result::field_too_long;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.locked = "0123456789abcdefg";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_FALSE(s.locked);
}

TEST(ButtonsSettingsTest, UpdateResetOfSixteenCharactersIsBadReset) {
	const buttons_update_result exp_res = buttons_update_result::bad_reset;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.reset = "0123456789abcdef";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateResetOfSeventeenCharactersIsTooLong) {
	const buttons_update_result exp_res = buttons_update_result::field_too_long;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.reset = "0123456789abcdefg";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateTooLongFieldWinsOverEarlierBadAction) {
	const buttons_update_result exp_res = buttons_update_result::field_too_long;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.action[0] = "bogus";
	request.arg[1] = "0123456789abcdefg";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, ResetRestoresDefaultBindings) {
	const buttons_update_result exp_res = buttons_update_result::ok;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0] = {button_action::show, lamp_mode::standard, lamp_param::eco2};
	s.buttons[3] = {button_action::mode, lamp_mode::night, lamp_param::total};
	buttons_request request = {};
	request.reset = "1";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, ResetKeepsTheLockOn) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.locked = true;
	s.buttons[1].action = button_action::none;
	buttons_request request = {};
	request.reset = "1";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_TRUE(s.locked);
	EXPECT_EQ(button_action::up, s.buttons[1].action);
}

TEST(ButtonsSettingsTest, ResetKeepsTheLockOff) {
	const buttons_update_result exp_res = buttons_update_result::ok;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[1].action = button_action::none;
	buttons_request request = {};
	request.reset = "1";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_FALSE(s.locked);
	EXPECT_EQ(button_action::up, s.buttons[1].action);
}

TEST(ButtonsSettingsTest, ResetWithActionIsRefused) {
	const buttons_update_result exp_res = buttons_update_result::reset_with_fields;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[2].action = button_action::none;
	buttons_request request = {};
	request.reset = "1";
	request.action[0] = "none";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::none, s.buttons[2].action);
}

TEST(ButtonsSettingsTest, ResetWithArgumentIsRefused) {
	const buttons_update_result exp_res = buttons_update_result::reset_with_fields;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[2].action = button_action::none;
	buttons_request request = {};
	request.reset = "1";
	request.arg[0] = "night";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::none, s.buttons[2].action);
}

TEST(ButtonsSettingsTest, ResetWithLockedIsRefused) {
	const buttons_update_result exp_res = buttons_update_result::reset_with_fields;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[2].action = button_action::none;
	buttons_request request = {};
	request.reset = "1";
	request.locked = "0";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::none, s.buttons[2].action);
}

TEST(ButtonsSettingsTest, ResetValueEmptyIsBadReset) {
	const buttons_update_result exp_res = buttons_update_result::bad_reset;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[2].action = button_action::none;
	buttons_request request = {};
	request.reset = "";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::none, s.buttons[2].action);
}

TEST(ButtonsSettingsTest, ResetValueZeroIsBadReset) {
	const buttons_update_result exp_res = buttons_update_result::bad_reset;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[2].action = button_action::none;
	buttons_request request = {};
	request.reset = "0";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::none, s.buttons[2].action);
}

TEST(ButtonsSettingsTest, ResetValueTrueIsBadReset) {
	const buttons_update_result exp_res = buttons_update_result::bad_reset;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[2].action = button_action::none;
	buttons_request request = {};
	request.reset = "true";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::none, s.buttons[2].action);
}

TEST(ButtonsSettingsTest, ResetValueElevenIsBadReset) {
	const buttons_update_result exp_res = buttons_update_result::bad_reset;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[2].action = button_action::none;
	buttons_request request = {};
	request.reset = "11";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(button_action::none, s.buttons[2].action);
}

TEST(ButtonsSettingsTest, ResetBadValueWithOtherFieldIsBadReset) {
	const buttons_update_result exp_res = buttons_update_result::bad_reset;
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_request request = {};
	request.reset = "0";
	request.action[0] = "none";

	buttons_update_result res;

	res = HAQuDA_ButtonsUpdate(s, request);

	EXPECT_EQ(exp_res, res);
	EXPECT_EQ(0, memcmp(&exp_settings, &s, sizeof(s)));
}

TEST(ButtonsSettingsTest, UpdateTextOfOkIsEmpty) {
	const char *res;

	res = HAQuDA_ButtonsUpdateText(buttons_update_result::ok);

	EXPECT_STREQ("", res);
}

TEST(ButtonsSettingsTest, UpdateTextOfNothingIsNotEmpty) {
	const char *res;

	res = HAQuDA_ButtonsUpdateText(buttons_update_result::nothing);

	EXPECT_STRNE("", res);
}

TEST(ButtonsSettingsTest, UpdateTextOfFieldTooLongIsNotEmpty) {
	const char *res;

	res = HAQuDA_ButtonsUpdateText(buttons_update_result::field_too_long);

	EXPECT_STRNE("", res);
}

TEST(ButtonsSettingsTest, UpdateTextOfBadActionIsNotEmpty) {
	const char *res;

	res = HAQuDA_ButtonsUpdateText(buttons_update_result::bad_action);

	EXPECT_STRNE("", res);
}

TEST(ButtonsSettingsTest, UpdateTextOfBadArgIsNotEmpty) {
	const char *res;

	res = HAQuDA_ButtonsUpdateText(buttons_update_result::bad_arg);

	EXPECT_STRNE("", res);
}

TEST(ButtonsSettingsTest, UpdateTextOfArgNotAllowedIsNotEmpty) {
	const char *res;

	res = HAQuDA_ButtonsUpdateText(buttons_update_result::arg_not_allowed);

	EXPECT_STRNE("", res);
}

TEST(ButtonsSettingsTest, UpdateTextOfBadLockedIsNotEmpty) {
	const char *res;

	res = HAQuDA_ButtonsUpdateText(buttons_update_result::bad_locked);

	EXPECT_STRNE("", res);
}

TEST(ButtonsSettingsTest, UpdateTextOfBadResetIsNotEmpty) {
	const char *res;

	res = HAQuDA_ButtonsUpdateText(buttons_update_result::bad_reset);

	EXPECT_STRNE("", res);
}

TEST(ButtonsSettingsTest, UpdateTextOfResetWithFieldsIsNotEmpty) {
	const char *res;

	res = HAQuDA_ButtonsUpdateText(buttons_update_result::reset_with_fields);

	EXPECT_STRNE("", res);
}

TEST(ButtonsSettingsTest, JsonOfDefaultsHasNoArguments) {
	const char *const exp_json
		= "{\"locked\":false,\"buttons\":{\"2\":{\"a\":\"power\"},\"6\":{\"a\":\"up\"},\"8\":{\"a\":\"next\"},\"9\":{\"a\":\"down\"}},\"saveErr\":0}";

	buttons_settings s = HAQuDA_ButtonsDefaults();

	char buf[BUTTONS_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildButtonsJson(s, 0, buf, sizeof(buf));

	EXPECT_STREQ(exp_json, buf);
	EXPECT_EQ(strlen(exp_json), res);
}

TEST(ButtonsSettingsTest, JsonLockedIsTrue) {
	const char *const exp_json
		= "{\"locked\":true,\"buttons\":{\"2\":{\"a\":\"power\"},\"6\":{\"a\":\"up\"},\"8\":{\"a\":\"next\"},\"9\":{\"a\":\"down\"}},\"saveErr\":0}";

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.locked = true;
	char buf[BUTTONS_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildButtonsJson(s, 0, buf, sizeof(buf));

	EXPECT_STREQ(exp_json, buf);
	EXPECT_EQ(strlen(exp_json), res);
}

TEST(ButtonsSettingsTest, JsonNoneAction) {
	const char *const exp_json
		= "{\"locked\":false,\"buttons\":{\"2\":{\"a\":\"none\"},\"6\":{\"a\":\"up\"},\"8\":{\"a\":\"next\"},\"9\":{\"a\":\"down\"}},\"saveErr\":0}";

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0].action = button_action::none;
	char buf[BUTTONS_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildButtonsJson(s, 0, buf, sizeof(buf));

	EXPECT_STREQ(exp_json, buf);
	EXPECT_EQ(strlen(exp_json), res);
}

TEST(ButtonsSettingsTest, JsonModeActionHasTheModeKey) {
	const char *const exp_json = "{\"locked\":false,\"buttons\":{\"2\":{\"a\":\"power\"},\"6\":{\"a\":\"up\"},\"8\":{\"a\":\"next\"},\"9\":{\"a\":\"mode\","
								 "\"arg\":\"night\"}},\"saveErr\":0}";

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[3] = {button_action::mode, lamp_mode::night, lamp_param::total};
	char buf[BUTTONS_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildButtonsJson(s, 0, buf, sizeof(buf));

	EXPECT_STREQ(exp_json, buf);
	EXPECT_EQ(strlen(exp_json), res);
}

TEST(ButtonsSettingsTest, JsonShowActionHasTheReadingKey) {
	const char *const exp_json = "{\"locked\":false,\"buttons\":{\"2\":{\"a\":\"show\",\"arg\":\"eco2\"},\"6\":{\"a\":\"up\"},\"8\":{\"a\":\"next\"},\"9\":{"
								 "\"a\":\"down\"}},\"saveErr\":0}";

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0] = {button_action::show, lamp_mode::standard, lamp_param::eco2};
	char buf[BUTTONS_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildButtonsJson(s, 0, buf, sizeof(buf));

	EXPECT_STREQ(exp_json, buf);
	EXPECT_EQ(strlen(exp_json), res);
}

TEST(ButtonsSettingsTest, JsonLongestCaseIsExactlyTheDocumentedLength) {
	const size_t exp_len = 185;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0] = {button_action::mode, lamp_mode::standard, lamp_param::total};
	s.buttons[1] = {button_action::mode, lamp_mode::standard, lamp_param::total};
	s.buttons[2] = {button_action::mode, lamp_mode::standard, lamp_param::total};
	s.buttons[3] = {button_action::mode, lamp_mode::standard, lamp_param::total};
	char buf[BUTTONS_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildButtonsJson(s, 4294967295u, buf, sizeof(buf));

	EXPECT_EQ(exp_len, res);
	EXPECT_LT(res, BUTTONS_JSON_BUF_LEN);
}

TEST(ButtonsSettingsTest, JsonBufferOfLengthPlusOneFits) {
	const char *const exp_json
		= "{\"locked\":false,\"buttons\":{\"2\":{\"a\":\"power\"},\"6\":{\"a\":\"up\"},\"8\":{\"a\":\"next\"},\"9\":{\"a\":\"down\"}},\"saveErr\":0}";

	const buttons_settings s = HAQuDA_ButtonsDefaults();
	char buf[BUTTONS_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildButtonsJson(s, 0, buf, strlen(exp_json) + 1);

	EXPECT_STREQ(exp_json, buf);
	EXPECT_EQ(strlen(exp_json), res);
}

TEST(ButtonsSettingsTest, JsonBufferOfExactLengthGivesZeroAndEmptyText) {
	const char *const json
		= "{\"locked\":false,\"buttons\":{\"2\":{\"a\":\"power\"},\"6\":{\"a\":\"up\"},\"8\":{\"a\":\"next\"},\"9\":{\"a\":\"down\"}},\"saveErr\":0}";
	const size_t exp_res = 0;

	const buttons_settings s = HAQuDA_ButtonsDefaults();
	char buf[BUTTONS_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildButtonsJson(s, 0, buf, strlen(json));

	EXPECT_EQ(exp_res, res);
	EXPECT_STREQ("", buf);
}

TEST(ButtonsSettingsTest, JsonBufferOfFourBytesGivesZero) {
	const size_t exp_res = 0;

	const buttons_settings s = HAQuDA_ButtonsDefaults();
	char buf[4] = {'x', 'x', 'x', 'x'};

	size_t res;

	res = HAQuDA_BuildButtonsJson(s, 0, buf, sizeof(buf));

	EXPECT_EQ(exp_res, res);
}

TEST(ButtonsSettingsTest, JsonSizeZeroGivesZero) {
	const size_t exp_res = 0;

	const buttons_settings s = HAQuDA_ButtonsDefaults();
	char buf[BUTTONS_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildButtonsJson(s, 0, buf, 0);

	EXPECT_EQ(exp_res, res);
}

TEST(ButtonsSettingsTest, JsonNullBufferGivesZero) {
	const size_t exp_res = 0;

	const buttons_settings s = HAQuDA_ButtonsDefaults();

	size_t res;

	res = HAQuDA_BuildButtonsJson(s, 0, nullptr, BUTTONS_JSON_BUF_LEN);

	EXPECT_EQ(exp_res, res);
}

TEST(ButtonsSettingsTest, JsonOfInvalidSettingsGivesZeroAndEmptyText) {
	const size_t exp_res = 0;
	const uint8_t outOfRangeAction = 9;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0].action = static_cast<button_action>(outOfRangeAction);
	char buf[BUTTONS_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildButtonsJson(s, 0, buf, sizeof(buf));

	EXPECT_EQ(exp_res, res);
	EXPECT_STREQ("", buf);
}

TEST(ButtonsRecordTest, RoundTripOfDefaults) {
	buttons_settings s = HAQuDA_ButtonsDefaults();

	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(s, record, sizeof(record));
	buttons_settings back = {};

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, sizeof(record), back);

	EXPECT_TRUE(res);
	EXPECT_EQ(0, memcmp(&s, &back, sizeof(s)));
}

TEST(ButtonsRecordTest, RoundTripOfLock) {
	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.locked = true;
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(s, record, sizeof(record));
	buttons_settings back = {};

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, sizeof(record), back);

	EXPECT_TRUE(res);
	EXPECT_EQ(0, memcmp(&s, &back, sizeof(s)));
}

TEST(ButtonsRecordTest, RoundTripOfNoneAction) {
	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[2].action = button_action::none;
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(s, record, sizeof(record));
	buttons_settings back = {};

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, sizeof(record), back);

	EXPECT_TRUE(res);
	EXPECT_EQ(0, memcmp(&s, &back, sizeof(s)));
}

TEST(ButtonsRecordTest, RoundTripOfPowerAction) {
	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[1].action = button_action::power;
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(s, record, sizeof(record));
	buttons_settings back = {};

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, sizeof(record), back);

	EXPECT_TRUE(res);
	EXPECT_EQ(0, memcmp(&s, &back, sizeof(s)));
}

TEST(ButtonsRecordTest, RoundTripOfUpAction) {
	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0].action = button_action::up;
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(s, record, sizeof(record));
	buttons_settings back = {};

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, sizeof(record), back);

	EXPECT_TRUE(res);
	EXPECT_EQ(0, memcmp(&s, &back, sizeof(s)));
}

TEST(ButtonsRecordTest, RoundTripOfDownAction) {
	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0].action = button_action::down;
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(s, record, sizeof(record));
	buttons_settings back = {};

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, sizeof(record), back);

	EXPECT_TRUE(res);
	EXPECT_EQ(0, memcmp(&s, &back, sizeof(s)));
}

TEST(ButtonsRecordTest, RoundTripOfNextAction) {
	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0].action = button_action::next;
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(s, record, sizeof(record));
	buttons_settings back = {};

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, sizeof(record), back);

	EXPECT_TRUE(res);
	EXPECT_EQ(0, memcmp(&s, &back, sizeof(s)));
}

TEST(ButtonsRecordTest, RoundTripOfModeActionWithLight) {
	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0] = {button_action::mode, lamp_mode::light, lamp_param::total};
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(s, record, sizeof(record));
	buttons_settings back = {};

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, sizeof(record), back);

	EXPECT_TRUE(res);
	EXPECT_EQ(0, memcmp(&s, &back, sizeof(s)));
}

TEST(ButtonsRecordTest, RoundTripOfModeActionWithStandard) {
	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0] = {button_action::mode, lamp_mode::standard, lamp_param::total};
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(s, record, sizeof(record));
	buttons_settings back = {};

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, sizeof(record), back);

	EXPECT_TRUE(res);
	EXPECT_EQ(0, memcmp(&s, &back, sizeof(s)));
}

TEST(ButtonsRecordTest, RoundTripOfShowActionWithPm25) {
	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[3] = {button_action::show, lamp_mode::standard, lamp_param::pm25};
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(s, record, sizeof(record));
	buttons_settings back = {};

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, sizeof(record), back);

	EXPECT_TRUE(res);
	EXPECT_EQ(0, memcmp(&s, &back, sizeof(s)));
}

TEST(ButtonsRecordTest, RoundTripOfShowActionWithTotal) {
	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[3] = {button_action::show, lamp_mode::standard, lamp_param::total};
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(s, record, sizeof(record));
	buttons_settings back = {};

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, sizeof(record), back);

	EXPECT_TRUE(res);
	EXPECT_EQ(0, memcmp(&s, &back, sizeof(s)));
}

TEST(ButtonsRecordTest, EncodeWritesMagicVersionLockBindingsAndChecksum) {
	const uint8_t exp_record[BUTTONS_RECORD_LEN] = {0x48, 0x42, 0x54, 0x54, 0x01, 0x01, 0x01, 0x00, 0x02, 0x00, 0x04, 0x00, 0x03, 0x00, 0x3F, 0x22};

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.locked = true;
	uint8_t record[BUTTONS_RECORD_LEN] = {};

	bool res;

	res = HAQuDA_EncodeButtonsRecord(s, record, sizeof(record));

	EXPECT_TRUE(res);
	EXPECT_EQ(0, memcmp(exp_record, record, sizeof(record)));
}

TEST(ButtonsRecordTest, EncodeWritesTheArgumentByteOfModeAndShow) {
	const uint8_t exp_record[BUTTONS_RECORD_LEN] = {0x48, 0x42, 0x54, 0x54, 0x01, 0x00, 0x06, 0x05, 0x02, 0x00, 0x04, 0x00, 0x05, 0x02, 0x4C, 0x6A};

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0] = {button_action::show, lamp_mode::standard, lamp_param::pm25};
	s.buttons[3] = {button_action::mode, lamp_mode::night, lamp_param::total};
	uint8_t record[BUTTONS_RECORD_LEN] = {};

	bool res;

	res = HAQuDA_EncodeButtonsRecord(s, record, sizeof(record));

	EXPECT_TRUE(res);
	EXPECT_EQ(0, memcmp(exp_record, record, sizeof(record)));
}

TEST(ButtonsRecordTest, EncodeRefusesInvalidSettingsAndWritesNothing) {
	const uint8_t exp_first = 0;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[2].mode = lamp_mode::night;
	uint8_t record[BUTTONS_RECORD_LEN] = {};

	bool res;

	res = HAQuDA_EncodeButtonsRecord(s, record, sizeof(record));

	EXPECT_FALSE(res);
	EXPECT_EQ(exp_first, record[0]);
}

TEST(ButtonsRecordTest, EncodeRefusesBufferOneByteTooSmall) {
	const buttons_settings s = HAQuDA_ButtonsDefaults();
	uint8_t record[BUTTONS_RECORD_LEN];

	bool res;

	res = HAQuDA_EncodeButtonsRecord(s, record, BUTTONS_RECORD_LEN - 1);

	EXPECT_FALSE(res);
}

TEST(ButtonsRecordTest, EncodeRefusesNullBuffer) {
	const buttons_settings s = HAQuDA_ButtonsDefaults();

	bool res;

	res = HAQuDA_EncodeButtonsRecord(s, nullptr, BUTTONS_RECORD_LEN);

	EXPECT_FALSE(res);
}

TEST(ButtonsRecordTest, EncodeAcceptsBufferLargerThanTheRecord) {
	const size_t bigSize = BUTTONS_RECORD_LEN + 4;

	const buttons_settings s = HAQuDA_ButtonsDefaults();
	uint8_t record[BUTTONS_RECORD_LEN + 4] = {};

	bool res;

	res = HAQuDA_EncodeButtonsRecord(s, record, bigSize);

	EXPECT_TRUE(res);
}

TEST(ButtonsRecordTest, DecodeRefusesLengthOneShort) {
	buttons_settings exp_out = HAQuDA_ButtonsDefaults();
	exp_out.buttons[0].action = button_action::none;

	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.locked = true;
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(settings, record, sizeof(record));

	buttons_settings out = exp_out;

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, BUTTONS_RECORD_LEN - 1, out);

	EXPECT_FALSE(res);
	EXPECT_EQ(0, memcmp(&exp_out, &out, sizeof(out)));
}

TEST(ButtonsRecordTest, DecodeRefusesLengthOneLong) {
	buttons_settings exp_out = HAQuDA_ButtonsDefaults();
	exp_out.buttons[0].action = button_action::none;

	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.locked = true;
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(settings, record, sizeof(record));

	buttons_settings out = exp_out;

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, BUTTONS_RECORD_LEN + 1, out);

	EXPECT_FALSE(res);
	EXPECT_EQ(0, memcmp(&exp_out, &out, sizeof(out)));
}

TEST(ButtonsRecordTest, DecodeRefusesLengthZero) {
	buttons_settings exp_out = HAQuDA_ButtonsDefaults();
	exp_out.buttons[0].action = button_action::none;

	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.locked = true;
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(settings, record, sizeof(record));

	buttons_settings out = exp_out;

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, 0, out);

	EXPECT_FALSE(res);
	EXPECT_EQ(0, memcmp(&exp_out, &out, sizeof(out)));
}

TEST(ButtonsRecordTest, DecodeRefusesNullData) {
	buttons_settings exp_out = HAQuDA_ButtonsDefaults();
	exp_out.buttons[0].action = button_action::none;

	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.locked = true;
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(settings, record, sizeof(record));

	buttons_settings out = exp_out;

	bool res;

	res = HAQuDA_DecodeButtonsRecord(nullptr, BUTTONS_RECORD_LEN, out);

	EXPECT_FALSE(res);
	EXPECT_EQ(0, memcmp(&exp_out, &out, sizeof(out)));
}

TEST(ButtonsRecordTest, DecodeRefusesWrongMagic) {
	buttons_settings exp_out = HAQuDA_ButtonsDefaults();
	exp_out.buttons[0].action = button_action::none;

	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.locked = true;
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(settings, record, sizeof(record));
	const size_t checksumOffset = BUTTONS_RECORD_LEN - 2;
	const size_t magicOffset = 0;
	const uint8_t wrongMagicByte = 0x00;
	record[magicOffset] = wrongMagicByte;
	HAQuDA_PutU16(record + checksumOffset, HAQuDA_Fletcher16(record, checksumOffset));
	buttons_settings out = exp_out;

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, BUTTONS_RECORD_LEN, out);

	EXPECT_FALSE(res);
	EXPECT_EQ(0, memcmp(&exp_out, &out, sizeof(out)));
}

TEST(ButtonsRecordTest, DecodeRefusesWrongVersion) {
	buttons_settings exp_out = HAQuDA_ButtonsDefaults();
	exp_out.buttons[0].action = button_action::none;

	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.locked = true;
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(settings, record, sizeof(record));
	const size_t checksumOffset = BUTTONS_RECORD_LEN - 2;
	const size_t versionOffset = 4;
	const uint8_t unknownVersion = 2;
	record[versionOffset] = unknownVersion;
	HAQuDA_PutU16(record + checksumOffset, HAQuDA_Fletcher16(record, checksumOffset));
	buttons_settings out = exp_out;

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, BUTTONS_RECORD_LEN, out);

	EXPECT_FALSE(res);
	EXPECT_EQ(0, memcmp(&exp_out, &out, sizeof(out)));
}

TEST(ButtonsRecordTest, DecodeRefusesVersionZero) {
	buttons_settings exp_out = HAQuDA_ButtonsDefaults();
	exp_out.buttons[0].action = button_action::none;

	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.locked = true;
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(settings, record, sizeof(record));
	const size_t checksumOffset = BUTTONS_RECORD_LEN - 2;
	const size_t versionOffset = 4;
	const uint8_t unknownVersion = 0;
	record[versionOffset] = unknownVersion;
	HAQuDA_PutU16(record + checksumOffset, HAQuDA_Fletcher16(record, checksumOffset));
	buttons_settings out = exp_out;

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, BUTTONS_RECORD_LEN, out);

	EXPECT_FALSE(res);
	EXPECT_EQ(0, memcmp(&exp_out, &out, sizeof(out)));
}

TEST(ButtonsRecordTest, DecodeRefusesWrongChecksumHighByte) {
	buttons_settings exp_out = HAQuDA_ButtonsDefaults();
	exp_out.buttons[0].action = button_action::none;

	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.locked = true;
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(settings, record, sizeof(record));
	const size_t checksumHighOffset = BUTTONS_RECORD_LEN - 1;
	const uint8_t wrongByte = 0x00;
	record[checksumHighOffset] = wrongByte;

	buttons_settings out = exp_out;

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, BUTTONS_RECORD_LEN, out);

	EXPECT_FALSE(res);
	EXPECT_EQ(0, memcmp(&exp_out, &out, sizeof(out)));
}

TEST(ButtonsRecordTest, DecodeRefusesWrongChecksumLowByte) {
	buttons_settings exp_out = HAQuDA_ButtonsDefaults();
	exp_out.buttons[0].action = button_action::none;

	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.locked = true;
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(settings, record, sizeof(record));
	const size_t checksumLowOffset = BUTTONS_RECORD_LEN - 2;
	const uint8_t wrongByte = 0x00;
	record[checksumLowOffset] = wrongByte;

	buttons_settings out = exp_out;

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, BUTTONS_RECORD_LEN, out);

	EXPECT_FALSE(res);
	EXPECT_EQ(0, memcmp(&exp_out, &out, sizeof(out)));
}

TEST(ButtonsRecordTest, DecodeRefusesBodyChangedWithoutNewChecksum) {
	buttons_settings exp_out = HAQuDA_ButtonsDefaults();
	exp_out.buttons[0].action = button_action::none;

	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.locked = true;
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(settings, record, sizeof(record));
	const size_t firstActionOffset = 6;
	record[firstActionOffset] = static_cast<uint8_t>(button_action::none);

	buttons_settings out = exp_out;

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, BUTTONS_RECORD_LEN, out);

	EXPECT_FALSE(res);
	EXPECT_EQ(0, memcmp(&exp_out, &out, sizeof(out)));
}

TEST(ButtonsRecordTest, DecodeRefusesLockedByteAboveOne) {
	buttons_settings exp_out = HAQuDA_ButtonsDefaults();
	exp_out.buttons[0].action = button_action::none;

	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.locked = true;
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(settings, record, sizeof(record));
	const size_t checksumOffset = BUTTONS_RECORD_LEN - 2;
	const size_t lockedOffset = 5;
	const uint8_t badLocked = 2;
	record[lockedOffset] = badLocked;
	HAQuDA_PutU16(record + checksumOffset, HAQuDA_Fletcher16(record, checksumOffset));
	buttons_settings out = exp_out;

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, BUTTONS_RECORD_LEN, out);

	EXPECT_FALSE(res);
	EXPECT_EQ(0, memcmp(&exp_out, &out, sizeof(out)));
}

TEST(ButtonsRecordTest, DecodeRefusesActionOutOfRange) {
	buttons_settings exp_out = HAQuDA_ButtonsDefaults();
	exp_out.buttons[0].action = button_action::none;

	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.locked = true;
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(settings, record, sizeof(record));
	const size_t checksumOffset = BUTTONS_RECORD_LEN - 2;
	const size_t firstActionOffset = 6;
	const uint8_t badAction = 7;
	record[firstActionOffset] = badAction;
	HAQuDA_PutU16(record + checksumOffset, HAQuDA_Fletcher16(record, checksumOffset));
	buttons_settings out = exp_out;

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, BUTTONS_RECORD_LEN, out);

	EXPECT_FALSE(res);
	EXPECT_EQ(0, memcmp(&exp_out, &out, sizeof(out)));
}

TEST(ButtonsRecordTest, DecodeRefusesActionOutOfRangeOnLastButton) {
	buttons_settings exp_out = HAQuDA_ButtonsDefaults();
	exp_out.buttons[0].action = button_action::none;

	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.locked = true;
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(settings, record, sizeof(record));
	const size_t checksumOffset = BUTTONS_RECORD_LEN - 2;
	const size_t lastActionOffset = 12;
	const uint8_t badAction = 7;
	record[lastActionOffset] = badAction;
	HAQuDA_PutU16(record + checksumOffset, HAQuDA_Fletcher16(record, checksumOffset));
	buttons_settings out = exp_out;

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, BUTTONS_RECORD_LEN, out);

	EXPECT_FALSE(res);
	EXPECT_EQ(0, memcmp(&exp_out, &out, sizeof(out)));
}

TEST(ButtonsRecordTest, DecodeRefusesModeArgumentOutOfRange) {
	buttons_settings exp_out = HAQuDA_ButtonsDefaults();
	exp_out.buttons[0].action = button_action::none;

	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.locked = true;
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(settings, record, sizeof(record));
	const size_t checksumOffset = BUTTONS_RECORD_LEN - 2;
	const size_t firstActionOffset = 6;
	const size_t firstArgOffset = 7;
	const uint8_t badMode = 4;
	record[firstActionOffset] = static_cast<uint8_t>(button_action::mode);
	record[firstArgOffset] = badMode;
	HAQuDA_PutU16(record + checksumOffset, HAQuDA_Fletcher16(record, checksumOffset));
	buttons_settings out = exp_out;

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, BUTTONS_RECORD_LEN, out);

	EXPECT_FALSE(res);
	EXPECT_EQ(0, memcmp(&exp_out, &out, sizeof(out)));
}

TEST(ButtonsRecordTest, DecodeRefusesShowArgumentOutOfRange) {
	buttons_settings exp_out = HAQuDA_ButtonsDefaults();
	exp_out.buttons[0].action = button_action::none;

	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.locked = true;
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(settings, record, sizeof(record));
	const size_t checksumOffset = BUTTONS_RECORD_LEN - 2;
	const size_t firstActionOffset = 6;
	const size_t firstArgOffset = 7;
	const uint8_t badParam = 6;
	record[firstActionOffset] = static_cast<uint8_t>(button_action::show);
	record[firstArgOffset] = badParam;
	HAQuDA_PutU16(record + checksumOffset, HAQuDA_Fletcher16(record, checksumOffset));
	buttons_settings out = exp_out;

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, BUTTONS_RECORD_LEN, out);

	EXPECT_FALSE(res);
	EXPECT_EQ(0, memcmp(&exp_out, &out, sizeof(out)));
}

TEST(ButtonsRecordTest, DecodeRefusesArgumentOnActionWithoutOne) {
	buttons_settings exp_out = HAQuDA_ButtonsDefaults();
	exp_out.buttons[0].action = button_action::none;

	buttons_settings settings = HAQuDA_ButtonsDefaults();
	settings.locked = true;
	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(settings, record, sizeof(record));
	const size_t checksumOffset = BUTTONS_RECORD_LEN - 2;
	const size_t firstArgOffset = 7;
	const uint8_t strayArg = 1;
	record[firstArgOffset] = strayArg;
	HAQuDA_PutU16(record + checksumOffset, HAQuDA_Fletcher16(record, checksumOffset));
	buttons_settings out = exp_out;

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, BUTTONS_RECORD_LEN, out);

	EXPECT_FALSE(res);
	EXPECT_EQ(0, memcmp(&exp_out, &out, sizeof(out)));
}

TEST(ButtonsRecordTest, DecodeAcceptsAValidRecord) {
	buttons_settings exp_out = HAQuDA_ButtonsDefaults();
	exp_out.locked = true;

	uint8_t record[BUTTONS_RECORD_LEN] = {};
	HAQuDA_EncodeButtonsRecord(exp_out, record, sizeof(record));
	buttons_settings out = {};

	bool res;

	res = HAQuDA_DecodeButtonsRecord(record, sizeof(record), out);

	EXPECT_TRUE(res);
	EXPECT_EQ(0, memcmp(&exp_out, &out, sizeof(out)));
}

TEST(RecordCodecTest, Fletcher16OfAbcde) {
	const uint16_t exp_sum = 0xC8F0;

	const uint8_t data[] = {'a', 'b', 'c', 'd', 'e'};

	uint16_t res;

	res = HAQuDA_Fletcher16(data, sizeof(data));

	EXPECT_EQ(exp_sum, res);
}

TEST(RecordCodecTest, Fletcher16OfAbcdef) {
	const uint16_t exp_sum = 0x2057;

	const uint8_t data[] = {'a', 'b', 'c', 'd', 'e', 'f'};

	uint16_t res;

	res = HAQuDA_Fletcher16(data, sizeof(data));

	EXPECT_EQ(exp_sum, res);
}

TEST(RecordCodecTest, Fletcher16OfAbcdefgh) {
	const uint16_t exp_sum = 0x0627;

	const uint8_t data[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};

	uint16_t res;

	res = HAQuDA_Fletcher16(data, sizeof(data));

	EXPECT_EQ(exp_sum, res);
}

TEST(RecordCodecTest, Fletcher16OfSingleByte) {
	const uint16_t exp_sum = 0x0101;

	const uint8_t data[] = {0x01};

	uint16_t res;

	res = HAQuDA_Fletcher16(data, sizeof(data));

	EXPECT_EQ(exp_sum, res);
}

TEST(RecordCodecTest, Fletcher16TreatsFFAsZero) {
	const uint16_t exp_sum = 0x0000;

	const uint8_t data[] = {0xFF};

	uint16_t res;

	res = HAQuDA_Fletcher16(data, sizeof(data));

	EXPECT_EQ(exp_sum, res);
}

TEST(RecordCodecTest, Fletcher16OfFFThenOne) {
	const uint16_t exp_sum = 0x0101;

	const uint8_t data[] = {0xFF, 0x01};

	uint16_t res;

	res = HAQuDA_Fletcher16(data, sizeof(data));

	EXPECT_EQ(exp_sum, res);
}

TEST(RecordCodecTest, Fletcher16HighByteIsTheSecondSum) {
	const uint16_t exp_sum = 0x0301;

	const uint8_t data[] = {0x01, 0x00, 0x00};

	uint16_t res;

	res = HAQuDA_Fletcher16(data, sizeof(data));

	EXPECT_EQ(exp_sum, res);
}

TEST(RecordCodecTest, Fletcher16OfNothingIsZero) {
	const uint16_t exp_sum = 0;

	const uint8_t data[] = {0x01};

	uint16_t res;

	res = HAQuDA_Fletcher16(data, 0);

	EXPECT_EQ(exp_sum, res);
}

TEST(RecordCodecTest, PutU16WritesLowByteFirst) {
	const uint8_t exp_low = 0x34;
	const uint8_t exp_high = 0x12;

	uint8_t out[2] = {};

	HAQuDA_PutU16(out, 0x1234);

	EXPECT_EQ(exp_low, out[0]);
	EXPECT_EQ(exp_high, out[1]);
}

TEST(RecordCodecTest, PutU16LeavesTheNextByteAlone) {
	const uint8_t exp_next = 0xAA;

	uint8_t out[3] = {0, 0, exp_next};

	HAQuDA_PutU16(out, 0xFFFF);

	EXPECT_EQ(exp_next, out[2]);
}

TEST(RecordCodecTest, GetU16ReadsLowByteFirst) {
	const uint16_t exp_value = 0x1234;

	const uint8_t in[2] = {0x34, 0x12};

	uint16_t res;

	res = HAQuDA_GetU16(in);

	EXPECT_EQ(exp_value, res);
}

TEST(RecordCodecTest, GetU16KeepsTheHighBit) {
	const uint16_t exp_value = 0x8001;

	const uint8_t in[2] = {0x01, 0x80};

	uint16_t res;

	res = HAQuDA_GetU16(in);

	EXPECT_EQ(exp_value, res);
}

TEST(RecordCodecTest, PutU32WritesLowByteFirst) {
	const uint8_t exp_bytes[4] = {0x78, 0x56, 0x34, 0x12};

	uint8_t out[4] = {};

	HAQuDA_PutU32(out, 0x12345678);

	EXPECT_EQ(0, memcmp(exp_bytes, out, sizeof(out)));
}

TEST(RecordCodecTest, PutU32LeavesTheNextByteAlone) {
	const uint8_t exp_next = 0xAA;

	uint8_t out[5] = {0, 0, 0, 0, exp_next};

	HAQuDA_PutU32(out, 0xFFFFFFFF);

	EXPECT_EQ(exp_next, out[4]);
}

TEST(RecordCodecTest, GetU32ReadsLowByteFirst) {
	const uint32_t exp_value = 0x12345678;

	const uint8_t in[4] = {0x78, 0x56, 0x34, 0x12};

	uint32_t res;

	res = HAQuDA_GetU32(in);

	EXPECT_EQ(exp_value, res);
}

TEST(RecordCodecTest, GetU32KeepsTheHighBit) {
	const uint32_t exp_value = 0x80000001u;

	const uint8_t in[4] = {0x01, 0x00, 0x00, 0x80};

	uint32_t res;

	res = HAQuDA_GetU32(in);

	EXPECT_EQ(exp_value, res);
}

TEST(LampKeysTest, ModeNameStandardIsTheKey) {
	const char *const exp_name = "standard";

	const char *res;

	res = HAQuDA_LampModeName(lamp_mode::standard);

	EXPECT_STREQ(exp_name, res);
}

TEST(LampKeysTest, ParseModeStandard) {
	const lamp_mode exp_mode = lamp_mode::standard;

	lamp_mode out = lamp_mode::light;

	bool res;

	res = HAQuDA_ParseLampMode("standard", out);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_mode, out);
}

TEST(LampKeysTest, ModeNameMultiIsTheKey) {
	const char *const exp_name = "multi";

	const char *res;

	res = HAQuDA_LampModeName(lamp_mode::multi);

	EXPECT_STREQ(exp_name, res);
}

TEST(LampKeysTest, ParseModeMulti) {
	const lamp_mode exp_mode = lamp_mode::multi;

	lamp_mode out = lamp_mode::light;

	bool res;

	res = HAQuDA_ParseLampMode("multi", out);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_mode, out);
}

TEST(LampKeysTest, ModeNameNightIsTheKey) {
	const char *const exp_name = "night";

	const char *res;

	res = HAQuDA_LampModeName(lamp_mode::night);

	EXPECT_STREQ(exp_name, res);
}

TEST(LampKeysTest, ParseModeNight) {
	const lamp_mode exp_mode = lamp_mode::night;

	lamp_mode out = lamp_mode::light;

	bool res;

	res = HAQuDA_ParseLampMode("night", out);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_mode, out);
}

TEST(LampKeysTest, ModeNameLightIsTheKey) {
	const char *const exp_name = "light";

	const char *res;

	res = HAQuDA_LampModeName(lamp_mode::light);

	EXPECT_STREQ(exp_name, res);
}

TEST(LampKeysTest, ParseModeLight) {
	const lamp_mode exp_mode = lamp_mode::light;

	lamp_mode out = lamp_mode::standard;

	bool res;

	res = HAQuDA_ParseLampMode("light", out);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_mode, out);
}

TEST(LampKeysTest, ParamNameTotalIsTheKey) {
	const char *const exp_name = "total";

	const char *res;

	res = HAQuDA_LampParamName(lamp_param::total);

	EXPECT_STREQ(exp_name, res);
}

TEST(LampKeysTest, ParseParamTotal) {
	const lamp_param exp_param = lamp_param::total;

	lamp_param out = lamp_param::tvoc;

	bool res;

	res = HAQuDA_ParseLampParam("total", out);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_param, out);
}

TEST(LampKeysTest, ParamNameTempIsTheKey) {
	const char *const exp_name = "temp";

	const char *res;

	res = HAQuDA_LampParamName(lamp_param::temp);

	EXPECT_STREQ(exp_name, res);
}

TEST(LampKeysTest, ParseParamTemp) {
	const lamp_param exp_param = lamp_param::temp;

	lamp_param out = lamp_param::tvoc;

	bool res;

	res = HAQuDA_ParseLampParam("temp", out);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_param, out);
}

TEST(LampKeysTest, ParamNameHumidIsTheKey) {
	const char *const exp_name = "humid";

	const char *res;

	res = HAQuDA_LampParamName(lamp_param::humid);

	EXPECT_STREQ(exp_name, res);
}

TEST(LampKeysTest, ParseParamHumid) {
	const lamp_param exp_param = lamp_param::humid;

	lamp_param out = lamp_param::tvoc;

	bool res;

	res = HAQuDA_ParseLampParam("humid", out);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_param, out);
}

TEST(LampKeysTest, ParamNameEco2IsTheKey) {
	const char *const exp_name = "eco2";

	const char *res;

	res = HAQuDA_LampParamName(lamp_param::eco2);

	EXPECT_STREQ(exp_name, res);
}

TEST(LampKeysTest, ParseParamEco2) {
	const lamp_param exp_param = lamp_param::eco2;

	lamp_param out = lamp_param::tvoc;

	bool res;

	res = HAQuDA_ParseLampParam("eco2", out);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_param, out);
}

TEST(LampKeysTest, ParamNameTvocIsTheKey) {
	const char *const exp_name = "tvoc";

	const char *res;

	res = HAQuDA_LampParamName(lamp_param::tvoc);

	EXPECT_STREQ(exp_name, res);
}

TEST(LampKeysTest, ParseParamTvoc) {
	const lamp_param exp_param = lamp_param::tvoc;

	lamp_param out = lamp_param::total;

	bool res;

	res = HAQuDA_ParseLampParam("tvoc", out);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_param, out);
}

TEST(LampKeysTest, ParamNamePm25IsTheKey) {
	const char *const exp_name = "pm25";

	const char *res;

	res = HAQuDA_LampParamName(lamp_param::pm25);

	EXPECT_STREQ(exp_name, res);
}

TEST(LampKeysTest, ParseParamPm25) {
	const lamp_param exp_param = lamp_param::pm25;

	lamp_param out = lamp_param::tvoc;

	bool res;

	res = HAQuDA_ParseLampParam("pm25", out);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_param, out);
}

TEST(LampKeysTest, ModeNameOutOfRangeFallsBackToStandard) {
	const char *const exp_name = "standard";
	const uint8_t outOfRangeMode = 9;

	const char *res;

	res = HAQuDA_LampModeName(static_cast<lamp_mode>(outOfRangeMode));

	EXPECT_STREQ(exp_name, res);
}

TEST(LampKeysTest, ParamNameOutOfRangeFallsBackToTotal) {
	const char *const exp_name = "total";
	const uint8_t outOfRangeParam = 9;

	const char *res;

	res = HAQuDA_LampParamName(static_cast<lamp_param>(outOfRangeParam));

	EXPECT_STREQ(exp_name, res);
}

TEST(LampKeysTest, ParseModeWrongCaseIsRefusedAndKeepsOutput) {
	const lamp_mode exp_mode = lamp_mode::multi;

	lamp_mode out = lamp_mode::multi;

	bool res;

	res = HAQuDA_ParseLampMode("Night", out);

	EXPECT_FALSE(res);
	EXPECT_EQ(exp_mode, out);
}

TEST(LampKeysTest, ParseModeEmptyIsRefusedAndKeepsOutput) {
	const lamp_mode exp_mode = lamp_mode::multi;

	lamp_mode out = lamp_mode::multi;

	bool res;

	res = HAQuDA_ParseLampMode("", out);

	EXPECT_FALSE(res);
	EXPECT_EQ(exp_mode, out);
}

TEST(LampKeysTest, ParseModeParamKeyIsRefusedAndKeepsOutput) {
	const lamp_mode exp_mode = lamp_mode::multi;

	lamp_mode out = lamp_mode::multi;

	bool res;

	res = HAQuDA_ParseLampMode("temp", out);

	EXPECT_FALSE(res);
	EXPECT_EQ(exp_mode, out);
}

TEST(LampKeysTest, ParseModeTrailingSpaceIsRefusedAndKeepsOutput) {
	const lamp_mode exp_mode = lamp_mode::multi;

	lamp_mode out = lamp_mode::multi;

	bool res;

	res = HAQuDA_ParseLampMode("night ", out);

	EXPECT_FALSE(res);
	EXPECT_EQ(exp_mode, out);
}

TEST(LampKeysTest, ParseModeNullIsRefusedAndKeepsOutput) {
	const lamp_mode exp_mode = lamp_mode::multi;

	lamp_mode out = lamp_mode::multi;

	bool res;

	res = HAQuDA_ParseLampMode(nullptr, out);

	EXPECT_FALSE(res);
	EXPECT_EQ(exp_mode, out);
}

TEST(LampKeysTest, ParseParamPm1IsRefusedAndKeepsOutput) {
	const lamp_param exp_param = lamp_param::eco2;

	lamp_param out = lamp_param::eco2;

	bool res;

	res = HAQuDA_ParseLampParam("pm1", out);

	EXPECT_FALSE(res);
	EXPECT_EQ(exp_param, out);
}

TEST(LampKeysTest, ParseParamPm10IsRefusedAndKeepsOutput) {
	const lamp_param exp_param = lamp_param::eco2;

	lamp_param out = lamp_param::eco2;

	bool res;

	res = HAQuDA_ParseLampParam("pm10", out);

	EXPECT_FALSE(res);
	EXPECT_EQ(exp_param, out);
}

TEST(LampKeysTest, ParseParamWrongCaseIsRefusedAndKeepsOutput) {
	const lamp_param exp_param = lamp_param::eco2;

	lamp_param out = lamp_param::eco2;

	bool res;

	res = HAQuDA_ParseLampParam("PM25", out);

	EXPECT_FALSE(res);
	EXPECT_EQ(exp_param, out);
}

TEST(LampKeysTest, ParseParamEmptyIsRefusedAndKeepsOutput) {
	const lamp_param exp_param = lamp_param::eco2;

	lamp_param out = lamp_param::eco2;

	bool res;

	res = HAQuDA_ParseLampParam("", out);

	EXPECT_FALSE(res);
	EXPECT_EQ(exp_param, out);
}

TEST(LampKeysTest, ParseParamModeKeyIsRefusedAndKeepsOutput) {
	const lamp_param exp_param = lamp_param::eco2;

	lamp_param out = lamp_param::eco2;

	bool res;

	res = HAQuDA_ParseLampParam("night", out);

	EXPECT_FALSE(res);
	EXPECT_EQ(exp_param, out);
}

TEST(LampKeysTest, ParseParamNullIsRefusedAndKeepsOutput) {
	const lamp_param exp_param = lamp_param::eco2;

	lamp_param out = lamp_param::eco2;

	bool res;

	res = HAQuDA_ParseLampParam(nullptr, out);

	EXPECT_FALSE(res);
	EXPECT_EQ(exp_param, out);
}

TEST(ButtonsSettingsTest, SameOfEqualSettingsIsTrue) {
	const buttons_settings a = HAQuDA_ButtonsDefaults();
	buttons_settings b = HAQuDA_ButtonsDefaults();

	bool res;

	res = HAQuDA_ButtonsSame(a, b);

	EXPECT_TRUE(res);
}

TEST(ButtonsSettingsTest, SameDifferentLockedIsFalse) {
	const buttons_settings a = HAQuDA_ButtonsDefaults();
	buttons_settings b = HAQuDA_ButtonsDefaults();
	b.locked = true;

	bool res;

	res = HAQuDA_ButtonsSame(a, b);

	EXPECT_FALSE(res);
}

TEST(ButtonsSettingsTest, SameDifferentActionIsFalse) {
	const buttons_settings a = HAQuDA_ButtonsDefaults();
	buttons_settings b = HAQuDA_ButtonsDefaults();
	b.buttons[0].action = button_action::none;

	bool res;

	res = HAQuDA_ButtonsSame(a, b);

	EXPECT_FALSE(res);
}

TEST(ButtonsSettingsTest, SameDifferentModeIsFalse) {
	const buttons_settings a = HAQuDA_ButtonsDefaults();
	buttons_settings b = HAQuDA_ButtonsDefaults();
	b.buttons[0].mode = lamp_mode::multi;

	bool res;

	res = HAQuDA_ButtonsSame(a, b);

	EXPECT_FALSE(res);
}

TEST(ButtonsSettingsTest, SameDifferentParamIsFalse) {
	const buttons_settings a = HAQuDA_ButtonsDefaults();
	buttons_settings b = HAQuDA_ButtonsDefaults();
	b.buttons[0].param = lamp_param::eco2;

	bool res;

	res = HAQuDA_ButtonsSame(a, b);

	EXPECT_FALSE(res);
}

TEST(ButtonsSettingsTest, SameDifferenceInTheLastButtonIsFalse) {
	const buttons_settings a = HAQuDA_ButtonsDefaults();
	buttons_settings b = HAQuDA_ButtonsDefaults();
	b.buttons[TOUCH_BUTTONS_NUM - 1].action = button_action::none;

	bool res;

	res = HAQuDA_ButtonsSame(a, b);

	EXPECT_FALSE(res);
}

TEST(ButtonsSettingsTest, JsonWorstCaseWithMaxSaveErrIsExactText) {
	const char *const exp_json = "{\"locked\":false,\"buttons\":{\"2\":{\"a\":\"mode\",\"arg\":\"standard\"},\"6\":{\"a\":\"mode\",\"arg\":\"standard\"},\"8\":"
								 "{\"a\":\"mode\",\"arg\":\"standard\"},\"9\":{\"a\":\"mode\",\"arg\":\"standard\"}},\"saveErr\":4294967295}";

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0] = {button_action::mode, lamp_mode::standard, lamp_param::total};
	s.buttons[1] = {button_action::mode, lamp_mode::standard, lamp_param::total};
	s.buttons[2] = {button_action::mode, lamp_mode::standard, lamp_param::total};
	s.buttons[3] = {button_action::mode, lamp_mode::standard, lamp_param::total};
	char buf[BUTTONS_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildButtonsJson(s, UINT32_MAX, buf, sizeof(buf));

	EXPECT_STREQ(exp_json, buf);
	EXPECT_EQ(strlen(exp_json), res);
}

TEST(ButtonsSettingsTest, JsonWorstCaseWithMaxSaveErrFitsInABufferOfLengthPlusOne) {
	const char *const exp_json = "{\"locked\":false,\"buttons\":{\"2\":{\"a\":\"mode\",\"arg\":\"standard\"},\"6\":{\"a\":\"mode\",\"arg\":\"standard\"},\"8\":"
								 "{\"a\":\"mode\",\"arg\":\"standard\"},\"9\":{\"a\":\"mode\",\"arg\":\"standard\"}},\"saveErr\":4294967295}";

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0] = {button_action::mode, lamp_mode::standard, lamp_param::total};
	s.buttons[1] = {button_action::mode, lamp_mode::standard, lamp_param::total};
	s.buttons[2] = {button_action::mode, lamp_mode::standard, lamp_param::total};
	s.buttons[3] = {button_action::mode, lamp_mode::standard, lamp_param::total};
	char buf[BUTTONS_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildButtonsJson(s, UINT32_MAX, buf, strlen(exp_json) + 1);

	EXPECT_STREQ(exp_json, buf);
	EXPECT_EQ(strlen(exp_json), res);
}

TEST(ButtonsSettingsTest, JsonWorstCaseWithMaxSaveErrInABufferOfExactLengthGivesZero) {
	const char *const json = "{\"locked\":false,\"buttons\":{\"2\":{\"a\":\"mode\",\"arg\":\"standard\"},\"6\":{\"a\":\"mode\",\"arg\":\"standard\"},\"8\":{"
							 "\"a\":\"mode\",\"arg\":\"standard\"},\"9\":{\"a\":\"mode\",\"arg\":\"standard\"}},\"saveErr\":4294967295}";
	const size_t exp_res = 0;

	buttons_settings s = HAQuDA_ButtonsDefaults();
	s.buttons[0] = {button_action::mode, lamp_mode::standard, lamp_param::total};
	s.buttons[1] = {button_action::mode, lamp_mode::standard, lamp_param::total};
	s.buttons[2] = {button_action::mode, lamp_mode::standard, lamp_param::total};
	s.buttons[3] = {button_action::mode, lamp_mode::standard, lamp_param::total};
	char buf[BUTTONS_JSON_BUF_LEN];

	size_t res;

	res = HAQuDA_BuildButtonsJson(s, UINT32_MAX, buf, strlen(json));

	EXPECT_EQ(exp_res, res);
}
