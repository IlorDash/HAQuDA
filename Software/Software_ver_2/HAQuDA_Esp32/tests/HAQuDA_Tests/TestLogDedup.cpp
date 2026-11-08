#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>

#include "HAQuDA_LogDedup.h"

// ---------------------------------------------------------------------------------------------
// Check(): first occurrence, repeats, distinct messages
// ---------------------------------------------------------------------------------------------

TEST(LogDedupTest, CheckFirstOccurrenceIsWritten) {
	const uint32_t nowMs = 1000;

	const bool exp_write = true;

	const char *text = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;

	log_dedup_result res;

	res = dedup.Check(log_dedup_level::info, text, nowMs);

	EXPECT_EQ(exp_write, res.write);
}

TEST(LogDedupTest, CheckFirstOccurrenceHasNoEvictedSummary) {
	const uint32_t nowMs = 1000;

	const bool exp_hasEvicted = false;

	const char *text = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;

	log_dedup_result res;

	res = dedup.Check(log_dedup_level::info, text, nowMs);

	EXPECT_EQ(exp_hasEvicted, res.hasEvicted);
}

TEST(LogDedupTest, CheckRepeatIsSuppressed) {
	const uint32_t firstMs = 1000;
	const uint32_t repeatMs = 2000;

	const bool exp_write = false;

	const char *text = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;

	log_dedup_result res;

	dedup.Check(log_dedup_level::info, text, firstMs);

	res = dedup.Check(log_dedup_level::info, text, repeatMs);

	EXPECT_EQ(exp_write, res.write);
}

TEST(LogDedupTest, CheckSecondRepeatIsSuppressed) {
	const uint32_t firstMs = 1000;
	const uint32_t repeat1Ms = 2000;
	const uint32_t repeat2Ms = 3000;

	const bool exp_write = false;

	const char *text = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;

	log_dedup_result res;

	dedup.Check(log_dedup_level::info, text, firstMs);
	dedup.Check(log_dedup_level::info, text, repeat1Ms);

	res = dedup.Check(log_dedup_level::info, text, repeat2Ms);

	EXPECT_EQ(exp_write, res.write);
}

TEST(LogDedupTest, CheckNullTextIsAlwaysWritten) {
	const uint32_t nowMs = 1000;

	const bool exp_write = true;

	const char *text = nullptr;
	HAQuDA_LogDedup dedup;

	log_dedup_result res;

	dedup.Check(log_dedup_level::info, text, nowMs);

	res = dedup.Check(log_dedup_level::info, text, nowMs);

	EXPECT_EQ(exp_write, res.write);
}

TEST(LogDedupTest, CheckInterleavedNewMessageAfterTwoOthersIsWritten) {
	const uint32_t eco2Ms = 1000;
	const uint32_t tvocMs = 1001;
	const uint32_t pmMs = 1002;

	const bool exp_write = true;

	const char *eco2Text = "Failed to get eCO2 meas";
	const char *tvocText = "Failed to get TVOC meas";
	const char *pmText = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;

	log_dedup_result res;

	dedup.Check(log_dedup_level::info, eco2Text, eco2Ms);
	dedup.Check(log_dedup_level::info, tvocText, tvocMs);

	res = dedup.Check(log_dedup_level::info, pmText, pmMs);

	EXPECT_EQ(exp_write, res.write);
}

TEST(LogDedupTest, CheckInterleavedFirstMessageRepeatedAfterOthersIsSuppressed) {
	const uint32_t eco2Ms = 1000;
	const uint32_t tvocMs = 1001;
	const uint32_t pmMs = 1002;
	const uint32_t eco2AgainMs = 2000;

	const bool exp_write = false;

	const char *eco2Text = "Failed to get eCO2 meas";
	const char *tvocText = "Failed to get TVOC meas";
	const char *pmText = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;

	log_dedup_result res;

	dedup.Check(log_dedup_level::info, eco2Text, eco2Ms);
	dedup.Check(log_dedup_level::info, tvocText, tvocMs);
	dedup.Check(log_dedup_level::info, pmText, pmMs);

	res = dedup.Check(log_dedup_level::info, eco2Text, eco2AgainMs);

	EXPECT_EQ(exp_write, res.write);
}

TEST(LogDedupTest, CheckInterleavedLastMessageRepeatedAfterOthersIsSuppressed) {
	const uint32_t eco2Ms = 1000;
	const uint32_t tvocMs = 1001;
	const uint32_t pmMs = 1002;
	const uint32_t pmAgainMs = 2000;

	const bool exp_write = false;

	const char *eco2Text = "Failed to get eCO2 meas";
	const char *tvocText = "Failed to get TVOC meas";
	const char *pmText = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;

	log_dedup_result res;

	dedup.Check(log_dedup_level::info, eco2Text, eco2Ms);
	dedup.Check(log_dedup_level::info, tvocText, tvocMs);
	dedup.Check(log_dedup_level::info, pmText, pmMs);

	res = dedup.Check(log_dedup_level::info, pmText, pmAgainMs);

	EXPECT_EQ(exp_write, res.write);
}

// ---------------------------------------------------------------------------------------------
// Check(): text comparison is limited to LOG_DEDUP_TEXT_SIZE - 1 characters
// ---------------------------------------------------------------------------------------------

TEST(LogDedupTest, CheckTextsEqualInFirst31CharsAreSameMessage) {
	const uint32_t firstMs = 1000;
	const uint32_t secondMs = 2000;

	const bool exp_write = false;

	// 31 identical characters, then different tails
	const char *textA = "0123456789012345678901234567890-first tail";
	const char *textB = "0123456789012345678901234567890-other tail";
	HAQuDA_LogDedup dedup;

	log_dedup_result res;

	dedup.Check(log_dedup_level::info, textA, firstMs);

	res = dedup.Check(log_dedup_level::info, textB, secondMs);

	EXPECT_EQ(exp_write, res.write);
}

TEST(LogDedupTest, CheckTextsDifferingAtChar31AreDistinctMessages) {
	const uint32_t firstMs = 1000;
	const uint32_t secondMs = 2000;

	const bool exp_write = true;

	// the 31st character (last one that is kept) differs
	const char *textA = "012345678901234567890123456789A";
	const char *textB = "012345678901234567890123456789B";
	HAQuDA_LogDedup dedup;

	log_dedup_result res;

	dedup.Check(log_dedup_level::info, textA, firstMs);

	res = dedup.Check(log_dedup_level::info, textB, secondMs);

	EXPECT_EQ(exp_write, res.write);
}

TEST(LogDedupTest, CheckPrefixOfKnownMessageIsDistinct) {
	const uint32_t firstMs = 1000;
	const uint32_t secondMs = 2000;

	const bool exp_write = true;

	const char *longText = "Failed to get PM 2.5 meas";
	const char *prefixText = "Failed to get PM";
	HAQuDA_LogDedup dedup;

	log_dedup_result res;

	dedup.Check(log_dedup_level::info, longText, firstMs);

	res = dedup.Check(log_dedup_level::info, prefixText, secondMs);

	EXPECT_EQ(exp_write, res.write);
}

// ---------------------------------------------------------------------------------------------
// PopExpired(): expiry, summaries
// ---------------------------------------------------------------------------------------------

TEST(LogDedupTest, PopExpiredOneMsBeforeLimitReturnsFalse) {
	const uint32_t firstMs = 1000;
	const uint32_t repeatMs = 2000;
	const uint32_t nowMs = repeatMs + LOG_DEDUP_EXPIRY_MS - 1;

	const bool exp_res = false;

	const char *text = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;
	log_dedup_summary summary;

	bool res;

	dedup.Check(log_dedup_level::info, text, firstMs);
	dedup.Check(log_dedup_level::info, text, repeatMs);

	res = dedup.PopExpired(nowMs, summary);

	EXPECT_EQ(exp_res, res);
}

TEST(LogDedupTest, PopExpiredExactlyAtLimitReturnsSummary) {
	const uint32_t firstMs = 1000;
	const uint32_t repeatMs = 2000;
	const uint32_t nowMs = repeatMs + LOG_DEDUP_EXPIRY_MS;

	const bool exp_res = true;

	const char *text = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;
	log_dedup_summary summary;

	bool res;

	dedup.Check(log_dedup_level::info, text, firstMs);
	dedup.Check(log_dedup_level::info, text, repeatMs);

	res = dedup.PopExpired(nowMs, summary);

	EXPECT_EQ(exp_res, res);
}

TEST(LogDedupTest, PopExpiredAfterLimitReturnsSummaryWithTextRepeatsAndLastSeen) {
	const uint32_t firstMs = 1000;
	const uint32_t repeat1Ms = 2000;
	const uint32_t repeat2Ms = 3000;
	const uint32_t nowMs = repeat2Ms + LOG_DEDUP_EXPIRY_MS + 500;

	const char *exp_text = "Failed to get PM 2.5 meas";
	const uint32_t exp_repeats = 2;
	const uint32_t exp_lastSeenMs = repeat2Ms;

	const char *text = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;
	log_dedup_summary summary;

	bool res;

	dedup.Check(log_dedup_level::info, text, firstMs);
	dedup.Check(log_dedup_level::info, text, repeat1Ms);
	dedup.Check(log_dedup_level::info, text, repeat2Ms);

	res = dedup.PopExpired(nowMs, summary);

	ASSERT_TRUE(res);
	EXPECT_STREQ(exp_text, summary.text);
	EXPECT_EQ(exp_repeats, summary.repeats);
	EXPECT_EQ(exp_lastSeenMs, summary.lastSeenMs);
}

TEST(LogDedupTest, PopExpiredCountsAgeFromLastRepeatNotFromFirstSeen) {
	const uint32_t firstMs = 0;
	const uint32_t repeatMs = 8000;
	const uint32_t nowMs = 12000; // 12 s after first seen, but only 4 s after the last repeat

	const bool exp_res = false;

	const char *text = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;
	log_dedup_summary summary;

	bool res;

	dedup.Check(log_dedup_level::info, text, firstMs);
	dedup.Check(log_dedup_level::info, text, repeatMs);

	res = dedup.PopExpired(nowMs, summary);

	EXPECT_EQ(exp_res, res);
}

TEST(LogDedupTest, PopExpiredWithoutRepeatsReturnsFalse) {
	const uint32_t firstMs = 1000;
	const uint32_t nowMs = firstMs + LOG_DEDUP_EXPIRY_MS;

	const bool exp_res = false;

	const char *text = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;
	log_dedup_summary summary;

	bool res;

	dedup.Check(log_dedup_level::info, text, firstMs);

	res = dedup.PopExpired(nowMs, summary);

	EXPECT_EQ(exp_res, res);
}

TEST(LogDedupTest, PopExpiredWithoutRepeatsFreesEntrySoNextCheckWrites) {
	const uint32_t firstMs = 1000;
	const uint32_t expiredMs = firstMs + LOG_DEDUP_EXPIRY_MS;
	const uint32_t againMs = expiredMs + 1;

	const bool exp_write = true;

	const char *text = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;
	log_dedup_summary summary;

	log_dedup_result res;

	dedup.Check(log_dedup_level::info, text, firstMs);
	dedup.PopExpired(expiredMs, summary);

	res = dedup.Check(log_dedup_level::info, text, againMs);

	EXPECT_EQ(exp_write, res.write);
}

TEST(LogDedupTest, PopExpiredWithRepeatsFreesEntrySoNextCheckWrites) {
	const uint32_t firstMs = 1000;
	const uint32_t repeatMs = 2000;
	const uint32_t expiredMs = repeatMs + LOG_DEDUP_EXPIRY_MS;
	const uint32_t againMs = expiredMs + 1;

	const bool exp_write = true;

	const char *text = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;
	log_dedup_summary summary;

	log_dedup_result res;

	dedup.Check(log_dedup_level::info, text, firstMs);
	dedup.Check(log_dedup_level::info, text, repeatMs);
	dedup.PopExpired(expiredMs, summary);

	res = dedup.Check(log_dedup_level::info, text, againMs);

	EXPECT_EQ(exp_write, res.write);
}

TEST(LogDedupTest, PopExpiredCalledAgainAfterSummaryTakenReturnsFalse) {
	const uint32_t firstMs = 1000;
	const uint32_t repeatMs = 2000;
	const uint32_t nowMs = repeatMs + LOG_DEDUP_EXPIRY_MS;

	const bool exp_res = false;

	const char *text = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;
	log_dedup_summary summary;

	bool res;

	dedup.Check(log_dedup_level::info, text, firstMs);
	dedup.Check(log_dedup_level::info, text, repeatMs);
	dedup.PopExpired(nowMs, summary);

	res = dedup.PopExpired(nowMs, summary);

	EXPECT_EQ(exp_res, res);
}

TEST(LogDedupTest, PopExpiredReturnsOneSummaryPerCallFirstCall) {
	const uint32_t startMs = 1000;
	const uint32_t nowMs = startMs + 1 + LOG_DEDUP_EXPIRY_MS;

	const char *exp_text = "Failed to get eCO2 meas";

	const char *eco2Text = "Failed to get eCO2 meas";
	const char *tvocText = "Failed to get TVOC meas";
	HAQuDA_LogDedup dedup;
	log_dedup_summary summary;

	bool res;

	dedup.Check(log_dedup_level::info, eco2Text, startMs);
	dedup.Check(log_dedup_level::info, eco2Text, startMs);
	dedup.Check(log_dedup_level::info, tvocText, startMs);
	dedup.Check(log_dedup_level::info, tvocText, startMs);

	res = dedup.PopExpired(nowMs, summary);

	ASSERT_TRUE(res);
	EXPECT_STREQ(exp_text, summary.text);
}

TEST(LogDedupTest, PopExpiredReturnsOneSummaryPerCallSecondCall) {
	const uint32_t startMs = 1000;
	const uint32_t nowMs = startMs + 1 + LOG_DEDUP_EXPIRY_MS;

	const char *exp_text = "Failed to get TVOC meas";

	const char *eco2Text = "Failed to get eCO2 meas";
	const char *tvocText = "Failed to get TVOC meas";
	HAQuDA_LogDedup dedup;
	log_dedup_summary summary;
	log_dedup_summary firstSummary;

	bool res;

	dedup.Check(log_dedup_level::info, eco2Text, startMs);
	dedup.Check(log_dedup_level::info, eco2Text, startMs);
	dedup.Check(log_dedup_level::info, tvocText, startMs);
	dedup.Check(log_dedup_level::info, tvocText, startMs);
	dedup.PopExpired(nowMs, firstSummary);

	res = dedup.PopExpired(nowMs, summary);

	ASSERT_TRUE(res);
	EXPECT_STREQ(exp_text, summary.text);
}

TEST(LogDedupTest, PopExpiredThirdCallAfterTwoSummariesReturnsFalse) {
	const uint32_t startMs = 1000;
	const uint32_t nowMs = startMs + 1 + LOG_DEDUP_EXPIRY_MS;

	const bool exp_res = false;

	const char *eco2Text = "Failed to get eCO2 meas";
	const char *tvocText = "Failed to get TVOC meas";
	HAQuDA_LogDedup dedup;
	log_dedup_summary summary;
	log_dedup_summary firstSummary;
	log_dedup_summary secondSummary;

	bool res;

	dedup.Check(log_dedup_level::info, eco2Text, startMs);
	dedup.Check(log_dedup_level::info, eco2Text, startMs);
	dedup.Check(log_dedup_level::info, tvocText, startMs);
	dedup.Check(log_dedup_level::info, tvocText, startMs);
	dedup.PopExpired(nowMs, firstSummary);
	dedup.PopExpired(nowMs, secondSummary);

	res = dedup.PopExpired(nowMs, summary);

	EXPECT_EQ(exp_res, res);
}

TEST(LogDedupTest, PopExpiredSkipsEntryWithoutRepeatsAndReturnsNextWithRepeats) {
	const uint32_t startMs = 1000;
	const uint32_t nowMs = startMs + LOG_DEDUP_EXPIRY_MS;

	const char *exp_text = "Failed to get TVOC meas";

	const char *eco2Text = "Failed to get eCO2 meas";
	const char *tvocText = "Failed to get TVOC meas";
	HAQuDA_LogDedup dedup;
	log_dedup_summary summary;

	bool res;

	dedup.Check(log_dedup_level::info, eco2Text, startMs);
	dedup.Check(log_dedup_level::info, tvocText, startMs);
	dedup.Check(log_dedup_level::info, tvocText, startMs);

	res = dedup.PopExpired(nowMs, summary);

	ASSERT_TRUE(res);
	EXPECT_STREQ(exp_text, summary.text);
}

TEST(LogDedupTest, PopExpiredKeepsFreshEntryWhenAnotherExpired) {
	const uint32_t oldMs = 1000;
	const uint32_t freshMs = 9000;
	const uint32_t nowMs = oldMs + LOG_DEDUP_EXPIRY_MS;

	const bool exp_write = false;

	const char *oldText = "Failed to get eCO2 meas";
	const char *freshText = "Failed to get TVOC meas";
	HAQuDA_LogDedup dedup;
	log_dedup_summary summary;

	log_dedup_result res;

	dedup.Check(log_dedup_level::info, oldText, oldMs);
	dedup.Check(log_dedup_level::info, oldText, oldMs);
	dedup.Check(log_dedup_level::info, freshText, freshMs);
	dedup.PopExpired(nowMs, summary);

	res = dedup.Check(log_dedup_level::info, freshText, nowMs);

	EXPECT_EQ(exp_write, res.write);
}

TEST(LogDedupTest, PopExpiredOnEmptyTableReturnsFalse) {
	const uint32_t nowMs = 100000;

	const bool exp_res = false;

	HAQuDA_LogDedup dedup;
	log_dedup_summary summary;

	bool res;

	res = dedup.PopExpired(nowMs, summary);

	EXPECT_EQ(exp_res, res);
}

TEST(LogDedupTest, PopExpiredSummaryTextOfLongMessageIsCutAndTerminated) {
	const uint32_t firstMs = 1000;
	const uint32_t repeatMs = 2000;
	const uint32_t nowMs = repeatMs + LOG_DEDUP_EXPIRY_MS;

	const char *exp_text = "0123456789012345678901234567890"; // 31 chars + terminator = field size

	const char *longText = "0123456789012345678901234567890-and a long tail that does not fit";
	HAQuDA_LogDedup dedup;
	log_dedup_summary summary;

	bool res;

	dedup.Check(log_dedup_level::info, longText, firstMs);
	dedup.Check(log_dedup_level::info, longText, repeatMs);

	res = dedup.PopExpired(nowMs, summary);

	ASSERT_TRUE(res);
	EXPECT_STREQ(exp_text, summary.text);
}

// ---------------------------------------------------------------------------------------------
// Check(): table full, eviction
// ---------------------------------------------------------------------------------------------

TEST(LogDedupTest, CheckFullTableEvictsLeastRecentlySeenWithRepeatsHasSummary) {
	const uint32_t m0Ms = 100;
	const uint32_t m0RepeatMs = 101;
	const uint32_t othersMs = 200;
	const uint32_t newMs = 300;

	const bool exp_write = true;
	const bool exp_hasEvicted = true;
	const char *exp_text = "message 0";
	const uint32_t exp_repeats = 1;
	const uint32_t exp_lastSeenMs = m0RepeatMs;

	HAQuDA_LogDedup dedup;
	const char *newText = "message new";

	log_dedup_result res;

	dedup.Check(log_dedup_level::info, "message 0", m0Ms);
	dedup.Check(log_dedup_level::info, "message 0", m0RepeatMs);
	dedup.Check(log_dedup_level::info, "message 1", othersMs);
	dedup.Check(log_dedup_level::info, "message 2", othersMs);
	dedup.Check(log_dedup_level::info, "message 3", othersMs);
	dedup.Check(log_dedup_level::info, "message 4", othersMs);
	dedup.Check(log_dedup_level::info, "message 5", othersMs);
	dedup.Check(log_dedup_level::info, "message 6", othersMs);
	dedup.Check(log_dedup_level::info, "message 7", othersMs);

	res = dedup.Check(log_dedup_level::info, newText, newMs);

	EXPECT_EQ(exp_write, res.write);
	EXPECT_EQ(exp_hasEvicted, res.hasEvicted);
	EXPECT_STREQ(exp_text, res.evicted.text);
	EXPECT_EQ(exp_repeats, res.evicted.repeats);
	EXPECT_EQ(exp_lastSeenMs, res.evicted.lastSeenMs);
}

TEST(LogDedupTest, CheckFullTableEvictsByLastSeenNotByInsertionOrder) {
	const uint32_t m0Ms = 100;
	const uint32_t m0RepeatMs = 900;
	const uint32_t m1Ms = 200;
	const uint32_t m1RepeatMs = 210;
	const uint32_t othersMs = 300;
	const uint32_t newMs = 1000;

	const bool exp_hasEvicted = true;
	const char *exp_text = "message 1"; // inserted second, but seen least recently

	HAQuDA_LogDedup dedup;
	const char *newText = "message new";

	log_dedup_result res;

	dedup.Check(log_dedup_level::info, "message 0", m0Ms);
	dedup.Check(log_dedup_level::info, "message 0", m0RepeatMs);
	dedup.Check(log_dedup_level::info, "message 1", m1Ms);
	dedup.Check(log_dedup_level::info, "message 1", m1RepeatMs);
	dedup.Check(log_dedup_level::info, "message 2", othersMs);
	dedup.Check(log_dedup_level::info, "message 3", othersMs);
	dedup.Check(log_dedup_level::info, "message 4", othersMs);
	dedup.Check(log_dedup_level::info, "message 5", othersMs);
	dedup.Check(log_dedup_level::info, "message 6", othersMs);
	dedup.Check(log_dedup_level::info, "message 7", othersMs);

	res = dedup.Check(log_dedup_level::info, newText, newMs);

	EXPECT_EQ(exp_hasEvicted, res.hasEvicted);
	EXPECT_STREQ(exp_text, res.evicted.text);
}

TEST(LogDedupTest, CheckFullTableEvictedWithoutRepeatsIsDroppedSilently) {
	const uint32_t m0Ms = 100;
	const uint32_t othersMs = 200;
	const uint32_t newMs = 300;

	const bool exp_write = true;
	const bool exp_hasEvicted = false;

	HAQuDA_LogDedup dedup;
	const char *newText = "message new";

	log_dedup_result res;

	dedup.Check(log_dedup_level::info, "message 0", m0Ms);
	dedup.Check(log_dedup_level::info, "message 1", othersMs);
	dedup.Check(log_dedup_level::info, "message 2", othersMs);
	dedup.Check(log_dedup_level::info, "message 3", othersMs);
	dedup.Check(log_dedup_level::info, "message 4", othersMs);
	dedup.Check(log_dedup_level::info, "message 5", othersMs);
	dedup.Check(log_dedup_level::info, "message 6", othersMs);
	dedup.Check(log_dedup_level::info, "message 7", othersMs);

	res = dedup.Check(log_dedup_level::info, newText, newMs);

	EXPECT_EQ(exp_write, res.write);
	EXPECT_EQ(exp_hasEvicted, res.hasEvicted);
}

TEST(LogDedupTest, CheckTableWithFreeSlotDoesNotEvictOlderEntryWithRepeats) {
	const uint32_t m0Ms = 100;
	const uint32_t m0RepeatMs = 101;
	const uint32_t othersMs = 200;
	const uint32_t newMs = 300;

	const bool exp_hasEvicted = false;

	HAQuDA_LogDedup dedup;
	const char *newText = "message new";

	log_dedup_result res;

	// 7 of 8 slots used, the oldest one has repeats
	dedup.Check(log_dedup_level::info, "message 0", m0Ms);
	dedup.Check(log_dedup_level::info, "message 0", m0RepeatMs);
	dedup.Check(log_dedup_level::info, "message 1", othersMs);
	dedup.Check(log_dedup_level::info, "message 2", othersMs);
	dedup.Check(log_dedup_level::info, "message 3", othersMs);
	dedup.Check(log_dedup_level::info, "message 4", othersMs);
	dedup.Check(log_dedup_level::info, "message 5", othersMs);
	dedup.Check(log_dedup_level::info, "message 6", othersMs);

	res = dedup.Check(log_dedup_level::info, newText, newMs);

	EXPECT_EQ(exp_hasEvicted, res.hasEvicted);
}

TEST(LogDedupTest, CheckEvictedMessageIsNewAgainAfterEviction) {
	const uint32_t m0Ms = 100;
	const uint32_t m0RepeatMs = 101;
	const uint32_t othersMs = 200;
	const uint32_t newMs = 300;
	const uint32_t againMs = 400;

	const bool exp_write = true;

	HAQuDA_LogDedup dedup;
	const char *newText = "message new";

	log_dedup_result res;

	dedup.Check(log_dedup_level::info, "message 0", m0Ms);
	dedup.Check(log_dedup_level::info, "message 0", m0RepeatMs);
	dedup.Check(log_dedup_level::info, "message 1", othersMs);
	dedup.Check(log_dedup_level::info, "message 2", othersMs);
	dedup.Check(log_dedup_level::info, "message 3", othersMs);
	dedup.Check(log_dedup_level::info, "message 4", othersMs);
	dedup.Check(log_dedup_level::info, "message 5", othersMs);
	dedup.Check(log_dedup_level::info, "message 6", othersMs);
	dedup.Check(log_dedup_level::info, "message 7", othersMs);
	dedup.Check(log_dedup_level::info, newText, newMs);

	res = dedup.Check(log_dedup_level::info, "message 0", againMs);

	EXPECT_EQ(exp_write, res.write);
}

TEST(LogDedupTest, CheckFullTableKnownMessageRepeatDoesNotEvict) {
	const uint32_t m0Ms = 100;
	const uint32_t m0RepeatMs = 101;
	const uint32_t othersMs = 200;
	const uint32_t repeatMs = 300;

	const bool exp_write = false;
	const bool exp_hasEvicted = false;

	HAQuDA_LogDedup dedup;

	log_dedup_result res;

	dedup.Check(log_dedup_level::info, "message 0", m0Ms);
	dedup.Check(log_dedup_level::info, "message 0", m0RepeatMs);
	dedup.Check(log_dedup_level::info, "message 1", othersMs);
	dedup.Check(log_dedup_level::info, "message 2", othersMs);
	dedup.Check(log_dedup_level::info, "message 3", othersMs);
	dedup.Check(log_dedup_level::info, "message 4", othersMs);
	dedup.Check(log_dedup_level::info, "message 5", othersMs);
	dedup.Check(log_dedup_level::info, "message 6", othersMs);
	dedup.Check(log_dedup_level::info, "message 7", othersMs);

	res = dedup.Check(log_dedup_level::info, "message 3", repeatMs);

	EXPECT_EQ(exp_write, res.write);
	EXPECT_EQ(exp_hasEvicted, res.hasEvicted);
}

TEST(LogDedupTest, CheckEvictedSummaryTextOfLongMessageIsCutAndTerminated) {
	const uint32_t m0Ms = 100;
	const uint32_t m0RepeatMs = 101;
	const uint32_t othersMs = 200;
	const uint32_t newMs = 300;

	const char *exp_text = "0123456789012345678901234567890"; // 31 chars + terminator = field size

	HAQuDA_LogDedup dedup;
	const char *longText = "0123456789012345678901234567890-and a long tail that does not fit";
	const char *newText = "message new";

	log_dedup_result res;

	dedup.Check(log_dedup_level::info, longText, m0Ms);
	dedup.Check(log_dedup_level::info, longText, m0RepeatMs);
	dedup.Check(log_dedup_level::info, "message 1", othersMs);
	dedup.Check(log_dedup_level::info, "message 2", othersMs);
	dedup.Check(log_dedup_level::info, "message 3", othersMs);
	dedup.Check(log_dedup_level::info, "message 4", othersMs);
	dedup.Check(log_dedup_level::info, "message 5", othersMs);
	dedup.Check(log_dedup_level::info, "message 6", othersMs);
	dedup.Check(log_dedup_level::info, "message 7", othersMs);

	res = dedup.Check(log_dedup_level::info, newText, newMs);

	ASSERT_TRUE(res.hasEvicted);
	EXPECT_STREQ(exp_text, res.evicted.text);
}

// ---------------------------------------------------------------------------------------------
// millis() wrap (49 days): ages are unsigned differences
// ---------------------------------------------------------------------------------------------

TEST(LogDedupTest, PopExpiredAfterMillisWrapIsExpired) {
	const uint32_t firstMs = UINT32_MAX - 100;
	const uint32_t repeatMs = UINT32_MAX - 50;
	const uint32_t nowMs = 20000; // wrapped: 20051 ms after the last repeat

	const bool exp_res = true;

	const char *text = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;
	log_dedup_summary summary;

	bool res;

	dedup.Check(log_dedup_level::info, text, firstMs);
	dedup.Check(log_dedup_level::info, text, repeatMs);

	res = dedup.PopExpired(nowMs, summary);

	EXPECT_EQ(exp_res, res);
}

TEST(LogDedupTest, PopExpiredAfterMillisWrapBeforeLimitIsNotExpired) {
	const uint32_t firstMs = UINT32_MAX - 100;
	const uint32_t repeatMs = UINT32_MAX - 50;
	const uint32_t nowMs = 50; // wrapped: 101 ms after the last repeat

	const bool exp_res = false;

	const char *text = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;
	log_dedup_summary summary;

	bool res;

	dedup.Check(log_dedup_level::info, text, firstMs);
	dedup.Check(log_dedup_level::info, text, repeatMs);

	res = dedup.PopExpired(nowMs, summary);

	EXPECT_EQ(exp_res, res);
}

TEST(LogDedupTest, PopExpiredAfterMillisWrapExactlyAtLimitIsExpired) {
	const uint32_t beforeWrapMs = 4999; // distance of the stamp from UINT32_MAX
	const uint32_t firstMs = UINT32_MAX - beforeWrapMs;
	const uint32_t repeatMs = UINT32_MAX - beforeWrapMs;
	const uint32_t nowMs = LOG_DEDUP_EXPIRY_MS - beforeWrapMs - 1; // exactly LOG_DEDUP_EXPIRY_MS after the stamp (UINT32_MAX + 1 == 0)

	const bool exp_res = true;

	const char *text = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;
	log_dedup_summary summary;

	bool res;

	dedup.Check(log_dedup_level::info, text, firstMs);
	dedup.Check(log_dedup_level::info, text, repeatMs);

	res = dedup.PopExpired(nowMs, summary);

	EXPECT_EQ(exp_res, res);
}

TEST(LogDedupTest, CheckFullTableEvictsEntrySeenBeforeMillisWrap) {
	const uint32_t m0Ms = UINT32_MAX - 1000;
	const uint32_t m0RepeatMs = UINT32_MAX - 999;
	const uint32_t othersMs = 5; // after the wrap
	const uint32_t newMs = 10;

	const bool exp_hasEvicted = true;
	const char *exp_text = "message 0"; // oldest in real time although its stamp is numerically the largest

	HAQuDA_LogDedup dedup;
	const char *newText = "message new";

	log_dedup_result res;

	dedup.Check(log_dedup_level::info, "message 0", m0Ms);
	dedup.Check(log_dedup_level::info, "message 0", m0RepeatMs);
	dedup.Check(log_dedup_level::info, "message 1", othersMs);
	dedup.Check(log_dedup_level::info, "message 2", othersMs);
	dedup.Check(log_dedup_level::info, "message 3", othersMs);
	dedup.Check(log_dedup_level::info, "message 4", othersMs);
	dedup.Check(log_dedup_level::info, "message 5", othersMs);
	dedup.Check(log_dedup_level::info, "message 6", othersMs);
	dedup.Check(log_dedup_level::info, "message 7", othersMs);

	res = dedup.Check(log_dedup_level::info, newText, newMs);

	EXPECT_EQ(exp_hasEvicted, res.hasEvicted);
	EXPECT_STREQ(exp_text, res.evicted.text);
}

// ---------------------------------------------------------------------------------------------
// Level is part of the key
// ---------------------------------------------------------------------------------------------

TEST(LogDedupTest, CheckSameTextDifferentLevelIsWritten) {
	const uint32_t firstMs = 1000;
	const uint32_t secondMs = 2000;

	const bool exp_write = true;

	const char *text = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;

	log_dedup_result res;

	dedup.Check(log_dedup_level::info, text, firstMs);

	res = dedup.Check(log_dedup_level::error, text, secondMs);

	EXPECT_EQ(exp_write, res.write);
}

TEST(LogDedupTest, CheckSameTextDifferentLevelFirstErrorThenInfoIsWritten) {
	const uint32_t firstMs = 1000;
	const uint32_t secondMs = 2000;

	const bool exp_write = true;

	const char *text = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;

	log_dedup_result res;

	dedup.Check(log_dedup_level::error, text, firstMs);

	res = dedup.Check(log_dedup_level::info, text, secondMs);

	EXPECT_EQ(exp_write, res.write);
}

TEST(LogDedupTest, CheckErrorRepeatIsSuppressed) {
	const uint32_t firstMs = 1000;
	const uint32_t repeatMs = 2000;

	const bool exp_write = false;

	const char *text = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;

	log_dedup_result res;

	dedup.Check(log_dedup_level::error, text, firstMs);

	res = dedup.Check(log_dedup_level::error, text, repeatMs);

	EXPECT_EQ(exp_write, res.write);
}

TEST(LogDedupTest, CheckErrorRepeatAfterInfoWithSameTextIsSuppressed) {
	const uint32_t infoMs = 1000;
	const uint32_t errorMs = 2000;
	const uint32_t errorRepeatMs = 3000;

	const bool exp_write = false;

	const char *text = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;

	log_dedup_result res;

	dedup.Check(log_dedup_level::info, text, infoMs);
	dedup.Check(log_dedup_level::error, text, errorMs);

	res = dedup.Check(log_dedup_level::error, text, errorRepeatMs);

	EXPECT_EQ(exp_write, res.write);
}

TEST(LogDedupTest, PopExpiredErrorEntrySummaryHasErrorLevel) {
	const uint32_t firstMs = 1000;
	const uint32_t repeatMs = 2000;
	const uint32_t nowMs = repeatMs + LOG_DEDUP_EXPIRY_MS;

	const log_dedup_level exp_level = log_dedup_level::error;

	const char *text = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;
	log_dedup_summary summary;

	bool res;

	dedup.Check(log_dedup_level::error, text, firstMs);
	dedup.Check(log_dedup_level::error, text, repeatMs);

	res = dedup.PopExpired(nowMs, summary);

	ASSERT_TRUE(res);
	EXPECT_EQ(exp_level, summary.level);
}

TEST(LogDedupTest, PopExpiredInfoEntrySummaryHasInfoLevel) {
	const uint32_t firstMs = 1000;
	const uint32_t repeatMs = 2000;
	const uint32_t nowMs = repeatMs + LOG_DEDUP_EXPIRY_MS;

	const log_dedup_level exp_level = log_dedup_level::info;

	const char *text = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;
	log_dedup_summary summary;

	bool res;

	dedup.Check(log_dedup_level::info, text, firstMs);
	dedup.Check(log_dedup_level::info, text, repeatMs);

	res = dedup.PopExpired(nowMs, summary);

	ASSERT_TRUE(res);
	EXPECT_EQ(exp_level, summary.level);
}

TEST(LogDedupTest, PopExpiredSameTextBothLevelsGivesTwoSummariesWithOwnLevels) {
	const uint32_t firstMs = 1000;
	const uint32_t repeatMs = 2000;
	const uint32_t nowMs = repeatMs + LOG_DEDUP_EXPIRY_MS;

	const log_dedup_level exp_firstLevel = log_dedup_level::info;
	const log_dedup_level exp_secondLevel = log_dedup_level::error;

	const char *text = "Failed to get PM 2.5 meas";
	HAQuDA_LogDedup dedup;
	log_dedup_summary first;
	log_dedup_summary second;

	bool res;

	dedup.Check(log_dedup_level::info, text, firstMs);
	dedup.Check(log_dedup_level::info, text, repeatMs);
	dedup.Check(log_dedup_level::error, text, firstMs);
	dedup.Check(log_dedup_level::error, text, repeatMs);

	res = dedup.PopExpired(nowMs, first);
	dedup.PopExpired(nowMs, second);

	ASSERT_TRUE(res);
	EXPECT_EQ(exp_firstLevel, first.level);
	EXPECT_EQ(exp_secondLevel, second.level);
}

TEST(LogDedupTest, CheckFullTableEvictedSummaryKeepsErrorLevel) {
	const uint32_t m0Ms = 100;
	const uint32_t m0RepeatMs = 101;
	const uint32_t othersMs = 200;
	const uint32_t newMs = 300;

	const bool exp_hasEvicted = true;
	const log_dedup_level exp_level = log_dedup_level::error;

	HAQuDA_LogDedup dedup;
	const char *newText = "message new";

	log_dedup_result res;

	dedup.Check(log_dedup_level::error, "message 0", m0Ms);
	dedup.Check(log_dedup_level::error, "message 0", m0RepeatMs);
	dedup.Check(log_dedup_level::info, "message 1", othersMs);
	dedup.Check(log_dedup_level::info, "message 2", othersMs);
	dedup.Check(log_dedup_level::info, "message 3", othersMs);
	dedup.Check(log_dedup_level::info, "message 4", othersMs);
	dedup.Check(log_dedup_level::info, "message 5", othersMs);
	dedup.Check(log_dedup_level::info, "message 6", othersMs);
	dedup.Check(log_dedup_level::info, "message 7", othersMs);

	res = dedup.Check(log_dedup_level::info, newText, newMs);

	EXPECT_EQ(exp_hasEvicted, res.hasEvicted);
	EXPECT_EQ(exp_level, res.evicted.level);
}

TEST(LogDedupTest, CheckFullTableEvictedSummaryKeepsInfoLevel) {
	const uint32_t m0Ms = 100;
	const uint32_t m0RepeatMs = 101;
	const uint32_t othersMs = 200;
	const uint32_t newMs = 300;

	const bool exp_hasEvicted = true;
	const log_dedup_level exp_level = log_dedup_level::info;

	HAQuDA_LogDedup dedup;
	const char *newText = "message new";

	log_dedup_result res;

	dedup.Check(log_dedup_level::info, "message 0", m0Ms);
	dedup.Check(log_dedup_level::info, "message 0", m0RepeatMs);
	dedup.Check(log_dedup_level::error, "message 1", othersMs);
	dedup.Check(log_dedup_level::error, "message 2", othersMs);
	dedup.Check(log_dedup_level::error, "message 3", othersMs);
	dedup.Check(log_dedup_level::error, "message 4", othersMs);
	dedup.Check(log_dedup_level::error, "message 5", othersMs);
	dedup.Check(log_dedup_level::error, "message 6", othersMs);
	dedup.Check(log_dedup_level::error, "message 7", othersMs);

	res = dedup.Check(log_dedup_level::error, newText, newMs);

	EXPECT_EQ(exp_hasEvicted, res.hasEvicted);
	EXPECT_EQ(exp_level, res.evicted.level);
}

// ---------------------------------------------------------------------------------------------
// HAQuDA_FormatUptime()
// ---------------------------------------------------------------------------------------------

TEST(LogDedupTest, FormatUptimeTypicalValue) {
	const size_t bufSize = 32;
	const uint32_t ms = 412345;

	const char *exp_text = "up 412.345 s";

	char out[bufSize];

	HAQuDA_FormatUptime(out, bufSize, ms);

	EXPECT_STREQ(exp_text, out);
}

TEST(LogDedupTest, FormatUptimeZero) {
	const size_t bufSize = 32;
	const uint32_t ms = 0;

	const char *exp_text = "up 0.000 s";

	char out[bufSize];

	HAQuDA_FormatUptime(out, bufSize, ms);

	EXPECT_STREQ(exp_text, out);
}

TEST(LogDedupTest, FormatUptimeMillisPartIsZeroPadded) {
	const size_t bufSize = 32;
	const uint32_t ms = 1005;

	const char *exp_text = "up 1.005 s";

	char out[bufSize];

	HAQuDA_FormatUptime(out, bufSize, ms);

	EXPECT_STREQ(exp_text, out);
}

TEST(LogDedupTest, FormatUptimeMaxValueFitsSeventeenBytes) {
	const size_t bufSize = 17; // 16 chars + terminator, the documented maximum
	const uint32_t ms = UINT32_MAX;

	const char *exp_text = "up 4294967.295 s";

	char out[bufSize];

	HAQuDA_FormatUptime(out, bufSize, ms);

	EXPECT_STREQ(exp_text, out);
}

TEST(LogDedupTest, FormatUptimeSmallBufferIsCutAndTerminated) {
	const size_t bufSize = 5;
	const uint32_t ms = 412345;

	const char *exp_text = "up 4";

	char out[bufSize];

	HAQuDA_FormatUptime(out, bufSize, ms);

	EXPECT_STREQ(exp_text, out);
}

TEST(LogDedupTest, FormatUptimeZeroSizeLeavesBufferUntouched) {
	const size_t bufSize = 4;
	const size_t zeroSize = 0;
	const uint32_t ms = 412345;

	const char exp_first = 'x';

	char out[bufSize] = {'x', 'x', 'x', '\0'};

	HAQuDA_FormatUptime(out, zeroSize, ms);

	EXPECT_EQ(exp_first, out[0]);
}

// ---------------------------------------------------------------------------------------------
// HAQuDA_FormatRepeatSummary()
// ---------------------------------------------------------------------------------------------

TEST(LogDedupTest, FormatRepeatSummaryCountComesFirst) {
	const size_t bufSize = LOG_DEDUP_TEXT_SIZE;
	const uint32_t repeats = 150;

	const char *exp_text = "+150 Failed to get PM 2.5 meas";

	const char *text = "Failed to get PM 2.5 meas";
	char out[bufSize];

	HAQuDA_FormatRepeatSummary(out, bufSize, text, repeats);

	EXPECT_STREQ(exp_text, out);
}

TEST(LogDedupTest, FormatRepeatSummaryLongTextIsCutAtTheEnd) {
	const size_t bufSize = LOG_DEDUP_TEXT_SIZE;
	const uint32_t repeats = 150;

	const char *exp_text = "+150 01234567890123456789012345"; // 5 + 26 chars, terminator in the 32nd byte

	const char *text = "0123456789012345678901234567890";
	char out[bufSize];

	HAQuDA_FormatRepeatSummary(out, bufSize, text, repeats);

	EXPECT_STREQ(exp_text, out);
}

TEST(LogDedupTest, FormatRepeatSummaryMaxCountIsNeverCut) {
	const size_t bufSize = LOG_DEDUP_TEXT_SIZE;
	const uint32_t repeats = UINT32_MAX;

	const char *exp_text = "+4294967295 0123456789012345678"; // 12 + 19 chars

	const char *text = "0123456789012345678901234567890";
	char out[bufSize];

	HAQuDA_FormatRepeatSummary(out, bufSize, text, repeats);

	EXPECT_STREQ(exp_text, out);
}

TEST(LogDedupTest, FormatRepeatSummaryVeryLongTextIsTerminatedInsideBuffer) {
	const size_t bufSize = LOG_DEDUP_TEXT_SIZE;
	const uint32_t repeats = 7;

	const size_t exp_len = LOG_DEDUP_TEXT_SIZE - 1;

	const char *text = "a text far longer than the thirty-two byte field can ever keep in one piece";
	char out[bufSize];

	HAQuDA_FormatRepeatSummary(out, bufSize, text, repeats);

	EXPECT_EQ(exp_len, strlen(out));
}

TEST(LogDedupTest, FormatRepeatSummaryNullTextGivesCountOnly) {
	const size_t bufSize = LOG_DEDUP_TEXT_SIZE;
	const uint32_t repeats = 3;

	const char *exp_text = "+3 ";

	const char *text = nullptr;
	char out[bufSize];

	HAQuDA_FormatRepeatSummary(out, bufSize, text, repeats);

	EXPECT_STREQ(exp_text, out);
}

TEST(LogDedupTest, FormatRepeatSummaryZeroSizeLeavesBufferUntouched) {
	const size_t bufSize = 4;
	const size_t zeroSize = 0;
	const uint32_t repeats = 150;

	const char exp_first = 'x';

	const char *text = "Failed to get PM 2.5 meas";
	char out[bufSize] = {'x', 'x', 'x', '\0'};

	HAQuDA_FormatRepeatSummary(out, zeroSize, text, repeats);

	EXPECT_EQ(exp_first, out[0]);
}
