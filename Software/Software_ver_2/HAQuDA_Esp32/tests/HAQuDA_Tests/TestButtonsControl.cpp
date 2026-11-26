#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <vector>

#include "HAQuDA_ButtonsControl.h"
#include "HAQuDA_ButtonsSettings.h"
#include "MockFileStorage.h"
#include "MockLogger.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::ElementsAreArray;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::SetArrayArgument;
using ::testing::StrEq;
using ::testing::StrictMock;

TEST(ButtonsControlTest, LoadMissingFileKeepsDefaultsWithoutLog) {
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	char exp_json[BUTTONS_JSON_BUF_LEN];
	HAQuDA_BuildButtonsJson(exp_settings, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);

	char res_json[BUTTONS_JSON_BUF_LEN];

	EXPECT_CALL(files, Exists(StrEq("/buttons"))).WillOnce(Return(false));

	control.Load();

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(ButtonsControlTest, LoadGoodRecordShowsLoadedSettingsInJsonAndLogsInfo) {
	const int record_size = static_cast<int>(BUTTONS_RECORD_LEN);

	buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	exp_settings.buttons[1] = {button_action::show, lamp_mode::standard, lamp_param::eco2};
	exp_settings.locked = true;
	char exp_json[BUTTONS_JSON_BUF_LEN];
	HAQuDA_BuildButtonsJson(exp_settings, 0, exp_json, sizeof(exp_json));
	uint8_t record[BUTTONS_RECORD_LEN];
	HAQuDA_EncodeButtonsRecord(exp_settings, record, sizeof(record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);

	char res_json[BUTTONS_JSON_BUF_LEN];

	EXPECT_CALL(files, Exists(StrEq("/buttons"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/buttons"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/buttons"), _, BUTTONS_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(record, record + BUTTONS_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Buttons settings loaded")));

	control.Load();

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(ButtonsControlTest, LoadGoodRecordGivesTheLoadedBindingsAndLockInCopySettings) {
	const int record_size = static_cast<int>(BUTTONS_RECORD_LEN);

	buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	exp_settings.buttons[1] = {button_action::show, lamp_mode::standard, lamp_param::eco2};
	exp_settings.buttons[3] = {button_action::mode, lamp_mode::night, lamp_param::total};
	exp_settings.locked = true;
	uint8_t record[BUTTONS_RECORD_LEN];
	HAQuDA_EncodeButtonsRecord(exp_settings, record, sizeof(record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);

	buttons_settings res_settings;

	EXPECT_CALL(files, Exists(StrEq("/buttons"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/buttons"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/buttons"), _, BUTTONS_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(record, record + BUTTONS_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Buttons settings loaded")));

	control.Load();

	res_settings = control.CopySettings();
	EXPECT_TRUE(res_settings.locked);
	EXPECT_EQ(button_action::show, res_settings.buttons[1].action);
	EXPECT_EQ(lamp_param::eco2, res_settings.buttons[1].param);
	EXPECT_EQ(button_action::mode, res_settings.buttons[3].action);
	EXPECT_EQ(lamp_mode::night, res_settings.buttons[3].mode);
}

TEST(ButtonsControlTest, LoadWrongFileSizeGivesDefaultsAndLogsError) {
	const int wrong_size = static_cast<int>(BUTTONS_RECORD_LEN) + 1;

	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	char exp_json[BUTTONS_JSON_BUF_LEN];
	HAQuDA_BuildButtonsJson(exp_settings, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);

	char res_json[BUTTONS_JSON_BUF_LEN];

	EXPECT_CALL(files, Exists(StrEq("/buttons"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/buttons"))).WillOnce(Return(wrong_size));
	EXPECT_CALL(logger, LogError(StrEq("Buttons settings bad, defaults")));

	control.Load();

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(ButtonsControlTest, LoadEmptyFileGivesDefaultsAndLogsError) {
	const int empty_size = 0;

	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	char exp_json[BUTTONS_JSON_BUF_LEN];
	HAQuDA_BuildButtonsJson(exp_settings, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);

	char res_json[BUTTONS_JSON_BUF_LEN];

	EXPECT_CALL(files, Exists(StrEq("/buttons"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/buttons"))).WillOnce(Return(empty_size));
	EXPECT_CALL(logger, LogError(StrEq("Buttons settings bad, defaults")));

	control.Load();

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(ButtonsControlTest, LoadFailedReadGivesDefaultsAndLogsError) {
	const int record_size = static_cast<int>(BUTTONS_RECORD_LEN);

	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	char exp_json[BUTTONS_JSON_BUF_LEN];
	HAQuDA_BuildButtonsJson(exp_settings, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);

	char res_json[BUTTONS_JSON_BUF_LEN];

	EXPECT_CALL(files, Exists(StrEq("/buttons"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/buttons"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/buttons"), _, BUTTONS_RECORD_LEN)).WillOnce(Return(false));
	EXPECT_CALL(logger, LogError(StrEq("Buttons settings bad, defaults")));

	control.Load();

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(ButtonsControlTest, LoadCorruptChecksumGivesDefaultsAndLogsError) {
	const int record_size = static_cast<int>(BUTTONS_RECORD_LEN);

	buttons_settings locked_settings = HAQuDA_ButtonsDefaults();
	locked_settings.locked = true;
	uint8_t record[BUTTONS_RECORD_LEN];
	HAQuDA_EncodeButtonsRecord(locked_settings, record, sizeof(record));
	record[BUTTONS_RECORD_LEN - 1] ^= 0xFF;

	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	char exp_json[BUTTONS_JSON_BUF_LEN];
	HAQuDA_BuildButtonsJson(exp_settings, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);

	char res_json[BUTTONS_JSON_BUF_LEN];

	EXPECT_CALL(files, Exists(StrEq("/buttons"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/buttons"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/buttons"), _, BUTTONS_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(record, record + BUTTONS_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogError(StrEq("Buttons settings bad, defaults")));

	control.Load();

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(ButtonsControlTest, SubmitOkReturnsOkAndChangesTheSettings) {
	buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	exp_settings.buttons[0] = {button_action::show, lamp_mode::standard, lamp_param::eco2};
	char exp_json[BUTTONS_JSON_BUF_LEN];
	HAQuDA_BuildButtonsJson(exp_settings, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	request.action[0] = "show";
	request.arg[0] = "eco2";

	buttons_update_result res;
	char res_json[BUTTONS_JSON_BUF_LEN];

	res = control.Submit(request);

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_EQ(buttons_update_result::ok, res);
	EXPECT_STREQ(exp_json, res_json);
}

TEST(ButtonsControlTest, SubmitFieldTooLongReturnsItAndKeepsTheSettings) {
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	char exp_json[BUTTONS_JSON_BUF_LEN];
	HAQuDA_BuildButtonsJson(exp_settings, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	request.action[0] = "none";
	request.arg[1] = "seventeen_chars_x";

	buttons_update_result res;
	char res_json[BUTTONS_JSON_BUF_LEN];

	res = control.Submit(request);

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_EQ(buttons_update_result::field_too_long, res);
	EXPECT_STREQ(exp_json, res_json);
}

TEST(ButtonsControlTest, SubmitOfFormFieldsModeNightOnTheSecondButtonChangesOnlyIt) {
	buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	exp_settings.buttons[1] = {button_action::mode, lamp_mode::night, lamp_param::total};
	char exp_json[BUTTONS_JSON_BUF_LEN];
	HAQuDA_BuildButtonsJson(exp_settings, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	HAQuDA_SetButtonsField(request, "a6", "mode");
	HAQuDA_SetButtonsField(request, "arg6", "night");

	buttons_update_result res;
	char res_json[BUTTONS_JSON_BUF_LEN];

	res = control.Submit(request);

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_EQ(buttons_update_result::ok, res);
	EXPECT_STREQ(exp_json, res_json);
}

TEST(ButtonsControlTest, SubmitOfFormFieldsWithARepeatedFieldUsesTheFirstValue) {
	buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	exp_settings.buttons[0] = {button_action::none, lamp_mode::standard, lamp_param::total};
	char exp_json[BUTTONS_JSON_BUF_LEN];
	HAQuDA_BuildButtonsJson(exp_settings, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	HAQuDA_SetButtonsField(request, "a2", "none");
	HAQuDA_SetButtonsField(request, "a2", "up");

	buttons_update_result res;
	char res_json[BUTTONS_JSON_BUF_LEN];

	res = control.Submit(request);

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_EQ(buttons_update_result::ok, res);
	EXPECT_STREQ(exp_json, res_json);
}

TEST(ButtonsControlTest, SubmitOfFormFieldsIgnoresAnUnknownFieldBesideReset) {
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	char exp_json[BUTTONS_JSON_BUF_LEN];
	HAQuDA_BuildButtonsJson(exp_settings, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request change = {};
	change.action[0] = "none";
	control.Submit(change);
	buttons_request request = {};
	HAQuDA_SetButtonsField(request, "extra", "1");
	HAQuDA_SetButtonsField(request, "reset", "1");

	buttons_update_result res;
	char res_json[BUTTONS_JSON_BUF_LEN];

	res = control.Submit(request);

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_EQ(buttons_update_result::ok, res);
	EXPECT_STREQ(exp_json, res_json);
}

TEST(ButtonsControlTest, SubmitResetRestoresTheBindingsAndKeepsTheLock) {
	buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	exp_settings.locked = true;
	char exp_json[BUTTONS_JSON_BUF_LEN];
	HAQuDA_BuildButtonsJson(exp_settings, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request change = {};
	change.action[0] = "down";
	change.action[2] = "none";
	control.Submit(change);
	control.SetLocked(true);
	buttons_request request = {};
	request.reset = "1";

	buttons_update_result res;
	char res_json[BUTTONS_JSON_BUF_LEN];

	res = control.Submit(request);

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_EQ(buttons_update_result::ok, res);
	EXPECT_STREQ(exp_json, res_json);
}

TEST(ButtonsControlTest, SubmitBadActionReturnsItAndKeepsTheSettings) {
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	char exp_json[BUTTONS_JSON_BUF_LEN];
	HAQuDA_BuildButtonsJson(exp_settings, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	request.action[0] = "bogus";

	buttons_update_result res;
	char res_json[BUTTONS_JSON_BUF_LEN];

	res = control.Submit(request);

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_EQ(buttons_update_result::bad_action, res);
	EXPECT_STREQ(exp_json, res_json);
}

TEST(ButtonsControlTest, SubmitModeWithoutArgumentReturnsBadArgAndKeepsTheSettings) {
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	char exp_json[BUTTONS_JSON_BUF_LEN];
	HAQuDA_BuildButtonsJson(exp_settings, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	request.action[0] = "mode";

	buttons_update_result res;
	char res_json[BUTTONS_JSON_BUF_LEN];

	res = control.Submit(request);

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_EQ(buttons_update_result::bad_arg, res);
	EXPECT_STREQ(exp_json, res_json);
}

TEST(ButtonsControlTest, SubmitResetWithAnotherFieldReturnsItAndKeepsTheSettings) {
	buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	exp_settings.buttons[0] = {button_action::down, lamp_mode::standard, lamp_param::total};
	char exp_json[BUTTONS_JSON_BUF_LEN];
	HAQuDA_BuildButtonsJson(exp_settings, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request change = {};
	change.action[0] = "down";
	control.Submit(change);
	buttons_request request = {};
	request.reset = "1";
	request.locked = "1";

	buttons_update_result res;
	char res_json[BUTTONS_JSON_BUF_LEN];

	res = control.Submit(request);

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_EQ(buttons_update_result::reset_with_fields, res);
	EXPECT_STREQ(exp_json, res_json);
}

TEST(ButtonsControlTest, SubmitWithAValidAndABadFieldChangesNothing) {
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	char exp_json[BUTTONS_JSON_BUF_LEN];
	HAQuDA_BuildButtonsJson(exp_settings, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	request.action[0] = "down";
	request.action[1] = "bogus";

	buttons_update_result res;
	char res_json[BUTTONS_JSON_BUF_LEN];

	res = control.Submit(request);

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_EQ(buttons_update_result::bad_action, res);
	EXPECT_STREQ(exp_json, res_json);
}

TEST(ButtonsControlTest, SubmitWithNothingToChangeReturnsNothing) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};

	buttons_update_result res;

	res = control.Submit(request);

	EXPECT_EQ(buttons_update_result::nothing, res);
}

TEST(ButtonsControlTest, HandleAfterRefusedSubmitWritesNothing) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	request.action[0] = "bogus";
	control.Submit(request);

	control.Handle();
}

TEST(ButtonsControlTest, HandleAfterSubmitWithNothingToChangeWritesNothing) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	control.Submit(request);

	control.Handle();
}

TEST(ButtonsControlTest, HandleWithoutPendingChangeWritesNothing) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);

	control.Handle();
}

TEST(ButtonsControlTest, HandleAfterSubmitWritesTheRecord) {
	buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	exp_settings.buttons[0] = {button_action::show, lamp_mode::standard, lamp_param::eco2};
	uint8_t exp_record[BUTTONS_RECORD_LEN];
	HAQuDA_EncodeButtonsRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	request.action[0] = "show";
	request.arg[0] = "eco2";
	control.Submit(request);

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), ElementsAreArray(exp_record))).WillOnce(Return(true));

	control.Handle();
}

TEST(ButtonsControlTest, HandleAfterTwoSubmitsWritesTheMergedStateOnce) {
	buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	exp_settings.buttons[0] = {button_action::show, lamp_mode::standard, lamp_param::eco2};
	exp_settings.buttons[2] = {button_action::mode, lamp_mode::multi, lamp_param::total};
	uint8_t exp_record[BUTTONS_RECORD_LEN];
	HAQuDA_EncodeButtonsRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request first = {};
	first.action[0] = "show";
	first.arg[0] = "eco2";
	buttons_request second = {};
	second.action[2] = "mode";
	second.arg[2] = "multi";
	control.Submit(first);
	control.Submit(second);

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), ElementsAreArray(exp_record))).WillOnce(Return(true));

	control.Handle();
}

TEST(ButtonsControlTest, HandleSecondTimeWithoutNewChangeWritesNothing) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	request.action[0] = "down";
	control.Submit(request);

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), _)).WillOnce(Return(true));
	control.Handle();

	control.Handle();
}

TEST(ButtonsControlTest, HandleFailedWriteLogsError) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	request.locked = "1";
	control.Submit(request);

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), _)).WillOnce(Return(false));
	EXPECT_CALL(logger, LogError(StrEq("Buttons settings: save failed")));

	control.Handle();
}

TEST(ButtonsControlTest, HandleAfterFailedWriteAndANewChangeWritesOnlyTheNewChange) {
	buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	exp_settings.locked = true;
	uint8_t exp_record[BUTTONS_RECORD_LEN];
	HAQuDA_EncodeButtonsRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	request.action[0] = "down";
	control.Submit(request);

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), _)).WillOnce(Return(false));
	EXPECT_CALL(logger, LogError(StrEq("Buttons settings: save failed")));
	control.Handle();
	control.SetLocked(true);

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), ElementsAreArray(exp_record))).WillOnce(Return(true));

	control.Handle();
}

TEST(ButtonsControlTest, SetLockedTrueReturnsTrueAndLocks) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);

	bool res;

	res = control.SetLocked(true);

	EXPECT_TRUE(res);
	EXPECT_TRUE(control.CopySettings().locked);
}

TEST(ButtonsControlTest, SetLockedFalseOnLockedButtonsUnlocks) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	control.SetLocked(true);

	bool res;

	res = control.SetLocked(false);

	EXPECT_FALSE(res);
	EXPECT_FALSE(control.CopySettings().locked);
}

TEST(ButtonsControlTest, HandleAfterSetLockedWritesTheRecord) {
	buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	exp_settings.locked = true;
	uint8_t exp_record[BUTTONS_RECORD_LEN];
	HAQuDA_EncodeButtonsRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	control.SetLocked(true);

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), ElementsAreArray(exp_record))).WillOnce(Return(true));

	control.Handle();
}

TEST(ButtonsControlTest, HandleAfterSetLockedToTheCurrentStateWritesNothing) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	control.SetLocked(false);

	control.Handle();
}

TEST(ButtonsControlTest, HandleAfterRepeatedSetLockedWritesNothingMore) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	control.SetLocked(true);

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), _)).WillOnce(Return(true));
	control.Handle();
	control.SetLocked(true);

	control.Handle();
}

TEST(ButtonsControlTest, ToggleLockedOnUnlockedButtonsReturnsTrue) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);

	bool res;

	res = control.ToggleLocked();

	EXPECT_TRUE(res);
	EXPECT_TRUE(control.CopySettings().locked);
}

TEST(ButtonsControlTest, ToggleLockedOnLockedButtonsReturnsFalse) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	control.ToggleLocked();

	bool res;

	res = control.ToggleLocked();

	EXPECT_FALSE(res);
	EXPECT_FALSE(control.CopySettings().locked);
}

TEST(ButtonsControlTest, HandleAfterTwoTogglesBackToTheStoredStateWritesNothing) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	control.ToggleLocked();
	control.ToggleLocked();

	control.Handle();
}

TEST(ButtonsControlTest, HandleAfterSubmitAndSetLockedWritesBothChanges) {
	buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	exp_settings.buttons[2] = {button_action::mode, lamp_mode::multi, lamp_param::total};
	exp_settings.locked = true;
	uint8_t exp_record[BUTTONS_RECORD_LEN];
	HAQuDA_EncodeButtonsRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	request.action[2] = "mode";
	request.arg[2] = "multi";
	control.Submit(request);
	control.SetLocked(true);

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), ElementsAreArray(exp_record))).WillOnce(Return(true));

	control.Handle();
}

TEST(ButtonsControlTest, SubmitAfterSetLockedKeepsTheLock) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	control.SetLocked(true);
	buttons_request request = {};
	request.action[0] = "down";

	buttons_update_result res;

	res = control.Submit(request);

	EXPECT_EQ(buttons_update_result::ok, res);
	EXPECT_TRUE(control.CopySettings().locked);
}

TEST(ButtonsControlTest, SubmitCanUnlockTheButtons) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	control.SetLocked(true);
	buttons_request request = {};
	request.locked = "0";

	buttons_update_result res;

	res = control.Submit(request);

	EXPECT_EQ(buttons_update_result::ok, res);
	EXPECT_FALSE(control.CopySettings().locked);
}

TEST(ButtonsControlTest, CopySettingsReflectsRepeatableBindings) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	request.action[0] = "up";
	request.action[1] = "power";
	control.Submit(request);

	buttons_settings res_settings;

	res_settings = control.CopySettings();

	EXPECT_TRUE(HAQuDA_ButtonRepeatable(res_settings.buttons[0]));
	EXPECT_FALSE(HAQuDA_ButtonRepeatable(res_settings.buttons[1]));
}

TEST(ButtonsControlTest, BuildJsonOfTheDefaultsIsExactText) {
	const char exp_json[]
		= "{\"locked\":false,\"buttons\":{\"2\":{\"a\":\"power\"},\"6\":{\"a\":\"up\"},\"8\":{\"a\":\"next\"},\"9\":{\"a\":\"down\"}},\"saveErr\":0}";
	const size_t exp_len = sizeof(exp_json) - 1;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);

	char res_json[BUTTONS_JSON_BUF_LEN];
	size_t res_len;

	res_len = control.BuildJson(res_json, sizeof(res_json));

	EXPECT_EQ(exp_len, res_len);
	EXPECT_STREQ(exp_json, res_json);
}

TEST(ButtonsControlTest, BuildJsonOfAChangedStateIsExactText) {
	const char exp_json[]
		= "{\"locked\":true,\"buttons\":{\"2\":{\"a\":\"power\"},\"6\":{\"a\":\"show\",\"arg\":\"eco2\"},\"8\":{\"a\":\"next\"},\"9\":{\"a\":\"mode\","
		  "\"arg\":\"night\"}},\"saveErr\":0}";
	const size_t exp_len = sizeof(exp_json) - 1;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	request.action[1] = "show";
	request.arg[1] = "eco2";
	request.action[3] = "mode";
	request.arg[3] = "night";
	request.locked = "1";
	control.Submit(request);

	char res_json[BUTTONS_JSON_BUF_LEN];
	size_t res_len;

	res_len = control.BuildJson(res_json, sizeof(res_json));

	EXPECT_EQ(exp_len, res_len);
	EXPECT_STREQ(exp_json, res_json);
}

TEST(ButtonsControlTest, BuildJsonIntoABufferOfExactlyTheLengthPlusTerminatorFits) {
	const char exp_json[]
		= "{\"locked\":false,\"buttons\":{\"2\":{\"a\":\"power\"},\"6\":{\"a\":\"up\"},\"8\":{\"a\":\"next\"},\"9\":{\"a\":\"down\"}},\"saveErr\":0}";
	const size_t exp_len = sizeof(exp_json) - 1;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);

	char res_json[sizeof(exp_json)];
	size_t res_len;

	res_len = control.BuildJson(res_json, sizeof(res_json));

	EXPECT_EQ(exp_len, res_len);
	EXPECT_STREQ(exp_json, res_json);
}

TEST(ButtonsControlTest, BuildJsonIntoABufferOfExactlyTheLengthReturnsZero) {
	const size_t exp_len = 0;
	const size_t json_len = std::strlen(
		"{\"locked\":false,\"buttons\":{\"2\":{\"a\":\"power\"},\"6\":{\"a\":\"up\"},\"8\":{\"a\":\"next\"},\"9\":{\"a\":\"down\"}},\"saveErr\":0}");

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);

	char res_json[BUTTONS_JSON_BUF_LEN];
	size_t res_len;

	res_len = control.BuildJson(res_json, json_len);

	EXPECT_EQ(exp_len, res_len);
}

TEST(ButtonsControlTest, BuildJsonTooSmallBufferReturnsZero) {
	const size_t exp_len = 0;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);

	char res_json[8];
	size_t res_len;

	res_len = control.BuildJson(res_json, sizeof(res_json));

	EXPECT_EQ(exp_len, res_len);
}

TEST(ButtonsControlTest, HandleFailedWriteShowsSaveErrOneInJson) {
	const char exp_json[]
		= "{\"locked\":false,\"buttons\":{\"2\":{\"a\":\"power\"},\"6\":{\"a\":\"up\"},\"8\":{\"a\":\"next\"},\"9\":{\"a\":\"down\"}},\"saveErr\":1}";

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	request.action[0] = "down";
	control.Submit(request);

	char res_json[BUTTONS_JSON_BUF_LEN];

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), _)).WillOnce(Return(false));
	EXPECT_CALL(logger, LogError(StrEq("Buttons settings: save failed")));

	control.Handle();

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(ButtonsControlTest, HandleFailedWriteRevertsTheSettingsToTheStoredOnes) {
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	request.action[0] = "down";
	control.Submit(request);

	buttons_settings res_settings;

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), _)).WillOnce(Return(false));
	EXPECT_CALL(logger, LogError(StrEq("Buttons settings: save failed")));

	control.Handle();

	res_settings = control.CopySettings();
	EXPECT_TRUE(HAQuDA_ButtonsSame(exp_settings, res_settings));
}

TEST(ButtonsControlTest, HandleSecondTimeAfterFailedWriteWritesNothingAndKeepsTheCounterAtOne) {
	const char exp_json[]
		= "{\"locked\":false,\"buttons\":{\"2\":{\"a\":\"power\"},\"6\":{\"a\":\"up\"},\"8\":{\"a\":\"next\"},\"9\":{\"a\":\"down\"}},\"saveErr\":1}";

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	request.action[0] = "down";
	control.Submit(request);

	char res_json[BUTTONS_JSON_BUF_LEN];

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), _)).WillOnce(Return(false));
	EXPECT_CALL(logger, LogError(StrEq("Buttons settings: save failed")));
	control.Handle();

	control.Handle();

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(ButtonsControlTest, HandleTwoFailedWritesInARowShowSaveErrTwo) {
	const char exp_json[]
		= "{\"locked\":false,\"buttons\":{\"2\":{\"a\":\"power\"},\"6\":{\"a\":\"up\"},\"8\":{\"a\":\"next\"},\"9\":{\"a\":\"down\"}},\"saveErr\":2}";

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	request.action[0] = "down";
	control.Submit(request);
	buttons_request second = {};
	second.action[1] = "none";

	char res_json[BUTTONS_JSON_BUF_LEN];

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), _)).WillOnce(Return(false)).WillOnce(Return(false));
	EXPECT_CALL(logger, LogError(StrEq("Buttons settings: save failed"))).Times(2);
	control.Handle();
	control.Submit(second);

	control.Handle();

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(ButtonsControlTest, HandleFailedWriteAfterASuccessfulOneRevertsToTheSavedStateNotTheDefaults) {
	buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	exp_settings.buttons[0] = {button_action::down, lamp_mode::standard, lamp_param::total};

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	request.action[0] = "down";
	control.Submit(request);
	buttons_request second = {};
	second.action[1] = "none";

	buttons_settings res_settings;

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), _)).WillOnce(Return(true));
	control.Handle();
	control.Submit(second);
	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), _)).WillOnce(Return(false));
	EXPECT_CALL(logger, LogError(StrEq("Buttons settings: save failed")));

	control.Handle();

	res_settings = control.CopySettings();
	EXPECT_TRUE(HAQuDA_ButtonsSame(exp_settings, res_settings));
}

TEST(ButtonsControlTest, HandleChangeAfterTheRevertWritesOnTopOfTheSavedState) {
	buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	exp_settings.buttons[0] = {button_action::down, lamp_mode::standard, lamp_param::total};
	exp_settings.locked = true;
	uint8_t exp_record[BUTTONS_RECORD_LEN];
	HAQuDA_EncodeButtonsRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	request.action[0] = "down";
	control.Submit(request);
	buttons_request second = {};
	second.action[1] = "none";

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), _)).WillOnce(Return(true));
	control.Handle();
	control.Submit(second);
	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), _)).WillOnce(Return(false));
	EXPECT_CALL(logger, LogError(StrEq("Buttons settings: save failed")));
	control.Handle();
	control.SetLocked(true);

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), ElementsAreArray(exp_record))).WillOnce(Return(true));

	control.Handle();
}

TEST(ButtonsControlTest, HandleChangeAndChangeBackAfterASuccessfulWriteWritesNothing) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	request.action[0] = "down";
	control.Submit(request);
	buttons_request away = {};
	away.action[0] = "power";
	buttons_request back = {};
	back.action[0] = "down";

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), _)).WillOnce(Return(true));
	control.Handle();
	control.Submit(away);
	control.Submit(back);

	control.Handle();
}

TEST(ButtonsControlTest, HandleChangeBackToTheLoadedStateWritesNothing) {
	const int record_size = static_cast<int>(BUTTONS_RECORD_LEN);

	buttons_settings loaded_settings = HAQuDA_ButtonsDefaults();
	loaded_settings.locked = true;
	uint8_t record[BUTTONS_RECORD_LEN];
	HAQuDA_EncodeButtonsRecord(loaded_settings, record, sizeof(record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);

	EXPECT_CALL(files, Exists(StrEq("/buttons"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/buttons"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/buttons"), _, BUTTONS_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(record, record + BUTTONS_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Buttons settings loaded")));
	control.Load();
	control.SetLocked(false);
	control.SetLocked(true);

	control.Handle();
}

TEST(ButtonsControlTest, HandleDefaultsAfterLoadOfABadFileWritesNothing) {
	const int wrong_size = static_cast<int>(BUTTONS_RECORD_LEN) + 1;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	request.reset = "1";

	EXPECT_CALL(files, Exists(StrEq("/buttons"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/buttons"))).WillOnce(Return(wrong_size));
	EXPECT_CALL(logger, LogError(StrEq("Buttons settings bad, defaults")));
	control.Load();
	control.Submit(request);

	control.Handle();
}

TEST(ButtonsControlTest, HandleChangeOfOnlyTheModeWritesTheRecord) {
	buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	exp_settings.buttons[0] = {button_action::mode, lamp_mode::multi, lamp_param::total};
	uint8_t exp_record[BUTTONS_RECORD_LEN];
	HAQuDA_EncodeButtonsRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request first = {};
	first.action[0] = "mode";
	first.arg[0] = "standard";
	buttons_request second = {};
	second.arg[0] = "multi";

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), _)).WillOnce(Return(true));
	control.Submit(first);
	control.Handle();
	control.Submit(second);

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), ElementsAreArray(exp_record))).WillOnce(Return(true));

	control.Handle();
}

TEST(ButtonsControlTest, HandleChangeOfOnlyTheParamWritesTheRecord) {
	buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	exp_settings.buttons[0] = {button_action::show, lamp_mode::standard, lamp_param::tvoc};
	uint8_t exp_record[BUTTONS_RECORD_LEN];
	HAQuDA_EncodeButtonsRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request first = {};
	first.action[0] = "show";
	first.arg[0] = "eco2";
	buttons_request second = {};
	second.arg[0] = "tvoc";

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), _)).WillOnce(Return(true));
	control.Submit(first);
	control.Handle();
	control.Submit(second);

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), ElementsAreArray(exp_record))).WillOnce(Return(true));

	control.Handle();
}

TEST(ButtonsControlTest, HandleChangeOfOnlyTheLockWritesTheRecord) {
	buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	exp_settings.buttons[0] = {button_action::down, lamp_mode::standard, lamp_param::total};
	exp_settings.locked = true;
	uint8_t exp_record[BUTTONS_RECORD_LEN];
	HAQuDA_EncodeButtonsRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request first = {};
	first.action[0] = "down";
	buttons_request second = {};
	second.locked = "1";

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), _)).WillOnce(Return(true));
	control.Submit(first);
	control.Handle();
	control.Submit(second);

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), ElementsAreArray(exp_record))).WillOnce(Return(true));

	control.Handle();
}

TEST(ButtonsControlTest, HandleChangeOfOnlyTheActionWritesTheRecord) {
	buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	exp_settings.buttons[0] = {button_action::none, lamp_mode::standard, lamp_param::total};
	uint8_t exp_record[BUTTONS_RECORD_LEN];
	HAQuDA_EncodeButtonsRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request first = {};
	first.action[0] = "down";
	buttons_request second = {};
	second.action[0] = "none";

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), _)).WillOnce(Return(true));
	control.Submit(first);
	control.Handle();
	control.Submit(second);

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), ElementsAreArray(exp_record))).WillOnce(Return(true));

	control.Handle();
}

TEST(ButtonsControlTest, HandleChangeDuringAFailedWriteIsReverted) {
	const buttons_settings exp_settings = HAQuDA_ButtonsDefaults();

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	request.action[0] = "down";
	control.Submit(request);

	buttons_settings res_settings;

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), _)).WillOnce(Invoke([&control](const char *, const std::vector<uint8_t> &) {
		control.SetLocked(true);
		return false;
	}));
	EXPECT_CALL(logger, LogError(StrEq("Buttons settings: save failed")));

	control.Handle();

	res_settings = control.CopySettings();
	EXPECT_TRUE(HAQuDA_ButtonsSame(exp_settings, res_settings));
}

TEST(ButtonsControlTest, HandleAfterAChangeDuringAFailedWriteWritesNothingAndCounterIsOne) {
	const char exp_json[]
		= "{\"locked\":false,\"buttons\":{\"2\":{\"a\":\"power\"},\"6\":{\"a\":\"up\"},\"8\":{\"a\":\"next\"},\"9\":{\"a\":\"down\"}},\"saveErr\":1}";

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	request.action[0] = "down";
	control.Submit(request);

	char res_json[BUTTONS_JSON_BUF_LEN];

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), _)).WillOnce(Invoke([&control](const char *, const std::vector<uint8_t> &) {
		control.SetLocked(true);
		return false;
	}));
	EXPECT_CALL(logger, LogError(StrEq("Buttons settings: save failed")));
	control.Handle();

	control.Handle();

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(ButtonsControlTest, HandleAfterAChangeDuringASuccessfulWriteWritesTheChange) {
	buttons_settings exp_settings = HAQuDA_ButtonsDefaults();
	exp_settings.buttons[0] = {button_action::down, lamp_mode::standard, lamp_param::total};
	exp_settings.locked = true;
	uint8_t exp_record[BUTTONS_RECORD_LEN];
	HAQuDA_EncodeButtonsRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLogger> logger;
	HAQuDA_ButtonsControl control(files, logger);
	buttons_request request = {};
	request.action[0] = "down";
	control.Submit(request);

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), _)).WillOnce(Invoke([&control](const char *, const std::vector<uint8_t> &) {
		control.SetLocked(true);
		return true;
	}));
	control.Handle();

	EXPECT_CALL(files, WriteFileBytes(StrEq("/buttons"), ElementsAreArray(exp_record))).WillOnce(Return(true));

	control.Handle();
}
