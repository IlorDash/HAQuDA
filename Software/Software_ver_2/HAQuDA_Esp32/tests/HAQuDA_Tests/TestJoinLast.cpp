#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "HAQuDA_JoinLast.h"

// ---------------------------------------------------------------- round trip

TEST(JoinLastTest, RoundTripOkKeepsResultAndName) {
	const join_last_result exp_result = join_last_result::ok;
	const join_reason exp_reason = join_reason::none;

	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::ok, join_reason::none, "Home", JOIN_SSID_BUF_LEN);
	join_view target;
	join_last_record record;

	bool res;

	HAQuDA_EncodeJoinLast(record, source);
	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_result, target.lastResult);
	EXPECT_EQ(exp_reason, target.lastReason);
	EXPECT_STREQ("Home", target.lastSsid);
}

TEST(JoinLastTest, RoundTripFailWrongPassword) {
	const join_last_result exp_result = join_last_result::fail;
	const join_reason exp_reason = join_reason::wrong_password;

	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::fail, join_reason::wrong_password, "Home", JOIN_SSID_BUF_LEN);
	join_view target;
	join_last_record record;

	bool res;

	HAQuDA_EncodeJoinLast(record, source);
	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_result, target.lastResult);
	EXPECT_EQ(exp_reason, target.lastReason);
	EXPECT_STREQ("Home", target.lastSsid);
}

TEST(JoinLastTest, RoundTripFailNoNetwork) {
	const join_last_result exp_result = join_last_result::fail;
	const join_reason exp_reason = join_reason::no_network;

	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::fail, join_reason::no_network, "Home", JOIN_SSID_BUF_LEN);
	join_view target;
	join_last_record record;

	bool res;

	HAQuDA_EncodeJoinLast(record, source);
	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_result, target.lastResult);
	EXPECT_EQ(exp_reason, target.lastReason);
}

TEST(JoinLastTest, RoundTripFailTimeout) {
	const join_last_result exp_result = join_last_result::fail;
	const join_reason exp_reason = join_reason::timeout;

	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::fail, join_reason::timeout, "Home", JOIN_SSID_BUF_LEN);
	join_view target;
	join_last_record record;

	bool res;

	HAQuDA_EncodeJoinLast(record, source);
	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_result, target.lastResult);
	EXPECT_EQ(exp_reason, target.lastReason);
}

TEST(JoinLastTest, RoundTripFailSaveFailed) {
	const join_last_result exp_result = join_last_result::fail;
	const join_reason exp_reason = join_reason::save_failed;

	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::fail, join_reason::save_failed, "Home", JOIN_SSID_BUF_LEN);
	join_view target;
	join_last_record record;

	bool res;

	HAQuDA_EncodeJoinLast(record, source);
	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_result, target.lastResult);
	EXPECT_EQ(exp_reason, target.lastReason);
}

TEST(JoinLastTest, RoundTripFailScanFailed) {
	const join_last_result exp_result = join_last_result::fail;
	const join_reason exp_reason = join_reason::scan_failed;

	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::fail, join_reason::scan_failed, "Home", JOIN_SSID_BUF_LEN);
	join_view target;
	join_last_record record;

	bool res;

	HAQuDA_EncodeJoinLast(record, source);
	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_result, target.lastResult);
	EXPECT_EQ(exp_reason, target.lastReason);
	EXPECT_STREQ("Home", target.lastSsid);
}

TEST(JoinLastTest, RoundTripNameOf31BytesIsKeptWhole) {
	const char exp_ssid[] = "0123456789012345678901234567890";

	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::ok, join_reason::none, "0123456789012345678901234567890", JOIN_SSID_BUF_LEN);
	join_view target;
	join_last_record record;

	bool res;

	HAQuDA_EncodeJoinLast(record, source);
	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_ssid, target.lastSsid);
}

TEST(JoinLastTest, RoundTripNameWithQuoteAndUtf8BytesIsKept) {
	const char exp_ssid[] = "Caf\xC3\xA9 \"5G\"";

	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::ok, join_reason::none, "Caf\xC3\xA9 \"5G\"", JOIN_SSID_BUF_LEN);
	join_view target;
	join_last_record record;

	bool res;

	HAQuDA_EncodeJoinLast(record, source);
	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_ssid, target.lastSsid);
}

TEST(JoinLastTest, DecodeReplacesLongerNameOfThePreviousViewWithoutTail) {
	const char exp_ssid[] = "A";

	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::ok, join_reason::none, "A", JOIN_SSID_BUF_LEN);
	join_view target;
	HAQuDA_SetJoinLast(target, join_last_result::fail, join_reason::timeout, "LongerOldName", JOIN_SSID_BUF_LEN);
	join_last_record record;

	bool res;

	HAQuDA_EncodeJoinLast(record, source);
	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_ssid, target.lastSsid);
	EXPECT_EQ(join_last_result::ok, target.lastResult);
	EXPECT_EQ(join_reason::none, target.lastReason);
}

TEST(JoinLastTest, DecodeDoesNotTouchTheCurrentStateFields) {
	const join_state exp_state = join_state::ok;
	const uint32_t exp_ip = 0x3201A8C0;

	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::ok, join_reason::none, "Home", JOIN_SSID_BUF_LEN);
	join_view target;
	target.state = join_state::ok;
	target.ip = 0x3201A8C0;
	HAQuDA_SetJoinSsid(target, "Current", JOIN_SSID_BUF_LEN);
	join_last_record record;

	bool res;

	HAQuDA_EncodeJoinLast(record, source);
	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_state, target.state);
	EXPECT_EQ(exp_ip, target.ip);
	EXPECT_STREQ("Current", target.ssid);
}

// ---------------------------------------------------------------- encode

TEST(JoinLastTest, EncodeWritesMagicAndZeroReserved) {
	const uint32_t exp_magic = JOIN_LAST_MAGIC;
	const uint16_t exp_reserved = 0;

	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::ok, join_reason::none, "Home", JOIN_SSID_BUF_LEN);
	join_last_record record;
	memset(&record, 0xFF, sizeof(record));

	HAQuDA_EncodeJoinLast(record, source);

	EXPECT_EQ(exp_magic, record.magic);
	EXPECT_EQ(exp_reserved, record.reserved);
}

TEST(JoinLastTest, EncodeWritesResultAndReasonAsNumbers) {
	const uint8_t exp_result = static_cast<uint8_t>(join_last_result::fail);
	const uint8_t exp_reason = static_cast<uint8_t>(join_reason::no_network);

	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::fail, join_reason::no_network, "Home", JOIN_SSID_BUF_LEN);
	join_last_record record;

	HAQuDA_EncodeJoinLast(record, source);

	EXPECT_EQ(exp_result, record.result);
	EXPECT_EQ(exp_reason, record.reason);
}

TEST(JoinLastTest, EncodeLeavesNoTailOfAnEarlierRecordInTheName) {
	const char exp_tail[JOIN_SSID_BUF_LEN - 1] = {0};

	join_view first;
	HAQuDA_SetJoinLast(first, join_last_result::ok, join_reason::none, "LongerOldName", JOIN_SSID_BUF_LEN);
	join_view second;
	HAQuDA_SetJoinLast(second, join_last_result::ok, join_reason::none, "A", JOIN_SSID_BUF_LEN);
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, first);

	HAQuDA_EncodeJoinLast(record, second);

	EXPECT_EQ('A', record.ssid[0]);
	EXPECT_EQ(0, memcmp(exp_tail, &record.ssid[1], sizeof(exp_tail)));
}

TEST(JoinLastTest, EncodeOfViewWithoutLastResultGivesInvalidRecord) {
	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::none, join_reason::none, "Home", JOIN_SSID_BUF_LEN);
	join_view target;
	join_last_record record;
	memset(&record, 0xFF, sizeof(record));

	bool res;

	HAQuDA_EncodeJoinLast(record, source);
	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, EncodeOfViewWithoutLastResultOverwritesAnEarlierValidRecord) {
	join_view first;
	HAQuDA_SetJoinLast(first, join_last_result::ok, join_reason::none, "Home", JOIN_SSID_BUF_LEN);
	join_view none_view;
	join_view target;
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, first);

	bool res;

	HAQuDA_EncodeJoinLast(record, none_view);
	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

// ---------------------------------------------------------------- decode rejects a record that is not ours

TEST(JoinLastTest, DecodeRejectsWrongMagic) {
	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::ok, join_reason::none, "Home", JOIN_SSID_BUF_LEN);
	join_view target;
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, source);
	record.magic = JOIN_LAST_MAGIC + 1;

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, DecodeRejectsFlippedLowByteOfMagic) {
	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::ok, join_reason::none, "Home", JOIN_SSID_BUF_LEN);
	join_view target;
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, source);
	record.magic ^= 0x000000FFu;

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, DecodeRejectsFlippedHighByteOfMagic) {
	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::ok, join_reason::none, "Home", JOIN_SSID_BUF_LEN);
	join_view target;
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, source);
	record.magic ^= 0xFF000000u;

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, DecodeRejectsChangedResultByte) {
	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::ok, join_reason::none, "Home", JOIN_SSID_BUF_LEN);
	join_view target;
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, source);
	record.result = static_cast<uint8_t>(join_last_result::fail);

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, DecodeRejectsChangedReasonByte) {
	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::fail, join_reason::no_network, "Home", JOIN_SSID_BUF_LEN);
	join_view target;
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, source);
	record.reason = static_cast<uint8_t>(join_reason::timeout);

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, DecodeRejectsNonZeroReservedLowByte) {
	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::ok, join_reason::none, "Home", JOIN_SSID_BUF_LEN);
	join_view target;
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, source);
	record.reserved = 0x0001;

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, DecodeRejectsNonZeroReservedHighByte) {
	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::ok, join_reason::none, "Home", JOIN_SSID_BUF_LEN);
	join_view target;
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, source);
	record.reserved = 0x0100;

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, DecodeRejectsChangedFirstByteOfName) {
	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::ok, join_reason::none, "Home", JOIN_SSID_BUF_LEN);
	join_view target;
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, source);
	record.ssid[0] ^= 0x01;

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, DecodeRejectsChangedLastCharacterOfName) {
	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::ok, join_reason::none, "Home", JOIN_SSID_BUF_LEN);
	join_view target;
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, source);
	record.ssid[3] ^= 0x01;

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, DecodeRejectsChangedByteAfterTheTerminator) {
	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::ok, join_reason::none, "Home", JOIN_SSID_BUF_LEN);
	join_view target;
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, source);
	record.ssid[10] = 'x';

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, DecodeRejectsNameWithoutTerminator) {
	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::ok, join_reason::none, "0123456789012345678901234567890", JOIN_SSID_BUF_LEN);
	join_view target;
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, source);
	record.ssid[JOIN_SSID_BUF_LEN - 1] = 'x';

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, DecodeRejectsFlippedLowByteOfChecksum) {
	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::ok, join_reason::none, "Home", JOIN_SSID_BUF_LEN);
	join_view target;
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, source);
	record.checksum ^= 0x000000FFu;

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, DecodeRejectsFlippedHighByteOfChecksum) {
	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::ok, join_reason::none, "Home", JOIN_SSID_BUF_LEN);
	join_view target;
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, source);
	record.checksum ^= 0xFF000000u;

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, DecodeRejectsSingleFlippedBitOfChecksum) {
	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::ok, join_reason::none, "Home", JOIN_SSID_BUF_LEN);
	join_view target;
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, source);
	record.checksum ^= 0x00000001u;

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

// The checksum of the records below is valid (the encoder does not check its input), so only the field checks reject them

TEST(JoinLastTest, DecodeRejectsResultOutOfRangeWithValidChecksum) {
	join_view source;
	memcpy(source.lastSsid, "Home", sizeof("Home"));
	source.lastResult = static_cast<join_last_result>(3);
	source.lastReason = join_reason::none;
	join_view target;
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, source);

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, DecodeRejectsHighestResultByteWithValidChecksum) {
	join_view source;
	memcpy(source.lastSsid, "Home", sizeof("Home"));
	source.lastResult = static_cast<join_last_result>(255);
	source.lastReason = join_reason::timeout;
	join_view target;
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, source);

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, DecodeRejectsReasonOutOfRangeWithValidChecksum) {
	const uint8_t reason_after_last = static_cast<uint8_t>(join_reason::scan_failed) + 1;

	join_view source;
	memcpy(source.lastSsid, "Home", sizeof("Home"));
	source.lastResult = join_last_result::fail;
	source.lastReason = static_cast<join_reason>(reason_after_last);
	join_view target;
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, source);

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, DecodeRejectsHighestReasonByteWithValidChecksum) {
	join_view source;
	memcpy(source.lastSsid, "Home", sizeof("Home"));
	source.lastResult = join_last_result::fail;
	source.lastReason = static_cast<join_reason>(255);
	join_view target;
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, source);

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, DecodeRejectsOkWithAReasonWithValidChecksum) {
	join_view source;
	memcpy(source.lastSsid, "Home", sizeof("Home"));
	source.lastResult = join_last_result::ok;
	source.lastReason = join_reason::timeout;
	join_view target;
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, source);

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, DecodeRejectsFailWithoutAReasonWithValidChecksum) {
	join_view source;
	memcpy(source.lastSsid, "Home", sizeof("Home"));
	source.lastResult = join_last_result::fail;
	source.lastReason = join_reason::none;
	join_view target;
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, source);

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, DecodeRejectsEmptyNameWithValidChecksum) {
	join_view source;
	source.lastResult = join_last_result::ok;
	source.lastReason = join_reason::none;
	join_view target;
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, source);

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, DecodeOfRejectedRecordLeavesTheViewUntouched) {
	const join_last_result exp_result = join_last_result::fail;
	const join_reason exp_reason = join_reason::timeout;

	join_view source;
	source.lastResult = join_last_result::ok;
	source.lastReason = join_reason::timeout;
	memcpy(source.lastSsid, "Home", sizeof("Home"));
	join_view target;
	HAQuDA_SetJoinLast(target, join_last_result::fail, join_reason::timeout, "Keep", JOIN_SSID_BUF_LEN);
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, source);

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
	EXPECT_EQ(exp_result, target.lastResult);
	EXPECT_EQ(exp_reason, target.lastReason);
	EXPECT_STREQ("Keep", target.lastSsid);
}

// ---------------------------------------------------------------- noise in RTC memory

TEST(JoinLastTest, DecodeRejectsAllZeroRecord) {
	join_view target;
	join_last_record record;
	memset(&record, 0x00, sizeof(record));

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, DecodeRejectsAllOnesRecord) {
	join_view target;
	join_last_record record;
	memset(&record, 0xFF, sizeof(record));

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, DecodeRejectsAlternatingBitsRecord) {
	join_view target;
	join_last_record record;
	memset(&record, 0xA5, sizeof(record));

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, DecodeRejectsRecordWithRightMagicAndZeroRest) {
	join_view target;
	join_last_record record;
	memset(&record, 0x00, sizeof(record));
	record.magic = JOIN_LAST_MAGIC;

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, DecodeRejectsPseudoRandomBytes) {
	const uint8_t noise[sizeof(join_last_record)]
		= {0x3C, 0x91, 0xE7, 0x08, 0x5A, 0xD2, 0x14, 0xB6, 0x7F, 0x20, 0xC9, 0x41, 0x88, 0x03, 0xFA, 0x6D, 0x95, 0x1B, 0xE0, 0x72, 0x2E, 0xC4,
		   0x59, 0x0A, 0xB3, 0xDD, 0x67, 0x38, 0x81, 0xF5, 0x4C, 0x16, 0xA9, 0x72, 0x0E, 0xD8, 0x53, 0xBB, 0x24, 0x9F, 0x61, 0xC7, 0x30, 0x8D};
	join_view target;
	join_last_record record;
	memcpy(&record, noise, sizeof(record));

	bool res;

	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, ClearedRecordIsRejected) {
	join_view source;
	HAQuDA_SetJoinLast(source, join_last_result::ok, join_reason::none, "Home", JOIN_SSID_BUF_LEN);
	join_view target;
	join_last_record record;
	HAQuDA_EncodeJoinLast(record, source);

	bool res;

	HAQuDA_ClearJoinLastRecord(record);
	res = HAQuDA_DecodeJoinLast(record, target);

	EXPECT_FALSE(res);
}

TEST(JoinLastTest, ClearZeroesEveryByteOfTheRecord) {
	const uint8_t exp_bytes[sizeof(join_last_record)] = {0};

	join_last_record record;
	memset(&record, 0xFF, sizeof(record));

	HAQuDA_ClearJoinLastRecord(record);

	EXPECT_EQ(0, memcmp(exp_bytes, &record, sizeof(record)));
}
