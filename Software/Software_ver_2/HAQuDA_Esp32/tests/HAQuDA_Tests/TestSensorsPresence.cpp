#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "HAQuDA_SensorsPresence.h"

// Every test builds its own simulated clock: delayMs() advances nowMs, so a test runs instantly and the wait loop is checked
// for termination.

TEST(SensorsPresenceTest, BothAnswerReturnsBothTrue) {
	const uint32_t powerOnMs = 0;

	const bool exp_dht22 = true;
	const bool exp_zh03b = true;

	uint32_t nowMs = 0;
	auto dhtPresent = []() { return true; };
	auto pmPresent = []() { return true; };
	auto clock = [&]() { return nowMs; };
	auto delayMs = [&](uint32_t ms) { nowMs += ms; };

	sensors_presence res;

	res = HAQuDA_WaitForSensors(dhtPresent, pmPresent, clock, delayMs, powerOnMs);

	EXPECT_EQ(exp_dht22, res.dht22);
	EXPECT_EQ(exp_zh03b, res.zh03b);
}

TEST(SensorsPresenceTest, BothAnswerReturnsAtFirstDhtReadNotAtTimeout) {
	const uint32_t powerOnMs = 0;

	const uint32_t exp_now = SENSORS_DHT_FIRST_READ_MS;

	uint32_t nowMs = 0;
	auto dhtPresent = []() { return true; };
	auto pmPresent = []() { return true; };
	auto clock = [&]() { return nowMs; };
	auto delayMs = [&](uint32_t ms) { nowMs += ms; };

	sensors_presence res;

	res = HAQuDA_WaitForSensors(dhtPresent, pmPresent, clock, delayMs, powerOnMs);

	EXPECT_EQ(exp_now, nowMs);
}

TEST(SensorsPresenceTest, DhtIsNotReadBeforeItsFirstReadTime) {
	const uint32_t powerOnMs = 0;

	const uint32_t exp_first_call_ms = SENSORS_DHT_FIRST_READ_MS;

	uint32_t nowMs = 0;
	std::vector<uint32_t> dhtCallTimesMs;
	auto dhtPresent = [&]() {
		dhtCallTimesMs.push_back(nowMs);
		return true;
	};
	auto pmPresent = []() { return true; };
	auto clock = [&]() { return nowMs; };
	auto delayMs = [&](uint32_t ms) { nowMs += ms; };

	sensors_presence res;

	res = HAQuDA_WaitForSensors(dhtPresent, pmPresent, clock, delayMs, powerOnMs);

	ASSERT_FALSE(dhtCallTimesMs.empty());
	EXPECT_EQ(exp_first_call_ms, dhtCallTimesMs[0]);
}

TEST(SensorsPresenceTest, NothingAnswersGivesUpAtTimeout) {
	const uint32_t powerOnMs = 0;

	const bool exp_dht22 = false;
	const bool exp_zh03b = false;
	const uint32_t exp_now = SENSORS_PRESENCE_TIMEOUT_MS;

	uint32_t nowMs = 0;
	auto dhtPresent = []() { return false; };
	auto pmPresent = []() { return false; };
	auto clock = [&]() { return nowMs; };
	auto delayMs = [&](uint32_t ms) { nowMs += ms; };

	sensors_presence res;

	res = HAQuDA_WaitForSensors(dhtPresent, pmPresent, clock, delayMs, powerOnMs);

	EXPECT_EQ(exp_dht22, res.dht22);
	EXPECT_EQ(exp_zh03b, res.zh03b);
	EXPECT_EQ(exp_now, nowMs);
}

TEST(SensorsPresenceTest, MissingDhtOnlyReportsDhtFalse) {
	const uint32_t powerOnMs = 0;
	const uint32_t pmAnswersFromMs = 1000;

	const bool exp_dht22 = false;
	const bool exp_zh03b = true;

	uint32_t nowMs = 0;
	auto dhtPresent = []() { return false; };
	auto pmPresent = [&]() { return nowMs >= pmAnswersFromMs; };
	auto clock = [&]() { return nowMs; };
	auto delayMs = [&](uint32_t ms) { nowMs += ms; };

	sensors_presence res;

	res = HAQuDA_WaitForSensors(dhtPresent, pmPresent, clock, delayMs, powerOnMs);

	EXPECT_EQ(exp_dht22, res.dht22);
	EXPECT_EQ(exp_zh03b, res.zh03b);
}

TEST(SensorsPresenceTest, MissingPmOnlyReportsPmFalse) {
	const uint32_t powerOnMs = 0;

	const bool exp_dht22 = true;
	const bool exp_zh03b = false;

	uint32_t nowMs = 0;
	auto dhtPresent = []() { return true; };
	auto pmPresent = []() { return false; };
	auto clock = [&]() { return nowMs; };
	auto delayMs = [&](uint32_t ms) { nowMs += ms; };

	sensors_presence res;

	res = HAQuDA_WaitForSensors(dhtPresent, pmPresent, clock, delayMs, powerOnMs);

	EXPECT_EQ(exp_dht22, res.dht22);
	EXPECT_EQ(exp_zh03b, res.zh03b);
}

TEST(SensorsPresenceTest, DhtAnsweringOnRetryIsAccepted) {
	// The first read at 2 s fails, the retry at 4 s succeeds
	const uint32_t powerOnMs = 0;
	const uint32_t dhtAnswersFromMs = 3000;

	const bool exp_dht22 = true;

	uint32_t nowMs = 0;
	auto dhtPresent = [&]() { return nowMs >= dhtAnswersFromMs; };
	auto pmPresent = []() { return true; };
	auto clock = [&]() { return nowMs; };
	auto delayMs = [&](uint32_t ms) { nowMs += ms; };

	sensors_presence res;

	res = HAQuDA_WaitForSensors(dhtPresent, pmPresent, clock, delayMs, powerOnMs);

	EXPECT_EQ(exp_dht22, res.dht22);
}

TEST(SensorsPresenceTest, DhtIsReadOnlyTwiceWithinTimeout) {
	// 2 s spacing between reads (the DHT22 needs at least 2 s): reads at 2 s and 4 s, none at 6 s
	const uint32_t powerOnMs = 0;

	const int exp_calls = 2;

	uint32_t nowMs = 0;
	int dhtCalls = 0;
	auto dhtPresent = [&]() {
		dhtCalls++;
		return false;
	};
	auto pmPresent = []() { return true; };
	auto clock = [&]() { return nowMs; };
	auto delayMs = [&](uint32_t ms) { nowMs += ms; };

	sensors_presence res;

	res = HAQuDA_WaitForSensors(dhtPresent, pmPresent, clock, delayMs, powerOnMs);

	EXPECT_EQ(exp_calls, dhtCalls);
}

TEST(SensorsPresenceTest, SucceededProbeIsNotRunAgain) {
	const uint32_t powerOnMs = 0;

	const int exp_calls = 1;

	uint32_t nowMs = 0;
	int pmCalls = 0;
	auto dhtPresent = []() { return false; };
	auto pmPresent = [&]() {
		pmCalls++;
		return true;
	};
	auto clock = [&]() { return nowMs; };
	auto delayMs = [&](uint32_t ms) { nowMs += ms; };

	sensors_presence res;

	res = HAQuDA_WaitForSensors(dhtPresent, pmPresent, clock, delayMs, powerOnMs);

	EXPECT_EQ(exp_calls, pmCalls);
}

TEST(SensorsPresenceTest, PmAnswersLateIsAcceptedBeforeTimeout) {
	const uint32_t powerOnMs = 0;
	const uint32_t pmAnswersFromMs = 4500;

	const bool exp_zh03b = true;

	uint32_t nowMs = 0;
	auto dhtPresent = []() { return true; };
	auto pmPresent = [&]() { return nowMs >= pmAnswersFromMs; };
	auto clock = [&]() { return nowMs; };
	auto delayMs = [&](uint32_t ms) { nowMs += ms; };

	sensors_presence res;

	res = HAQuDA_WaitForSensors(dhtPresent, pmPresent, clock, delayMs, powerOnMs);

	EXPECT_EQ(exp_zh03b, res.zh03b);
}

TEST(SensorsPresenceTest, NothingAnswersAcrossMillisWrapGivesUpAtTimeout) {
	// Power-on 1 s before the 32-bit wrap: the elapsed time must still be counted, not read as ~49 days or as 0
	const uint32_t powerOnMs = UINT32_MAX - 1000;

	const bool exp_dht22 = false;
	const bool exp_zh03b = false;
	const uint32_t exp_elapsed_ms = SENSORS_PRESENCE_TIMEOUT_MS;

	uint32_t nowMs = powerOnMs;
	auto dhtPresent = []() { return false; };
	auto pmPresent = []() { return false; };
	auto clock = [&]() { return nowMs; };
	auto delayMs = [&](uint32_t ms) { nowMs += ms; };

	sensors_presence res;

	res = HAQuDA_WaitForSensors(dhtPresent, pmPresent, clock, delayMs, powerOnMs);

	EXPECT_EQ(exp_dht22, res.dht22);
	EXPECT_EQ(exp_zh03b, res.zh03b);
	EXPECT_EQ(exp_elapsed_ms, (uint32_t)(nowMs - powerOnMs));
}

TEST(SensorsPresenceTest, PmAnswersAfterMillisWrapIsAcceptedAtItsTime) {
	// Power-on 1 s before the wrap, the ZH03B answers 3 s later (2 s after the wrap): accepted then, not at the timeout
	const uint32_t powerOnMs = UINT32_MAX - 1000;
	const uint32_t pmAnswersAfterMs = 3000;

	const bool exp_dht22 = true;
	const bool exp_zh03b = true;
	const uint32_t exp_elapsed_ms = pmAnswersAfterMs;

	uint32_t nowMs = powerOnMs;
	auto dhtPresent = []() { return true; };
	auto pmPresent = [&]() { return (uint32_t)(nowMs - powerOnMs) >= pmAnswersAfterMs; };
	auto clock = [&]() { return nowMs; };
	auto delayMs = [&](uint32_t ms) { nowMs += ms; };

	sensors_presence res;

	res = HAQuDA_WaitForSensors(dhtPresent, pmPresent, clock, delayMs, powerOnMs);

	EXPECT_EQ(exp_dht22, res.dht22);
	EXPECT_EQ(exp_zh03b, res.zh03b);
	EXPECT_EQ(exp_elapsed_ms, (uint32_t)(nowMs - powerOnMs));
}

TEST(SensorsPresenceTest, LateStartPastDeadlineStillProbesPmOnce) {
	// The CCS811 start may eat into the budget; a sensor that already talks must still be accepted
	const uint32_t powerOnMs = 0;
	const uint32_t startNowMs = SENSORS_PRESENCE_TIMEOUT_MS + 1000;

	const bool exp_zh03b = true;

	uint32_t nowMs = startNowMs;
	auto dhtPresent = []() { return false; };
	auto pmPresent = []() { return true; };
	auto clock = [&]() { return nowMs; };
	auto delayMs = [&](uint32_t ms) { nowMs += ms; };

	sensors_presence res;

	res = HAQuDA_WaitForSensors(dhtPresent, pmPresent, clock, delayMs, powerOnMs);

	EXPECT_EQ(exp_zh03b, res.zh03b);
}
