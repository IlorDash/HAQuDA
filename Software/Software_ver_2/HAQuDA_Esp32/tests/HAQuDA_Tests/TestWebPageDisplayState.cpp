#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "HAQuDA_WebPageDisplayState.h"

// ---------------------------------------------------------------- Init

TEST(WebPageDisplayStateTest, InitSnakeDefaults) {
	const uint8_t brightness = 50;

	const web_snake_params exp_snake = {0x99FF33, 200, 5};

	web_picture_state state{};

	HAQuDA_WebPictureInit(state, brightness);

	EXPECT_EQ(exp_snake.color, state.snake.color);
	EXPECT_EQ(exp_snake.movePauseMs, state.snake.movePauseMs);
	EXPECT_EQ(exp_snake.tailLen, state.snake.tailLen);
}

TEST(WebPageDisplayStateTest, InitGrowingDefaults) {
	const uint8_t brightness = 50;

	const web_growing_params exp_growing = {0x00FFFF, 200};

	web_picture_state state{};

	HAQuDA_WebPictureInit(state, brightness);

	EXPECT_EQ(exp_growing.color, state.growing.color);
	EXPECT_EQ(exp_growing.growPauseMs, state.growing.growPauseMs);
}

TEST(WebPageDisplayStateTest, InitRandomDefaults) {
	const uint8_t brightness = 50;

	const web_random_params exp_random = {200, 1000};

	web_picture_state state{};

	HAQuDA_WebPictureInit(state, brightness);

	EXPECT_EQ(exp_random.pauseMs, state.randomLed.pauseMs);
	EXPECT_EQ(exp_random.endPauseMs, state.randomLed.endPauseMs);
}

TEST(WebPageDisplayStateTest, InitFadingDefaults) {
	const uint8_t brightness = 50;

	const web_fading_params exp_fading = {0x00FFFF, 10, 100, 0, 5};

	web_picture_state state{};

	HAQuDA_WebPictureInit(state, brightness);

	EXPECT_EQ(exp_fading.color, state.fading.color);
	EXPECT_EQ(exp_fading.fadePauseMs, state.fading.fadePauseMs);
	EXPECT_EQ(exp_fading.startBright, state.fading.startBright);
	EXPECT_EQ(exp_fading.stopBright, state.fading.stopBright);
	EXPECT_EQ(exp_fading.step, state.fading.step);
}

TEST(WebPageDisplayStateTest, InitStaticColorDefault) {
	const uint8_t brightness = 50;

	const uint32_t exp_color = 0x01FF00;

	web_picture_state state{};

	HAQuDA_WebPictureInit(state, brightness);

	EXPECT_EQ(exp_color, state.staticColor);
}

TEST(WebPageDisplayStateTest, InitMultiKeysDefaultToTempEco2Pm25) {
	const uint8_t brightness = 50;

	const web_meas_key exp_keys[3] = {web_meas_key::temp, web_meas_key::eco2, web_meas_key::pm25};

	web_picture_state state{};

	HAQuDA_WebPictureInit(state, brightness);

	EXPECT_EQ(exp_keys[0], state.multiKeys[0].key);
	EXPECT_EQ(exp_keys[1], state.multiKeys[1].key);
	EXPECT_EQ(exp_keys[2], state.multiKeys[2].key);
	EXPECT_TRUE(state.multiKeys[0].valid);
	EXPECT_TRUE(state.multiKeys[1].valid);
	EXPECT_TRUE(state.multiKeys[2].valid);
}

TEST(WebPageDisplayStateTest, InitStandardKeyIsInvalid) {
	const uint8_t brightness = 50;

	web_picture_state state{};

	HAQuDA_WebPictureInit(state, brightness);

	EXPECT_FALSE(state.standardKey.valid);
}

TEST(WebPageDisplayStateTest, InitNightWindowIs21To9) {
	const uint8_t brightness = 50;

	const uint8_t exp_start = 21;
	const uint8_t exp_finish = 9;

	web_picture_state state{};

	HAQuDA_WebPictureInit(state, brightness);

	EXPECT_EQ(exp_start, state.nightStart);
	EXPECT_EQ(exp_finish, state.nightFinish);
}

TEST(WebPageDisplayStateTest, InitSourceEffectLayoutAndErrorAreNone) {
	const uint8_t brightness = 50;

	const web_source exp_source = web_source::none;
	const web_effect exp_effect = web_effect::none;
	const web_meas_layout exp_layout = web_meas_layout::none;
	const web_error exp_error = web_error::none;

	web_picture_state state{};

	HAQuDA_WebPictureInit(state, brightness);

	EXPECT_EQ(exp_source, state.source);
	EXPECT_EQ(exp_effect, state.effect);
	EXPECT_EQ(exp_layout, state.layout);
	EXPECT_EQ(exp_error, state.seenError);
}

TEST(WebPageDisplayStateTest, InitSeqAndStartAreZero) {
	const uint8_t brightness = 50;

	const uint32_t exp_zero = 0;

	web_picture_state state{};

	HAQuDA_WebPictureInit(state, brightness);

	EXPECT_EQ(exp_zero, state.seq);
	EXPECT_EQ(exp_zero, state.startMs);
}

TEST(WebPageDisplayStateTest, InitStoresBrightness) {
	const uint8_t brightness = 50;

	const uint8_t exp_brightness = 50;

	web_picture_state state{};

	HAQuDA_WebPictureInit(state, brightness);

	EXPECT_EQ(exp_brightness, state.brightPercent);
}

TEST(WebPageDisplayStateTest, InitResetsDirtyState) {
	const uint8_t brightness = 50;
	const uint8_t dirty_byte = 0xFF;

	const uint32_t exp_zero = 0;
	const web_source exp_source = web_source::none;
	const web_error exp_error = web_error::none;
	const web_effect exp_effect = web_effect::none;
	const web_meas_layout exp_layout = web_meas_layout::none;
	const uint8_t exp_night_start = 21;

	web_picture_state state;
	std::memset(&state, dirty_byte, sizeof(state));

	HAQuDA_WebPictureInit(state, brightness);

	EXPECT_EQ(exp_zero, state.seq);
	EXPECT_EQ(exp_zero, state.startMs);
	EXPECT_EQ(exp_source, state.source);
	EXPECT_EQ(exp_error, state.seenError);
	EXPECT_EQ(exp_effect, state.effect);
	EXPECT_EQ(exp_layout, state.layout);
	EXPECT_FALSE(state.standardKey.valid);
	EXPECT_EQ(exp_night_start, state.nightStart);
	EXPECT_EQ(brightness, state.brightPercent);
}

// ---------------------------------------------------------------- SetSource

TEST(WebPageDisplayStateTest, SetSourceSameSourceReturnsFalseAndChangesNothing) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_error current_error = web_error::ntp;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetSource(state, web_source::meas, current_error, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
}

TEST(WebPageDisplayStateTest, SetSourceSameErrorSourceKeepsSeenError) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_error exp_seen = web_error::wifi;

	const web_error current_error = web_error::ntp;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::error;
	state.seenError = web_error::wifi;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetSource(state, web_source::error, current_error, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
	EXPECT_EQ(exp_seen, state.seenError);
}

TEST(WebPageDisplayStateTest, SetSourceDifferentSourceBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const web_error current_error = web_error::ntp;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetSource(state, web_source::effect, current_error, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetSourceFromNoneToEventBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const web_error current_error = web_error::ntp;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetSource(state, web_source::event, current_error, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetSourceToMeasStopsEffect) {
	const uint8_t brightness = 50;
	const uint32_t now_ms = 7000;

	const web_effect exp_effect = web_effect::none;

	const web_error current_error = web_error::ntp;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::snake;

	bool res;

	res = HAQuDA_WebPictureSetSource(state, web_source::meas, current_error, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_effect, state.effect);
}

TEST(WebPageDisplayStateTest, SetSourceToNoneStopsEffect) {
	const uint8_t brightness = 50;
	const uint32_t now_ms = 7000;

	const web_effect exp_effect = web_effect::none;

	const web_error current_error = web_error::ntp;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::fading;

	bool res;

	res = HAQuDA_WebPictureSetSource(state, web_source::none, current_error, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_effect, state.effect);
}

TEST(WebPageDisplayStateTest, SetSourceToErrorStopsEffect) {
	const uint8_t brightness = 50;
	const uint32_t now_ms = 7000;

	const web_effect exp_effect = web_effect::none;

	const web_error current_error = web_error::ntp;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::growing;

	bool res;

	res = HAQuDA_WebPictureSetSource(state, web_source::error, current_error, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_effect, state.effect);
}

TEST(WebPageDisplayStateTest, SetSourceToEventStopsEffect) {
	const uint8_t brightness = 50;
	const uint32_t now_ms = 7000;

	const web_effect exp_effect = web_effect::none;

	const web_error current_error = web_error::ntp;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::random_led;

	bool res;

	res = HAQuDA_WebPictureSetSource(state, web_source::event, current_error, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_effect, state.effect);
}

TEST(WebPageDisplayStateTest, SetSourceToEffectKeepsEffect) {
	const uint8_t brightness = 50;
	const uint32_t now_ms = 7000;

	const web_effect exp_effect = web_effect::growing;

	const web_error current_error = web_error::ntp;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.effect = web_effect::growing;

	bool res;

	res = HAQuDA_WebPictureSetSource(state, web_source::effect, current_error, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_effect, state.effect);
}

TEST(WebPageDisplayStateTest, SetSourceToErrorStoresSeenError) {
	const uint8_t brightness = 50;
	const uint32_t now_ms = 7000;

	const web_error exp_seen = web_error::ntp;

	const web_error current_error = web_error::ntp;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.effect = web_effect::none;
	state.seenError = web_error::wifi;

	bool res;

	res = HAQuDA_WebPictureSetSource(state, web_source::error, current_error, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seen, state.seenError);
}

TEST(WebPageDisplayStateTest, SetSourceToMeasDoesNotTouchSeenError) {
	const uint8_t brightness = 50;
	const uint32_t now_ms = 7000;

	const web_error exp_seen = web_error::wifi;

	const web_error current_error = web_error::ntp;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::none;
	state.seenError = web_error::wifi;

	bool res;

	res = HAQuDA_WebPictureSetSource(state, web_source::meas, current_error, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seen, state.seenError);
}

TEST(WebPageDisplayStateTest, SetSourceToEffectDoesNotTouchSeenError) {
	const uint8_t brightness = 50;
	const uint32_t now_ms = 7000;

	const web_error exp_seen = web_error::wifi;

	const web_error current_error = web_error::ntp;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.effect = web_effect::none;
	state.seenError = web_error::wifi;

	bool res;

	res = HAQuDA_WebPictureSetSource(state, web_source::effect, current_error, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seen, state.seenError);
}

// ---------------------------------------------------------------- SetEffect

TEST(WebPageDisplayStateTest, SetEffectInEffectSourceBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffect(state, web_effect::snake, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetEffectToNoneInEffectSourceBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::snake;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffect(state, web_effect::none, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetEffectSameEffectReturnsFalse) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::snake;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffect(state, web_effect::snake, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
}

TEST(WebPageDisplayStateTest, SetEffectInMeasSourceStoresWithoutBump) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_effect exp_effect = web_effect::snake;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffect(state, web_effect::snake, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
	EXPECT_EQ(exp_effect, state.effect);
}

TEST(WebPageDisplayStateTest, SetEffectInErrorSourceStoresWithoutBump) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_effect exp_effect = web_effect::snake;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::error;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffect(state, web_effect::snake, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
	EXPECT_EQ(exp_effect, state.effect);
}

// ---------------------------------------------------------------- SetEffectParam and SetStaticColor

TEST(WebPageDisplayStateTest, SetEffectParamSnakeColorChangeBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const web_snake_params params = {0x112233, 200, 5};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::snake;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffectParam(state, params, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetEffectParamSnakeMovePauseChangeBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const web_snake_params params = {0x99FF33, 300, 5};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::snake;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffectParam(state, params, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetEffectParamSnakeTailLenChangeBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const web_snake_params params = {0x99FF33, 200, 6};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::snake;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffectParam(state, params, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetEffectParamSnakeIdenticalReturnsFalse) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_snake_params params = {0x99FF33, 200, 5};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::snake;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffectParam(state, params, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
}

TEST(WebPageDisplayStateTest, SetEffectParamSnakeOtherEffectSelectedStoresWithoutBump) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_snake_params params = {0x112233, 200, 5};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::growing;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffectParam(state, params, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
	EXPECT_EQ(params.color, state.snake.color);
}

TEST(WebPageDisplayStateTest, SetEffectParamSnakeOtherSourceStoresWithoutBump) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_snake_params params = {0x112233, 200, 5};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.effect = web_effect::snake;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffectParam(state, params, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
	EXPECT_EQ(params.color, state.snake.color);
}

TEST(WebPageDisplayStateTest, SetEffectParamGrowingColorChangeBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const web_growing_params params = {0x112233, 200};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::growing;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffectParam(state, params, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetEffectParamGrowingGrowPauseChangeBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const web_growing_params params = {0x00FFFF, 300};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::growing;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffectParam(state, params, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetEffectParamGrowingIdenticalReturnsFalse) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_growing_params params = {0x00FFFF, 200};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::growing;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffectParam(state, params, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
}

TEST(WebPageDisplayStateTest, SetEffectParamGrowingOtherEffectSelectedStoresWithoutBump) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_growing_params params = {0x112233, 200};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::snake;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffectParam(state, params, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
	EXPECT_EQ(params.color, state.growing.color);
}

TEST(WebPageDisplayStateTest, SetEffectParamGrowingOtherSourceStoresWithoutBump) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_growing_params params = {0x112233, 200};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.effect = web_effect::growing;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffectParam(state, params, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
	EXPECT_EQ(params.color, state.growing.color);
}

TEST(WebPageDisplayStateTest, SetEffectParamRandomPauseChangeBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const web_random_params params = {300, 1000};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::random_led;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffectParam(state, params, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetEffectParamRandomEndPauseChangeBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const web_random_params params = {200, 2000};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::random_led;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffectParam(state, params, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetEffectParamRandomIdenticalReturnsFalse) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_random_params params = {200, 1000};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::random_led;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffectParam(state, params, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
}

TEST(WebPageDisplayStateTest, SetEffectParamRandomOtherEffectSelectedStoresWithoutBump) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_random_params params = {300, 1000};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::snake;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffectParam(state, params, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
	EXPECT_EQ(params.pauseMs, state.randomLed.pauseMs);
}

TEST(WebPageDisplayStateTest, SetEffectParamRandomOtherSourceStoresWithoutBump) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_random_params params = {300, 1000};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.effect = web_effect::random_led;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffectParam(state, params, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
	EXPECT_EQ(params.pauseMs, state.randomLed.pauseMs);
}

TEST(WebPageDisplayStateTest, SetEffectParamFadingColorChangeBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const web_fading_params params = {0x112233, 10, 100, 0, 5};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::fading;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffectParam(state, params, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetEffectParamFadingFadePauseChangeBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const web_fading_params params = {0x00FFFF, 20, 100, 0, 5};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::fading;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffectParam(state, params, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetEffectParamFadingStartBrightChangeBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const web_fading_params params = {0x00FFFF, 10, 90, 0, 5};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::fading;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffectParam(state, params, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetEffectParamFadingStopBrightChangeBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const web_fading_params params = {0x00FFFF, 10, 100, 10, 5};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::fading;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffectParam(state, params, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetEffectParamFadingStepChangeBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const web_fading_params params = {0x00FFFF, 10, 100, 0, 6};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::fading;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffectParam(state, params, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetEffectParamFadingIdenticalReturnsFalse) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_fading_params params = {0x00FFFF, 10, 100, 0, 5};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::fading;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffectParam(state, params, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
}

TEST(WebPageDisplayStateTest, SetEffectParamFadingOtherEffectSelectedStoresWithoutBump) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_fading_params params = {0x112233, 10, 100, 0, 5};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::snake;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffectParam(state, params, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
	EXPECT_EQ(params.color, state.fading.color);
}

TEST(WebPageDisplayStateTest, SetEffectParamFadingOtherSourceStoresWithoutBump) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_fading_params params = {0x112233, 10, 100, 0, 5};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.effect = web_effect::fading;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetEffectParam(state, params, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
	EXPECT_EQ(params.color, state.fading.color);
}

TEST(WebPageDisplayStateTest, SetStaticColorChangeBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const uint32_t color = 0x112233;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::static_color;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetStaticColor(state, color, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetStaticColorIdenticalReturnsFalse) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t color = 0x01FF00;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::static_color;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetStaticColor(state, color, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
}

TEST(WebPageDisplayStateTest, SetStaticColorOtherEffectSelectedStoresWithoutBump) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t color = 0x112233;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = web_effect::snake;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetStaticColor(state, color, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
	EXPECT_EQ(color, state.staticColor);
}

TEST(WebPageDisplayStateTest, SetStaticColorOtherSourceStoresWithoutBump) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t color = 0x112233;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.effect = web_effect::static_color;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetStaticColor(state, color, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
	EXPECT_EQ(color, state.staticColor);
}

// ---------------------------------------------------------------- SetLayout

TEST(WebPageDisplayStateTest, SetLayoutNoneIsIgnored) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_meas_layout exp_layout = web_meas_layout::standard;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::standard;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetLayout(state, web_meas_layout::none, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
	EXPECT_EQ(exp_layout, state.layout);
}

TEST(WebPageDisplayStateTest, SetLayoutSameLayoutReturnsFalse) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::multi;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetLayout(state, web_meas_layout::multi, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
}

TEST(WebPageDisplayStateTest, SetLayoutChangeInMeasSourceBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::standard;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetLayout(state, web_meas_layout::multi, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetLayoutFromNoneInMeasSourceBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetLayout(state, web_meas_layout::night, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetLayoutChangeInEffectSourceStoresWithoutBump) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_meas_layout exp_layout = web_meas_layout::night;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.layout = web_meas_layout::standard;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetLayout(state, web_meas_layout::night, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
	EXPECT_EQ(exp_layout, state.layout);
}

// ---------------------------------------------------------------- SetStandardKey

TEST(WebPageDisplayStateTest, SetStandardKeyChangeInMeasStandardBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const web_key key = {web_meas_key::eco2, true};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::standard;
	state.standardKey = {web_meas_key::temp, true};
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetStandardKey(state, key, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetStandardKeyChangeInMeasNightBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const web_key key = {web_meas_key::eco2, true};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::night;
	state.standardKey = {web_meas_key::temp, true};
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetStandardKey(state, key, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetStandardKeyValidToInvalidBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const web_key key = {web_meas_key::temp, false};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::standard;
	state.standardKey = {web_meas_key::temp, true};
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetStandardKey(state, key, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetStandardKeyInvalidToValidBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const web_key key = {web_meas_key::temp, true};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::standard;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetStandardKey(state, key, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetStandardKeyIdenticalReturnsFalse) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_key key = {web_meas_key::temp, true};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::standard;
	state.standardKey = {web_meas_key::temp, true};
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetStandardKey(state, key, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
}

TEST(WebPageDisplayStateTest, SetStandardKeyTwoInvalidKeysAreEqual) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_key key = {web_meas_key::humid, false};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::standard;
	state.standardKey = {web_meas_key::total, false};
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetStandardKey(state, key, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
}

TEST(WebPageDisplayStateTest, SetStandardKeyInMeasMultiStoresWithoutBump) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_meas_key exp_key = web_meas_key::eco2;

	const web_key key = {web_meas_key::eco2, true};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::multi;
	state.standardKey = {web_meas_key::temp, true};
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetStandardKey(state, key, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
	EXPECT_EQ(exp_key, state.standardKey.key);
}

TEST(WebPageDisplayStateTest, SetStandardKeyInMeasLayoutNoneStoresWithoutBump) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_meas_key exp_key = web_meas_key::eco2;

	const web_key key = {web_meas_key::eco2, true};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.standardKey = {web_meas_key::temp, true};
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetStandardKey(state, key, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
	EXPECT_EQ(exp_key, state.standardKey.key);
}

TEST(WebPageDisplayStateTest, SetStandardKeyOutsideMeasStoresWithoutBump) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_meas_key exp_key = web_meas_key::eco2;

	const web_key key = {web_meas_key::eco2, true};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.layout = web_meas_layout::standard;
	state.standardKey = {web_meas_key::temp, true};
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetStandardKey(state, key, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
	EXPECT_EQ(exp_key, state.standardKey.key);
}

// ---------------------------------------------------------------- SetMultiKeys

TEST(WebPageDisplayStateTest, SetMultiKeysFirstKeyChangeBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const web_key keys[3] = {{web_meas_key::humid, true}, {web_meas_key::eco2, true}, {web_meas_key::pm25, true}};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::multi;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetMultiKeys(state, keys, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetMultiKeysSecondKeyChangeBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const web_key keys[3] = {{web_meas_key::temp, true}, {web_meas_key::tvoc, true}, {web_meas_key::pm25, true}};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::multi;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetMultiKeys(state, keys, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetMultiKeysThirdKeyChangeBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const web_key keys[3] = {{web_meas_key::temp, true}, {web_meas_key::eco2, true}, {web_meas_key::total, true}};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::multi;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetMultiKeys(state, keys, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetMultiKeysSecondKeyBecomesMissingBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const web_key keys[3] = {{web_meas_key::temp, true}, {web_meas_key::eco2, false}, {web_meas_key::pm25, true}};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::multi;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetMultiKeys(state, keys, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetMultiKeysIdenticalReturnsFalse) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_key keys[3] = {{web_meas_key::temp, true}, {web_meas_key::eco2, true}, {web_meas_key::pm25, true}};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::multi;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetMultiKeys(state, keys, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
}

TEST(WebPageDisplayStateTest, SetMultiKeysInMeasStandardStoresWithoutBump) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_meas_key exp_first = web_meas_key::humid;
	const web_meas_key exp_second = web_meas_key::tvoc;
	const web_meas_key exp_third = web_meas_key::total;

	const web_key keys[3] = {{web_meas_key::humid, true}, {web_meas_key::tvoc, true}, {web_meas_key::total, true}};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::standard;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetMultiKeys(state, keys, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
	EXPECT_EQ(exp_first, state.multiKeys[0].key);
	EXPECT_EQ(exp_second, state.multiKeys[1].key);
	EXPECT_EQ(exp_third, state.multiKeys[2].key);
}

TEST(WebPageDisplayStateTest, SetMultiKeysOutsideMeasStoresWithoutBump) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_meas_key exp_first = web_meas_key::humid;
	const web_meas_key exp_second = web_meas_key::tvoc;
	const web_meas_key exp_third = web_meas_key::total;

	const web_key keys[3] = {{web_meas_key::humid, true}, {web_meas_key::tvoc, true}, {web_meas_key::total, true}};
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.layout = web_meas_layout::multi;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetMultiKeys(state, keys, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
	EXPECT_EQ(exp_first, state.multiKeys[0].key);
	EXPECT_EQ(exp_second, state.multiKeys[1].key);
	EXPECT_EQ(exp_third, state.multiKeys[2].key);
}

// ---------------------------------------------------------------- SetNightWindow

TEST(WebPageDisplayStateTest, SetNightWindowStartChangeBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const uint8_t start = 22;
	const uint8_t finish = 9;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::night;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetNightWindow(state, start, finish, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetNightWindowFinishChangeBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const uint8_t start = 21;
	const uint8_t finish = 8;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::night;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetNightWindow(state, start, finish, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, SetNightWindowIdenticalReturnsFalse) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint8_t start = 21;
	const uint8_t finish = 9;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::night;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetNightWindow(state, start, finish, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
}

TEST(WebPageDisplayStateTest, SetNightWindowInMeasStandardStoresWithoutBump) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint8_t start = 22;
	const uint8_t finish = 8;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::standard;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetNightWindow(state, start, finish, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
	EXPECT_EQ(start, state.nightStart);
	EXPECT_EQ(finish, state.nightFinish);
}

TEST(WebPageDisplayStateTest, SetNightWindowOutsideMeasStoresWithoutBump) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint8_t start = 22;
	const uint8_t finish = 8;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.layout = web_meas_layout::night;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureSetNightWindow(state, start, finish, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
	EXPECT_EQ(start, state.nightStart);
	EXPECT_EQ(finish, state.nightFinish);
}

// ---------------------------------------------------------------- CheckError

TEST(WebPageDisplayStateTest, CheckErrorDifferentKindInErrorSourceBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const web_error current_error = web_error::ntp;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::error;
	state.seenError = web_error::wifi;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureCheckError(state, current_error, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, CheckErrorKindClearedInErrorSourceBumps) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 5;

	const web_error current_error = web_error::none;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::error;
	state.seenError = web_error::wifi;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureCheckError(state, current_error, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seq, state.seq);
	EXPECT_EQ(now_ms, state.startMs);
}

TEST(WebPageDisplayStateTest, CheckErrorSameKindReturnsFalse) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_error current_error = web_error::ntp;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::error;
	state.seenError = web_error::ntp;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureCheckError(state, current_error, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
}

TEST(WebPageDisplayStateTest, CheckErrorOutsideErrorSourceReturnsFalseAndKeepsSeenError) {
	const uint8_t brightness = 50;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;
	const uint32_t now_ms = 7000;

	const web_error exp_seen = web_error::wifi;

	const web_error current_error = web_error::ntp;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.seenError = web_error::wifi;
	state.seq = seq_before;
	state.startMs = start_before;

	bool res;

	res = HAQuDA_WebPictureCheckError(state, current_error, now_ms);

	EXPECT_FALSE(res);
	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
	EXPECT_EQ(exp_seen, state.seenError);
}

TEST(WebPageDisplayStateTest, CheckErrorDifferentKindUpdatesSeenError) {
	const uint8_t brightness = 50;
	const uint32_t now_ms = 7000;

	const web_error exp_seen = web_error::ntp;

	const web_error current_error = web_error::ntp;
	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::error;
	state.seenError = web_error::wifi;

	bool res;

	res = HAQuDA_WebPictureCheckError(state, current_error, now_ms);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_seen, state.seenError);
}

// ---------------------------------------------------------------- SetBrightness

TEST(WebPageDisplayStateTest, SetBrightnessStoresValue) {
	const uint8_t start_brightness = 50;
	const uint8_t new_brightness = 80;

	const uint8_t exp_brightness = 80;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, start_brightness);

	HAQuDA_WebPictureSetBrightness(state, new_brightness);

	EXPECT_EQ(exp_brightness, state.brightPercent);
}

TEST(WebPageDisplayStateTest, SetBrightnessDoesNotTouchSeqOrStart) {
	const uint8_t start_brightness = 50;
	const uint8_t new_brightness = 80;
	const uint32_t seq_before = 4;
	const uint32_t start_before = 1000;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, start_brightness);
	state.seq = seq_before;
	state.startMs = start_before;

	HAQuDA_WebPictureSetBrightness(state, new_brightness);

	EXPECT_EQ(seq_before, state.seq);
	EXPECT_EQ(start_before, state.startMs);
}

// ---------------------------------------------------------------- Build

TEST(WebPageDisplayStateTest, BuildAgeIsNowMinusStart) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::none;
	const bool hour_known = true;
	const uint8_t hour = 23;
	const uint32_t now_ms = 1500;
	const uint32_t start_ms = 1000;

	const uint32_t exp_age = 500;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.startMs = start_ms;
	web_display_state out{};

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_age, out.ageMs);
}

TEST(WebPageDisplayStateTest, BuildAgeSurvivesMillisWrap) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::none;
	const bool hour_known = true;
	const uint8_t hour = 23;
	const uint32_t now_ms = 100;
	const uint32_t start_ms = UINT32_MAX - 99;

	const uint32_t exp_age = 200;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.startMs = start_ms;
	web_display_state out{};

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_age, out.ageMs);
}

TEST(WebPageDisplayStateTest, BuildAgeIsZeroAtStartTime) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::none;
	const bool hour_known = true;
	const uint8_t hour = 23;
	const uint32_t now_ms = 1000;
	const uint32_t start_ms = 1000;

	const uint32_t exp_age = 0;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.startMs = start_ms;
	web_display_state out{};

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_age, out.ageMs);
}

TEST(WebPageDisplayStateTest, BuildCopiesSeqSourceAndBrightness) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::none;
	const bool hour_known = true;
	const uint8_t hour = 23;
	const uint32_t now_ms = 7000;

	const uint32_t exp_seq = 42;
	const web_source exp_source = web_source::meas;
	const uint8_t exp_brightness = 50;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.seq = exp_seq;
	state.source = web_source::meas;
	web_display_state out{};

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_seq, out.seq);
	EXPECT_EQ(exp_source, out.source);
	EXPECT_EQ(exp_brightness, out.brightPercent);
}

TEST(WebPageDisplayStateTest, BuildEffectSourceCopiesEffectAndAllParams) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::none;
	const bool hour_known = true;
	const uint8_t hour = 23;
	const uint32_t now_ms = 7000;

	const web_effect exp_effect = web_effect::fading;
	const web_snake_params exp_snake = {0x111111, 301, 7};
	const web_growing_params exp_growing = {0x222222, 302};
	const web_random_params exp_random = {303, 304};
	const web_fading_params exp_fading = {0x333333, 305, 90, 10, 3};
	const uint32_t exp_static = 0x444444;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.effect = exp_effect;
	state.snake = exp_snake;
	state.growing = exp_growing;
	state.randomLed = exp_random;
	state.fading = exp_fading;
	state.staticColor = exp_static;
	web_display_state out{};

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_effect, out.effect);
	EXPECT_EQ(exp_snake.color, out.snake.color);
	EXPECT_EQ(exp_snake.movePauseMs, out.snake.movePauseMs);
	EXPECT_EQ(exp_snake.tailLen, out.snake.tailLen);
	EXPECT_EQ(exp_growing.color, out.growing.color);
	EXPECT_EQ(exp_growing.growPauseMs, out.growing.growPauseMs);
	EXPECT_EQ(exp_random.pauseMs, out.randomLed.pauseMs);
	EXPECT_EQ(exp_random.endPauseMs, out.randomLed.endPauseMs);
	EXPECT_EQ(exp_fading.color, out.fading.color);
	EXPECT_EQ(exp_fading.fadePauseMs, out.fading.fadePauseMs);
	EXPECT_EQ(exp_fading.startBright, out.fading.startBright);
	EXPECT_EQ(exp_fading.stopBright, out.fading.stopBright);
	EXPECT_EQ(exp_fading.step, out.fading.step);
	EXPECT_EQ(exp_static, out.staticColor);
}

TEST(WebPageDisplayStateTest, BuildEffectSourceHasNoErrorOrLayout) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::wifi;
	const bool hour_known = true;
	const uint8_t hour = 23;
	const uint32_t now_ms = 7000;

	const web_error exp_error = web_error::none;
	const web_meas_layout exp_layout = web_meas_layout::none;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::effect;
	state.layout = web_meas_layout::multi;
	web_display_state out{};

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_error, out.error);
	EXPECT_EQ(exp_layout, out.layout);
}

TEST(WebPageDisplayStateTest, BuildErrorSourceTakesCurrentErrorNotSeenError) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::ntp;
	const bool hour_known = true;
	const uint8_t hour = 23;
	const uint32_t now_ms = 7000;

	const web_error exp_error = web_error::ntp;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::error;
	state.seenError = web_error::wifi;
	web_display_state out{};

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_error, out.error);
}

TEST(WebPageDisplayStateTest, BuildErrorSourceHasNoEffect) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::ntp;
	const bool hour_known = true;
	const uint8_t hour = 23;
	const uint32_t now_ms = 7000;

	const web_effect exp_effect = web_effect::none;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::error;
	state.effect = web_effect::snake;
	web_display_state out{};

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_effect, out.effect);
}

TEST(WebPageDisplayStateTest, BuildEventSourceIsBare) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::ntp;
	const bool hour_known = true;
	const uint8_t hour = 23;
	const uint32_t now_ms = 7000;

	const web_effect exp_effect = web_effect::none;
	const web_error exp_error = web_error::none;
	const web_meas_layout exp_layout = web_meas_layout::none;
	const uint8_t exp_night_hour = 0;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::event;
	state.effect = web_effect::snake;
	state.layout = web_meas_layout::multi;
	web_display_state out{};

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_effect, out.effect);
	EXPECT_EQ(exp_error, out.error);
	EXPECT_EQ(exp_layout, out.layout);
	EXPECT_EQ(exp_night_hour, out.nightHour);
}

TEST(WebPageDisplayStateTest, BuildNoneSourceIsBare) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::ntp;
	const bool hour_known = true;
	const uint8_t hour = 23;
	const uint32_t now_ms = 7000;

	const web_effect exp_effect = web_effect::none;
	const web_error exp_error = web_error::none;
	const web_meas_layout exp_layout = web_meas_layout::none;
	const uint8_t exp_night_hour = 0;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::none;
	state.effect = web_effect::snake;
	state.layout = web_meas_layout::multi;
	web_display_state out{};

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_effect, out.effect);
	EXPECT_EQ(exp_error, out.error);
	EXPECT_EQ(exp_layout, out.layout);
	EXPECT_EQ(exp_night_hour, out.nightHour);
}

TEST(WebPageDisplayStateTest, BuildMeasStandardUsesStandardKey) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::none;
	const bool hour_known = true;
	const uint8_t hour = 23;
	const uint32_t now_ms = 7000;

	const web_meas_layout exp_layout = web_meas_layout::standard;
	const web_meas_key exp_key = web_meas_key::pm25;
	const web_meas_key exp_unused = web_meas_key::total;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::standard;
	state.standardKey = {web_meas_key::pm25, true};
	state.multiKeys[1] = {web_meas_key::humid, true};
	web_display_state out{};

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_layout, out.layout);
	EXPECT_EQ(exp_key, out.keys[0]);
	EXPECT_EQ(exp_unused, out.keys[1]);
	EXPECT_EQ(exp_unused, out.keys[2]);
}

TEST(WebPageDisplayStateTest, BuildMeasStandardMissingKeyMakesLayoutNone) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::none;
	const bool hour_known = true;
	const uint8_t hour = 23;
	const uint32_t now_ms = 7000;

	const web_meas_layout exp_layout = web_meas_layout::none;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::standard;
	state.standardKey = {web_meas_key::pm25, false};
	web_display_state out{};

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_layout, out.layout);
}

TEST(WebPageDisplayStateTest, BuildMeasNightUsesStandardKey) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::none;
	const bool hour_known = true;
	const uint8_t hour = 23;
	const uint32_t now_ms = 7000;

	const web_meas_layout exp_layout = web_meas_layout::night;
	const web_meas_key exp_key = web_meas_key::pm25;
	const web_meas_key exp_unused = web_meas_key::total;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::night;
	state.standardKey = {web_meas_key::pm25, true};
	state.multiKeys[1] = {web_meas_key::humid, true};
	web_display_state out{};

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_layout, out.layout);
	EXPECT_EQ(exp_key, out.keys[0]);
	EXPECT_EQ(exp_unused, out.keys[1]);
	EXPECT_EQ(exp_unused, out.keys[2]);
}

TEST(WebPageDisplayStateTest, BuildMeasNightMissingKeyMakesLayoutNone) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::none;
	const bool hour_known = true;
	const uint8_t hour = 23;
	const uint32_t now_ms = 7000;

	const web_meas_layout exp_layout = web_meas_layout::none;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::night;
	state.standardKey = {web_meas_key::pm25, false};
	web_display_state out{};

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_layout, out.layout);
}

TEST(WebPageDisplayStateTest, BuildMeasMultiCopiesAllThreeKeys) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::none;
	const bool hour_known = true;
	const uint8_t hour = 23;
	const uint32_t now_ms = 7000;

	const web_meas_layout exp_layout = web_meas_layout::multi;
	const web_meas_key exp_keys[3] = {web_meas_key::humid, web_meas_key::tvoc, web_meas_key::total};

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::multi;
	state.multiKeys[0] = {exp_keys[0], true};
	state.multiKeys[1] = {exp_keys[1], true};
	state.multiKeys[2] = {exp_keys[2], true};
	web_display_state out{};

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_layout, out.layout);
	EXPECT_EQ(exp_keys[0], out.keys[0]);
	EXPECT_EQ(exp_keys[1], out.keys[1]);
	EXPECT_EQ(exp_keys[2], out.keys[2]);
}

TEST(WebPageDisplayStateTest, BuildMeasMultiFirstKeyMissingMakesLayoutNone) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::none;
	const bool hour_known = true;
	const uint8_t hour = 23;
	const uint32_t now_ms = 7000;

	const web_meas_layout exp_layout = web_meas_layout::none;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::multi;
	state.multiKeys[0].valid = false;
	web_display_state out{};

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_layout, out.layout);
}

TEST(WebPageDisplayStateTest, BuildMeasMultiSecondKeyMissingMakesLayoutNone) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::none;
	const bool hour_known = true;
	const uint8_t hour = 23;
	const uint32_t now_ms = 7000;

	const web_meas_layout exp_layout = web_meas_layout::none;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::multi;
	state.multiKeys[1].valid = false;
	web_display_state out{};

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_layout, out.layout);
}

TEST(WebPageDisplayStateTest, BuildMeasMultiThirdKeyMissingMakesLayoutNone) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::none;
	const bool hour_known = true;
	const uint8_t hour = 23;
	const uint32_t now_ms = 7000;

	const web_meas_layout exp_layout = web_meas_layout::none;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::multi;
	state.multiKeys[2].valid = false;
	web_display_state out{};

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_layout, out.layout);
}

TEST(WebPageDisplayStateTest, BuildMeasLayoutNoneStaysNone) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::none;
	const bool hour_known = true;
	const uint8_t hour = 23;
	const uint32_t now_ms = 7000;

	const web_meas_layout exp_layout = web_meas_layout::none;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	web_display_state out{};

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_layout, out.layout);
}

TEST(WebPageDisplayStateTest, BuildMeasSourceHasNoEffectOrError) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::ntp;
	const bool hour_known = true;
	const uint8_t hour = 23;
	const uint32_t now_ms = 7000;

	const web_effect exp_effect = web_effect::none;
	const web_error exp_error = web_error::none;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::standard;
	state.effect = web_effect::snake;
	web_display_state out{};

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_effect, out.effect);
	EXPECT_EQ(exp_error, out.error);
}

TEST(WebPageDisplayStateTest, BuildMeasHourUnknownGivesNightHourZero) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::none;
	const bool hour_known = false;
	const uint8_t hour = 23;
	const uint32_t now_ms = 7000;

	const uint8_t exp_night_hour = 0;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::night;
	state.standardKey = {web_meas_key::pm25, true};
	web_display_state out{};

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_night_hour, out.nightHour);
}

TEST(WebPageDisplayStateTest, BuildMeasHourKnownUsesDefaultNightWindow) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::none;
	const bool hour_known = true;
	const uint8_t hour = 23;
	const uint32_t now_ms = 7000;

	const uint8_t exp_night_hour = 2;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::night;
	state.standardKey = {web_meas_key::pm25, true};
	web_display_state out{};

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_night_hour, out.nightHour);
}

TEST(WebPageDisplayStateTest, BuildMeasHourKnownUsesStoredNightWindow) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::none;
	const bool hour_known = true;
	const uint8_t hour = 5;
	const uint32_t now_ms = 7000;

	const uint8_t exp_night_hour = 4;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::night;
	state.standardKey = {web_meas_key::pm25, true};
	state.nightStart = 1;
	state.nightFinish = 6;
	web_display_state out{};

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_night_hour, out.nightHour);
}

TEST(WebPageDisplayStateTest, BuildMeasHourOutsideWindowIsTwelve) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::none;
	const bool hour_known = true;
	const uint8_t hour = 15;
	const uint32_t now_ms = 7000;

	const uint8_t exp_night_hour = 12;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::meas;
	state.layout = web_meas_layout::night;
	state.standardKey = {web_meas_key::pm25, true};
	web_display_state out{};

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_night_hour, out.nightHour);
}

TEST(WebPageDisplayStateTest, BuildResetsStaleOutput) {
	const uint8_t brightness = 50;
	const web_error current_error = web_error::none;
	const bool hour_known = true;
	const uint8_t hour = 23;
	const uint32_t now_ms = 7000;
	const uint8_t dirty_byte = 0xFF;

	const uint32_t exp_zero = 0;
	const web_error exp_error = web_error::none;
	const web_effect exp_effect = web_effect::none;
	const web_meas_layout exp_layout = web_meas_layout::none;
	const web_meas_key exp_key = web_meas_key::total;
	const uint8_t exp_night_hour = 0;

	web_picture_state state{};
	HAQuDA_WebPictureInit(state, brightness);
	state.source = web_source::event;
	web_display_state out;
	std::memset(&out, dirty_byte, sizeof(out));

	HAQuDA_WebPictureBuild(state, current_error, hour_known, hour, now_ms, out);

	EXPECT_EQ(exp_error, out.error);
	EXPECT_EQ(exp_effect, out.effect);
	EXPECT_EQ(exp_layout, out.layout);
	EXPECT_EQ(exp_key, out.keys[0]);
	EXPECT_EQ(exp_key, out.keys[1]);
	EXPECT_EQ(exp_key, out.keys[2]);
	EXPECT_EQ(exp_night_hour, out.nightHour);
	EXPECT_EQ(exp_zero, out.snake.color);
	EXPECT_EQ(exp_zero, out.growing.color);
	EXPECT_EQ(exp_zero, out.fading.color);
	EXPECT_EQ(exp_zero, out.staticColor);
}
