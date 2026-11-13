#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "HAQuDA_WebPageDisplayManip_Translator.h"

// ---------------------------------------------------------------- snake

TEST(WebPageDisplayTranslatorTest, SnakeShape) {
	const size_t buf_size = 256;

	// lap 21900 ms / 24 cells = 912.5, rounded up; tail 5 * 24 / 109 = 1.1
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"snake\",\"color\":\"#99ff33\",\"step\":913,\"tail\":1}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::snake;
	state.snake = {0x99FF33, 200, 5};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, SnakeStepRoundsDown) {
	const size_t buf_size = 256;

	// lap 11000 ms / 24 = 458.33
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"snake\",\"color\":\"#ff0000\",\"step\":458,\"tail\":1}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::snake;
	state.snake = {0xFF0000, 100, 1};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, SnakeStepRoundsUp) {
	const size_t buf_size = 256;

	// lap 209 ms / 24 = 8.71
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"snake\",\"color\":\"#ff0000\",\"step\":9,\"tail\":1}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::snake;
	state.snake = {0xFF0000, 1, 1};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, SnakeZeroPauseStepIsShowPauseOnly) {
	const size_t buf_size = 256;

	// lap = only the 100 ms show pause, 100 / 24 = 4.17
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"snake\",\"color\":\"#ff0000\",\"step\":4,\"tail\":1}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::snake;
	state.snake = {0xFF0000, 0, 1};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, SnakeMaxPauseDoesNotOverflow) {
	const size_t buf_size = 256;

	// lap 7143415 ms / 24 = 297642.3
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"snake\",\"color\":\"#ff0000\",\"step\":297642,\"tail\":1}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::snake;
	state.snake = {0xFF0000, 65535, 1};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, SnakeTailZeroClampsToOne) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"snake\",\"color\":\"#ff0000\",\"step\":458,\"tail\":1}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::snake;
	state.snake = {0xFF0000, 100, 0};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, SnakeTailScalesToPageCells) {
	const size_t buf_size = 256;

	// tail 50 * 24 / 109 = 11.01
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"snake\",\"color\":\"#ff0000\",\"step\":458,\"tail\":11}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::snake;
	state.snake = {0xFF0000, 100, 50};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, SnakeTailFullLampIsPageCells) {
	const size_t buf_size = 256;

	// tail 109 * 24 / 109 = 24 exactly, no cap needed
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"snake\",\"color\":\"#ff0000\",\"step\":458,\"tail\":24}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::snake;
	state.snake = {0xFF0000, 100, 109};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, SnakeTailCapsAtPageCells) {
	const size_t buf_size = 256;

	// 255 * 24 / 109 = 56 before the cap
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"snake\",\"color\":\"#ff0000\",\"step\":458,\"tail\":24}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::snake;
	state.snake = {0xFF0000, 100, 255};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

// ---------------------------------------------------------------- growing

TEST(WebPageDisplayTranslatorTest, GrowingShape) {
	const size_t buf_size = 256;

	// two lamp rows per page row: 2 * 100 ms
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"growing\",\"color\":\"#ff0000\",\"step\":200,\"end\":100}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::growing;
	state.growing = {0xFF0000, 100};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, GrowingZeroPause) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"growing\",\"color\":\"#00ff00\",\"step\":0,\"end\":100}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::growing;
	state.growing = {0x00FF00, 0};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, GrowingMaxPauseDoesNotOverflow) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"growing\",\"color\":\"#ff0000\",\"step\":131070,\"end\":100}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::growing;
	state.growing = {0xFF0000, 65535};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

// ---------------------------------------------------------------- random

TEST(WebPageDisplayTranslatorTest, RandomShape) {
	const size_t buf_size = 256;

	// 109 * 50 ms / 24 cells = 227.08; end 500 + 100 ms show pause
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"random\",\"step\":227,\"end\":600}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::random_led;
	state.randomLed = {50, 500};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, RandomStepRoundsUp) {
	const size_t buf_size = 256;

	// 109 / 24 = 4.54
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"random\",\"step\":5,\"end\":100}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::random_led;
	state.randomLed = {1, 0};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, RandomZeroPauseStepIsZero) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"random\",\"step\":0,\"end\":100}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::random_led;
	state.randomLed = {0, 0};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, RandomMaxValuesDoNotOverflow) {
	const size_t buf_size = 256;

	// 109 * 65535 / 24 = 297638.6
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"random\",\"step\":297638,\"end\":65635}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::random_led;
	state.randomLed = {65535, 65535};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

// ---------------------------------------------------------------- fading

TEST(WebPageDisplayTranslatorTest, FadingShape) {
	const size_t buf_size = 256;

	// steps = (100 - 0) / 5
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"fading\",\"color\":\"#ff0000\",\"from\":100,\"to\":0,\"steps\":"
						   "20,\"step\":10,\"end\":100}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::fading;
	state.fading = {0xFF0000, 10, 100, 0, 5};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, FadingStepsRoundUp) {
	const size_t buf_size = 256;

	// 100 / 30 = 3.33
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"fading\",\"color\":\"#ff0000\",\"from\":100,\"to\":0,\"steps\":"
						   "4,\"step\":10,\"end\":100}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::fading;
	state.fading = {0xFF0000, 10, 100, 0, 30};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, FadingStepsExactDivision) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"fading\",\"color\":\"#ff0000\",\"from\":100,\"to\":50,\"steps\":"
						   "5,\"step\":10,\"end\":100}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::fading;
	state.fading = {0xFF0000, 10, 100, 50, 10};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, FadingRangeOfOneIsOneStep) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"fading\",\"color\":\"#ff0000\",\"from\":51,\"to\":50,\"steps\":"
						   "1,\"step\":10,\"end\":100}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::fading;
	state.fading = {0xFF0000, 10, 51, 50, 5};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, FadingStepLargerThanRangeIsOneStep) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"fading\",\"color\":\"#ff0000\",\"from\":100,\"to\":0,\"steps\":"
						   "1,\"step\":10,\"end\":100}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::fading;
	state.fading = {0xFF0000, 10, 100, 0, 255};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, FadingStepZeroIsStaticAtStartBright) {
	const size_t buf_size = 256;

	// no decrement: the lamp stays at startBright
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"static\",\"color\":\"#ff0000\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::fading;
	state.fading = {0xFF0000, 10, 100, 0, 0};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, FadingStartBelowStopIsStaticScaledByStart) {
	const size_t buf_size = 256;

	// the NTP error effect: starts and stays dark
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"static\",\"color\":\"#000000\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::fading;
	state.fading = {0xFFFF00, 10, 0, 100, 10};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, FadingStartEqualsStopIsStaticScaledByStart) {
	const size_t buf_size = 256;

	// 0xFF * 50 / 100 = 127.5, rounded up to 0x80
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"static\",\"color\":\"#800000\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::fading;
	state.fading = {0xFF0000, 10, 50, 50, 5};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, FadingStaticScalesEveryChannel) {
	const size_t buf_size = 256;

	// 0x33 -> 0x1a, 0x66 -> 0x33, 0x99 -> 0x4d at 50 %
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"static\",\"color\":\"#1a334d\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::fading;
	state.fading = {0x336699, 10, 50, 50, 5};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, FadingStartAbove100IsClamped) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"fading\",\"color\":\"#ff0000\",\"from\":100,\"to\":0,\"steps\":"
						   "10,\"step\":10,\"end\":100}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::fading;
	state.fading = {0xFF0000, 10, 255, 0, 10};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, FadingStopAbove100IsClamped) {
	const size_t buf_size = 256;

	// stop clamps to 100 = start, so nothing to fade
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"static\",\"color\":\"#ff0000\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::fading;
	state.fading = {0xFF0000, 10, 100, 200, 10};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

// ---------------------------------------------------------------- static, hold, colour

TEST(WebPageDisplayTranslatorTest, StaticShape) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"static\",\"color\":\"#01ff00\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::static_color;
	state.staticColor = 0x01FF00;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, ColorZeroIsBlack) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"static\",\"color\":\"#000000\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::static_color;
	state.staticColor = 0x000000;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, ColorWhite) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"static\",\"color\":\"#ffffff\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::static_color;
	state.staticColor = 0xFFFFFF;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, ColorKeepsLeadingZeros) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"static\",\"color\":\"#00000a\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::static_color;
	state.staticColor = 0x00000A;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, ColorHighByteIsIgnored) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"static\",\"color\":\"#123456\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::static_color;
	state.staticColor = 0xAB123456;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, EffectNoneIsHold) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"hold\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::none;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, EffectUnknownValueIsHold) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"hold\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = static_cast<web_effect>(99);
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, SourceNoneIsHold) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"none\",\"bright\":50,\"fx\":\"hold\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::none;
	state.brightPercent = 50;
	state.effect = web_effect::snake;
	state.layout = web_meas_layout::standard;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, SourceEventIsHold) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"event\",\"bright\":50,\"fx\":\"hold\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::event;
	state.brightPercent = 50;
	state.effect = web_effect::snake;
	state.layout = web_meas_layout::standard;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

// ---------------------------------------------------------------- error

TEST(WebPageDisplayTranslatorTest, ErrorKindNoneWithoutEffectIsHold) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"error\",\"err\":\"none\",\"bright\":50,\"fx\":\"hold\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::error;
	state.brightPercent = 50;
	state.error = web_error::none;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, ErrorKindWifiWithoutEffectIsHold) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"error\",\"err\":\"wifi\",\"bright\":50,\"fx\":\"hold\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::error;
	state.brightPercent = 50;
	state.error = web_error::wifi;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, ErrorKindApWithoutEffectIsHold) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"error\",\"err\":\"ap\",\"bright\":50,\"fx\":\"hold\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::error;
	state.brightPercent = 50;
	state.error = web_error::ap;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, ErrorKindFsWithoutEffectIsHold) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"error\",\"err\":\"fs\",\"bright\":50,\"fx\":\"hold\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::error;
	state.brightPercent = 50;
	state.error = web_error::fs;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, ErrorKindSensorsWithoutEffectIsHold) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"error\",\"err\":\"sensors\",\"bright\":50,\"fx\":\"hold\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::error;
	state.brightPercent = 50;
	state.error = web_error::sensors;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, ErrorKindNtpWithoutEffectIsHold) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"error\",\"err\":\"ntp\",\"bright\":50,\"fx\":\"hold\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::error;
	state.brightPercent = 50;
	state.error = web_error::ntp;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, ErrorSensorsRandomShape) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"error\",\"err\":\"sensors\",\"bright\":50,\"fx\":\"random\",\"step\":227,\"end\":600}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::error;
	state.brightPercent = 50;
	state.error = web_error::sensors;
	state.effect = web_effect::random_led;
	state.randomLed = {50, 500};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, ErrorNtpFadingEndsDark) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"error\",\"err\":\"ntp\",\"bright\":50,\"fx\":\"static\",\"color\":\"#000000\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::error;
	state.brightPercent = 50;
	state.error = web_error::ntp;
	state.effect = web_effect::fading;
	state.fading = {0xFFFF00, 10, 0, 100, 10};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, ErrorWifiGrowingShape) {
	const size_t buf_size = 256;

	const char *exp_json
		= "{\"seq\":17,\"age\":4200,\"src\":\"error\",\"err\":\"wifi\",\"bright\":50,\"fx\":\"growing\",\"color\":\"#ff0000\",\"step\":200,\"end\":100}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::error;
	state.brightPercent = 50;
	state.error = web_error::wifi;
	state.effect = web_effect::growing;
	state.growing = {0xFF0000, 100};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, ErrorApSnakeShape) {
	const size_t buf_size = 256;

	const char *exp_json
		= "{\"seq\":17,\"age\":4200,\"src\":\"error\",\"err\":\"ap\",\"bright\":50,\"fx\":\"snake\",\"color\":\"#ff0000\",\"step\":458,\"tail\":1}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::error;
	state.brightPercent = 50;
	state.error = web_error::ap;
	state.effect = web_effect::snake;
	state.snake = {0xFF0000, 100, 1};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, ErrorFsFadingShape) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"error\",\"err\":\"fs\",\"bright\":50,\"fx\":\"fading\",\"color\":\"#ff0000\",\"from\":100,"
						   "\"to\":0,\"steps\":20,\"step\":10,\"end\":100}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::error;
	state.brightPercent = 50;
	state.error = web_error::fs;
	state.effect = web_effect::fading;
	state.fading = {0xFF0000, 10, 100, 0, 5};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, ErrorFieldOmittedOutsideErrorSource) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"static\",\"color\":\"#01ff00\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.error = web_error::wifi;
	state.effect = web_effect::static_color;
	state.staticColor = 0x01FF00;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

// ---------------------------------------------------------------- meas

TEST(WebPageDisplayTranslatorTest, MeasStandardPm25Shape) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"meas\",\"layout\":\"standard\",\"keys\":[\"pm25\"]}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = web_meas_layout::standard;
	state.keys[0] = web_meas_key::pm25;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, MeasStandardKeyTotal) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"meas\",\"layout\":\"standard\",\"keys\":[\"total\"]}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = web_meas_layout::standard;
	state.keys[0] = web_meas_key::total;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, MeasStandardKeyEco2) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"meas\",\"layout\":\"standard\",\"keys\":[\"eco2\"]}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = web_meas_layout::standard;
	state.keys[0] = web_meas_key::eco2;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, MeasStandardKeyTvoc) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"meas\",\"layout\":\"standard\",\"keys\":[\"tvoc\"]}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = web_meas_layout::standard;
	state.keys[0] = web_meas_key::tvoc;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, MeasStandardKeyTemp) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"meas\",\"layout\":\"standard\",\"keys\":[\"temp\"]}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = web_meas_layout::standard;
	state.keys[0] = web_meas_key::temp;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, MeasStandardKeyHumid) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"meas\",\"layout\":\"standard\",\"keys\":[\"humid\"]}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = web_meas_layout::standard;
	state.keys[0] = web_meas_key::humid;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, MeasStandardIgnoresSecondAndThirdKey) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"meas\",\"layout\":\"standard\",\"keys\":[\"pm25\"]}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = web_meas_layout::standard;
	state.keys[0] = web_meas_key::pm25;
	state.keys[1] = web_meas_key::eco2;
	state.keys[2] = web_meas_key::temp;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, MeasMultiShape) {
	const size_t buf_size = 256;

	const char *exp_json
		= "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"meas\",\"layout\":\"multi\",\"keys\":[\"pm25\",\"eco2\",\"temp\"]}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = web_meas_layout::multi;
	state.keys[0] = web_meas_key::pm25;
	state.keys[1] = web_meas_key::eco2;
	state.keys[2] = web_meas_key::temp;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, MeasMultiKeepsKeyOrder) {
	const size_t buf_size = 256;

	const char *exp_json
		= "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"meas\",\"layout\":\"multi\",\"keys\":[\"humid\",\"total\",\"tvoc\"]}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = web_meas_layout::multi;
	state.keys[0] = web_meas_key::humid;
	state.keys[1] = web_meas_key::total;
	state.keys[2] = web_meas_key::tvoc;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, MeasNightShape) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"meas\",\"layout\":\"night\",\"keys\":[\"pm25\"],\"white\":3}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = web_meas_layout::night;
	state.keys[0] = web_meas_key::pm25;
	state.nightHour = 6;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, MeasNightIgnoresSecondAndThirdKey) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"meas\",\"layout\":\"night\",\"keys\":[\"pm25\"],\"white\":3}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = web_meas_layout::night;
	state.keys[0] = web_meas_key::pm25;
	state.keys[1] = web_meas_key::eco2;
	state.keys[2] = web_meas_key::temp;
	state.nightHour = 6;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, MeasNightHour0WhiteRows) {
	const size_t buf_size = 256;

	// night start: no white rows
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"meas\",\"layout\":\"night\",\"keys\":[\"pm25\"],\"white\":0}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = web_meas_layout::night;
	state.keys[0] = web_meas_key::pm25;
	state.nightHour = 0;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, MeasNightHour1WhiteRows) {
	const size_t buf_size = 256;

	// 1 * 6 / 12 = 0.5, rounded up
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"meas\",\"layout\":\"night\",\"keys\":[\"pm25\"],\"white\":1}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = web_meas_layout::night;
	state.keys[0] = web_meas_key::pm25;
	state.nightHour = 1;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, MeasNightHour2WhiteRows) {
	const size_t buf_size = 256;

	// 2 * 6 / 12 = 1 exactly
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"meas\",\"layout\":\"night\",\"keys\":[\"pm25\"],\"white\":1}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = web_meas_layout::night;
	state.keys[0] = web_meas_key::pm25;
	state.nightHour = 2;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, MeasNightHour6WhiteRows) {
	const size_t buf_size = 256;

	// 6 * 6 / 12 = 3 exactly
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"meas\",\"layout\":\"night\",\"keys\":[\"pm25\"],\"white\":3}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = web_meas_layout::night;
	state.keys[0] = web_meas_key::pm25;
	state.nightHour = 6;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, MeasNightHour7WhiteRows) {
	const size_t buf_size = 256;

	// 7 * 6 / 12 = 3.5, rounded up
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"meas\",\"layout\":\"night\",\"keys\":[\"pm25\"],\"white\":4}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = web_meas_layout::night;
	state.keys[0] = web_meas_key::pm25;
	state.nightHour = 7;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, MeasNightHour11WhiteRows) {
	const size_t buf_size = 256;

	// 11 * 6 / 12 = 5.5, rounded up
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"meas\",\"layout\":\"night\",\"keys\":[\"pm25\"],\"white\":6}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = web_meas_layout::night;
	state.keys[0] = web_meas_key::pm25;
	state.nightHour = 11;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, MeasNightHour12WhiteRows) {
	const size_t buf_size = 256;

	// 12 = outside the night window: all rows white
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"meas\",\"layout\":\"night\",\"keys\":[\"pm25\"],\"white\":6}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = web_meas_layout::night;
	state.keys[0] = web_meas_key::pm25;
	state.nightHour = 12;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, MeasNightHour13WhiteRows) {
	const size_t buf_size = 256;

	// above 12 is clamped to 12
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"meas\",\"layout\":\"night\",\"keys\":[\"pm25\"],\"white\":6}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = web_meas_layout::night;
	state.keys[0] = web_meas_key::pm25;
	state.nightHour = 13;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, MeasNightHour255WhiteRows) {
	const size_t buf_size = 256;

	// far above 12 is clamped to 12
	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"meas\",\"layout\":\"night\",\"keys\":[\"pm25\"],\"white\":6}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = web_meas_layout::night;
	state.keys[0] = web_meas_key::pm25;
	state.nightHour = 255;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, MeasWhiteOnlyInNightLayout) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"meas\",\"layout\":\"standard\",\"keys\":[\"pm25\"]}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = web_meas_layout::standard;
	state.keys[0] = web_meas_key::pm25;
	state.nightHour = 6;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, MeasLayoutNoneIsHold) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"hold\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = web_meas_layout::none;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, MeasLayoutUnknownValueIsHold) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"hold\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = static_cast<web_meas_layout>(99);
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, MeasUnknownKeyFallsBackToTotal) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"meas\",\"layout\":\"standard\",\"keys\":[\"total\"]}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = web_meas_layout::standard;
	state.keys[0] = static_cast<web_meas_key>(99);
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

// ---------------------------------------------------------------- header fields

TEST(WebPageDisplayTranslatorTest, SeqAndAgePassThroughAtMaximum) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":4294967295,\"age\":4294967295,\"src\":\"none\",\"bright\":50,\"fx\":\"hold\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 4294967295u;
	state.ageMs = 4294967295u;
	state.source = web_source::none;
	state.brightPercent = 50;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, SeqAndAgeZero) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":0,\"age\":0,\"src\":\"none\",\"bright\":50,\"fx\":\"hold\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 0;
	state.ageMs = 0;
	state.source = web_source::none;
	state.brightPercent = 50;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, BrightnessZero) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"none\",\"bright\":0,\"fx\":\"hold\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::none;
	state.brightPercent = 0;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, BrightnessAt100IsKept) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"none\",\"bright\":100,\"fx\":\"hold\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::none;
	state.brightPercent = 100;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, BrightnessAbove100IsClamped) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"none\",\"bright\":100,\"fx\":\"hold\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::none;
	state.brightPercent = 101;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, BrightnessMaxByteIsClamped) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"none\",\"bright\":100,\"fx\":\"hold\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::none;
	state.brightPercent = 255;
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, UnknownSourceReturnsZeroAndEmptyString) {
	const size_t buf_size = 256;

	const size_t exp_len = 0;
	const char exp_first = '\0';

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = static_cast<web_source>(99);
	char buf[buf_size];
	std::memset(buf, 0xAA, sizeof(buf));

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_EQ(exp_first, buf[0]);
}

TEST(WebPageDisplayTranslatorTest, UnknownErrorOfErrorSourceReturnsZeroAndEmptyString) {
	const size_t buf_size = 256;

	const size_t exp_len = 0;
	const char exp_first = '\0';

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::error;
	state.error = static_cast<web_error>(99);
	char buf[buf_size];
	std::memset(buf, 0xAA, sizeof(buf));

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_EQ(exp_first, buf[0]);
}

TEST(WebPageDisplayTranslatorTest, UnknownErrorIsIgnoredForEffectSource) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"static\",\"color\":\"#01ff00\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::static_color;
	state.staticColor = 0x01FF00;
	state.error = static_cast<web_error>(99);
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, UnknownErrorIsIgnoredForNoneSource) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"none\",\"bright\":50,\"fx\":\"hold\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::none;
	state.brightPercent = 50;
	state.error = static_cast<web_error>(99);
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WebPageDisplayTranslatorTest, UnknownErrorIsIgnoredForMeasSource) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"meas\",\"bright\":50,\"fx\":\"meas\",\"layout\":\"standard\",\"keys\":[\"pm25\"]}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::meas;
	state.brightPercent = 50;
	state.layout = web_meas_layout::standard;
	state.keys[0] = web_meas_key::pm25;
	state.error = static_cast<web_error>(99);
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

// ---------------------------------------------------------------- longest output

TEST(WebPageDisplayTranslatorTest, LongestFadingOutputFitsIn256Bytes) {
	const size_t buf_size = 256;

	const char *exp_json = "{\"seq\":4294967295,\"age\":4294967295,\"src\":\"error\",\"err\":\"sensors\",\"bright\":100,\"fx\":\"fading\",\"color\":\"#"
						   "ffffff\",\"from\":100,\"to\":0,\"steps\":100,\"step\":65535,\"end\":100}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 4294967295u;
	state.ageMs = 4294967295u;
	state.source = web_source::error;
	state.brightPercent = 100;
	state.error = web_error::sensors;
	state.effect = web_effect::fading;
	state.fading = {0xFFFFFF, 65535, 100, 0, 1};
	char buf[buf_size];

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, buf_size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
}

// ---------------------------------------------------------------- buffer handling

TEST(WebPageDisplayTranslatorTest, BufferExactFitSucceeds) {
	const size_t buf_size = 256;
	const char exp_sentinel = static_cast<char>(0xAA);

	const char *exp_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"static\",\"color\":\"#01ff00\"}";
	const size_t exp_len = std::strlen(exp_json);

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::static_color;
	state.staticColor = 0x01FF00;
	char buf[buf_size];
	std::memset(buf, 0xAA, sizeof(buf));
	const size_t size = exp_len + 1; // text plus the terminating NUL

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, size);

	EXPECT_EQ(exp_len, res);
	EXPECT_STREQ(exp_json, buf);
	EXPECT_EQ(exp_sentinel, buf[size]);
}

TEST(WebPageDisplayTranslatorTest, BufferOneByteTooSmallReturnsZero) {
	const size_t buf_size = 256;
	const char exp_sentinel = static_cast<char>(0xAA);
	const char exp_first = '\0';
	const size_t exp_len = 0;

	const char *full_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"static\",\"color\":\"#01ff00\"}";

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::static_color;
	state.staticColor = 0x01FF00;
	char buf[buf_size];
	std::memset(buf, 0xAA, sizeof(buf));
	const size_t size = std::strlen(full_json); // only the NUL does not fit

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, size);

	EXPECT_EQ(exp_len, res);
	EXPECT_EQ(exp_first, buf[0]);
	EXPECT_EQ(exp_sentinel, buf[size]);
}

TEST(WebPageDisplayTranslatorTest, BufferHalfSizeReturnsZero) {
	const size_t buf_size = 256;
	const char exp_sentinel = static_cast<char>(0xAA);
	const char exp_first = '\0';
	const size_t exp_len = 0;

	const char *full_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"static\",\"color\":\"#01ff00\"}";

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::static_color;
	state.staticColor = 0x01FF00;
	char buf[buf_size];
	std::memset(buf, 0xAA, sizeof(buf));
	const size_t size = std::strlen(full_json) / 2; // runs out inside the colour field

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, size);

	EXPECT_EQ(exp_len, res);
	EXPECT_EQ(exp_first, buf[0]);
	EXPECT_EQ(exp_sentinel, buf[size]);
}

TEST(WebPageDisplayTranslatorTest, BufferTenBytesReturnsZero) {
	const size_t buf_size = 256;
	const char exp_sentinel = static_cast<char>(0xAA);
	const char exp_first = '\0';
	const size_t exp_len = 0;

	const char *full_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"static\",\"color\":\"#01ff00\"}";

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::static_color;
	state.staticColor = 0x01FF00;
	char buf[buf_size];
	std::memset(buf, 0xAA, sizeof(buf));
	const size_t size = 10; // runs out inside the first field

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, size);

	EXPECT_EQ(exp_len, res);
	EXPECT_EQ(exp_first, buf[0]);
	EXPECT_EQ(exp_sentinel, buf[size]);
}

TEST(WebPageDisplayTranslatorTest, BufferTwoBytesReturnsZero) {
	const size_t buf_size = 256;
	const char exp_sentinel = static_cast<char>(0xAA);
	const char exp_first = '\0';
	const size_t exp_len = 0;

	const char *full_json = "{\"seq\":17,\"age\":4200,\"src\":\"effect\",\"bright\":50,\"fx\":\"static\",\"color\":\"#01ff00\"}";

	web_display_state state{};
	state.seq = 17;
	state.ageMs = 4200;
	state.source = web_source::effect;
	state.brightPercent = 50;
	state.effect = web_effect::static_color;
	state.staticColor = 0x01FF00;
	char buf[buf_size];
	std::memset(buf, 0xAA, sizeof(buf));
	const size_t size = 2; // runs out right after the opening brace

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, size);

	EXPECT_EQ(exp_len, res);
	EXPECT_EQ(exp_first, buf[0]);
	EXPECT_EQ(exp_sentinel, buf[size]);
}

TEST(WebPageDisplayTranslatorTest, BufferSizeOneReturnsZeroAndTerminates) {
	const char exp_first = '\0';
	const char exp_untouched = 'x';
	const size_t exp_len = 0;

	web_display_state state{};
	state.source = web_source::none;
	char buf[4] = {'x', 'x', 'x', 'x'};
	const size_t size = 1;

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, size);

	EXPECT_EQ(exp_len, res);
	EXPECT_EQ(exp_first, buf[0]);
	EXPECT_EQ(exp_untouched, buf[1]);
}

TEST(WebPageDisplayTranslatorTest, BufferSizeZeroWritesNothing) {
	const char exp_untouched = 'x';
	const size_t exp_len = 0;

	web_display_state state{};
	state.source = web_source::none;
	char buf[4] = {'x', 'x', 'x', 'x'};
	const size_t size = 0;

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, size);

	EXPECT_EQ(exp_len, res);
	EXPECT_EQ(exp_untouched, buf[0]);
}

TEST(WebPageDisplayTranslatorTest, BufferNullReturnsZero) {
	const size_t exp_len = 0;

	web_display_state state{};
	state.source = web_source::none;
	char *buf = nullptr;
	const size_t size = 100;

	size_t res;

	res = HAQuDA_TranslateDisplayToJson(state, buf, size);

	EXPECT_EQ(exp_len, res);
}
