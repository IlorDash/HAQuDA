#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "HAQuDA_AnimationSet.h"
#include "MockLEDs.h"

namespace {

	class LedRecorder {
	  public:
		LedRecorder() : pixels(ANIM_PIXELS, 0) {
			using testing::_;
			using testing::Invoke;
			ON_CALL(mock, FillColor(_)).WillByDefault(Invoke([this](uint32_t color) { fill(color); }));
			ON_CALL(mock, FillColor(_, _, _)).WillByDefault(Invoke([this](uint8_t r, uint8_t g, uint8_t b) { fill(pack(r, g, b)); }));
			ON_CALL(mock, SetPixelColor(_, _)).WillByDefault(Invoke([this](uint16_t pixel, uint32_t color) { setPixel(pixel, color); }));
			ON_CALL(mock, FillColorFrom(_, _, _)).WillByDefault(Invoke([this](uint32_t color, uint8_t start, uint8_t count) {
				fillFrom(color, start, count);
			}));
			ON_CALL(mock, FillColorFrom(_, _, _, _, _)).WillByDefault(Invoke([this](uint8_t r, uint8_t g, uint8_t b, uint8_t start, uint8_t count) {
				fillFrom(pack(r, g, b), start, count);
			}));
			ON_CALL(mock, Show()).WillByDefault(Invoke([this]() {
				blockingCalls++;
				sendFrame();
			}));
			ON_CALL(mock, Show(_)).WillByDefault(Invoke([this](uint16_t delayMs) {
				if (delayMs != 0) {
					blockingCalls++;
				}
				sendFrame();
			}));
			ON_CALL(mock, Clear()).WillByDefault(Invoke([this]() {
				blockingCalls++;
				fill(0);
				sendFrame();
			}));
			ON_CALL(mock, SetBright(_)).WillByDefault(Invoke([this](uint8_t percent) {
				brightCalls.push_back(percent);
				sendFrame();
			}));
			ON_CALL(mock, GetBright()).WillByDefault(Invoke([this]() {
				getBrightCalls++;
				return static_cast<uint8_t>(0);
			}));
		}

		static uint32_t pack(uint8_t r, uint8_t g, uint8_t b) {
			return (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | b;
		}

		testing::NiceMock<MockLEDs> mock;
		std::vector<std::vector<uint32_t>> frames;
		std::vector<uint8_t> brightCalls;
		int blockingCalls = 0;
		int getBrightCalls = 0;
		std::vector<uint32_t> pixels;

		size_t litCount(const std::vector<uint32_t> &frame) const {
			size_t count = 0;
			for (uint32_t color : frame) {
				if (color != 0) {
					count++;
				}
			}
			return count;
		}

	  private:
		void fill(uint32_t color) {
			for (uint32_t &pixel : pixels) {
				pixel = color;
			}
		}
		void setPixel(uint16_t index, uint32_t color) {
			if (index < pixels.size()) {
				pixels[index] = color;
			}
		}
		void fillFrom(uint32_t color, uint8_t start, uint8_t count) {
			for (size_t i = start; (i < static_cast<size_t>(start) + count) && (i < pixels.size()); i++) {
				pixels[i] = color;
			}
		}
		void sendFrame() {
			frames.push_back(pixels);
		}
	};

	const anim_context NO_CONTEXT = {};

	anim_context contextWithBright(uint8_t percent) {
		anim_context context = {};
		context.lampBrightPercent = percent;
		return context;
	}

	std::vector<uint32_t> stepsUntilTotal(HAQuDA_Animation &animation, size_t stepsNum) {
		std::vector<uint32_t> waits;
		for (size_t i = 0; i < stepsNum; i++) {
			waits.push_back(animation.Step(0));
		}
		return waits;
	}

	uint32_t sum(const std::vector<uint32_t> &values) {
		uint32_t total = 0;
		for (uint32_t value : values) {
			total += value;
		}
		return total;
	}

} // namespace

TEST(AnimationHelpersTest, PixelOfFollowsTheSerpentineChain) {
	EXPECT_EQ(0, HAQuDA_PixelOf(0, 0));
	EXPECT_EQ(11, HAQuDA_PixelOf(0, 11));
	EXPECT_EQ(23, HAQuDA_PixelOf(1, 0));
	EXPECT_EQ(12, HAQuDA_PixelOf(1, 11));
	EXPECT_EQ(96, HAQuDA_PixelOf(8, 0));
	EXPECT_EQ(107, HAQuDA_PixelOf(8, 11));
}

TEST(AnimationHelpersTest, PixelOfCoversEveryPixelOnce) {
	std::set<uint16_t> seen;
	for (uint8_t x = 0; x < ANIM_COLUMNS; x++) {
		for (uint8_t y = 0; y < ANIM_ROWS; y++) {
			seen.insert(HAQuDA_PixelOf(x, y));
		}
	}
	EXPECT_EQ(static_cast<size_t>(ANIM_PIXELS), seen.size());
	EXPECT_EQ(0, *seen.begin());
	EXPECT_EQ(ANIM_PIXELS - 1, *seen.rbegin());
}

TEST(AnimationHelpersTest, PixelOutsideTheMatrixIsNoPixel) {
	EXPECT_EQ(ANIM_PIXELS, HAQuDA_PixelOf(ANIM_COLUMNS, 0));
	EXPECT_EQ(ANIM_PIXELS, HAQuDA_PixelOf(0, ANIM_ROWS));
}

TEST(AnimationHelpersTest, XorShift32MatchesTheReferenceValue) {
	uint32_t state = 1;

	// 1 ^ (1 << 13) = 8193, ^ (8193 >> 17) = 8193, ^ (8193 << 5) = 270369
	EXPECT_EQ(270369u, HAQuDA_XorShift32(state));
	EXPECT_EQ(270369u, state);
}

TEST(AnimationHelpersTest, MatrixHas108Pixels) {
	EXPECT_EQ(108, ANIM_PIXELS);
}

TEST(SnakeAnimTest, FirstStepIsTheDarkFrameOf200Ms) {
	LedRecorder leds;
	HAQuDA_SnakeAnim snake(leds.mock);
	ASSERT_TRUE(snake.Begin(HAQuDA_SnakeParams(ANIM_COLOR_LIME, 200, 5), NO_CONTEXT));

	const uint32_t wait = snake.Step(0);

	EXPECT_EQ(200u, wait);
	ASSERT_EQ(1u, leds.frames.size());
	EXPECT_EQ(0u, leds.litCount(leds.frames[0]));
}

TEST(SnakeAnimTest, HeadAndTailWalkTheChain) {
	LedRecorder leds;
	HAQuDA_SnakeAnim snake(leds.mock);
	ASSERT_TRUE(snake.Begin(HAQuDA_SnakeParams(ANIM_COLOR_LIME, 200, 5), NO_CONTEXT));
	snake.Step(0);

	const uint32_t wait0 = snake.Step(0);
	const uint32_t wait1 = snake.Step(0);
	const uint32_t wait2 = snake.Step(0);

	EXPECT_EQ(200u, wait0);
	EXPECT_EQ(200u, wait1);
	EXPECT_EQ(200u, wait2);
	ASSERT_EQ(4u, leds.frames.size());
	for (uint16_t pixel = 0; pixel < ANIM_PIXELS; pixel++) {
		EXPECT_EQ(pixel <= 4 ? ANIM_COLOR_LIME : 0u, leds.frames[1][pixel]) << "frame 1 pixel " << pixel;
		EXPECT_EQ(((pixel >= 2) && (pixel <= 6)) ? ANIM_COLOR_LIME : 0u, leds.frames[3][pixel]) << "frame 3 pixel " << pixel;
	}
}

TEST(SnakeAnimTest, PassIsDarkPlus108StepsPlusThe100MsPause) {
	LedRecorder leds;
	HAQuDA_SnakeAnim snake(leds.mock);
	ASSERT_TRUE(snake.Begin(HAQuDA_SnakeParams(ANIM_COLOR_LIME, 200, 5), NO_CONTEXT));

	const std::vector<uint32_t> waits = stepsUntilTotal(snake, 1 + ANIM_PIXELS);

	EXPECT_EQ(200u + 108u * 200u + 100u, sum(waits));
	EXPECT_EQ(200u + 100u, waits.back());
}

TEST(SnakeAnimTest, TailBeyondTheLastPixelIsNotDrawn) {
	LedRecorder leds;
	HAQuDA_SnakeAnim snake(leds.mock);
	ASSERT_TRUE(snake.Begin(HAQuDA_SnakeParams(ANIM_COLOR_LIME, 100, 5), NO_CONTEXT));
	stepsUntilTotal(snake, 1 + ANIM_PIXELS);

	const std::vector<uint32_t> &lastFrame = leds.frames.back();

	EXPECT_EQ(1u, leds.litCount(lastFrame));
	EXPECT_EQ(ANIM_COLOR_LIME, lastFrame[ANIM_PIXELS - 1]);
}

TEST(SnakeAnimTest, NextPassStartsWithTheDarkFrameAgain) {
	LedRecorder leds;
	HAQuDA_SnakeAnim snake(leds.mock);
	ASSERT_TRUE(snake.Begin(HAQuDA_SnakeParams(ANIM_COLOR_LIME, 100, 5), NO_CONTEXT));
	stepsUntilTotal(snake, 1 + ANIM_PIXELS);

	const uint32_t wait = snake.Step(0);

	EXPECT_EQ(200u, wait);
	EXPECT_EQ(0u, leds.litCount(leds.frames.back()));
}

TEST(SnakeAnimTest, TailOfOneLightsOnlyTheHead) {
	LedRecorder leds;
	HAQuDA_SnakeAnim snake(leds.mock);
	ASSERT_TRUE(snake.Begin(HAQuDA_SnakeParams(ANIM_COLOR_RED, 100, 1), NO_CONTEXT));
	stepsUntilTotal(snake, 3);

	EXPECT_EQ(1u, leds.litCount(leds.frames.back()));
	EXPECT_EQ(ANIM_COLOR_RED, leds.frames.back()[1]);
}

TEST(SnakeAnimTest, NeverUsesTheBlockingDriverCalls) {
	LedRecorder leds;
	HAQuDA_SnakeAnim snake(leds.mock);
	ASSERT_TRUE(snake.Begin(HAQuDA_SnakeParams(ANIM_COLOR_LIME, 100, 5), NO_CONTEXT));

	stepsUntilTotal(snake, 2 * (1 + ANIM_PIXELS));

	EXPECT_EQ(0, leds.blockingCalls);
	EXPECT_EQ(0, leds.getBrightCalls);
	EXPECT_TRUE(leds.brightCalls.empty());
}

TEST(SnakeAnimTest, StopEndsThePassAtOnce) {
	LedRecorder leds;
	HAQuDA_SnakeAnim snake(leds.mock);
	ASSERT_TRUE(snake.Begin(HAQuDA_SnakeParams(ANIM_COLOR_LIME, 100, 5), NO_CONTEXT));
	stepsUntilTotal(snake, 11);
	const size_t framesBefore = leds.frames.size();

	snake.Stop();
	const uint32_t wait = snake.Step(0);

	EXPECT_EQ(ANIM_IDLE_MS, wait);
	EXPECT_EQ(framesBefore, leds.frames.size());
}

TEST(SnakeAnimTest, BeginAfterStopStartsFromTheDarkFrame) {
	LedRecorder leds;
	HAQuDA_SnakeAnim snake(leds.mock);
	ASSERT_TRUE(snake.Begin(HAQuDA_SnakeParams(ANIM_COLOR_LIME, 100, 5), NO_CONTEXT));
	stepsUntilTotal(snake, 11);
	snake.Stop();
	ASSERT_TRUE(snake.Begin(HAQuDA_SnakeParams(ANIM_COLOR_LIME, 100, 5), NO_CONTEXT));

	const uint32_t wait = snake.Step(0);

	EXPECT_EQ(200u, wait);
	EXPECT_EQ(0u, leds.litCount(leds.frames.back()));
}

TEST(SnakeAnimTest, BeginRejectsOtherAnimationsAndAnEmptyTail) {
	LedRecorder leds;
	HAQuDA_SnakeAnim snake(leds.mock);

	EXPECT_FALSE(snake.Begin(HAQuDA_GrowingParams(ANIM_COLOR_LIME, 100), NO_CONTEXT));
	EXPECT_FALSE(snake.Begin(HAQuDA_SnakeParams(ANIM_COLOR_LIME, 100, 0), NO_CONTEXT));
	EXPECT_EQ(ANIM_IDLE_MS, snake.Step(0));
	EXPECT_TRUE(leds.frames.empty());
}

TEST(SnakeAnimTest, StepBeforeBeginDrawsNothing) {
	LedRecorder leds;
	HAQuDA_SnakeAnim snake(leds.mock);

	EXPECT_EQ(ANIM_IDLE_MS, snake.Step(0));
	EXPECT_TRUE(leds.frames.empty());
}

TEST(GrowingAnimTest, RowsFillFromTheTopRowDown) {
	LedRecorder leds;
	HAQuDA_GrowingAnim growing(leds.mock);
	ASSERT_TRUE(growing.Begin(HAQuDA_GrowingParams(ANIM_COLOR_RED, 100), NO_CONTEXT));
	growing.Step(0);

	const uint32_t wait = growing.Step(0);

	EXPECT_EQ(100u, wait);
	EXPECT_EQ(static_cast<size_t>(ANIM_COLUMNS), leds.litCount(leds.frames.back()));
	for (uint8_t x = 0; x < ANIM_COLUMNS; x++) {
		EXPECT_EQ(ANIM_COLOR_RED, leds.frames.back()[HAQuDA_PixelOf(x, ANIM_ROWS - 1)]) << "column " << static_cast<int>(x);
	}
}

TEST(GrowingAnimTest, EveryStepAddsOneRowAndTheLastFillsTheMatrix) {
	LedRecorder leds;
	HAQuDA_GrowingAnim growing(leds.mock);
	ASSERT_TRUE(growing.Begin(HAQuDA_GrowingParams(ANIM_COLOR_RED, 100), NO_CONTEXT));
	growing.Step(0);

	for (uint8_t row = 0; row < ANIM_ROWS; row++) {
		growing.Step(0);
		EXPECT_EQ(static_cast<size_t>(ANIM_COLUMNS) * (row + 1), leds.litCount(leds.frames.back())) << "row step " << static_cast<int>(row);
	}
	EXPECT_EQ(static_cast<size_t>(ANIM_PIXELS), leds.litCount(leds.frames.back()));
}

TEST(GrowingAnimTest, PassIsDarkPlus12StepsPlusThe100MsPause) {
	LedRecorder leds;
	HAQuDA_GrowingAnim growing(leds.mock);
	ASSERT_TRUE(growing.Begin(HAQuDA_GrowingParams(ANIM_COLOR_RED, 100), NO_CONTEXT));

	const std::vector<uint32_t> waits = stepsUntilTotal(growing, 1 + ANIM_ROWS);

	EXPECT_EQ(200u + 12u * 100u + 100u, sum(waits));
	EXPECT_EQ(100u + 100u, waits.back());
	EXPECT_EQ(0, leds.blockingCalls);
}

TEST(GrowingAnimTest, NextPassStartsDarkAgain) {
	LedRecorder leds;
	HAQuDA_GrowingAnim growing(leds.mock);
	ASSERT_TRUE(growing.Begin(HAQuDA_GrowingParams(ANIM_COLOR_RED, 100), NO_CONTEXT));
	stepsUntilTotal(growing, 1 + ANIM_ROWS);

	EXPECT_EQ(200u, growing.Step(0));
	EXPECT_EQ(0u, leds.litCount(leds.frames.back()));
}

namespace {
	struct RandomPass {
		std::vector<uint16_t> pixels;
		std::vector<uint32_t> colors;
	};

	RandomPass runRandomPass(uint32_t seed) {
		using testing::_;
		testing::NiceMock<MockLEDs> mock;
		RandomPass pass;
		ON_CALL(mock, SetPixelColor(_, _)).WillByDefault(testing::Invoke([&pass](uint16_t pixel, uint32_t color) {
			pass.pixels.push_back(pixel);
			pass.colors.push_back(color);
		}));
		HAQuDA_RandomPixelsAnim random(mock);
		anim_context context = {};
		context.seed = seed;
		random.Begin(HAQuDA_RandomPixelsParams(50, 500), context);
		stepsUntilTotal(random, 1 + ANIM_PIXELS);
		return pass;
	}
} // namespace

TEST(RandomPixelsAnimTest, LightsEveryPixelExactlyOncePerPass) {
	const RandomPass pass = runRandomPass(12345);

	ASSERT_EQ(static_cast<size_t>(ANIM_PIXELS), pass.pixels.size());
	const std::set<uint16_t> distinct(pass.pixels.begin(), pass.pixels.end());
	EXPECT_EQ(static_cast<size_t>(ANIM_PIXELS), distinct.size());
	EXPECT_EQ(ANIM_PIXELS - 1, *distinct.rbegin());
}

TEST(RandomPixelsAnimTest, SameSeedGivesTheSameSequence) {
	const RandomPass first = runRandomPass(777);
	const RandomPass second = runRandomPass(777);

	EXPECT_EQ(first.pixels, second.pixels);
	EXPECT_EQ(first.colors, second.colors);
}

TEST(RandomPixelsAnimTest, OtherSeedGivesAnotherOrder) {
	const RandomPass first = runRandomPass(777);
	const RandomPass second = runRandomPass(778);

	EXPECT_NE(first.pixels, second.pixels);
}

TEST(RandomPixelsAnimTest, ChannelsStayBelow255) {
	const RandomPass pass = runRandomPass(99);

	for (uint32_t color : pass.colors) {
		EXPECT_LT((color >> 16) & 0xFF, 255u);
		EXPECT_LT((color >> 8) & 0xFF, 255u);
		EXPECT_LT(color & 0xFF, 255u);
		EXPECT_EQ(0u, color >> 24);
	}
}

TEST(RandomPixelsAnimTest, SequenceFollowsTheDocumentedGenerator) {
	// The web page replays this sequence, so it is derived here independently of the animation
	uint32_t state = 1;
	std::vector<uint8_t> order(ANIM_PIXELS);
	for (uint16_t i = 0; i < ANIM_PIXELS; i++) {
		order[i] = static_cast<uint8_t>(i);
	}
	for (uint16_t i = ANIM_PIXELS - 1; i >= 1; i--) {
		const uint16_t j = HAQuDA_XorShift32(state) % (i + 1);
		std::swap(order[i], order[j]);
	}
	const uint32_t r = HAQuDA_XorShift32(state) % 255;
	const uint32_t g = HAQuDA_XorShift32(state) % 255;
	const uint32_t b = HAQuDA_XorShift32(state) % 255;

	const RandomPass pass = runRandomPass(1);

	EXPECT_EQ(order[0], pass.pixels[0]);
	EXPECT_EQ((r << 16) | (g << 8) | b, pass.colors[0]);
}

TEST(RandomPixelsAnimTest, SeedZeroBecomesOne) {
	LedRecorder leds;
	HAQuDA_RandomPixelsAnim random(leds.mock);

	ASSERT_TRUE(random.Begin(HAQuDA_RandomPixelsParams(50, 500), NO_CONTEXT));

	EXPECT_EQ(1u, random.Seed());
}

TEST(RandomPixelsAnimTest, SeedIsExposed) {
	LedRecorder leds;
	HAQuDA_RandomPixelsAnim random(leds.mock);
	anim_context context = {};
	context.seed = 0xDEADBEEF;

	ASSERT_TRUE(random.Begin(HAQuDA_RandomPixelsParams(50, 500), context));

	EXPECT_EQ(0xDEADBEEFu, random.Seed());
}

TEST(RandomPixelsAnimTest, PassTimingIsDarkPixelsEndPauseAndShowPause) {
	LedRecorder leds;
	HAQuDA_RandomPixelsAnim random(leds.mock);
	ASSERT_TRUE(random.Begin(HAQuDA_RandomPixelsParams(50, 500), NO_CONTEXT));

	const std::vector<uint32_t> waits = stepsUntilTotal(random, 1 + ANIM_PIXELS);

	EXPECT_EQ(200u + 108u * 50u + 500u + 100u, sum(waits));
	EXPECT_EQ(50u + 500u + 100u, waits.back());
	EXPECT_EQ(0, leds.blockingCalls);
}

TEST(RandomPixelsAnimTest, StopMidPassDrawsNothingMore) {
	LedRecorder leds;
	HAQuDA_RandomPixelsAnim random(leds.mock);
	ASSERT_TRUE(random.Begin(HAQuDA_RandomPixelsParams(50, 500), NO_CONTEXT));
	stepsUntilTotal(random, 20);
	const size_t framesBefore = leds.frames.size();

	random.Stop();

	EXPECT_EQ(ANIM_IDLE_MS, random.Step(0));
	EXPECT_EQ(framesBefore, leds.frames.size());
}

TEST(RandomPixelsAnimTest, EachPassUsesTheNextSeedOfTheXorShiftChain) {
	// The web page replays pass n from xorshift32 applied n times to the seed of the start
	const uint32_t startSeed = 5;
	std::vector<uint32_t> passSeeds = {startSeed};
	for (int pass = 1; pass < 3; pass++) {
		uint32_t next = passSeeds.back();
		HAQuDA_XorShift32(next);
		passSeeds.push_back(next);
	}

	testing::NiceMock<MockLEDs> mock;
	std::vector<std::vector<uint16_t>> passPixels(1);
	ON_CALL(mock, SetPixelColor(testing::_, testing::_)).WillByDefault(testing::Invoke([&passPixels](uint16_t pixel, uint32_t) {
		if (passPixels.back().size() == ANIM_PIXELS) {
			passPixels.emplace_back();
		}
		passPixels.back().push_back(pixel);
	}));
	HAQuDA_RandomPixelsAnim random(mock);
	anim_context context = {};
	context.seed = startSeed;
	ASSERT_TRUE(random.Begin(HAQuDA_RandomPixelsParams(50, 500), context));
	stepsUntilTotal(random, 3 * (1 + ANIM_PIXELS));

	ASSERT_EQ(3u, passPixels.size());
	for (size_t pass = 0; pass < 3; pass++) {
		uint32_t state = passSeeds[pass];
		std::vector<uint8_t> order(ANIM_PIXELS);
		for (uint16_t i = 0; i < ANIM_PIXELS; i++) {
			order[i] = static_cast<uint8_t>(i);
		}
		for (uint16_t i = ANIM_PIXELS - 1; i >= 1; i--) {
			std::swap(order[i], order[HAQuDA_XorShift32(state) % (i + 1)]);
		}
		for (size_t step = 0; step < ANIM_PIXELS; step++) {
			EXPECT_EQ(order[step], passPixels[pass][step]) << "pass " << pass << " step " << step;
		}
	}
	EXPECT_NE(passPixels[0], passPixels[1]);
	EXPECT_NE(passPixels[1], passPixels[2]);
}

TEST(RandomPixelsAnimTest, SeedFollowsThePassInProgress) {
	LedRecorder leds;
	HAQuDA_RandomPixelsAnim random(leds.mock);
	anim_context context = {};
	context.seed = 5;
	ASSERT_TRUE(random.Begin(HAQuDA_RandomPixelsParams(50, 500), context));
	stepsUntilTotal(random, 1 + ANIM_PIXELS);
	EXPECT_EQ(5u, random.Seed());
	uint32_t second = 5;
	HAQuDA_XorShift32(second);

	stepsUntilTotal(random, 1 + ANIM_PIXELS);

	EXPECT_EQ(second, random.Seed());
}

TEST(RandomPixelsAnimTest, ABeginAgainStartsTheChainOverFromTheNewSeed) {
	LedRecorder leds;
	HAQuDA_RandomPixelsAnim random(leds.mock);
	anim_context context = {};
	context.seed = 5;
	ASSERT_TRUE(random.Begin(HAQuDA_RandomPixelsParams(50, 500), context));
	stepsUntilTotal(random, 2 * (1 + ANIM_PIXELS));

	ASSERT_TRUE(random.Begin(HAQuDA_RandomPixelsParams(50, 500), context));
	stepsUntilTotal(random, 1 + ANIM_PIXELS);

	EXPECT_EQ(5u, random.Seed());
}

TEST(FadingAnimTest, CountingDownPassTimingMatchesTheOldEffect) {
	LedRecorder leds;
	HAQuDA_FadingAnim fading(leds.mock);
	ASSERT_TRUE(fading.Begin(HAQuDA_FadingParams(ANIM_COLOR_AQUA, 10, 100, 0, 5), contextWithBright(33)));

	// dark (400) + coloured frame (200) + levels 100, 95 ... 5 (20, each 200 + 10) + 100 ms pause
	const std::vector<uint32_t> waits = stepsUntilTotal(fading, 2 + 20);

	EXPECT_EQ(400u, waits[0]);
	EXPECT_EQ(200u, waits[1]);
	EXPECT_EQ(210u, waits[2]);
	EXPECT_EQ(210u + 100u, waits.back());
	EXPECT_EQ(400u + 200u + 20u * 210u + 100u, sum(waits));
}

TEST(FadingAnimTest, CountingDownSetsEachLevelInPercentAndEndsAboveTheStopLevel) {
	LedRecorder leds;
	HAQuDA_FadingAnim fading(leds.mock);
	ASSERT_TRUE(fading.Begin(HAQuDA_FadingParams(ANIM_COLOR_AQUA, 10, 100, 0, 5), contextWithBright(33)));
	stepsUntilTotal(fading, 2 + 20);

	std::vector<uint8_t> expected = {100};
	for (int level = 100; level >= 5; level -= 5) {
		expected.push_back(static_cast<uint8_t>(level));
	}
	EXPECT_EQ(expected, leds.brightCalls);
}

TEST(FadingAnimTest, CountingUpReallyFades) {
	LedRecorder leds;
	HAQuDA_FadingAnim fading(leds.mock);
	ASSERT_TRUE(fading.Begin(HAQuDA_FadingParams(ANIM_COLOR_YELLOW, 10, 0, 100, 10), contextWithBright(33)));

	const std::vector<uint32_t> waits = stepsUntilTotal(fading, 2 + 10);

	std::vector<uint8_t> expected = {0};
	for (int level = 0; level <= 90; level += 10) {
		expected.push_back(static_cast<uint8_t>(level));
	}
	EXPECT_EQ(expected, leds.brightCalls);
	EXPECT_EQ(400u + 200u + 10u * 210u + 100u, sum(waits));
}

TEST(FadingAnimTest, ColourIsOnTheMatrixDuringTheLevels) {
	LedRecorder leds;
	HAQuDA_FadingAnim fading(leds.mock);
	ASSERT_TRUE(fading.Begin(HAQuDA_FadingParams(ANIM_COLOR_AQUA, 10, 100, 0, 5), contextWithBright(33)));
	stepsUntilTotal(fading, 3);

	for (uint32_t color : leds.frames.back()) {
		EXPECT_EQ(ANIM_COLOR_AQUA, color);
	}
}

TEST(FadingAnimTest, DarkPhaseIsDarkAtTheStartLevel) {
	LedRecorder leds;
	HAQuDA_FadingAnim fading(leds.mock);
	ASSERT_TRUE(fading.Begin(HAQuDA_FadingParams(ANIM_COLOR_AQUA, 10, 100, 0, 5), contextWithBright(33)));

	fading.Step(0);

	ASSERT_FALSE(leds.frames.empty());
	EXPECT_EQ(0u, leds.litCount(leds.frames.back()));
	EXPECT_EQ(std::vector<uint8_t>({100}), leds.brightCalls);
}

TEST(FadingAnimTest, NextPassStartsDarkAgain) {
	LedRecorder leds;
	HAQuDA_FadingAnim fading(leds.mock);
	ASSERT_TRUE(fading.Begin(HAQuDA_FadingParams(ANIM_COLOR_AQUA, 10, 100, 0, 5), contextWithBright(33)));
	stepsUntilTotal(fading, 2 + 20);

	EXPECT_EQ(400u, fading.Step(0));
	EXPECT_EQ(0u, leds.litCount(leds.frames.back()));
}

TEST(FadingAnimTest, EqualStartAndStopShowOnlyTheColourFrame) {
	LedRecorder leds;
	HAQuDA_FadingAnim fading(leds.mock);
	ASSERT_TRUE(fading.Begin(HAQuDA_FadingParams(ANIM_COLOR_AQUA, 10, 50, 50, 5), contextWithBright(33)));

	const std::vector<uint32_t> waits = stepsUntilTotal(fading, 3);

	EXPECT_EQ(400u, waits[0]);
	EXPECT_EQ(200u + 100u, waits[1]);
	EXPECT_EQ(400u, waits[2]);
}

TEST(FadingAnimTest, StopMidFadePutsTheLampBrightnessBackInPercent) {
	LedRecorder leds;
	HAQuDA_FadingAnim fading(leds.mock);
	ASSERT_TRUE(fading.Begin(HAQuDA_FadingParams(ANIM_COLOR_AQUA, 10, 100, 0, 5), contextWithBright(33)));
	stepsUntilTotal(fading, 8);

	fading.Stop();

	ASSERT_FALSE(leds.brightCalls.empty());
	EXPECT_EQ(33, leds.brightCalls.back());
	// the restore frame is dark: no flash of the faded colour at the lamp level
	EXPECT_EQ(0u, leds.litCount(leds.frames.back()));
	EXPECT_EQ(0, leds.getBrightCalls);
}

TEST(FadingAnimTest, NoBrightnessValueEverExceedsAPercent) {
	LedRecorder leds;
	HAQuDA_FadingAnim fading(leds.mock);
	ASSERT_TRUE(fading.Begin(HAQuDA_FadingParams(ANIM_COLOR_AQUA, 10, 100, 0, 5), contextWithBright(33)));
	stepsUntilTotal(fading, 2 * 22);
	fading.Stop();

	for (uint8_t percent : leds.brightCalls) {
		EXPECT_LE(percent, 100);
	}
}

TEST(FadingAnimTest, StopBeforeAnyStepLeavesTheBrightnessAlone) {
	LedRecorder leds;
	HAQuDA_FadingAnim fading(leds.mock);
	ASSERT_TRUE(fading.Begin(HAQuDA_FadingParams(ANIM_COLOR_AQUA, 10, 100, 0, 5), contextWithBright(33)));

	fading.Stop();

	EXPECT_TRUE(leds.brightCalls.empty());
}

TEST(FadingAnimTest, SecondStopDoesNotRestoreAgain) {
	LedRecorder leds;
	HAQuDA_FadingAnim fading(leds.mock);
	ASSERT_TRUE(fading.Begin(HAQuDA_FadingParams(ANIM_COLOR_AQUA, 10, 100, 0, 5), contextWithBright(33)));
	stepsUntilTotal(fading, 4);
	fading.Stop();
	const size_t callsBefore = leds.brightCalls.size();

	fading.Stop();

	EXPECT_EQ(callsBefore, leds.brightCalls.size());
}

TEST(FadingAnimTest, BeginWhileFadingRestoresTheBrightnessFirst) {
	LedRecorder leds;
	HAQuDA_FadingAnim fading(leds.mock);
	ASSERT_TRUE(fading.Begin(HAQuDA_FadingParams(ANIM_COLOR_AQUA, 10, 100, 0, 5), contextWithBright(40)));
	stepsUntilTotal(fading, 4);

	ASSERT_TRUE(fading.Begin(HAQuDA_FadingParams(ANIM_COLOR_RED, 10, 100, 0, 5), contextWithBright(40)));

	EXPECT_EQ(40, leds.brightCalls.back());
}

TEST(FadingAnimTest, RestoreLevelAbove100IsClamped) {
	LedRecorder leds;
	HAQuDA_FadingAnim fading(leds.mock);
	ASSERT_TRUE(fading.Begin(HAQuDA_FadingParams(ANIM_COLOR_AQUA, 10, 100, 0, 5), contextWithBright(150)));
	stepsUntilTotal(fading, 3);

	fading.Stop();

	EXPECT_EQ(100, leds.brightCalls.back());
}

TEST(FadingAnimTest, BeginRejectsAZeroStepAndLevelsAbove100) {
	LedRecorder leds;
	HAQuDA_FadingAnim fading(leds.mock);

	EXPECT_FALSE(fading.Begin(HAQuDA_FadingParams(ANIM_COLOR_AQUA, 10, 100, 0, 0), NO_CONTEXT));
	EXPECT_FALSE(fading.Begin(HAQuDA_FadingParams(ANIM_COLOR_AQUA, 10, 101, 0, 5), NO_CONTEXT));
	EXPECT_FALSE(fading.Begin(HAQuDA_FadingParams(ANIM_COLOR_AQUA, 10, 0, 101, 5), NO_CONTEXT));
	EXPECT_FALSE(fading.Begin(HAQuDA_SnakeParams(ANIM_COLOR_AQUA, 10, 5), NO_CONTEXT));
	EXPECT_EQ(ANIM_IDLE_MS, fading.Step(0));
}

TEST(FadingAnimTest, StepOfZeroIsAllowedWhenStartEqualsStop) {
	LedRecorder leds;
	HAQuDA_FadingAnim fading(leds.mock);

	EXPECT_TRUE(fading.Begin(HAQuDA_FadingParams(ANIM_COLOR_AQUA, 10, 30, 30, 0), NO_CONTEXT));
}

TEST(FadingAnimTest, StepLargerThanTheRangeShowsTheStartLevelOnly) {
	LedRecorder leds;
	HAQuDA_FadingAnim fading(leds.mock);
	ASSERT_TRUE(fading.Begin(HAQuDA_FadingParams(ANIM_COLOR_AQUA, 10, 100, 0, 250), contextWithBright(33)));

	const std::vector<uint32_t> waits = stepsUntilTotal(fading, 3);

	EXPECT_EQ(400u, waits[0]);
	EXPECT_EQ(200u, waits[1]);
	EXPECT_EQ(210u + 100u, waits[2]);
	EXPECT_EQ(400u, fading.Step(0));
}

TEST(StaticColorAnimTest, FillsAndResendsEvery300Ms) {
	LedRecorder leds;
	HAQuDA_StaticColorAnim staticColor(leds.mock);
	ASSERT_TRUE(staticColor.Begin(HAQuDA_StaticColorParams(ANIM_COLOR_GREEN), NO_CONTEXT));

	const uint32_t wait0 = staticColor.Step(0);
	const uint32_t wait1 = staticColor.Step(0);

	EXPECT_EQ(300u, wait0);
	EXPECT_EQ(300u, wait1);
	ASSERT_EQ(2u, leds.frames.size());
	EXPECT_EQ(static_cast<size_t>(ANIM_PIXELS), leds.litCount(leds.frames[1]));
	EXPECT_EQ(ANIM_COLOR_GREEN, leds.frames[1][0]);
	EXPECT_EQ(ANIM_COLOR_GREEN, leds.frames[1][ANIM_PIXELS - 1]);
	EXPECT_EQ(0, leds.blockingCalls);
}

TEST(StaticColorAnimTest, StopDrawsNothingMore) {
	LedRecorder leds;
	HAQuDA_StaticColorAnim staticColor(leds.mock);
	ASSERT_TRUE(staticColor.Begin(HAQuDA_StaticColorParams(ANIM_COLOR_GREEN), NO_CONTEXT));
	staticColor.Step(0);

	staticColor.Stop();

	EXPECT_EQ(ANIM_IDLE_MS, staticColor.Step(0));
	EXPECT_EQ(1u, leds.frames.size());
}

namespace {
	meas_anim_data lampData() {
		meas_anim_data data = {};
		data.dots[0] = {20, 26, 30};
		data.dots[1] = {40, 60, 80};
		data.dots[2] = {400, 1000, 5000};
		data.dots[3] = {220, 660, 1000};
		data.dots[4] = {15, 20, 45};
		data.values[0] = 23;
		data.values[1] = 50;
		data.values[2] = 700;
		data.values[3] = 400;
		data.values[4] = 18;
		return data;
	}

	anim_context measContext(const meas_anim_data &data, uint8_t percent) {
		anim_context context = {};
		context.lampBrightPercent = percent;
		context.meas = data;
		return context;
	}

	uint32_t packed(const rgb_color &color) {
		return LedRecorder::pack(color.r, color.g, color.b);
	}

	rgb_color scaleColor(float value, div_dots dots) {
		rgb_color color = {0, 0, 0};
		HAQuDA_GetMeasRGB(value, dots, color);
		return color;
	}
} // namespace

TEST(MeasAnimTest, StandardDrawsThePictureAtOnceWithoutADarkFrameThenResendsIt) {
	LedRecorder leds;
	HAQuDA_MeasStandardAnim meas(leds.mock);
	const meas_anim_data data = lampData();
	ASSERT_TRUE(meas.Begin(HAQuDA_MeasParams(anim_id::measStandard), measContext(data, 100)));

	const uint32_t drawWait = meas.Step(0);
	const uint32_t holdWait = meas.Step(0);

	EXPECT_EQ(300u, drawWait);
	EXPECT_EQ(300u, holdWait);
	ASSERT_EQ(2u, leds.frames.size());
	const uint32_t expected = packed(scaleColor(23, data.dots[0]));
	for (uint32_t color : leds.frames[0]) {
		EXPECT_EQ(expected, color);
	}
	EXPECT_EQ(leds.frames[0], leds.frames[1]);
	EXPECT_EQ(0, leds.blockingCalls);
}

TEST(MeasAnimTest, ColoursDoNotDependOnTheLampBrightness) {
	LedRecorder leds;
	HAQuDA_MeasStandardAnim meas(leds.mock);
	const meas_anim_data data = lampData();
	ASSERT_TRUE(meas.Begin(HAQuDA_MeasParams(anim_id::measStandard), measContext(data, 50)));
	meas.Step(0);
	meas.Step(0);

	const rgb_color expected = scaleColor(23, data.dots[0]);
	EXPECT_EQ(packed(expected), leds.frames.back()[0]);
	EXPECT_EQ(expected.r, meas.Drawn().colors[0].r);
	EXPECT_EQ(expected.g, meas.Drawn().colors[0].g);
	EXPECT_EQ(expected.b, meas.Drawn().colors[0].b);
	EXPECT_EQ(1, meas.Drawn().colorCount);
}

TEST(MeasAnimTest, NeverTouchesTheBrightnessOfTheLamp) {
	LedRecorder leds;
	HAQuDA_MeasMultiAnim meas(leds.mock);
	ASSERT_TRUE(meas.Begin(HAQuDA_MeasParams(anim_id::measMulti), measContext(lampData(), 50)));

	meas.Step(0);
	meas.Step(0);
	meas.Stop();

	EXPECT_TRUE(leds.brightCalls.empty());
	EXPECT_EQ(0, leds.getBrightCalls);
}

TEST(MeasAnimTest, DrawnIsEmptyBeforeTheFirstPicture) {
	LedRecorder leds;
	HAQuDA_MeasStandardAnim meas(leds.mock);
	ASSERT_TRUE(meas.Begin(HAQuDA_MeasParams(anim_id::measStandard), measContext(lampData(), 100)));

	EXPECT_EQ(0, meas.Drawn().colorCount);
}

TEST(MeasAnimTest, DrawnCarriesTheKeysAndTheLitCellsOfTheData) {
	LedRecorder leds;
	HAQuDA_MeasNightAnim meas(leds.mock);
	meas_anim_data data = lampData();
	data.keys[0] = PM2_5;
	data.keys[1] = humid;
	data.keys[2] = eCO2;
	data.nightLitCells = 7;
	ASSERT_TRUE(meas.Begin(HAQuDA_MeasParams(anim_id::measNight), measContext(data, 100)));

	meas.Step(0);

	EXPECT_EQ(PM2_5, meas.Drawn().keys[0]);
	EXPECT_EQ(humid, meas.Drawn().keys[1]);
	EXPECT_EQ(eCO2, meas.Drawn().keys[2]);
	EXPECT_EQ(7, meas.Drawn().nightLitCells);
}

TEST(MeasAnimTest, ABeginAgainRedrawsTheNewDataInPlaceWithoutADarkFrame) {
	LedRecorder leds;
	HAQuDA_MeasStandardAnim meas(leds.mock);
	meas_anim_data data = lampData();
	ASSERT_TRUE(meas.Begin(HAQuDA_MeasParams(anim_id::measStandard), measContext(data, 100)));
	meas.Step(0);
	data.values[0] = 10;
	ASSERT_TRUE(meas.Begin(HAQuDA_MeasParams(anim_id::measStandard), measContext(data, 100)));

	meas.Step(0);

	ASSERT_EQ(2u, leds.frames.size());
	EXPECT_EQ(static_cast<size_t>(ANIM_PIXELS), leds.litCount(leds.frames[1]));
	EXPECT_EQ(packed(scaleColor(10, data.dots[0])), leds.frames[1][0]);
	EXPECT_EQ(packed(scaleColor(10, data.dots[0])), packed(meas.Drawn().colors[0]));
}

TEST(MeasAnimTest, NaNValueGivesADarkPicture) {
	LedRecorder leds;
	HAQuDA_MeasStandardAnim meas(leds.mock);
	meas_anim_data data = lampData();
	data.values[0] = std::numeric_limits<float>::quiet_NaN();
	ASSERT_TRUE(meas.Begin(HAQuDA_MeasParams(anim_id::measStandard), measContext(data, 100)));

	meas.Step(0);
	meas.Step(0);

	EXPECT_EQ(0u, leds.litCount(leds.frames.back()));
}

TEST(MeasAnimTest, BeginRejectsThresholdsWithoutARange) {
	LedRecorder leds;
	HAQuDA_MeasStandardAnim meas(leds.mock);
	meas_anim_data data = lampData();
	data.dots[0] = {20, 20, 20};

	EXPECT_FALSE(meas.Begin(HAQuDA_MeasParams(anim_id::measStandard), measContext(data, 100)));
	EXPECT_EQ(ANIM_IDLE_MS, meas.Step(0));
	EXPECT_TRUE(leds.frames.empty());
}

TEST(MeasAnimTest, BeginRejectsAnotherAnimation) {
	LedRecorder leds;
	HAQuDA_MeasStandardAnim meas(leds.mock);

	EXPECT_FALSE(meas.Begin(HAQuDA_MeasParams(anim_id::measMulti), measContext(lampData(), 100)));
	EXPECT_FALSE(meas.Begin(HAQuDA_SnakeParams(ANIM_COLOR_LIME, 100, 5), measContext(lampData(), 100)));
}

TEST(MeasAnimTest, StopDrawsNothingMore) {
	LedRecorder leds;
	HAQuDA_MeasStandardAnim meas(leds.mock);
	ASSERT_TRUE(meas.Begin(HAQuDA_MeasParams(anim_id::measStandard), measContext(lampData(), 100)));
	meas.Step(0);

	meas.Stop();

	EXPECT_EQ(ANIM_IDLE_MS, meas.Step(0));
	EXPECT_EQ(1u, leds.frames.size());
}

TEST(MeasAnimTest, MultiPaintsEveryPixelOverAPreviousPicture) {
	LedRecorder leds;
	HAQuDA_MeasMultiAnim meas(leds.mock);
	std::fill(leds.pixels.begin(), leds.pixels.end(), 0xFFFFFFu);
	ASSERT_TRUE(meas.Begin(HAQuDA_MeasParams(anim_id::measMulti), measContext(lampData(), 100)));

	meas.Step(0);

	ASSERT_EQ(1u, leds.frames.size());
	for (uint8_t x = 0; x < ANIM_COLUMNS; x++) {
		for (uint8_t row : {0, 4, 8}) {
			EXPECT_NE(0xFFFFFFu, leds.frames[0][HAQuDA_PixelOf(x, row)]) << "column " << static_cast<int>(x) << " row " << static_cast<int>(row);
		}
	}
}

TEST(MeasAnimTest, NightPaintsEveryPixelOverAPreviousPicture) {
	LedRecorder leds;
	HAQuDA_MeasNightAnim meas(leds.mock);
	std::fill(leds.pixels.begin(), leds.pixels.end(), 0xFFFFFFu);
	meas_anim_data data = lampData();
	data.nightLitCells = 4;
	ASSERT_TRUE(meas.Begin(HAQuDA_MeasParams(anim_id::measNight), measContext(data, 100)));

	meas.Step(0);

	ASSERT_EQ(1u, leds.frames.size());
	EXPECT_EQ(0u, leds.frames[0][HAQuDA_PixelOf(1, ANIM_ROWS - 1)]);
	EXPECT_EQ(0u, leds.frames[0][HAQuDA_PixelOf(8, ANIM_ROWS - 1)]);
	for (uint32_t color : leds.frames[0]) {
		EXPECT_NE(0xFFFFFFu, color);
	}
}

TEST(MeasAnimTest, MultiPutsTheThreeZonesInEveryColumn) {
	LedRecorder leds;
	HAQuDA_MeasMultiAnim meas(leds.mock);
	meas_anim_data data = lampData();
	data.values[0] = 10;
	data.values[1] = 26;
	data.values[2] = 100;
	data.dots[1] = {20, 26, 30};
	data.dots[2] = {20, 26, 30};
	ASSERT_TRUE(meas.Begin(HAQuDA_MeasParams(anim_id::measMulti), measContext(data, 100)));
	meas.Step(0);
	meas.Step(0);
	const uint32_t c0 = packed(scaleColor(data.values[0], data.dots[0]));
	const uint32_t c1 = packed(scaleColor(data.values[1], data.dots[1]));
	const uint32_t c2 = packed(scaleColor(data.values[2], data.dots[2]));
	ASSERT_NE(c0, c1);
	ASSERT_NE(c1, c2);
	ASSERT_NE(c0, c2);

	const std::vector<uint32_t> &frame = leds.frames.back();

	for (uint8_t row = 0; row < ANIM_ROWS; row++) {
		uint32_t expected = 0;
		if ((row >= 1) && (row <= 3)) {
			expected = c0;
		} else if ((row >= 5) && (row <= 7)) {
			expected = c1;
		} else if ((row >= 9) && (row <= 11)) {
			expected = c2;
		}
		EXPECT_EQ(expected, frame[HAQuDA_PixelOf(0, row)]) << "column 0 row " << static_cast<int>(row);
		EXPECT_EQ(expected, frame[HAQuDA_PixelOf(1, row)]) << "column 1 row " << static_cast<int>(row);
		EXPECT_EQ(expected, frame[HAQuDA_PixelOf(8, row)]) << "column 8 row " << static_cast<int>(row);
	}
	EXPECT_EQ(3, meas.Drawn().colorCount);
	EXPECT_EQ(packed(meas.Drawn().colors[1]), c1);
}

TEST(MeasAnimTest, NightLightGrowsWithTheLitCellsOnTheOtherColumns) {
	LedRecorder leds;
	HAQuDA_MeasNightAnim meas(leds.mock);
	meas_anim_data data = lampData();
	data.nightLitCells = 4;
	ASSERT_TRUE(meas.Begin(HAQuDA_MeasParams(anim_id::measNight), measContext(data, 100)));
	meas.Step(0);
	meas.Step(0);
	const uint32_t colour = packed(scaleColor(23, data.dots[0]));
	// 70 % of the largest colour channel (150)
	const uint32_t white = LedRecorder::pack(105, 105, 105);

	const std::vector<uint32_t> &frame = leds.frames.back();

	for (uint8_t column : {0, 3, 6}) {
		for (uint8_t row = 0; row < ANIM_ROWS; row++) {
			EXPECT_EQ(colour, frame[HAQuDA_PixelOf(column, row)]) << "column " << static_cast<int>(column);
		}
	}
	// four cells: two in the first pair of columns, one in each of the others, two lamp rows per cell
	for (uint8_t column : {1, 2, 4, 5, 7, 8}) {
		const uint8_t litRows = (column < 3) ? 4 : 2;
		for (uint8_t row = 0; row < ANIM_ROWS; row++) {
			EXPECT_EQ(row < litRows ? white : 0u, frame[HAQuDA_PixelOf(column, row)])
				<< "column " << static_cast<int>(column) << " row " << static_cast<int>(row);
		}
	}
	EXPECT_EQ(packed(meas.Drawn().white), white);
	EXPECT_EQ(packed(meas.Drawn().colors[0]), colour);
}

TEST(MeasAnimTest, NightAllCellsFillTheLightAndNoCellEmptiesIt) {
	LedRecorder leds;
	HAQuDA_MeasNightAnim meas(leds.mock);
	meas_anim_data data = lampData();
	data.nightLitCells = 18;
	ASSERT_TRUE(meas.Begin(HAQuDA_MeasParams(anim_id::measNight), measContext(data, 100)));
	meas.Step(0);
	meas.Step(0);

	EXPECT_EQ(static_cast<size_t>(ANIM_PIXELS), leds.litCount(leds.frames.back()));

	data.nightLitCells = 0;
	ASSERT_TRUE(meas.Begin(HAQuDA_MeasParams(anim_id::measNight), measContext(data, 100)));
	meas.Step(0);
	meas.Step(0);

	EXPECT_EQ(static_cast<size_t>(3 * ANIM_ROWS), leds.litCount(leds.frames.back()));
}

TEST(MeasAnimTest, NightLitCellsAboveTheMaximumAreClampedToAFullLight) {
	LedRecorder leds;
	HAQuDA_MeasNightAnim meas(leds.mock);
	meas_anim_data data = lampData();
	data.nightLitCells = 200;
	ASSERT_TRUE(meas.Begin(HAQuDA_MeasParams(anim_id::measNight), measContext(data, 100)));
	meas.Step(0);
	meas.Step(0);

	EXPECT_EQ(static_cast<size_t>(ANIM_PIXELS), leds.litCount(leds.frames.back()));
}

TEST(MeasAnimTest, TotalAveragesTheFiveColoursWithoutOverflow) {
	LedRecorder leds;
	HAQuDA_MeasTotalAnim meas(leds.mock);
	meas_anim_data data = lampData();
	// every value exactly at its middle threshold: five equal colours whose channels sum above 255
	data.values[0] = 26;
	data.values[1] = 60;
	data.values[2] = 1000;
	data.values[3] = 660;
	data.values[4] = 20;
	ASSERT_TRUE(meas.Begin(HAQuDA_MeasParams(anim_id::measTotal), measContext(data, 100)));
	meas.Step(0);
	meas.Step(0);

	uint16_t rSum = 0;
	uint16_t gSum = 0;
	uint16_t bSum = 0;
	for (uint8_t i = 0; i < MEAS_ANIM_MAX_PARAMS; i++) {
		const rgb_color color = scaleColor(data.values[i], data.dots[i]);
		rSum += color.r;
		gSum += color.g;
		bSum += color.b;
	}
	ASSERT_GT(gSum, 255);

	EXPECT_EQ(LedRecorder::pack(static_cast<uint8_t>(rSum / 5), static_cast<uint8_t>(gSum / 5), static_cast<uint8_t>(bSum / 5)), leds.frames.back()[0]);
	EXPECT_EQ(static_cast<size_t>(ANIM_PIXELS), leds.litCount(leds.frames.back()));
}

TEST(MeasAnimTest, TotalShowsTheFirstParameterOutsideItsRange) {
	LedRecorder leds;
	HAQuDA_MeasTotalAnim meas(leds.mock);
	meas_anim_data data = lampData();
	data.values[2] = 5200;
	ASSERT_TRUE(meas.Begin(HAQuDA_MeasParams(anim_id::measTotal), measContext(data, 100)));
	meas.Step(0);
	meas.Step(0);

	EXPECT_EQ(packed(scaleColor(5200, data.dots[2])), leds.frames.back()[0]);
}

TEST(MeasAnimTest, TotalTreatsTempAndHumidAsBadOnBothSides) {
	LedRecorder leds;
	HAQuDA_MeasTotalAnim meas(leds.mock);
	meas_anim_data data = lampData();
	// temperature at the lower threshold counts as bad (blue); a high eCO2 behind it is not reached
	data.values[0] = 20;
	data.values[2] = 5200;
	ASSERT_TRUE(meas.Begin(HAQuDA_MeasParams(anim_id::measTotal), measContext(data, 100)));
	meas.Step(0);
	meas.Step(0);

	EXPECT_EQ(packed(scaleColor(20, data.dots[0])), leds.frames.back()[0]);
}

TEST(MeasAnimTest, TotalLeavesOutReadingsWithoutASample) {
	LedRecorder leds;
	HAQuDA_MeasTotalAnim meas(leds.mock);
	meas_anim_data data = lampData();
	data.values[0] = 26;
	data.values[1] = 60;
	data.values[4] = 20;
	data.noSample[2] = true;
	data.noSample[3] = true;
	ASSERT_TRUE(meas.Begin(HAQuDA_MeasParams(anim_id::measTotal), measContext(data, 100)));
	meas.Step(0);
	meas.Step(0);

	uint16_t rSum = 0;
	uint16_t gSum = 0;
	uint16_t bSum = 0;
	for (const uint8_t i : {0, 1, 4}) {
		const rgb_color color = scaleColor(data.values[i], data.dots[i]);
		rSum += color.r;
		gSum += color.g;
		bSum += color.b;
	}

	EXPECT_EQ(LedRecorder::pack(static_cast<uint8_t>(rSum / 3), static_cast<uint8_t>(gSum / 3), static_cast<uint8_t>(bSum / 3)), leds.frames.back()[0]);
}

TEST(MeasAnimTest, TotalNeedsAllFiveThresholdSets) {
	LedRecorder leds;
	HAQuDA_MeasTotalAnim meas(leds.mock);
	meas_anim_data data = lampData();
	data.dots[4] = {0, 0, 0};

	EXPECT_FALSE(meas.Begin(HAQuDA_MeasParams(anim_id::measTotal), measContext(data, 100)));
}

TEST(AnimationSetTest, EveryAnimationIdHasAnObjectWithThatId) {
	LedRecorder leds;
	HAQuDA_AnimationSet set(leds.mock);

	for (uint8_t i = 0; i < ANIM_COUNT; i++) {
		HAQuDA_Animation *animation = set.Get(static_cast<anim_id>(i));
		ASSERT_NE(nullptr, animation) << "anim_id " << static_cast<int>(i);
		EXPECT_EQ(static_cast<anim_id>(i), animation->Id());
		EXPECT_STRNE("?", animation->Name());
	}
}

TEST(AnimationSetTest, NoneHasNoObject) {
	LedRecorder leds;
	HAQuDA_AnimationSet set(leds.mock);

	EXPECT_EQ(nullptr, set.Get(anim_id::none));
	EXPECT_EQ(nullptr, set.Get(static_cast<anim_id>(200)));
}

TEST(AnimationSetTest, NamesAreUnique) {
	std::set<std::string> names;
	for (uint8_t i = 0; i < ANIM_COUNT; i++) {
		names.insert(HAQuDA_AnimName(static_cast<anim_id>(i)));
	}

	EXPECT_EQ(static_cast<size_t>(ANIM_COUNT), names.size());
	EXPECT_STREQ("?", HAQuDA_AnimName(anim_id::none));
}

TEST(AnimationSetTest, RandomPixelsAccessorIsTheSameObjectAsGet) {
	LedRecorder leds;
	HAQuDA_AnimationSet set(leds.mock);

	EXPECT_EQ(static_cast<HAQuDA_Animation *>(&set.RandomPixels()), set.Get(anim_id::randomPixels));
}
