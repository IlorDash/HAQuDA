#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>

#include "HAQuDA_LEDsParams.h"
#include "HAQuDA_LampControl.h"
#include "HAQuDA_LampSettings.h"
#include "MockFileStorage.h"
#include "MockLampDisplay.h"
#include "MockLogger.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::HasSubstr;
using ::testing::InSequence;
using ::testing::Not;
using ::testing::Return;
using ::testing::SetArrayArgument;
using ::testing::StrEq;
using ::testing::StrictMock;

TEST(LampControlTest, LoadMissingFileKeepsDefaultsWithoutLog) {
	const lamp_settings exp_settings = HAQuDA_LampDefaults();
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(exp_settings, false, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);

	char res_json[LAMP_JSON_BUF_LEN];

	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(false));

	control.Load();

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(LampControlTest, LoadGoodRecordLoadsSettingsAndLogsInfo) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);

	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.mode = lamp_mode::standard;
	exp_settings.standardParam = lamp_param::temp;
	exp_settings.brightPercent = 60;
	exp_settings.color = 3;
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(exp_settings, false, 0, exp_json, sizeof(exp_json));
	uint8_t record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, record, sizeof(record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);

	char res_json[LAMP_JSON_BUF_LEN];

	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN)).WillOnce(DoAll(SetArrayArgument<1>(record, record + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Lamp settings loaded")));

	control.Load();

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(LampControlTest, LoadWrongFileSizeGivesDefaultsAndLogsError) {
	const int wrong_size = static_cast<int>(LAMP_RECORD_LEN) + 1;

	const lamp_settings exp_settings = HAQuDA_LampDefaults();
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(exp_settings, false, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);

	char res_json[LAMP_JSON_BUF_LEN];

	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(wrong_size));
	EXPECT_CALL(logger, LogError(StrEq("Lamp settings bad, defaults")));

	control.Load();

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(LampControlTest, LoadFailedReadGivesDefaultsAndLogsError) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);

	const lamp_settings exp_settings = HAQuDA_LampDefaults();
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(exp_settings, false, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);

	char res_json[LAMP_JSON_BUF_LEN];

	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN)).WillOnce(Return(false));
	EXPECT_CALL(logger, LogError(StrEq("Lamp settings bad, defaults")));

	control.Load();

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(LampControlTest, LoadBadRecordGivesDefaultsAndLogsError) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);

	const lamp_settings exp_settings = HAQuDA_LampDefaults();
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(exp_settings, false, 0, exp_json, sizeof(exp_json));

	const uint8_t zeros[LAMP_RECORD_LEN] = {};

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);

	char res_json[LAMP_JSON_BUF_LEN];

	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN)).WillOnce(DoAll(SetArrayArgument<1>(zeros, zeros + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogError(StrEq("Lamp settings bad, defaults")));

	control.Load();

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(LampControlTest, SubmitValidRequestChangesSettingsAndTouchesNothing) {
	const lamp_update_result exp_result = lamp_update_result::ok;
	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.mode = lamp_mode::standard;
	exp_settings.standardParam = lamp_param::temp;
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(exp_settings, false, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	lamp_request request = {};
	request.mode = "standard";
	request.standardParam = "temp";

	lamp_update_result res;
	char res_json[LAMP_JSON_BUF_LEN];

	res = control.Submit(request);

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_EQ(exp_result, res);
	EXPECT_STREQ(exp_json, res_json);
}

TEST(LampControlTest, SubmitInvalidRequestIsRefusedAndKeepsSettings) {
	const lamp_update_result exp_result = lamp_update_result::bad_bright;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(exp_settings, false, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	lamp_request request = {};
	request.on = "0";
	request.bright = "7";

	lamp_update_result res;
	char res_json[LAMP_JSON_BUF_LEN];

	res = control.Submit(request);

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_EQ(exp_result, res);
	EXPECT_STREQ(exp_json, res_json);
}

TEST(LampControlTest, SubmitInvalidRequestIsNotSavedByHandle) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	lamp_request request = {};
	request.bright = "7";
	control.Submit(request);

	control.Handle(false, 0);
}

TEST(LampControlTest, SubmitNightWithoutTimeIsRefused) {
	const lamp_update_result exp_result = lamp_update_result::night_needs_time;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	lamp_request request = {};
	request.mode = "night";

	lamp_update_result res;

	control.Handle(false, 0);

	res = control.Submit(request);

	EXPECT_EQ(exp_result, res);
}

TEST(LampControlTest, SubmitNightWithTimeIsAccepted) {
	const lamp_update_result exp_result = lamp_update_result::ok;
	const uint16_t minute_of_day = 10 * 60;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	lamp_request request = {};
	request.mode = "night";

	lamp_update_result res;

	control.Handle(true, minute_of_day);

	res = control.Submit(request);

	EXPECT_EQ(exp_result, res);
}

TEST(LampControlTest, HandleSavesBeforeApplyingTheSameSettings) {
	const uint8_t bright = 60;
	const uint8_t color_index = 2;
	const uint32_t exp_color = HAQuDA_LampColor(color_index);
	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.mode = lamp_mode::light;
	exp_settings.brightPercent = bright;
	exp_settings.color = color_index;
	uint8_t exp_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	lamp_request request = {};
	request.bright = "60";
	request.mode = "light";
	request.color = "2";
	control.Submit(request);

	{
		InSequence seq;
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_record))).WillOnce(Return(true));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(bright));
		EXPECT_CALL(display, ShowColor(exp_color));
	}

	control.Handle(false, 0);
}

TEST(LampControlTest, HandleWithoutPendingChangeDoesNothing) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);

	control.Handle(false, 0);
}

TEST(LampControlTest, HandleFailedWriteLogsAndStillApplies) {
	const uint8_t bright = 60;
	const uint32_t exp_color = HAQuDA_LampColor(0);

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	lamp_request request = {};
	request.bright = "60";
	request.mode = "light";
	control.Submit(request);

	{
		InSequence seq;
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), _)).WillOnce(Return(false));
		EXPECT_CALL(logger, LogError(StrEq("Lamp settings: save failed")));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(bright));
		EXPECT_CALL(display, ShowColor(exp_color));
	}

	control.Handle(false, 0);
}

TEST(LampControlTest, HandleTwoSubmitsBeforeOnePassSavesAndAppliesTheNewest) {
	const uint8_t newest_bright = 80;
	const uint32_t exp_color = HAQuDA_LampColor(0);
	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.mode = lamp_mode::light;
	exp_settings.brightPercent = newest_bright;
	uint8_t exp_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	lamp_request first_request = {};
	first_request.bright = "20";
	first_request.mode = "light";
	lamp_request second_request = {};
	second_request.bright = "80";
	control.Submit(first_request);
	control.Submit(second_request);

	EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_record))).WillOnce(Return(true));
	EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
	EXPECT_CALL(display, SetBrightnessPercent(newest_bright));
	EXPECT_CALL(display, ShowColor(exp_color));

	control.Handle(false, 0);
}

TEST(LampControlTest, HandleSubmitAfterPassIsHandledOnTheNextPass) {
	const uint8_t first_bright = 20;
	const uint8_t second_bright = 80;
	const uint32_t exp_first_color = HAQuDA_LampColor(1);
	const uint32_t exp_second_color = HAQuDA_LampColor(4);
	lamp_settings exp_first_settings = HAQuDA_LampDefaults();
	exp_first_settings.mode = lamp_mode::light;
	exp_first_settings.brightPercent = first_bright;
	exp_first_settings.color = 1;
	uint8_t exp_first_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_first_settings, exp_first_record, sizeof(exp_first_record));
	lamp_settings exp_second_settings = exp_first_settings;
	exp_second_settings.brightPercent = second_bright;
	exp_second_settings.color = 4;
	uint8_t exp_second_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_second_settings, exp_second_record, sizeof(exp_second_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	lamp_request first_request = {};
	first_request.bright = "20";
	first_request.mode = "light";
	first_request.color = "1";
	lamp_request second_request = {};
	second_request.bright = "80";
	second_request.color = "4";

	EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_first_record))).WillOnce(Return(true));
	EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_second_record))).WillOnce(Return(true));
	EXPECT_CALL(display, ErrorShown()).Times(2).WillRepeatedly(Return(false));
	EXPECT_CALL(display, SetBrightnessPercent(first_bright));
	EXPECT_CALL(display, SetBrightnessPercent(second_bright));
	EXPECT_CALL(display, ShowColor(exp_first_color));
	EXPECT_CALL(display, ShowColor(exp_second_color));

	control.Submit(first_request);
	control.Handle(false, 0);
	control.Submit(second_request);

	control.Handle(false, 0);
}

TEST(LampControlTest, HandleSecondPassWithoutNewSubmitDoesNothing) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	lamp_request request = {};
	request.mode = "light";
	control.Submit(request);

	EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), _)).WillOnce(Return(true));
	EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
	EXPECT_CALL(display, SetBrightnessPercent(_));
	EXPECT_CALL(display, ShowColor(_));
	control.Handle(false, 0);

	control.Handle(false, 0);
}

TEST(LampControlTest, ApplyMeasModeBeforeSensorsShowsSensorsError) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);

	EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
	EXPECT_CALL(display, ShowSensorsError());

	control.Apply();
}

TEST(LampControlTest, ApplyMeasModeBeforeSensorsDoesNotRepeatShownError) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);

	EXPECT_CALL(display, ErrorShown()).WillOnce(Return(true));

	control.Apply();
}

TEST(LampControlTest, ApplyLightBeforeSensorsClearsShownErrorThenDraws) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t exp_color = HAQuDA_LampColor(0);

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	lamp_request request = {};
	request.mode = "light";
	control.Submit(request);

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(true));
		EXPECT_CALL(display, ClearError());
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowColor(exp_color));
	}

	control.Apply();
}

TEST(LampControlTest, ApplyLightBeforeSensorsWithoutErrorDoesNotClear) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t exp_color = HAQuDA_LampColor(0);

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	lamp_request request = {};
	request.mode = "light";
	control.Submit(request);

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowColor(exp_color));
	}

	control.Apply();
}

TEST(LampControlTest, ApplyOffBeforeSensorsClearsShownErrorThenShowsOff) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t exp_color = static_cast<uint32_t>(COLOR_OFF);

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	lamp_request request = {};
	request.on = "0";
	control.Submit(request);

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(true));
		EXPECT_CALL(display, ClearError());
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowColor(exp_color));
	}

	control.Apply();
}

TEST(LampControlTest, ApplyMeasModeAfterSensorsStartedDrawsNormally) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);

	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.Apply();
}

TEST(LampControlTest, ShowSetsBrightnessBeforeTheColor) {
	const uint8_t bright = 60;
	const uint32_t exp_color = HAQuDA_LampColor(0);

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	lamp_request request = {};
	request.mode = "light";
	request.bright = "60";
	control.Submit(request);

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(bright));
		EXPECT_CALL(display, ShowColor(exp_color));
	}

	control.Apply();
}

TEST(LampControlTest, ShowOffDrawsColorOffOnly) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t exp_color = static_cast<uint32_t>(COLOR_OFF);

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	lamp_request request = {};
	request.on = "0";
	control.Submit(request);

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowColor(exp_color));
	}

	control.Apply();
}

TEST(LampControlTest, ShowLightDrawsTheChosenLampColor) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t exp_color = HAQuDA_LampColor(3);

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	lamp_request request = {};
	request.mode = "light";
	request.color = "3";
	control.Submit(request);

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowColor(exp_color));
	}

	control.Apply();
}

TEST(LampControlTest, ShowStandardPassesTheStandardParam) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	lamp_request request = {};
	request.mode = "standard";
	request.standardParam = "humid";
	control.Submit(request);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(lamp_param::humid));
	}

	control.Apply();
}

TEST(LampControlTest, ShowNightPassesTheParamAndTheWindow) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint16_t minute_of_day = 10 * 60;
	const uint16_t exp_from = 22 * 60 + 15;
	const uint16_t exp_to = 6 * 60 + 45;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	lamp_request request = {};
	request.mode = "night";
	request.nightParam = "tvoc";
	request.nightFrom = "22:15";
	request.nightTo = "06:45";
	control.Handle(true, minute_of_day);
	control.Submit(request);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowNight(lamp_param::tvoc, exp_from, exp_to));
	}

	control.Apply();
}

TEST(LampControlTest, ShowMultiReversesTheBandsToBottomToTopOnce) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	lamp_request request = {};
	request.mode = "multi";
	request.multi = "humid,tvoc,temp";
	control.Submit(request);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(ElementsAre(lamp_param::temp, lamp_param::tvoc, lamp_param::humid))).Times(1);
	}

	control.Apply();
}

TEST(LampControlTest, BuildJsonAfterHandleWithTimeCarriesTheMinuteOfDay) {
	const uint16_t minute_of_day = 10 * 60 + 5;
	const char *const exp_now = "\"now\":\"10:05\"";
	const char *const exp_time = "\"time\":true";

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.Handle(true, minute_of_day);

	char res_json[LAMP_JSON_BUF_LEN];

	control.BuildJson(res_json, sizeof(res_json));

	EXPECT_THAT(res_json, HasSubstr(exp_now));
	EXPECT_THAT(res_json, HasSubstr(exp_time));
}

TEST(LampControlTest, BuildJsonAfterHandleWithTimeMatchesTheJsonWriter) {
	const uint16_t minute_of_day = 17 * 60 + 42;
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(HAQuDA_LampDefaults(), true, minute_of_day, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.Handle(true, minute_of_day);

	char res_json[LAMP_JSON_BUF_LEN];

	control.BuildJson(res_json, sizeof(res_json));

	EXPECT_STREQ(exp_json, res_json);
}

TEST(LampControlTest, BuildJsonAfterHandleAtMidnightCarriesZeroMinute) {
	const uint16_t midnight = 0;
	const char *const exp_now = "\"now\":\"00:00\"";

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.Handle(true, midnight);

	char res_json[LAMP_JSON_BUF_LEN];

	control.BuildJson(res_json, sizeof(res_json));

	EXPECT_THAT(res_json, HasSubstr(exp_now));
}

TEST(LampControlTest, BuildJsonAfterHandleWithoutTimeHasNoNow) {
	const uint16_t ignored_minute = 10 * 60;
	const char *const exp_time = "\"time\":false";
	const char *const exp_now = "\"now\"";

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.Handle(false, ignored_minute);

	char res_json[LAMP_JSON_BUF_LEN];

	control.BuildJson(res_json, sizeof(res_json));

	EXPECT_THAT(res_json, HasSubstr(exp_time));
	EXPECT_THAT(res_json, Not(HasSubstr(exp_now)));
}

TEST(LampControlTest, BuildJsonBeforeAnyHandleHasNoNow) {
	const char *const exp_time = "\"time\":false";
	const char *const exp_now = "\"now\"";

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);

	char res_json[LAMP_JSON_BUF_LEN];

	control.BuildJson(res_json, sizeof(res_json));

	EXPECT_THAT(res_json, HasSubstr(exp_time));
	EXPECT_THAT(res_json, Not(HasSubstr(exp_now)));
}

TEST(LampControlTest, BuildJsonForgetsTheMinuteWhenTheTimeIsLost) {
	const uint16_t minute_of_day = 10 * 60;
	const char *const exp_time = "\"time\":false";
	const char *const exp_now = "\"now\"";

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.Handle(true, minute_of_day);
	control.Handle(false, minute_of_day);

	char res_json[LAMP_JSON_BUF_LEN];

	control.BuildJson(res_json, sizeof(res_json));

	EXPECT_THAT(res_json, HasSubstr(exp_time));
	EXPECT_THAT(res_json, Not(HasSubstr(exp_now)));
}
