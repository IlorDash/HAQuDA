#include <gtest/gtest.h>

#include <cstdint>
#include <string>

#include "HAQuDA_WebPageDisplayState.h"

namespace {

	constexpr uint32_t START_MS = 1000;
	constexpr uint32_t NOW_MS = 5000;
	constexpr uint8_t BRIGHT_PERCENT = 35;

	display_shown itemPicture(display_source source, display_item item, uint32_t seed = 0) {
		display_shown shown = {};
		shown.seq = 7;
		shown.startMs = START_MS;
		shown.source = source;
		shown.item = item;
		shown.anim = HAQuDA_GetDisplayEntry(item)->anim;
		shown.seed = seed;
		return shown;
	}

	display_shown solidPicture(uint32_t color) {
		display_shown shown = {};
		shown.seq = 7;
		shown.startMs = START_MS;
		shown.source = display_source::solid;
		shown.item = DISPLAY_ITEM_NONE;
		shown.anim = HAQuDA_StaticColorParams(color);
		return shown;
	}

	display_shown measPicture(anim_id anim, bool drawn) {
		display_shown shown = {};
		shown.seq = 7;
		shown.startMs = START_MS;
		shown.source = display_source::meas;
		shown.item = DISPLAY_ITEM_NONE;
		shown.anim = HAQuDA_MeasParams(anim);
		shown.measDrawn = drawn;
		return shown;
	}

	std::string jsonOf(const display_shown &shown) {
		web_display_state state;
		HAQuDA_WebPictureBuild(shown, BRIGHT_PERCENT, NOW_MS, state);
		char buf[WEB_DISPLAY_JSON_BUF_LEN];
		if (HAQuDA_TranslateDisplayToJson(state, buf, sizeof(buf)) == 0) {
			return "";
		}
		return buf;
	}

	const std::string EFFECT_PREFIX = "{\"seq\":7,\"age\":4000,\"src\":\"effect\",\"bright\":35,";
	const std::string MEAS_PREFIX = "{\"seq\":7,\"age\":4000,\"src\":\"meas\",\"bright\":35,";

	std::string errorPrefix(const char *word) {
		return std::string("{\"seq\":7,\"age\":4000,\"src\":\"error\",\"err\":\"") + word + "\",\"bright\":35,";
	}

} // namespace

// ---------------------------------------------------------------- WiFi and start pictures

TEST(WebPageDisplayStateTest, WifiJoinedIsAGreenStaticEffect) {
	const std::string exp_json = EFFECT_PREFIX + "\"fx\":\"static\",\"color\":\"#01ff00\"}";

	std::string res;

	res = jsonOf(itemPicture(display_source::event, HAQuDA_ItemOf(display_event::wifiJoined)));

	EXPECT_EQ(exp_json, res);
}

TEST(WebPageDisplayStateTest, WifiLostIsARedStaticEffectWithoutAnErrorWord) {
	const std::string exp_json = EFFECT_PREFIX + "\"fx\":\"static\",\"color\":\"#ff0000\"}";

	std::string res;

	res = jsonOf(itemPicture(display_source::error, HAQuDA_ItemOf(display_error::wifiLost)));

	EXPECT_EQ(exp_json, res);
}

TEST(WebPageDisplayStateTest, WifiReconnectingIsALimeSnake) {
	const std::string exp_json = EFFECT_PREFIX + "\"fx\":\"snake\",\"color\":\"#99ff33\",\"dark\":200,\"step\":904,\"tail\":1}";

	std::string res;

	res = jsonOf(itemPicture(display_source::event, HAQuDA_ItemOf(display_event::wifiReconnecting)));

	EXPECT_EQ(exp_json, res);
}

TEST(WebPageDisplayStateTest, ApRunningIsAnAquaFade) {
	const std::string exp_json
		= EFFECT_PREFIX + "\"fx\":\"fading\",\"color\":\"#00ffff\",\"from\":100,\"to\":0,\"steps\":20,\"dark\":400,\"first\":410,\"step\":210,\"end\":100}";

	std::string res;

	res = jsonOf(itemPicture(display_source::event, HAQuDA_ItemOf(display_event::apRunning)));

	EXPECT_EQ(exp_json, res);
}

TEST(WebPageDisplayStateTest, TheBootSnakeIsAnEffect) {
	const std::string exp_json = EFFECT_PREFIX + "\"fx\":\"snake\",\"color\":\"#99ff33\",\"dark\":200,\"step\":904,\"tail\":1}";

	std::string res;

	res = jsonOf(itemPicture(display_source::event, HAQuDA_ItemOf(display_event::bootSnake)));

	EXPECT_EQ(exp_json, res);
}

TEST(WebPageDisplayStateTest, TheWaitingPictureIsAnEffectLikeItsItem) {
	const std::string exp_json
		= EFFECT_PREFIX + "\"fx\":\"fading\",\"color\":\"#00ffff\",\"from\":100,\"to\":0,\"steps\":20,\"dark\":400,\"first\":410,\"step\":210,\"end\":100}";

	std::string res;

	res = jsonOf(itemPicture(display_source::waiting, HAQuDA_ItemOf(display_event::apRunning)));

	EXPECT_EQ(exp_json, res);
}

// ---------------------------------------------------------------- error pictures

TEST(WebPageDisplayStateTest, WifiConnectFailedIsARedGrowingWithItsWord) {
	const std::string exp_json = errorPrefix("wifi") + "\"fx\":\"growing\",\"color\":\"#ff0000\",\"dark\":200,\"step\":200,\"end\":100}";

	std::string res;

	res = jsonOf(itemPicture(display_source::error, HAQuDA_ItemOf(display_error::wifiConnectFailed)));

	EXPECT_EQ(exp_json, res);
}

TEST(WebPageDisplayStateTest, ApFailedIsARedSnakeWithItsWord) {
	const std::string exp_json = errorPrefix("ap") + "\"fx\":\"snake\",\"color\":\"#ff0000\",\"dark\":200,\"step\":454,\"tail\":1}";

	std::string res;

	res = jsonOf(itemPicture(display_source::error, HAQuDA_ItemOf(display_error::apFailed)));

	EXPECT_EQ(exp_json, res);
}

TEST(WebPageDisplayStateTest, FsFailedIsARedFadeWithItsWord) {
	const std::string exp_json
		= errorPrefix("fs") + "\"fx\":\"fading\",\"color\":\"#ff0000\",\"from\":100,\"to\":0,\"steps\":20,\"dark\":400,\"first\":410,\"step\":210,\"end\":100}";

	std::string res;

	res = jsonOf(itemPicture(display_source::error, HAQuDA_ItemOf(display_error::fsFailed)));

	EXPECT_EQ(exp_json, res);
}

TEST(WebPageDisplayStateTest, SensorsFailedIsRandomPixelsWithItsSeed) {
	const std::string exp_json = errorPrefix("sensors") + "\"fx\":\"random\",\"dark\":200,\"step\":225,\"end\":600,\"seed\":123456}";

	std::string res;

	res = jsonOf(itemPicture(display_source::error, HAQuDA_ItemOf(display_error::sensorsFailed), 123456));

	EXPECT_EQ(exp_json, res);
}

TEST(WebPageDisplayStateTest, NtpFailedIsAYellowFadeUp) {
	const std::string exp_json
		= errorPrefix("ntp")
		  + "\"fx\":\"fading\",\"color\":\"#ffff00\",\"from\":0,\"to\":100,\"steps\":10,\"dark\":400,\"first\":410,\"step\":210,\"end\":100}";

	std::string res;

	res = jsonOf(itemPicture(display_source::error, HAQuDA_ItemOf(display_error::ntpFailed)));

	EXPECT_EQ(exp_json, res);
}

TEST(WebPageDisplayStateTest, ARotationOfTwoErrorsReportsEachWordInTurn) {
	display_shown sensors = itemPicture(display_source::error, HAQuDA_ItemOf(display_error::sensorsFailed), 5);
	display_shown ntp = itemPicture(display_source::error, HAQuDA_ItemOf(display_error::ntpFailed));
	ntp.seq = 8;
	ntp.startMs = 4000;
	const std::string exp_sensors = errorPrefix("sensors") + "\"fx\":\"random\",\"dark\":200,\"step\":225,\"end\":600,\"seed\":5}";
	const std::string exp_ntp
		= "{\"seq\":8,\"age\":1000,\"src\":\"error\",\"err\":\"ntp\",\"bright\":35,\"fx\":\"fading\",\"color\":\"#ffff00\",\"from\":0,\"to\":100,\"steps\":10,"
		  "\"dark\":400,\"first\":410,\"step\":210,\"end\":100}";

	std::string resSensors = jsonOf(sensors);
	std::string resNtp = jsonOf(ntp);

	EXPECT_EQ(exp_sensors, resSensors);
	EXPECT_EQ(exp_ntp, resNtp);
}

TEST(WebPageDisplayStateTest, EveryCatalogItemIsAPictureNotAHold) {
	for (display_item item = 0; item < DISPLAY_ITEM_COUNT; item++) {
		const display_class cls = HAQuDA_GetDisplayEntry(item)->cls;
		const display_source source = (cls == display_class::event) ? display_source::event : display_source::error;

		const std::string res = jsonOf(itemPicture(source, item));

		EXPECT_FALSE(res.empty()) << HAQuDA_DisplayItemName(item);
		EXPECT_EQ(std::string::npos, res.find("hold")) << HAQuDA_DisplayItemName(item);
	}
}

// ---------------------------------------------------------------- solid light and off, nothing shown

TEST(WebPageDisplayStateTest, SolidLightIsAStaticEffectOfItsColour) {
	const std::string exp_json = EFFECT_PREFIX + "\"fx\":\"static\",\"color\":\"#ffb46b\"}";

	std::string res;

	res = jsonOf(solidPicture(0xFFB46B));

	EXPECT_EQ(exp_json, res);
}

TEST(WebPageDisplayStateTest, SolidOffIsABlackStaticEffect) {
	const std::string exp_json = EFFECT_PREFIX + "\"fx\":\"static\",\"color\":\"#000000\"}";

	std::string res;

	res = jsonOf(solidPicture(0));

	EXPECT_EQ(exp_json, res);
}

TEST(WebPageDisplayStateTest, NothingShownYetHoldsTheLastFrame) {
	const std::string exp_json = "{\"seq\":0,\"age\":4000,\"src\":\"none\",\"bright\":35,\"fx\":\"hold\"}";
	display_shown shown = {};
	shown.startMs = START_MS;
	shown.item = DISPLAY_ITEM_NONE;

	std::string res;

	res = jsonOf(shown);

	EXPECT_EQ(exp_json, res);
}

// ---------------------------------------------------------------- measurement pictures

TEST(WebPageDisplayStateTest, StandardMeasShowsItsKeyAndColour) {
	const std::string exp_json = MEAS_PREFIX + "\"fx\":\"meas\",\"layout\":\"standard\",\"keys\":[\"temp\"],\"colors\":[\"#00ff00\"]}";
	display_shown shown = measPicture(anim_id::measStandard, true);
	shown.keys[0] = temp;
	shown.colorCount = 1;
	shown.colors[0] = rgb_color{0, 150, 0};

	std::string res;

	res = jsonOf(shown);

	EXPECT_EQ(exp_json, res);
}

TEST(WebPageDisplayStateTest, TotalMeasIsTheStandardLayoutWithTheTotalKey) {
	const std::string exp_json = MEAS_PREFIX + "\"fx\":\"meas\",\"layout\":\"standard\",\"keys\":[\"total\"],\"colors\":[\"#ff0000\"]}";
	display_shown shown = measPicture(anim_id::measTotal, true);
	shown.keys[0] = humid;
	shown.colorCount = 1;
	shown.colors[0] = rgb_color{150, 0, 0};

	std::string res;

	res = jsonOf(shown);

	EXPECT_EQ(exp_json, res);
}

TEST(WebPageDisplayStateTest, MultiMeasShowsThreeKeysAndColoursInOrder) {
	const std::string exp_json
		= MEAS_PREFIX + "\"fx\":\"meas\",\"layout\":\"multi\",\"keys\":[\"temp\",\"eco2\",\"pm25\"],\"colors\":[\"#ff0000\",\"#00ff00\",\"#0000ff\"]}";
	display_shown shown = measPicture(anim_id::measMulti, true);
	shown.keys[0] = temp;
	shown.keys[1] = eCO2;
	shown.keys[2] = PM2_5;
	shown.colorCount = 3;
	shown.colors[0] = rgb_color{150, 0, 0};
	shown.colors[1] = rgb_color{0, 150, 0};
	shown.colors[2] = rgb_color{0, 0, 150};

	std::string res;

	res = jsonOf(shown);

	EXPECT_EQ(exp_json, res);
}

TEST(WebPageDisplayStateTest, NightMeasShowsTheLitCellsAndTheGreyOfTheBar) {
	// 105 of 150 on the lamp is 178 of 255 on the page
	const std::string exp_json
		= MEAS_PREFIX + "\"fx\":\"meas\",\"layout\":\"night\",\"keys\":[\"eco2\"],\"colors\":[\"#0000ff\"],\"lit\":7,\"whiteColor\":\"#b2b2b2\"}";
	display_shown shown = measPicture(anim_id::measNight, true);
	shown.keys[0] = eCO2;
	shown.colorCount = 1;
	shown.colors[0] = rgb_color{0, 0, 150};
	shown.white = rgb_color{105, 105, 105};
	shown.nightLitCells = 7;

	std::string res;

	res = jsonOf(shown);

	EXPECT_EQ(exp_json, res);
}

TEST(WebPageDisplayStateTest, NightMeasWithoutLitCellsReportsZero) {
	const std::string exp_json
		= MEAS_PREFIX + "\"fx\":\"meas\",\"layout\":\"night\",\"keys\":[\"tvoc\"],\"colors\":[\"#00ff00\"],\"lit\":0,\"whiteColor\":\"#b2b2b2\"}";
	display_shown shown = measPicture(anim_id::measNight, true);
	shown.keys[0] = TVOC;
	shown.colorCount = 1;
	shown.colors[0] = rgb_color{0, 150, 0};
	shown.white = rgb_color{105, 105, 105};

	std::string res;

	res = jsonOf(shown);

	EXPECT_EQ(exp_json, res);
}

TEST(WebPageDisplayStateTest, AMeasPictureBeforeItsFirstDrawIsAHold) {
	const std::string exp_json = MEAS_PREFIX + "\"fx\":\"hold\"}";
	display_shown shown = measPicture(anim_id::measMulti, false);
	shown.keys[0] = temp;
	shown.keys[1] = eCO2;
	shown.keys[2] = PM2_5;

	std::string res;

	res = jsonOf(shown);

	EXPECT_EQ(exp_json, res);
}

TEST(WebPageDisplayStateTest, AMeasKeyWithoutAReadingIsAHold) {
	const std::string exp_json = MEAS_PREFIX + "\"fx\":\"hold\"}";
	display_shown shown = measPicture(anim_id::measStandard, true);
	shown.keys[0] = noneType;
	shown.colorCount = 1;

	std::string res;

	res = jsonOf(shown);

	EXPECT_EQ(exp_json, res);
}

TEST(WebPageDisplayStateTest, EveryMeasKeyHasItsWord) {
	const meas_type types[] = {total, eCO2, TVOC, PM2_5, temp, humid};
	const char *words[] = {"total", "eco2", "tvoc", "pm25", "temp", "humid"};
	for (size_t i = 0; i < 6; i++) {
		display_shown shown = measPicture(anim_id::measStandard, true);
		shown.keys[0] = types[i];
		shown.colorCount = 1;

		const std::string res = jsonOf(shown);

		EXPECT_NE(std::string::npos, res.find(std::string("\"keys\":[\"") + words[i] + "\"]")) << words[i];
	}
}

// ---------------------------------------------------------------- counters and clock

TEST(WebPageDisplayStateTest, SeqAndBrightnessPassThrough) {
	display_shown shown = solidPicture(0xFFB46B);
	shown.seq = 4000000000u;
	web_display_state state;

	HAQuDA_WebPictureBuild(shown, 80, NOW_MS, state);

	EXPECT_EQ(4000000000u, state.seq);
	EXPECT_EQ(80, state.brightPercent);
}

TEST(WebPageDisplayStateTest, AgeSurvivesTheMillisWrap) {
	display_shown shown = solidPicture(0xFFB46B);
	shown.startMs = 0xFFFFFF00u;
	web_display_state state;

	HAQuDA_WebPictureBuild(shown, BRIGHT_PERCENT, 100, state);

	EXPECT_EQ(356u, state.ageMs);
}

TEST(WebPageDisplayStateTest, TheSeedIsReportedOnlyForRandomPixels) {
	web_display_state state;

	HAQuDA_WebPictureBuild(itemPicture(display_source::event, HAQuDA_ItemOf(display_event::bootSnake), 99), BRIGHT_PERCENT, NOW_MS, state);

	EXPECT_EQ(0u, state.seed);
}
