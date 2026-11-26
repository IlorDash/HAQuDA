#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <string>

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
	HAQuDA_BuildLampJson(exp_settings, false, 0, 0, exp_json, sizeof(exp_json));

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
	HAQuDA_BuildLampJson(exp_settings, false, 0, 0, exp_json, sizeof(exp_json));
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
	HAQuDA_BuildLampJson(exp_settings, false, 0, 0, exp_json, sizeof(exp_json));

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
	HAQuDA_BuildLampJson(exp_settings, false, 0, 0, exp_json, sizeof(exp_json));

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
	HAQuDA_BuildLampJson(exp_settings, false, 0, 0, exp_json, sizeof(exp_json));

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
	HAQuDA_BuildLampJson(exp_settings, false, 0, 0, exp_json, sizeof(exp_json));

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
	HAQuDA_BuildLampJson(exp_settings, false, 0, 0, exp_json, sizeof(exp_json));

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

	control.Handle(0, false, 0);
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

	control.Handle(0, false, 0);

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

	control.Handle(0, true, minute_of_day);

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

	control.Handle(0, false, 0);
}

TEST(LampControlTest, HandleWithoutPendingChangeDoesNothing) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);

	control.Handle(0, false, 0);
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

	control.Handle(0, false, 0);
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
	control.Handle(0, false, 0);
	control.Submit(second_request);

	control.Handle(0, false, 0);
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
	control.Handle(0, false, 0);

	control.Handle(0, false, 0);
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
	control.Handle(0, true, minute_of_day);
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
	control.Handle(0, true, minute_of_day);

	char res_json[LAMP_JSON_BUF_LEN];

	control.BuildJson(res_json, sizeof(res_json));

	EXPECT_THAT(res_json, HasSubstr(exp_now));
	EXPECT_THAT(res_json, HasSubstr(exp_time));
}

TEST(LampControlTest, BuildJsonAfterHandleWithTimeMatchesTheJsonWriter) {
	const uint16_t minute_of_day = 17 * 60 + 42;
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(HAQuDA_LampDefaults(), true, minute_of_day, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.Handle(0, true, minute_of_day);

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
	control.Handle(0, true, midnight);

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
	control.Handle(0, false, ignored_minute);

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
	control.Handle(0, true, minute_of_day);
	control.Handle(0, false, minute_of_day);

	char res_json[LAMP_JSON_BUF_LEN];

	control.BuildJson(res_json, sizeof(res_json));

	EXPECT_THAT(res_json, HasSubstr(exp_time));
	EXPECT_THAT(res_json, Not(HasSubstr(exp_now)));
}

TEST(LampActionsTest, TogglePowerBeforeHandleTouchesNothing) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	control.TogglePower();
}

TEST(LampActionsTest, NextModeBeforeHandleTouchesNothing) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	control.NextMode();
}

TEST(LampActionsTest, SetModeBeforeHandleTouchesNothing) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	bool res;

	res = control.SetMode(lamp_mode::standard);

	EXPECT_TRUE(res);
}

TEST(LampActionsTest, TogglePowerOffSavesThenShowsDark) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;

	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.on = false;
	uint8_t exp_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_record))).WillOnce(Return(true));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowColor(static_cast<uint32_t>(COLOR_OFF)));
	}

	control.TogglePower();

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, TogglePowerOnFromOffSavesThenShowsThePicture) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;

	lamp_settings off = HAQuDA_LampDefaults();
	off.on = false;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();
	uint8_t off_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(off, off_record, sizeof(off_record));
	uint8_t exp_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(off_record, off_record + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Lamp settings loaded")));
	control.Load();

	{
		InSequence seq;
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_record))).WillOnce(Return(true));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.TogglePower();

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, TogglePowerTwiceBeforeHandleWritesNothingAndDrawsTheStoredState) {
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

	control.TogglePower();
	control.TogglePower();

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, StepBrightnessUpDrawsOnlyTheBrightness) {
	const uint8_t exp_stepped = DEFAULT_BRIGHTNESS + LAMP_BRIGHT_STEP;

	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.brightPercent = exp_stepped;
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(exp_settings, false, 0, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	bool res;
	char res_json[LAMP_JSON_BUF_LEN];

	EXPECT_CALL(display, SetBrightnessPercent(exp_stepped));

	res = control.StepBrightness(true);

	EXPECT_TRUE(res);
	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(LampActionsTest, StepBrightnessUpIsNotSavedByHandle) {
	const uint8_t exp_stepped = DEFAULT_BRIGHTNESS + LAMP_BRIGHT_STEP;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	EXPECT_CALL(display, SetBrightnessPercent(exp_stepped));

	control.StepBrightness(true);

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, StepBrightnessDownSubtractsOneStep) {
	const uint8_t exp_lowered = DEFAULT_BRIGHTNESS - LAMP_BRIGHT_STEP;

	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.brightPercent = exp_lowered;
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(exp_settings, false, 0, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	bool res;
	char res_json[LAMP_JSON_BUF_LEN];

	EXPECT_CALL(display, SetBrightnessPercent(exp_lowered));

	res = control.StepBrightness(false);

	EXPECT_TRUE(res);
	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(LampActionsTest, CommitBrightnessAfterOneStepWritesTheSteppedValue) {
	const uint8_t exp_stepped = DEFAULT_BRIGHTNESS + LAMP_BRIGHT_STEP;

	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.brightPercent = exp_stepped;
	uint8_t exp_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(display, SetBrightnessPercent(exp_stepped));
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_record))).WillOnce(Return(true));
	}

	control.StepBrightness(true);
	control.CommitBrightness();

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, CommitBrightnessAfterOneStepSavesOnlyOnce) {
	const uint8_t exp_stepped = DEFAULT_BRIGHTNESS + LAMP_BRIGHT_STEP;

	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.brightPercent = exp_stepped;
	uint8_t exp_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(display, SetBrightnessPercent(exp_stepped));
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_record))).WillOnce(Return(true));
	}

	control.StepBrightness(true);
	control.CommitBrightness();
	control.Handle(0, false, 0);

	control.Handle(1, false, 0);
}

TEST(LampActionsTest, CommitBrightnessAfterSeveralStepsWritesTheLastValueOnce) {
	const uint8_t exp_first = DEFAULT_BRIGHTNESS + LAMP_BRIGHT_STEP;
	const uint8_t exp_second = DEFAULT_BRIGHTNESS + 2 * LAMP_BRIGHT_STEP;
	const uint8_t exp_third = DEFAULT_BRIGHTNESS + 3 * LAMP_BRIGHT_STEP;

	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.brightPercent = exp_third;
	uint8_t exp_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(display, SetBrightnessPercent(exp_first));
		EXPECT_CALL(display, SetBrightnessPercent(exp_second));
		EXPECT_CALL(display, SetBrightnessPercent(exp_third));
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_record))).WillOnce(Return(true));
	}

	control.StepBrightness(true);
	control.StepBrightness(true);
	control.StepBrightness(true);
	control.CommitBrightness();

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, CommitBrightnessWithoutAStepWritesNothingAndDrawsNothing) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	control.CommitBrightness();

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, StepBrightnessUpAtTheMaximumChangesNothing) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);

	lamp_settings top = HAQuDA_LampDefaults();
	top.brightPercent = LAMP_BRIGHT_MAX;
	uint8_t top_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(top, top_record, sizeof(top_record));
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(top, false, 0, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(top_record, top_record + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Lamp settings loaded")));
	control.Load();

	bool res;
	char res_json[LAMP_JSON_BUF_LEN];

	res = control.StepBrightness(true);

	EXPECT_FALSE(res);
	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(LampActionsTest, CommitBrightnessAfterAStepAtTheMaximumWritesNothing) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);

	lamp_settings top = HAQuDA_LampDefaults();
	top.brightPercent = LAMP_BRIGHT_MAX;
	uint8_t top_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(top, top_record, sizeof(top_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(top_record, top_record + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Lamp settings loaded")));
	control.Load();

	control.StepBrightness(true);
	control.CommitBrightness();

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, StepBrightnessDownAtTheMinimumChangesNothing) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);

	lamp_settings bottom = HAQuDA_LampDefaults();
	bottom.brightPercent = LAMP_BRIGHT_MIN;
	uint8_t bottom_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(bottom, bottom_record, sizeof(bottom_record));
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(bottom, false, 0, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(bottom_record, bottom_record + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Lamp settings loaded")));
	control.Load();

	bool res;
	char res_json[LAMP_JSON_BUF_LEN];

	res = control.StepBrightness(false);

	EXPECT_FALSE(res);
	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(LampActionsTest, CommitBrightnessAfterAStepAtTheMinimumWritesNothing) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);

	lamp_settings bottom = HAQuDA_LampDefaults();
	bottom.brightPercent = LAMP_BRIGHT_MIN;
	uint8_t bottom_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(bottom, bottom_record, sizeof(bottom_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(bottom_record, bottom_record + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Lamp settings loaded")));
	control.Load();

	control.StepBrightness(false);
	control.CommitBrightness();

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, StepBrightnessOnAnOffLampTurnsItOnAndDrawsThePicture) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);
	const uint8_t exp_stepped = DEFAULT_BRIGHTNESS + LAMP_BRIGHT_STEP;

	lamp_settings off = HAQuDA_LampDefaults();
	off.on = false;
	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.brightPercent = exp_stepped;
	uint8_t off_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(off, off_record, sizeof(off_record));
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(exp_settings, false, 0, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(off_record, off_record + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Lamp settings loaded")));
	control.Load();

	bool res;
	char res_json[LAMP_JSON_BUF_LEN];

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_stepped));
		EXPECT_CALL(display, ShowMulti(_));
	}

	res = control.StepBrightness(true);

	EXPECT_TRUE(res);
	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(LampActionsTest, CommitBrightnessAfterAStepOnAnOffLampSavesTheLampOnAtTheNewBrightness) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);
	const uint8_t exp_stepped = DEFAULT_BRIGHTNESS + LAMP_BRIGHT_STEP;

	lamp_settings off = HAQuDA_LampDefaults();
	off.on = false;
	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.brightPercent = exp_stepped;
	uint8_t off_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(off, off_record, sizeof(off_record));
	uint8_t exp_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(off_record, off_record + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Lamp settings loaded")));
	control.Load();

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_stepped));
		EXPECT_CALL(display, ShowMulti(_));
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_record))).WillOnce(Return(true));
	}

	control.StepBrightness(true);
	control.CommitBrightness();

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, StepBrightnessOnAnOffLampAtTheMaximumStillTurnsItOn) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);

	lamp_settings off = HAQuDA_LampDefaults();
	off.on = false;
	off.brightPercent = LAMP_BRIGHT_MAX;

	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.brightPercent = LAMP_BRIGHT_MAX;
	uint8_t off_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(off, off_record, sizeof(off_record));
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(exp_settings, false, 0, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(off_record, off_record + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Lamp settings loaded")));
	control.Load();

	bool res;
	char res_json[LAMP_JSON_BUF_LEN];

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(LAMP_BRIGHT_MAX));
		EXPECT_CALL(display, ShowMulti(_));
	}

	res = control.StepBrightness(true);

	EXPECT_TRUE(res);
	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(LampActionsTest, StepBrightnessEndsAShownReadingAndDrawsThePicture) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint8_t exp_stepped = DEFAULT_BRIGHTNESS + LAMP_BRIGHT_STEP;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	bool res;

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(lamp_param::eco2));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_stepped));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.ShowReading(lamp_param::eco2);

	res = control.StepBrightness(true);

	EXPECT_TRUE(res);
}

TEST(LampActionsTest, ReadingEndedByAStepDoesNotRedrawWhenItsTimeRuns) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint8_t exp_stepped = DEFAULT_BRIGHTNESS + LAMP_BRIGHT_STEP;
	const uint32_t start_ms = 1000;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(lamp_param::eco2));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_stepped));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.ShowReading(lamp_param::eco2);
	control.Handle(start_ms, false, 0);
	control.StepBrightness(true);

	control.Handle(start_ms + LAMP_SHOW_READING_MS, false, 0);
}

TEST(LampActionsTest, NextModeFromMultiWithTimeGoesToNight) {
	const uint16_t minute_of_day = 600;
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;

	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.mode = lamp_mode::night;
	uint8_t exp_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_record))).WillOnce(Return(true));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowNight(exp_settings.nightParam, exp_settings.nightFromMin, exp_settings.nightToMin));
	}

	control.Handle(0, true, minute_of_day);
	control.NextMode();

	control.Handle(0, true, minute_of_day);
}

TEST(LampActionsTest, NextModeFromMultiWithoutTimeSkipsNight) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;

	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.mode = lamp_mode::light;
	uint8_t exp_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_record))).WillOnce(Return(true));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowColor(HAQuDA_LampColor(exp_settings.color)));
	}

	control.NextMode();

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, NextModeFromStandardGoesToMulti) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;

	lamp_settings standard = HAQuDA_LampDefaults();
	standard.mode = lamp_mode::standard;
	const lamp_settings exp_settings = HAQuDA_LampDefaults();
	uint8_t standard_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(standard, standard_record, sizeof(standard_record));
	uint8_t exp_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(standard_record, standard_record + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Lamp settings loaded")));
	control.Load();

	{
		InSequence seq;
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_record))).WillOnce(Return(true));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.NextMode();

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, NextModeFromNightGoesToLight) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;

	lamp_settings night = HAQuDA_LampDefaults();
	night.mode = lamp_mode::night;
	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.mode = lamp_mode::light;
	uint8_t night_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(night, night_record, sizeof(night_record));
	uint8_t exp_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(night_record, night_record + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Lamp settings loaded")));
	control.Load();

	{
		InSequence seq;
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_record))).WillOnce(Return(true));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowColor(HAQuDA_LampColor(exp_settings.color)));
	}

	control.NextMode();

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, NextModeFromLightWrapsToStandard) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;

	lamp_settings light = HAQuDA_LampDefaults();
	light.mode = lamp_mode::light;
	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.mode = lamp_mode::standard;
	uint8_t light_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(light, light_record, sizeof(light_record));
	uint8_t exp_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(light_record, light_record + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Lamp settings loaded")));
	control.Load();

	{
		InSequence seq;
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_record))).WillOnce(Return(true));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(exp_settings.standardParam));
	}

	control.NextMode();

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, NextModeOnAnOffLampTurnsItOnAndSwitchesTheMode) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;

	lamp_settings off = HAQuDA_LampDefaults();
	off.on = false;
	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.mode = lamp_mode::light;
	uint8_t off_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(off, off_record, sizeof(off_record));
	uint8_t exp_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(off_record, off_record + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Lamp settings loaded")));
	control.Load();

	{
		InSequence seq;
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_record))).WillOnce(Return(true));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowColor(HAQuDA_LampColor(exp_settings.color)));
	}

	control.NextMode();

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, SetModeReturnsTrueForAPlainMode) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	bool res;

	res = control.SetMode(lamp_mode::standard);

	EXPECT_TRUE(res);
}

TEST(LampActionsTest, SetModeSavesThenShowsTheMode) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;

	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.mode = lamp_mode::standard;
	uint8_t exp_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_record))).WillOnce(Return(true));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(exp_settings.standardParam));
	}

	control.SetMode(lamp_mode::standard);

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, SetModeNightWithTimeIsAccepted) {
	const uint16_t minute_of_day = 600;

	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.mode = lamp_mode::night;
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(exp_settings, true, minute_of_day, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	bool res;
	char res_json[LAMP_JSON_BUF_LEN];

	control.Handle(0, true, minute_of_day);

	res = control.SetMode(lamp_mode::night);

	EXPECT_TRUE(res);
	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(LampActionsTest, SetModeNightWithoutTimeIsRefusedAndChangesNothing) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);

	lamp_settings off = HAQuDA_LampDefaults();
	off.on = false;
	uint8_t off_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(off, off_record, sizeof(off_record));
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(off, false, 0, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(off_record, off_record + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Lamp settings loaded")));
	control.Load();

	bool res;
	char res_json[LAMP_JSON_BUF_LEN];

	res = control.SetMode(lamp_mode::night);

	EXPECT_FALSE(res);
	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(LampActionsTest, SetModeNightWithoutTimeWritesAndDrawsNothing) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);

	lamp_settings off = HAQuDA_LampDefaults();
	off.on = false;
	uint8_t off_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(off, off_record, sizeof(off_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(off_record, off_record + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Lamp settings loaded")));
	control.Load();

	control.SetMode(lamp_mode::night);

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, SetModeOutOfRangeIsRefused) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	bool res;

	res = control.SetMode(static_cast<lamp_mode>(LAMP_MODE_NUM));

	EXPECT_FALSE(res);
}

TEST(LampActionsTest, SetModeOnAnOffLampTurnsItOn) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;

	lamp_settings off = HAQuDA_LampDefaults();
	off.on = false;
	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.mode = lamp_mode::standard;
	uint8_t off_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(off, off_record, sizeof(off_record));
	uint8_t exp_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(off_record, off_record + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Lamp settings loaded")));
	control.Load();

	{
		InSequence seq;
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_record))).WillOnce(Return(true));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(exp_settings.standardParam));
	}

	control.SetMode(lamp_mode::standard);

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, ShowReadingDrawsTheStandardPictureOfTheParam) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	bool res;

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(lamp_param::pm25));
	}

	res = control.ShowReading(lamp_param::pm25);

	EXPECT_TRUE(res);
}

TEST(LampActionsTest, ShowReadingDrawsAtTheLampBrightness) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);
	const uint8_t exp_bright = 60;

	lamp_settings loaded = HAQuDA_LampDefaults();
	loaded.brightPercent = exp_bright;
	uint8_t loaded_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(loaded, loaded_record, sizeof(loaded_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(loaded_record, loaded_record + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Lamp settings loaded")));
	control.Load();

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(lamp_param::pm25));
	}

	control.ShowReading(lamp_param::pm25);
}

TEST(LampActionsTest, ShowReadingLeavesTheSettingsUntouched) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;

	const lamp_settings exp_settings = HAQuDA_LampDefaults();
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(exp_settings, false, 0, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	char res_json[LAMP_JSON_BUF_LEN];

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(lamp_param::pm25));
	}

	control.ShowReading(lamp_param::pm25);

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(LampActionsTest, ShowReadingIsShownUntilJustBeforeItsTime) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t start_ms = 1000;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(lamp_param::eco2));
	}

	control.ShowReading(lamp_param::eco2);
	control.Handle(start_ms, false, 0);

	control.Handle(start_ms + LAMP_SHOW_READING_MS - 1, false, 0);
}

TEST(LampActionsTest, ShowReadingEndsAfterItsTimeAndShowsTheSettingsAgain) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t start_ms = 1000;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(lamp_param::eco2));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.ShowReading(lamp_param::eco2);
	control.Handle(start_ms, false, 0);

	control.Handle(start_ms + LAMP_SHOW_READING_MS, false, 0);
}

TEST(LampActionsTest, ShowReadingEndsOnlyOnce) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t start_ms = 1000;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(lamp_param::eco2));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.ShowReading(lamp_param::eco2);
	control.Handle(start_ms, false, 0);
	control.Handle(start_ms + LAMP_SHOW_READING_MS, false, 0);

	control.Handle(start_ms + 2 * LAMP_SHOW_READING_MS, false, 0);
}

TEST(LampActionsTest, ShowReadingTimeRunsFromTheFirstHandleAfterTheCall) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t first_pass_ms = 5000;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(lamp_param::tvoc));
	}

	control.ShowReading(lamp_param::tvoc);
	control.Handle(first_pass_ms, false, 0);

	control.Handle(first_pass_ms + LAMP_SHOW_READING_MS - 1, false, 0);
}

TEST(LampActionsTest, ShowReadingEndsRightAcrossTheMillisWrap) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t start_ms = 0xFFFFFF00u;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(lamp_param::temp));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.ShowReading(lamp_param::temp);
	control.Handle(start_ms, false, 0);

	control.Handle(start_ms + LAMP_SHOW_READING_MS, false, 0);
}

TEST(LampActionsTest, ShowReadingOnAnOffLampShowsTheReading) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t start_ms = 1000;

	lamp_settings off = HAQuDA_LampDefaults();
	off.on = false;
	uint8_t off_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(off, off_record, sizeof(off_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(off_record, off_record + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Lamp settings loaded")));
	control.Load();

	bool res;

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(lamp_param::humid));
	}

	res = control.ShowReading(lamp_param::humid);

	EXPECT_TRUE(res);
}

TEST(LampActionsTest, ShowReadingOnAnOffLampReturnsToDarkAtItsEnd) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t start_ms = 1000;

	lamp_settings off = HAQuDA_LampDefaults();
	off.on = false;
	uint8_t off_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(off, off_record, sizeof(off_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(off_record, off_record + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Lamp settings loaded")));
	control.Load();

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(lamp_param::humid));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowColor(static_cast<uint32_t>(COLOR_OFF)));
	}

	control.ShowReading(lamp_param::humid);
	control.Handle(start_ms, false, 0);

	control.Handle(start_ms + LAMP_SHOW_READING_MS, false, 0);
}

TEST(LampActionsTest, ShowReadingOnAnOffLampKeepsTheSettingsOff) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t start_ms = 1000;

	lamp_settings off = HAQuDA_LampDefaults();
	off.on = false;
	uint8_t off_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(off, off_record, sizeof(off_record));
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(off, false, 0, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(off_record, off_record + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Lamp settings loaded")));
	control.Load();

	char res_json[LAMP_JSON_BUF_LEN];

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(lamp_param::humid));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowColor(static_cast<uint32_t>(COLOR_OFF)));
	}

	control.ShowReading(lamp_param::humid);
	control.Handle(start_ms, false, 0);

	control.Handle(start_ms + LAMP_SHOW_READING_MS, false, 0);

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(LampActionsTest, ShowReadingBeforeTheSensorsRunIsRefused) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);

	bool res;

	res = control.ShowReading(lamp_param::eco2);

	EXPECT_FALSE(res);
}

TEST(LampActionsTest, ShowReadingOverAShownErrorIsRefused) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	bool res;

	EXPECT_CALL(display, ErrorShown()).WillOnce(Return(true));

	res = control.ShowReading(lamp_param::eco2);

	EXPECT_FALSE(res);
}

TEST(LampActionsTest, ShowReadingOverAShownErrorLeavesNoTimerBehind) {
	const uint32_t start_ms = 1000;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	EXPECT_CALL(display, ErrorShown()).WillOnce(Return(true));

	control.ShowReading(lamp_param::eco2);
	control.Handle(start_ms, false, 0);

	control.Handle(start_ms + LAMP_SHOW_READING_MS, false, 0);
}

TEST(LampActionsTest, ShowReadingWithAParamOutOfRangeIsRefused) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	bool res;

	res = control.ShowReading(static_cast<lamp_param>(LAMP_PARAM_NUM));

	EXPECT_FALSE(res);
}

TEST(LampActionsTest, ShowReadingIsCancelledByApply) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t start_ms = 1000;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(lamp_param::eco2));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.ShowReading(lamp_param::eco2);
	control.Handle(start_ms, false, 0);

	control.Apply();
}

TEST(LampActionsTest, ShowReadingIsNotRedrawnAtItsTimeAfterApply) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t start_ms = 1000;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(lamp_param::eco2));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.ShowReading(lamp_param::eco2);
	control.Handle(start_ms, false, 0);
	control.Apply();

	control.Handle(start_ms + LAMP_SHOW_READING_MS, false, 0);
}

TEST(LampActionsTest, ShowReadingIsCancelledBySubmitWhichIsSavedAndApplied) {
	const uint32_t start_ms = 1000;
	const uint8_t exp_bright = 60;

	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.brightPercent = exp_bright;
	uint8_t exp_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	lamp_request request = {};
	request.bright = "60";

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(DEFAULT_BRIGHTNESS));
		EXPECT_CALL(display, ShowStandard(lamp_param::eco2));
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_record))).WillOnce(Return(true));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.ShowReading(lamp_param::eco2);
	control.Handle(start_ms, false, 0);
	control.Submit(request);

	control.Handle(start_ms + 100, false, 0);
}

TEST(LampActionsTest, ShowReadingIsNotRedrawnAtItsTimeAfterSubmit) {
	const uint32_t start_ms = 1000;
	const uint8_t exp_bright = 60;

	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.brightPercent = exp_bright;
	uint8_t exp_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	lamp_request request = {};
	request.bright = "60";

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(DEFAULT_BRIGHTNESS));
		EXPECT_CALL(display, ShowStandard(lamp_param::eco2));
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_record))).WillOnce(Return(true));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.ShowReading(lamp_param::eco2);
	control.Handle(start_ms, false, 0);
	control.Submit(request);
	control.Handle(start_ms + 100, false, 0);

	control.Handle(start_ms + LAMP_SHOW_READING_MS, false, 0);
}

TEST(LampActionsTest, ShowReadingIsCancelledByAnAction) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t start_ms = 1000;

	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.on = false;
	uint8_t exp_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(lamp_param::eco2));
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_record))).WillOnce(Return(true));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowColor(static_cast<uint32_t>(COLOR_OFF)));
	}

	control.ShowReading(lamp_param::eco2);
	control.Handle(start_ms, false, 0);
	control.TogglePower();

	control.Handle(start_ms + 100, false, 0);
}

TEST(LampActionsTest, ShowReadingIsNotRedrawnAtItsTimeAfterAnAction) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t start_ms = 1000;

	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.on = false;
	uint8_t exp_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(lamp_param::eco2));
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_record))).WillOnce(Return(true));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowColor(static_cast<uint32_t>(COLOR_OFF)));
	}

	control.ShowReading(lamp_param::eco2);
	control.Handle(start_ms, false, 0);
	control.TogglePower();
	control.Handle(start_ms + 100, false, 0);

	control.Handle(start_ms + LAMP_SHOW_READING_MS, false, 0);
}

TEST(LampActionsTest, SecondShowReadingRestartsTheTime) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t start_ms = 1000;
	const uint32_t second_ms = 8000;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(lamp_param::eco2));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(lamp_param::tvoc));
	}

	control.ShowReading(lamp_param::eco2);
	control.Handle(start_ms, false, 0);
	control.ShowReading(lamp_param::tvoc);
	control.Handle(second_ms, false, 0);

	control.Handle(second_ms + LAMP_SHOW_READING_MS - 1, false, 0);
}

TEST(LampActionsTest, FlashLockedShowsItsColourAtTheLampBrightness) {
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
		EXPECT_CALL(display, ShowFlash(lamp_flash::locked));
	}

	control.Flash(lamp_flash::locked);
}

TEST(LampActionsTest, FlashUnlockedShowsItsColourAtTheLampBrightness) {
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
		EXPECT_CALL(display, ShowFlash(lamp_flash::unlocked));
	}

	control.Flash(lamp_flash::unlocked);
}

TEST(LampActionsTest, FlashRefusedShowsItsColourAtTheLampBrightness) {
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
		EXPECT_CALL(display, ShowFlash(lamp_flash::refused));
	}

	control.Flash(lamp_flash::refused);
}

TEST(LampActionsTest, FlashIsShownUntilJustBeforeItsTime) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t start_ms = 2000;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowFlash(lamp_flash::locked));
	}

	control.Flash(lamp_flash::locked);
	control.Handle(start_ms, false, 0);

	control.Handle(start_ms + LAMP_FLASH_MS - 1, false, 0);
}

TEST(LampActionsTest, FlashEndsAfterItsTimeAndShowsTheSettingsAgain) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t start_ms = 2000;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowFlash(lamp_flash::locked));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.Flash(lamp_flash::locked);
	control.Handle(start_ms, false, 0);

	control.Handle(start_ms + LAMP_FLASH_MS, false, 0);
}

TEST(LampActionsTest, FlashEndsOnlyOnce) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t start_ms = 2000;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowFlash(lamp_flash::locked));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.Flash(lamp_flash::locked);
	control.Handle(start_ms, false, 0);
	control.Handle(start_ms + LAMP_FLASH_MS, false, 0);

	control.Handle(start_ms + 2 * LAMP_FLASH_MS, false, 0);
}

TEST(LampActionsTest, FlashWritesNothingAndKeepsTheSettings) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;

	const lamp_settings exp_settings = HAQuDA_LampDefaults();
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(exp_settings, false, 0, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	char res_json[LAMP_JSON_BUF_LEN];

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowFlash(lamp_flash::refused));
	}

	control.Flash(lamp_flash::refused);

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(LampActionsTest, FlashWhileAnErrorIsShownDrawsNothing) {
	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	EXPECT_CALL(display, ErrorShown()).WillOnce(Return(true));

	control.Flash(lamp_flash::locked);
}

TEST(LampActionsTest, FlashWhileAnErrorIsShownLeavesNoTimerBehind) {
	const uint32_t start_ms = 2000;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	EXPECT_CALL(display, ErrorShown()).WillOnce(Return(true));

	control.Flash(lamp_flash::locked);
	control.Handle(start_ms, false, 0);

	control.Handle(start_ms + LAMP_FLASH_MS, false, 0);
}

TEST(LampActionsTest, FlashOnAnOffLampReturnsToDark) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t start_ms = 2000;

	lamp_settings off = HAQuDA_LampDefaults();
	off.on = false;
	uint8_t off_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(off, off_record, sizeof(off_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(off_record, off_record + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Lamp settings loaded")));
	control.Load();

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowFlash(lamp_flash::unlocked));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowColor(static_cast<uint32_t>(COLOR_OFF)));
	}

	control.Flash(lamp_flash::unlocked);
	control.Handle(start_ms, false, 0);

	control.Handle(start_ms + LAMP_FLASH_MS, false, 0);
}

TEST(LampActionsTest, FlashOnAnOffLampKeepsTheSettingsOff) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t start_ms = 2000;

	lamp_settings off = HAQuDA_LampDefaults();
	off.on = false;
	uint8_t off_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(off, off_record, sizeof(off_record));
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(off, false, 0, 0, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(off_record, off_record + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Lamp settings loaded")));
	control.Load();

	char res_json[LAMP_JSON_BUF_LEN];

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowFlash(lamp_flash::unlocked));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowColor(static_cast<uint32_t>(COLOR_OFF)));
	}

	control.Flash(lamp_flash::unlocked);
	control.Handle(start_ms, false, 0);

	control.Handle(start_ms + LAMP_FLASH_MS, false, 0);

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(LampActionsTest, FlashDuringAShownReadingBringsBackTheSettingsNotTheReading) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t start_ms = 1000;
	const uint32_t flash_ms = 3000;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(lamp_param::eco2));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowFlash(lamp_flash::refused));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.ShowReading(lamp_param::eco2);
	control.Handle(start_ms, false, 0);
	control.Flash(lamp_flash::refused);
	control.Handle(flash_ms, false, 0);

	control.Handle(flash_ms + LAMP_FLASH_MS, false, 0);
}

TEST(LampActionsTest, FlashDuringAShownReadingLeavesNoReadingTimerBehind) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t start_ms = 1000;
	const uint32_t flash_ms = 3000;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(lamp_param::eco2));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowFlash(lamp_flash::refused));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.ShowReading(lamp_param::eco2);
	control.Handle(start_ms, false, 0);
	control.Flash(lamp_flash::refused);
	control.Handle(flash_ms, false, 0);
	control.Handle(flash_ms + LAMP_FLASH_MS, false, 0);

	control.Handle(start_ms + LAMP_SHOW_READING_MS + LAMP_FLASH_MS, false, 0);
}

TEST(LampActionsTest, FlashWithAKindOutOfRangeDoesNothing) {
	const lamp_flash bad_kind = static_cast<lamp_flash>(static_cast<uint8_t>(lamp_flash::refused) + 1);

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	control.Flash(bad_kind);
}

TEST(LampActionsTest, SubmitAndTogglePowerBeforeOnePassAreSavedTogether) {
	const uint8_t exp_bright = 60;

	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.brightPercent = exp_bright;
	exp_settings.on = false;
	uint8_t exp_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	lamp_request request = {};
	request.bright = "60";

	{
		InSequence seq;
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_record))).WillOnce(Return(true));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowColor(static_cast<uint32_t>(COLOR_OFF)));
	}

	control.Submit(request);
	control.TogglePower();

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, SetModeAndSubmitBeforeOnePassAreSavedTogether) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;

	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.mode = lamp_mode::light;
	exp_settings.color = 3;
	uint8_t exp_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	lamp_request request = {};
	request.color = "3";

	{
		InSequence seq;
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_record))).WillOnce(Return(true));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowColor(HAQuDA_LampColor(exp_settings.color)));
	}

	control.SetMode(lamp_mode::light);
	control.Submit(request);

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, SubmitAndABrightnessStepAreSavedTogether) {
	const uint8_t exp_stepped = DEFAULT_BRIGHTNESS + LAMP_BRIGHT_STEP;

	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.mode = lamp_mode::light;
	exp_settings.brightPercent = exp_stepped;
	uint8_t exp_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	lamp_request request = {};
	request.mode = "light";

	{
		InSequence seq;
		EXPECT_CALL(display, SetBrightnessPercent(exp_stepped));
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_record))).WillOnce(Return(true));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_stepped));
		EXPECT_CALL(display, ShowColor(HAQuDA_LampColor(exp_settings.color)));
	}

	control.Submit(request);
	control.StepBrightness(true);

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, SubmitEqualToTheStoredStateWritesNothingButStillShowsIt) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	lamp_request request = {};
	request.mode = "multi";
	request.bright = "35";

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.Submit(request);

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, SubmitEqualToTheLoadedStateWritesNothing) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);
	const uint8_t exp_bright = 60;

	lamp_settings loaded = HAQuDA_LampDefaults();
	loaded.brightPercent = exp_bright;
	uint8_t loaded_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(loaded, loaded_record, sizeof(loaded_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(loaded_record, loaded_record + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Lamp settings loaded")));
	control.Load();
	lamp_request request = {};
	request.bright = "60";

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.Submit(request);

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, ChangeThatReturnsToTheStoredStateWritesNothing) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	lamp_request first_request = {};
	first_request.bright = "60";
	lamp_request second_request = {};
	second_request.bright = "35";

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.Submit(first_request);
	control.Submit(second_request);

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, FailedWriteGoesBackToTheStoredStateAndShowsIt) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	lamp_request request = {};
	request.bright = "60";
	request.mode = "light";

	{
		InSequence seq;
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), _)).WillOnce(Return(false));
		EXPECT_CALL(logger, LogError(StrEq("Lamp settings: save failed")));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.Submit(request);

	control.Handle(0, false, 0);
}

TEST(LampActionsTest, FailedWriteRaisesTheErrorCounterAndRevertsTheJson) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t exp_errors = 1;

	const lamp_settings exp_settings = HAQuDA_LampDefaults();
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(exp_settings, false, 0, exp_errors, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	lamp_request request = {};
	request.bright = "60";
	request.mode = "light";

	char res_json[LAMP_JSON_BUF_LEN];

	{
		InSequence seq;
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), _)).WillOnce(Return(false));
		EXPECT_CALL(logger, LogError(StrEq("Lamp settings: save failed")));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.Submit(request);

	control.Handle(0, false, 0);

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(LampActionsTest, FailedWriteIsNotRetried) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t second_ms = 1000;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	lamp_request request = {};
	request.bright = "60";

	{
		InSequence seq;
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), _)).WillOnce(Return(false));
		EXPECT_CALL(logger, LogError(StrEq("Lamp settings: save failed")));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.Submit(request);
	control.Handle(0, false, 0);

	control.Handle(second_ms, false, 0);
}

TEST(LampActionsTest, NextChangeAfterAFailedWriteIsWrittenOverTheStoredState) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint8_t exp_next_bright = 80;

	lamp_settings exp_settings = HAQuDA_LampDefaults();
	exp_settings.brightPercent = exp_next_bright;
	uint8_t exp_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(exp_settings, exp_record, sizeof(exp_record));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	lamp_request failed_request = {};
	failed_request.mode = "light";
	lamp_request next_request = {};
	next_request.bright = "80";

	{
		InSequence seq;
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), _)).WillOnce(Return(false));
		EXPECT_CALL(logger, LogError(StrEq("Lamp settings: save failed")));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(exp_record))).WillOnce(Return(true));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_next_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.Submit(failed_request);
	control.Handle(0, false, 0);
	control.Submit(next_request);

	control.Handle(1000, false, 0);
}

TEST(LampActionsTest, FailedWriteGoesBackToTheLastGoodWriteNotToTheDefaults) {
	const uint8_t exp_good_bright = 60;

	lamp_settings good = HAQuDA_LampDefaults();
	good.brightPercent = exp_good_bright;
	uint8_t good_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(good, good_record, sizeof(good_record));
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(good, false, 0, 1, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	lamp_request first_request = {};
	first_request.bright = "60";
	lamp_request second_request = {};
	second_request.mode = "light";

	char res_json[LAMP_JSON_BUF_LEN];

	{
		InSequence seq;
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), ElementsAreArray(good_record))).WillOnce(Return(true));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_good_bright));
		EXPECT_CALL(display, ShowMulti(_));
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), _)).WillOnce(Return(false));
		EXPECT_CALL(logger, LogError(StrEq("Lamp settings: save failed")));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_good_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.Submit(first_request);
	control.Handle(0, false, 0);
	control.Submit(second_request);

	control.Handle(1000, false, 0);

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(LampActionsTest, FailedWriteOfACommittedBrightnessDrawsTheStoredBrightnessAgain) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint8_t exp_stepped = DEFAULT_BRIGHTNESS + LAMP_BRIGHT_STEP;
	const uint32_t exp_errors = 1;

	const lamp_settings exp_settings = HAQuDA_LampDefaults();
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(exp_settings, false, 0, exp_errors, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();

	char res_json[LAMP_JSON_BUF_LEN];

	{
		InSequence seq;
		EXPECT_CALL(display, SetBrightnessPercent(exp_stepped));
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), _)).WillOnce(Return(false));
		EXPECT_CALL(logger, LogError(StrEq("Lamp settings: save failed")));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.StepBrightness(true);
	control.CommitBrightness();

	control.Handle(0, false, 0);

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(LampActionsTest, SecondFailedWriteRaisesTheErrorCounterToTwo) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t exp_errors = 2;

	const lamp_settings exp_settings = HAQuDA_LampDefaults();
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(exp_settings, false, 0, exp_errors, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	lamp_request request = {};
	request.mode = "light";

	char res_json[LAMP_JSON_BUF_LEN];

	{
		InSequence seq;
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), _)).WillOnce(Return(false));
		EXPECT_CALL(logger, LogError(StrEq("Lamp settings: save failed")));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), _)).WillOnce(Return(false));
		EXPECT_CALL(logger, LogError(StrEq("Lamp settings: save failed")));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.Submit(request);
	control.Handle(0, false, 0);
	control.Submit(request);

	control.Handle(1000, false, 0);

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}

TEST(LampActionsTest, FailedWriteDuringAShownReadingDrawsTheStoredPicture) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t start_ms = 1000;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	lamp_request request = {};
	request.bright = "60";

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(lamp_param::eco2));
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), _)).WillOnce(Return(false));
		EXPECT_CALL(logger, LogError(StrEq("Lamp settings: save failed")));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.ShowReading(lamp_param::eco2);
	control.Handle(start_ms, false, 0);
	control.Submit(request);

	control.Handle(start_ms + 100, false, 0);
}

TEST(LampActionsTest, FailedWriteDuringAShownReadingCancelsItsEnd) {
	const uint8_t exp_bright = DEFAULT_BRIGHTNESS;
	const uint32_t start_ms = 1000;

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	lamp_request request = {};
	request.bright = "60";

	{
		InSequence seq;
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowStandard(lamp_param::eco2));
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), _)).WillOnce(Return(false));
		EXPECT_CALL(logger, LogError(StrEq("Lamp settings: save failed")));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.ShowReading(lamp_param::eco2);
	control.Handle(start_ms, false, 0);
	control.Submit(request);
	control.Handle(start_ms + 100, false, 0);

	control.Handle(start_ms + LAMP_SHOW_READING_MS, false, 0);
}

TEST(LampActionsTest, FailedWriteGoesBackToTheLoadedStateWhenNothingWasWritten) {
	const int record_size = static_cast<int>(LAMP_RECORD_LEN);
	const uint8_t exp_loaded_bright = 60;
	const uint32_t exp_errors = 1;

	lamp_settings loaded = HAQuDA_LampDefaults();
	loaded.brightPercent = exp_loaded_bright;
	uint8_t loaded_record[LAMP_RECORD_LEN];
	HAQuDA_EncodeLampRecord(loaded, loaded_record, sizeof(loaded_record));
	char exp_json[LAMP_JSON_BUF_LEN];
	HAQuDA_BuildLampJson(loaded, false, 0, exp_errors, exp_json, sizeof(exp_json));

	StrictMock<MockFileStorage> files;
	StrictMock<MockLampDisplay> display;
	StrictMock<MockLogger> logger;
	HAQuDA_LampControl control(files, display, logger);
	control.SensorsStarted();
	EXPECT_CALL(files, Exists(StrEq("/lamp"))).WillOnce(Return(true));
	EXPECT_CALL(files, FileSize(StrEq("/lamp"))).WillOnce(Return(record_size));
	EXPECT_CALL(files, ReadFile(StrEq("/lamp"), _, LAMP_RECORD_LEN))
		.WillOnce(DoAll(SetArrayArgument<1>(loaded_record, loaded_record + LAMP_RECORD_LEN), Return(true)));
	EXPECT_CALL(logger, LogInfo(StrEq("Lamp settings loaded")));
	control.Load();
	lamp_request request = {};
	request.bright = "80";

	char res_json[LAMP_JSON_BUF_LEN];

	{
		InSequence seq;
		EXPECT_CALL(files, WriteFileBytes(StrEq("/lamp"), _)).WillOnce(Return(false));
		EXPECT_CALL(logger, LogError(StrEq("Lamp settings: save failed")));
		EXPECT_CALL(display, ErrorShown()).WillOnce(Return(false));
		EXPECT_CALL(display, SetBrightnessPercent(exp_loaded_bright));
		EXPECT_CALL(display, ShowMulti(_));
	}

	control.Submit(request);

	control.Handle(0, false, 0);

	control.BuildJson(res_json, sizeof(res_json));
	EXPECT_STREQ(exp_json, res_json);
}
