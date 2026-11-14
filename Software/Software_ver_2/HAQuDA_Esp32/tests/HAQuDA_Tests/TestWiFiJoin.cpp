#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>

#include "HAQuDA_WiFiJoin.h"

// Disconnect reason codes below are the wifi_err_reason_t values of ESP-IDF 4.4 (esp_wifi_types.h):
// AUTH_EXPIRE 2, MIC_FAILURE 14, 4WAY_HANDSHAKE_TIMEOUT 15, 802_1X_AUTH_FAILED 23, NO_AP_FOUND 201,
// AUTH_FAIL 202, HANDSHAKE_TIMEOUT 204.

// ---------------------------------------------------------------- HAQuDA_MapDisconnectReason

TEST(WiFiJoinTest, MapDisconnectReasonAuthFailIsWrongPassword) {
	const uint8_t code_auth_fail = 202;

	const join_reason exp_reason = join_reason::wrong_password;

	join_reason res;

	res = HAQuDA_MapDisconnectReason(code_auth_fail);

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, MapDisconnectReason4WayHandshakeTimeoutIsWrongPassword) {
	const uint8_t code_4way_handshake_timeout = 15;

	const join_reason exp_reason = join_reason::wrong_password;

	join_reason res;

	res = HAQuDA_MapDisconnectReason(code_4way_handshake_timeout);

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, MapDisconnectReasonHandshakeTimeoutIsWrongPassword) {
	const uint8_t code_handshake_timeout = 204;

	const join_reason exp_reason = join_reason::wrong_password;

	join_reason res;

	res = HAQuDA_MapDisconnectReason(code_handshake_timeout);

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, MapDisconnectReason8021xAuthFailedIsWrongPassword) {
	const uint8_t code_8021x_auth_failed = 23;

	const join_reason exp_reason = join_reason::wrong_password;

	join_reason res;

	res = HAQuDA_MapDisconnectReason(code_8021x_auth_failed);

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, MapDisconnectReasonMicFailureIsWrongPassword) {
	const uint8_t code_mic_failure = 14;

	const join_reason exp_reason = join_reason::wrong_password;

	join_reason res;

	res = HAQuDA_MapDisconnectReason(code_mic_failure);

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, MapDisconnectReasonNoApFoundIsNoNetwork) {
	const uint8_t code_no_ap_found = 201;

	const join_reason exp_reason = join_reason::no_network;

	join_reason res;

	res = HAQuDA_MapDisconnectReason(code_no_ap_found);

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, MapDisconnectReasonAuthExpireIsTimeout) {
	const uint8_t code_auth_expire = 2;

	const join_reason exp_reason = join_reason::timeout;

	join_reason res;

	res = HAQuDA_MapDisconnectReason(code_auth_expire);

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, MapDisconnectReasonZeroIsTimeout) {
	const uint8_t code_none = 0;

	const join_reason exp_reason = join_reason::timeout;

	join_reason res;

	res = HAQuDA_MapDisconnectReason(code_none);

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, MapDisconnectReasonUnknownCodeIsTimeout) {
	const uint8_t code_unknown = 123;

	const join_reason exp_reason = join_reason::timeout;

	join_reason res;

	res = HAQuDA_MapDisconnectReason(code_unknown);

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, MapDisconnectReasonHighestCodeIsTimeout) {
	const uint8_t code_max = 255;

	const join_reason exp_reason = join_reason::timeout;

	join_reason res;

	res = HAQuDA_MapDisconnectReason(code_max);

	EXPECT_EQ(exp_reason, res);
}

// ---------------------------------------------------------------- HAQuDA_MergeDisconnectReason

TEST(WiFiJoinTest, MergeDisconnectReasonFirstCodeIsKept) {
	const uint8_t code_none = 0;
	const uint8_t code_auth_fail = 202;

	const uint8_t exp_code = 202;

	uint8_t res;

	res = HAQuDA_MergeDisconnectReason(code_none, code_auth_fail);

	EXPECT_EQ(exp_code, res);
}

TEST(WiFiJoinTest, MergeDisconnectReasonWrongPasswordBeatsNoNetwork) {
	const uint8_t code_no_ap_found = 201;
	const uint8_t code_auth_fail = 202;

	const uint8_t exp_code = 202;

	uint8_t res;

	res = HAQuDA_MergeDisconnectReason(code_no_ap_found, code_auth_fail);

	EXPECT_EQ(exp_code, res);
}

TEST(WiFiJoinTest, MergeDisconnectReasonLateNoApFoundKeepsWrongPassword) {
	const uint8_t code_auth_fail = 202;
	const uint8_t code_no_ap_found = 201;

	const uint8_t exp_code = 202;

	uint8_t res;

	res = HAQuDA_MergeDisconnectReason(code_auth_fail, code_no_ap_found);

	EXPECT_EQ(exp_code, res);
}

TEST(WiFiJoinTest, MergeDisconnectReasonLateNoApFoundKeepsHandshakeTimeout) {
	const uint8_t code_4way_handshake_timeout = 15;
	const uint8_t code_no_ap_found = 201;

	const uint8_t exp_code = 15;

	uint8_t res;

	res = HAQuDA_MergeDisconnectReason(code_4way_handshake_timeout, code_no_ap_found);

	EXPECT_EQ(exp_code, res);
}

TEST(WiFiJoinTest, MergeDisconnectReasonNoNetworkBeatsOthers) {
	const uint8_t code_auth_expire = 2;
	const uint8_t code_no_ap_found = 201;

	const uint8_t exp_code = 201;

	uint8_t res;

	res = HAQuDA_MergeDisconnectReason(code_auth_expire, code_no_ap_found);

	EXPECT_EQ(exp_code, res);
}

TEST(WiFiJoinTest, MergeDisconnectReasonSameRankKeepsNewerWrongPassword) {
	const uint8_t code_auth_fail = 202;
	const uint8_t code_4way_handshake_timeout = 15;

	const uint8_t exp_code = 15;

	uint8_t res;

	res = HAQuDA_MergeDisconnectReason(code_auth_fail, code_4way_handshake_timeout);

	EXPECT_EQ(exp_code, res);
}

TEST(WiFiJoinTest, MergeDisconnectReasonSameCodeIsKept) {
	const uint8_t code_no_ap_found = 201;

	const uint8_t exp_code = 201;

	uint8_t res;

	res = HAQuDA_MergeDisconnectReason(code_no_ap_found, code_no_ap_found);

	EXPECT_EQ(exp_code, res);
}

TEST(WiFiJoinTest, MergeDisconnectReasonUninformativeNeverReplacesWrongPassword) {
	const uint8_t code_auth_fail = 202;
	const uint8_t code_auth_expire = 2;

	const uint8_t exp_code = 202;

	uint8_t res;

	res = HAQuDA_MergeDisconnectReason(code_auth_fail, code_auth_expire);

	EXPECT_EQ(exp_code, res);
}

TEST(WiFiJoinTest, MergeDisconnectReasonUninformativeNeverReplacesNoNetwork) {
	const uint8_t code_no_ap_found = 201;
	const uint8_t code_unknown = 123;

	const uint8_t exp_code = 201;

	uint8_t res;

	res = HAQuDA_MergeDisconnectReason(code_no_ap_found, code_unknown);

	EXPECT_EQ(exp_code, res);
}

TEST(WiFiJoinTest, MergeDisconnectReasonZeroNeverReplacesRankedCode) {
	const uint8_t code_auth_fail = 202;
	const uint8_t code_none = 0;

	const uint8_t exp_code = 202;

	uint8_t res;

	res = HAQuDA_MergeDisconnectReason(code_auth_fail, code_none);

	EXPECT_EQ(exp_code, res);
}

TEST(WiFiJoinTest, MergeDisconnectReasonTwoUninformativeCodesGiveZero) {
	const uint8_t code_auth_expire = 2;
	const uint8_t code_unknown = 123;

	const uint8_t exp_code = 0;

	uint8_t res;

	res = HAQuDA_MergeDisconnectReason(code_auth_expire, code_unknown);

	EXPECT_EQ(exp_code, res);
}

TEST(WiFiJoinTest, MergeDisconnectReasonNothingKeptAndNothingNewGivesZero) {
	const uint8_t code_none = 0;

	const uint8_t exp_code = 0;

	uint8_t res;

	res = HAQuDA_MergeDisconnectReason(code_none, code_none);

	EXPECT_EQ(exp_code, res);
}

// ---------------------------------------------------------------- names

TEST(WiFiJoinTest, JoinStateNameIdle) {
	const char *exp_name = "idle";

	const char *res;

	res = HAQuDA_JoinStateName(join_state::idle);

	EXPECT_STREQ(exp_name, res);
}

TEST(WiFiJoinTest, JoinStateNameChecking) {
	const char *exp_name = "checking";

	const char *res;

	res = HAQuDA_JoinStateName(join_state::checking);

	EXPECT_STREQ(exp_name, res);
}

TEST(WiFiJoinTest, JoinStateNameOk) {
	const char *exp_name = "ok";

	const char *res;

	res = HAQuDA_JoinStateName(join_state::ok);

	EXPECT_STREQ(exp_name, res);
}

TEST(WiFiJoinTest, JoinStateNameFail) {
	const char *exp_name = "fail";

	const char *res;

	res = HAQuDA_JoinStateName(join_state::fail);

	EXPECT_STREQ(exp_name, res);
}

TEST(WiFiJoinTest, JoinReasonNameNoneIsEmpty) {
	const char *exp_name = "";

	const char *res;

	res = HAQuDA_JoinReasonName(join_reason::none);

	EXPECT_STREQ(exp_name, res);
}

TEST(WiFiJoinTest, JoinReasonNameWrongPassword) {
	const char *exp_name = "wrong_password";

	const char *res;

	res = HAQuDA_JoinReasonName(join_reason::wrong_password);

	EXPECT_STREQ(exp_name, res);
}

TEST(WiFiJoinTest, JoinReasonNameNoNetwork) {
	const char *exp_name = "no_network";

	const char *res;

	res = HAQuDA_JoinReasonName(join_reason::no_network);

	EXPECT_STREQ(exp_name, res);
}

TEST(WiFiJoinTest, JoinReasonNameTimeout) {
	const char *exp_name = "timeout";

	const char *res;

	res = HAQuDA_JoinReasonName(join_reason::timeout);

	EXPECT_STREQ(exp_name, res);
}

TEST(WiFiJoinTest, JoinReasonNameSaveFailed) {
	const char *exp_name = "save_failed";

	const char *res;

	res = HAQuDA_JoinReasonName(join_reason::save_failed);

	EXPECT_STREQ(exp_name, res);
}

// ---------------------------------------------------------------- HAQuDA_SetJoinSsid

TEST(WiFiJoinTest, SetJoinSsidCopiesShortName) {
	const char *exp_ssid = "Home";

	const char *ssid = "Home";
	join_view view;

	HAQuDA_SetJoinSsid(view, ssid, JOIN_SSID_BUF_LEN);

	EXPECT_STREQ(exp_ssid, view.ssid);
}

TEST(WiFiJoinTest, SetJoinSsidTruncatesLongNameToBufferMinusTerminator) {
	const char *exp_ssid = "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"; // 31 characters

	const char *ssid = "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"; // 40 characters
	join_view view;

	HAQuDA_SetJoinSsid(view, ssid, 40);

	EXPECT_STREQ(exp_ssid, view.ssid);
	EXPECT_EQ(JOIN_SSID_BUF_LEN - 1, strlen(view.ssid));
}

TEST(WiFiJoinTest, SetJoinSsidStopsAtMaxLenOfUnterminatedText) {
	const char *exp_ssid = "Hom";

	const char ssid[4] = {'H', 'o', 'm', 'e'}; // no terminator
	const size_t max_len = 3;
	join_view view;

	HAQuDA_SetJoinSsid(view, ssid, max_len);

	EXPECT_STREQ(exp_ssid, view.ssid);
}

TEST(WiFiJoinTest, SetJoinSsidNullClearsPreviousName) {
	const char *exp_ssid = "";

	join_view view;
	HAQuDA_SetJoinSsid(view, "Old", JOIN_SSID_BUF_LEN);

	HAQuDA_SetJoinSsid(view, nullptr, JOIN_SSID_BUF_LEN);

	EXPECT_STREQ(exp_ssid, view.ssid);
}

TEST(WiFiJoinTest, SetJoinSsidShorterNameLeavesNoTailOfPreviousName) {
	const char *exp_ssid = "Ab";

	const char *ssid = "Ab";
	join_view view;
	HAQuDA_SetJoinSsid(view, "LongerOldName", JOIN_SSID_BUF_LEN);

	HAQuDA_SetJoinSsid(view, ssid, JOIN_SSID_BUF_LEN);

	EXPECT_STREQ(exp_ssid, view.ssid);
	EXPECT_EQ('\0', view.ssid[3]);
}

// ---------------------------------------------------------------- HAQuDA_EscapeJson

TEST(WiFiJoinTest, EscapeJsonPlainTextIsUnchanged) {
	const char *exp_text = "Home WiFi 5G";

	const char *text = "Home WiFi 5G";
	char out[64];

	HAQuDA_EscapeJson(text, 64, out, sizeof(out));

	EXPECT_STREQ(exp_text, out);
}

TEST(WiFiJoinTest, EscapeJsonEmptyTextGivesEmptyOutput) {
	const char *exp_text = "";

	const char *text = "";
	char out[8];
	out[0] = 'x';

	HAQuDA_EscapeJson(text, 8, out, sizeof(out));

	EXPECT_STREQ(exp_text, out);
}

TEST(WiFiJoinTest, EscapeJsonNullTextGivesEmptyOutput) {
	const char *exp_text = "";

	char out[8];
	out[0] = 'x';

	HAQuDA_EscapeJson(nullptr, 8, out, sizeof(out));

	EXPECT_STREQ(exp_text, out);
}

TEST(WiFiJoinTest, EscapeJsonQuoteGetsBackslash) {
	const char *exp_text = "a\\\"b"; // a\"b

	const char *text = "a\"b";
	char out[16];

	HAQuDA_EscapeJson(text, 16, out, sizeof(out));

	EXPECT_STREQ(exp_text, out);
}

TEST(WiFiJoinTest, EscapeJsonBackslashIsDoubled) {
	const char *exp_text = "a\\\\b"; // a\\b

	const char *text = "a\\b";
	char out[16];

	HAQuDA_EscapeJson(text, 16, out, sizeof(out));

	EXPECT_STREQ(exp_text, out);
}

TEST(WiFiJoinTest, EscapeJsonNewlineBecomesUnicodeEscape) {
	const char *exp_text = "a\\u000ab"; // a\u000ab

	const char *text = "a\nb";
	char out[16];

	HAQuDA_EscapeJson(text, 16, out, sizeof(out));

	EXPECT_STREQ(exp_text, out);
}

TEST(WiFiJoinTest, EscapeJsonLastControlCharacterBecomesUnicodeEscape) {
	const char *exp_text = "\\u001f";

	const char *text = "\x1f";
	char out[16];

	HAQuDA_EscapeJson(text, 16, out, sizeof(out));

	EXPECT_STREQ(exp_text, out);
}

TEST(WiFiJoinTest, EscapeJsonSpaceIsNotEscaped) {
	const char *exp_text = " ";

	const char *text = " ";
	char out[16];

	HAQuDA_EscapeJson(text, 16, out, sizeof(out));

	EXPECT_STREQ(exp_text, out);
}

TEST(WiFiJoinTest, EscapeJsonDeleteCharacterPassesThrough) {
	const char *exp_text = "\x7f";

	const char *text = "\x7f";
	char out[16];

	HAQuDA_EscapeJson(text, 16, out, sizeof(out));

	EXPECT_STREQ(exp_text, out);
}

TEST(WiFiJoinTest, EscapeJsonUtf8BytesPassThrough) {
	const char *exp_text = "Caf\xC3\xA9 \xD0\x94\xD0\xBE\xD0\xBC"; // "Cafe" with acute e, Cyrillic "Dom"

	const char *text = "Caf\xC3\xA9 \xD0\x94\xD0\xBE\xD0\xBC";
	char out[32];

	HAQuDA_EscapeJson(text, 32, out, sizeof(out));

	EXPECT_STREQ(exp_text, out);
}

TEST(WiFiJoinTest, EscapeJsonStopsAtMaxLenOfUnterminatedText) {
	const char *exp_text = "abc";

	const char text[6] = {'a', 'b', 'c', 'd', 'e', 'f'}; // no terminator
	const size_t max_len = 3;
	char out[16];

	HAQuDA_EscapeJson(text, max_len, out, sizeof(out));

	EXPECT_STREQ(exp_text, out);
}

TEST(WiFiJoinTest, EscapeJsonFullOutputIsTerminated) {
	const char *exp_text = "abc";

	const char *text = "abcdef";
	char out[4]; // room for 3 characters and the terminator

	HAQuDA_EscapeJson(text, 6, out, sizeof(out));

	EXPECT_STREQ(exp_text, out);
}

TEST(WiFiJoinTest, EscapeJsonEscapeThatDoesNotFitIsDroppedWhole) {
	const char *exp_text = "a";

	const char *text = "a\"b";
	char out[3]; // "a" + terminator leaves 1 byte, the escape needs 2 plus terminator

	HAQuDA_EscapeJson(text, 3, out, sizeof(out));

	EXPECT_STREQ(exp_text, out);
}

TEST(WiFiJoinTest, EscapeJsonOutputOfSizeOneIsEmpty) {
	const char *exp_text = "";

	const char *text = "abc";
	char out[1];
	out[0] = 'x';

	HAQuDA_EscapeJson(text, 3, out, sizeof(out));

	EXPECT_STREQ(exp_text, out);
}

TEST(WiFiJoinTest, EscapeJsonOutputOfSizeZeroIsNotTouched) {
	const char exp_char = 'x';

	const char *text = "abc";
	char out[1];
	out[0] = 'x';

	HAQuDA_EscapeJson(text, 3, out, 0);

	EXPECT_EQ(exp_char, out[0]);
}

TEST(WiFiJoinTest, EscapeJsonNullOutputDoesNotCrash) {
	const char *text = "abc";

	HAQuDA_EscapeJson(text, 3, nullptr, 8);

	SUCCEED();
}

// ---------------------------------------------------------------- HAQuDA_BuildJoinJson

TEST(WiFiJoinTest, BuildJoinJsonIdleHasStateSsidAndHostOnly) {
	const char exp_json[] = "{\"state\":\"idle\",\"ssid\":\"\",\"host\":\"haquda.local\"}";

	const char *host = "haquda.local";
	join_view view;
	char buf[128];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonCheckingHasNoIpAndNoReason) {
	const char exp_json[] = "{\"state\":\"checking\",\"ssid\":\"Home\",\"host\":\"haquda.local\"}";

	const char *host = "haquda.local";
	join_view view;
	view.state = join_state::checking;
	view.reason = join_reason::wrong_password; // stale value, must not be shown
	view.ip = 0x3201A8C0;					   // stale value, must not be shown
	HAQuDA_SetJoinSsid(view, "Home", JOIN_SSID_BUF_LEN);
	char buf[128];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonOkHasIpFirstOctetInLowestByte) {
	const char exp_json[] = "{\"state\":\"ok\",\"ssid\":\"Home\",\"ip\":\"192.168.1.50\",\"restart_ms\":0,\"host\":\"haquda.local\"}";

	const char *host = "haquda.local";
	join_view view;
	view.state = join_state::ok;
	view.ip = 192u | (168u << 8) | (1u << 16) | (50u << 24);
	HAQuDA_SetJoinSsid(view, "Home", JOIN_SSID_BUF_LEN);
	char buf[128];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonOkHasNoReason) {
	const char exp_json[] = "{\"state\":\"ok\",\"ssid\":\"Home\",\"ip\":\"10.0.0.7\",\"restart_ms\":0,\"host\":\"haquda.local\"}";

	const char *host = "haquda.local";
	join_view view;
	view.state = join_state::ok;
	view.reason = join_reason::timeout; // stale value, must not be shown
	view.ip = 10u | (0u << 8) | (0u << 16) | (7u << 24);
	HAQuDA_SetJoinSsid(view, "Home", JOIN_SSID_BUF_LEN);
	char buf[128];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonFailHasReasonAndNoIp) {
	const char exp_json[] = "{\"state\":\"fail\",\"ssid\":\"Home\",\"host\":\"haquda.local\",\"reason\":\"wrong_password\"}";

	const char *host = "haquda.local";
	join_view view;
	view.state = join_state::fail;
	view.reason = join_reason::wrong_password;
	view.ip = 0x3201A8C0; // stale value, must not be shown
	HAQuDA_SetJoinSsid(view, "Home", JOIN_SSID_BUF_LEN);
	char buf[160];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonFailSaveFailedReason) {
	const char exp_json[] = "{\"state\":\"fail\",\"ssid\":\"Home\",\"host\":\"haquda.local\",\"reason\":\"save_failed\"}";

	const char *host = "haquda.local";
	join_view view;
	view.state = join_state::fail;
	view.reason = join_reason::save_failed;
	HAQuDA_SetJoinSsid(view, "Home", JOIN_SSID_BUF_LEN);
	char buf[160];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonHostIsWrittenAsItIs) {
	const char exp_json[] = "{\"state\":\"idle\",\"ssid\":\"\",\"host\":\"192.168.4.1\"}";

	const char *host = "192.168.4.1";
	join_view view;
	char buf[128];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonSsidQuoteAndBackslashAreEscaped) {
	const char exp_json[] = "{\"state\":\"idle\",\"ssid\":\"a\\\"b\\\\c\",\"host\":\"h\"}"; // ssid a\"b\\c

	const char *host = "h";
	join_view view;
	HAQuDA_SetJoinSsid(view, "a\"b\\c", JOIN_SSID_BUF_LEN);
	char buf[128];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonSsidControlCharacterIsEscaped) {
	const char exp_json[] = "{\"state\":\"idle\",\"ssid\":\"a\\u0009b\",\"host\":\"h\"}"; // ssid a\u0009b

	const char *host = "h";
	join_view view;
	HAQuDA_SetJoinSsid(view, "a\tb", JOIN_SSID_BUF_LEN);
	char buf[128];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonSsidUtf8PassesThrough) {
	const char exp_json[] = "{\"state\":\"idle\",\"ssid\":\"Caf\xC3\xA9\",\"host\":\"h\"}";

	const char *host = "h";
	join_view view;
	HAQuDA_SetJoinSsid(view, "Caf\xC3\xA9", JOIN_SSID_BUF_LEN);
	char buf[128];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonMaxLengthPlainSsidFits) {
	const char exp_json[] = "{\"state\":\"idle\",\"ssid\":\"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\",\"host\":\"h\"}"; // 31 A

	const char *host = "h";
	join_view view;
	HAQuDA_SetJoinSsid(view, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", 40);
	char buf[256];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonMaxLengthSsidOfControlCharactersIsNotCut) {
	const char prefix[] = "{\"state\":\"idle\",\"ssid\":\"";
	const char suffix[] = "\",\"host\":\"h\"}";
	const size_t ssid_chars = JOIN_SSID_BUF_LEN - 1;
	const size_t escape_len = 6; // \u0001

	const size_t exp_len = (sizeof(prefix) - 1) + ssid_chars * escape_len + (sizeof(suffix) - 1);

	const char *host = "h";
	join_view view;
	memset(view.ssid, 0x01, ssid_chars); // the last byte stays the terminator
	char buf[512];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_len, strlen(buf));
	EXPECT_EQ('}', buf[exp_len - 1]);
}

TEST(WiFiJoinTest, BuildJoinJsonMaxLengthSsidOfQuotesIsNotCut) {
	const char prefix[] = "{\"state\":\"idle\",\"ssid\":\"";
	const char suffix[] = "\",\"host\":\"h\"}";
	const size_t ssid_chars = JOIN_SSID_BUF_LEN - 1;
	const size_t escape_len = 2; // \"

	const size_t exp_len = (sizeof(prefix) - 1) + ssid_chars * escape_len + (sizeof(suffix) - 1);

	const char *host = "h";
	join_view view;
	memset(view.ssid, '"', ssid_chars);
	char buf[256];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_len, strlen(buf));
}

TEST(WiFiJoinTest, BuildJoinJsonBufferOfExactSizeFits) {
	const char exp_json[] = "{\"state\":\"idle\",\"ssid\":\"\",\"host\":\"h\"}";

	const char *host = "h";
	join_view view;
	char buf[sizeof(exp_json)]; // text plus terminator

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonBufferOneByteTooSmallReturnsFalse) {
	const char exp_json[] = "{\"state\":\"idle\",\"ssid\":\"\",\"host\":\"h\"}";

	const char *host = "h";
	join_view view;
	char buf[sizeof(exp_json) - 1]; // no room for the terminator
	memset(buf, 'x', sizeof(buf));

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_FALSE(res);
	EXPECT_EQ('\0', buf[0]);
}

TEST(WiFiJoinTest, BuildJoinJsonBufferCutInTheMiddleOfIpReturnsFalseAndEmpty) {
	const char *host = "haquda.local";
	join_view view;
	view.state = join_state::ok;
	view.ip = 192u | (168u << 8) | (1u << 16) | (50u << 24);
	HAQuDA_SetJoinSsid(view, "Home", JOIN_SSID_BUF_LEN);
	char buf[40]; // the text is longer
	memset(buf, 'x', sizeof(buf));

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_FALSE(res);
	EXPECT_EQ('\0', buf[0]);
}

TEST(WiFiJoinTest, BuildJoinJsonBufferOfSizeOneReturnsFalseAndEmpty) {
	const char *host = "h";
	join_view view;
	char buf[1];
	buf[0] = 'x';

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_FALSE(res);
	EXPECT_EQ('\0', buf[0]);
}

TEST(WiFiJoinTest, BuildJoinJsonBufferOfSizeZeroReturnsFalse) {
	const char *host = "h";
	join_view view;
	char buf[1];
	buf[0] = 'x';

	bool res;

	res = HAQuDA_BuildJoinJson(buf, 0, view, host);

	EXPECT_FALSE(res);
	EXPECT_EQ('x', buf[0]);
}

TEST(WiFiJoinTest, BuildJoinJsonNullHostReturnsFalseAndEmpty) {
	join_view view;
	char buf[128];
	memset(buf, 'x', sizeof(buf));

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, nullptr);

	EXPECT_FALSE(res);
	EXPECT_EQ('\0', buf[0]);
}

TEST(WiFiJoinTest, BuildJoinJsonNullBufferReturnsFalse) {
	const char *host = "h";
	join_view view;

	bool res;

	res = HAQuDA_BuildJoinJson(nullptr, 128, view, host);

	EXPECT_FALSE(res);
}

// ---------------------------------------------------------------- HAQuDA_JoinCheck

TEST(WiFiJoinTest, JoinCheckNewIsIdle) {
	const join_state exp_state = join_state::idle;

	const HAQuDA_JoinCheck check;

	join_state res;

	res = check.State();

	EXPECT_EQ(exp_state, res);
	EXPECT_EQ(join_reason::none, check.Reason());
	EXPECT_EQ(0u, check.Ip());
}

TEST(WiFiJoinTest, JoinCheckStartReturnsTrue) {
	const uint32_t start_ms = 1000;

	HAQuDA_JoinCheck check;

	bool res;

	res = check.Start(start_ms);

	EXPECT_TRUE(res);
}

TEST(WiFiJoinTest, JoinCheckStartMovesToChecking) {
	const join_state exp_state = join_state::checking;

	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;

	check.Start(start_ms);

	EXPECT_EQ(exp_state, check.State());
	EXPECT_EQ(join_reason::none, check.Reason());
	EXPECT_EQ(0u, check.Ip());
}

TEST(WiFiJoinTest, JoinCheckStartWhileCheckingReturnsFalse) {
	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	bool res;

	res = check.Start(start_ms + 5000);

	EXPECT_FALSE(res);
}

TEST(WiFiJoinTest, JoinCheckStartWhileCheckingKeepsFirstStartTime) {
	const join_action exp_action = join_action::leave_station;

	const uint32_t start_ms = 1000;
	const uint32_t second_start_ms = start_ms + 5000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Start(second_start_ms);

	join_action res;

	res = check.Poll(start_ms + JOIN_TIMEOUT_MS, false, no_ip, no_reason);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiJoinTest, JoinCheckStartWhileSavingReturnsFalse) {
	const uint32_t start_ms = 1000;
	const uint32_t ip = 0x0100A8C0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + 10, true, ip, no_reason); // save_network, waiting for SaveFinished

	bool res;

	res = check.Start(start_ms + 20);

	EXPECT_FALSE(res);
}

TEST(WiFiJoinTest, JoinCheckPollBeforeStartReturnsNone) {
	const join_action exp_action = join_action::none;

	const uint32_t ip = 0x0100A8C0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;

	join_action res;

	res = check.Poll(JOIN_TIMEOUT_MS * 2, true, ip, no_reason);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(join_state::idle, check.State());
}

TEST(WiFiJoinTest, JoinCheckPollConnectedWithIpReturnsSaveNetwork) {
	const join_action exp_action = join_action::save_network;

	const uint32_t start_ms = 1000;
	const uint32_t ip = 0x0100A8C0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	join_action res;

	res = check.Poll(start_ms + 3000, true, ip, no_reason);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiJoinTest, JoinCheckPollConnectedKeepsCheckingUntilSaveFinished) {
	const join_state exp_state = join_state::checking;

	const uint32_t start_ms = 1000;
	const uint32_t ip = 0x0100A8C0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	check.Poll(start_ms + 3000, true, ip, no_reason);

	EXPECT_EQ(exp_state, check.State());
	EXPECT_EQ(ip, check.Ip());
}

TEST(WiFiJoinTest, JoinCheckPollSaveNetworkIsReturnedOnce) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t ip = 0x0100A8C0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + 3000, true, ip, no_reason);

	join_action res;

	res = check.Poll(start_ms + 3010, true, ip, no_reason);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiJoinTest, JoinCheckPollWhileSavingIgnoresTimeout) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t ip = 0x0100A8C0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + 3000, true, ip, no_reason);

	join_action res;

	res = check.Poll(start_ms + JOIN_TIMEOUT_MS * 2, false, 0, no_reason);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(join_state::checking, check.State());
}

TEST(WiFiJoinTest, JoinCheckPollConnectedWithZeroIpKeepsChecking) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t zero_ip = 0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	join_action res;

	res = check.Poll(start_ms + 3000, true, zero_ip, no_reason);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(join_state::checking, check.State());
	EXPECT_EQ(0u, check.Ip());
}

TEST(WiFiJoinTest, JoinCheckPollNotConnectedWithIpKeepsChecking) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t ip = 0x0100A8C0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	join_action res;

	res = check.Poll(start_ms + 3000, false, ip, no_reason);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(join_state::checking, check.State());
}

TEST(WiFiJoinTest, JoinCheckSaveFinishedTrueGivesOkWithIp) {
	const join_state exp_state = join_state::ok;

	const uint32_t start_ms = 1000;
	const uint32_t ip = 0x0100A8C0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + 3000, true, ip, no_reason);

	join_action res;

	res = check.SaveFinished(true);

	EXPECT_EQ(join_action::none, res);
	EXPECT_EQ(exp_state, check.State());
	EXPECT_EQ(ip, check.Ip());
	EXPECT_EQ(join_reason::none, check.Reason());
}

TEST(WiFiJoinTest, JoinCheckSaveFinishedFalseGivesFailSaveFailed) {
	const join_state exp_state = join_state::fail;
	const join_reason exp_reason = join_reason::save_failed;

	const uint32_t start_ms = 1000;
	const uint32_t ip = 0x0100A8C0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + 3000, true, ip, no_reason);

	join_action res;

	res = check.SaveFinished(false);

	EXPECT_EQ(join_action::leave_station, res);
	EXPECT_EQ(exp_state, check.State());
	EXPECT_EQ(exp_reason, check.Reason());
}

TEST(WiFiJoinTest, JoinCheckSaveFinishedFalseClearsIp) {
	const uint32_t exp_ip = 0;

	const uint32_t start_ms = 1000;
	const uint32_t ip = 0x0100A8C0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + 3000, true, ip, no_reason);

	check.SaveFinished(false);

	EXPECT_EQ(exp_ip, check.Ip());
}

TEST(WiFiJoinTest, JoinCheckSaveFinishedWithoutSaveRequestReturnsNone) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.Start(start_ms); // still joining, no save requested

	join_action res;

	res = check.SaveFinished(false);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(join_state::checking, check.State());
}

TEST(WiFiJoinTest, JoinCheckSaveFinishedWhenIdleReturnsNone) {
	const join_action exp_action = join_action::none;

	HAQuDA_JoinCheck check;

	join_action res;

	res = check.SaveFinished(true);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(join_state::idle, check.State());
}

TEST(WiFiJoinTest, JoinCheckSaveFinishedAfterOkDoesNotChangeResult) {
	const join_state exp_state = join_state::ok;

	const uint32_t start_ms = 1000;
	const uint32_t ip = 0x0100A8C0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + 3000, true, ip, no_reason);
	check.SaveFinished(true);

	join_action res;

	res = check.SaveFinished(false);

	EXPECT_EQ(join_action::none, res);
	EXPECT_EQ(exp_state, check.State());
}

TEST(WiFiJoinTest, JoinCheckPollJustBeforeTimeoutReturnsNone) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 5000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	join_action res;

	res = check.Poll(start_ms + JOIN_TIMEOUT_MS - 1, false, no_ip, no_reason);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(join_state::checking, check.State());
}

TEST(WiFiJoinTest, JoinCheckPollAtTimeoutReturnsLeaveStation) {
	const join_action exp_action = join_action::leave_station;

	const uint32_t start_ms = 5000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	join_action res;

	res = check.Poll(start_ms + JOIN_TIMEOUT_MS, false, no_ip, no_reason);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(join_state::fail, check.State());
}

TEST(WiFiJoinTest, JoinCheckPollAfterTimeoutReturnsLeaveStation) {
	const join_action exp_action = join_action::leave_station;

	const uint32_t start_ms = 5000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	join_action res;

	res = check.Poll(start_ms + JOIN_TIMEOUT_MS + 1, false, no_ip, no_reason);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(join_state::fail, check.State());
}

TEST(WiFiJoinTest, JoinCheckPollLeaveStationIsReturnedOnce) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 5000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + JOIN_TIMEOUT_MS, false, no_ip, no_reason);

	join_action res;

	res = check.Poll(start_ms + JOIN_TIMEOUT_MS + 10, false, no_ip, no_reason);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiJoinTest, JoinCheckTimeoutWithoutReasonIsTimeout) {
	const join_reason exp_reason = join_reason::timeout;

	const uint32_t start_ms = 5000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + JOIN_TIMEOUT_MS, false, no_ip, no_reason);

	join_reason res;

	res = check.Reason();

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, JoinCheckTimeoutWithAuthFailIsWrongPassword) {
	const join_reason exp_reason = join_reason::wrong_password;

	const uint32_t start_ms = 5000;
	const uint32_t no_ip = 0;
	const uint8_t code_auth_fail = 202;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + JOIN_TIMEOUT_MS, false, no_ip, code_auth_fail);

	join_reason res;

	res = check.Reason();

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, JoinCheckTimeoutWithNoApFoundIsNoNetwork) {
	const join_reason exp_reason = join_reason::no_network;

	const uint32_t start_ms = 5000;
	const uint32_t no_ip = 0;
	const uint8_t code_no_ap_found = 201;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + JOIN_TIMEOUT_MS, false, no_ip, code_no_ap_found);

	join_reason res;

	res = check.Reason();

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, JoinCheckTimeoutWithAuthExpireIsTimeout) {
	const join_reason exp_reason = join_reason::timeout;

	const uint32_t start_ms = 5000;
	const uint32_t no_ip = 0;
	const uint8_t code_auth_expire = 2;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + JOIN_TIMEOUT_MS, false, no_ip, code_auth_expire);

	join_reason res;

	res = check.Reason();

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, JoinCheckTimeoutWithConnectedButZeroIpFails) {
	const join_action exp_action = join_action::leave_station;

	const uint32_t start_ms = 5000;
	const uint32_t zero_ip = 0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	join_action res;

	res = check.Poll(start_ms + JOIN_TIMEOUT_MS, true, zero_ip, no_reason);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(join_reason::timeout, check.Reason());
}

TEST(WiFiJoinTest, JoinCheckJoinOnDeadlinePassStillCounts) {
	const join_action exp_action = join_action::save_network;

	const uint32_t start_ms = 5000;
	const uint32_t ip = 0x0100A8C0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	join_action res;

	res = check.Poll(start_ms + JOIN_TIMEOUT_MS, true, ip, no_reason);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiJoinTest, JoinCheckFailedCheckIgnoresLaterConnection) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 5000;
	const uint32_t no_ip = 0;
	const uint32_t ip = 0x0100A8C0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + JOIN_TIMEOUT_MS, false, no_ip, no_reason);

	join_action res;

	res = check.Poll(start_ms + JOIN_TIMEOUT_MS + 100, true, ip, no_reason);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(join_state::fail, check.State());
}

TEST(WiFiJoinTest, JoinCheckWrapStartJustBeforeTimeoutReturnsNone) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = UINT32_MAX - 5000; // millis() wraps 5001 ms after the start
	const uint32_t now_ms = static_cast<uint32_t>(start_ms + JOIN_TIMEOUT_MS - 1);
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	join_action res;

	res = check.Poll(now_ms, false, no_ip, no_reason);

	EXPECT_EQ(exp_action, res);
	EXPECT_LT(now_ms, start_ms); // the clock has wrapped
}

TEST(WiFiJoinTest, JoinCheckWrapStartAtTimeoutReturnsLeaveStation) {
	const join_action exp_action = join_action::leave_station;

	const uint32_t start_ms = UINT32_MAX - 5000;
	const uint32_t now_ms = static_cast<uint32_t>(start_ms + JOIN_TIMEOUT_MS);
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	join_action res;

	res = check.Poll(now_ms, false, no_ip, no_reason);

	EXPECT_EQ(exp_action, res);
	EXPECT_LT(now_ms, start_ms); // the clock has wrapped
}

TEST(WiFiJoinTest, JoinCheckWrapShortlyAfterWrapDoesNotTimeOutEarly) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = UINT32_MAX - 10;
	const uint32_t now_ms = 5; // 16 ms after the start
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	join_action res;

	res = check.Poll(now_ms, false, no_ip, no_reason);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiJoinTest, JoinCheckStartAfterFailReturnsTrue) {
	const uint32_t start_ms = 5000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + JOIN_TIMEOUT_MS, false, no_ip, no_reason);

	bool res;

	res = check.Start(start_ms + JOIN_TIMEOUT_MS + 1000);

	EXPECT_TRUE(res);
	EXPECT_EQ(join_state::checking, check.State());
}

TEST(WiFiJoinTest, JoinCheckStartAfterFailDoesNotKeepOldReason) {
	const join_reason exp_reason = join_reason::timeout;

	const uint32_t start_ms = 5000;
	const uint32_t no_ip = 0;
	const uint8_t code_auth_fail = 202;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + JOIN_TIMEOUT_MS, false, no_ip, code_auth_fail);
	const uint32_t second_start_ms = start_ms + JOIN_TIMEOUT_MS + 1000;
	check.Start(second_start_ms);
	check.Poll(second_start_ms + JOIN_TIMEOUT_MS, false, no_ip, no_reason);

	join_reason res;

	res = check.Reason();

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, JoinCheckStartAfterFailRestartsTheClock) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 5000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + JOIN_TIMEOUT_MS, false, no_ip, no_reason);
	const uint32_t second_start_ms = start_ms + JOIN_TIMEOUT_MS + 1000;
	check.Start(second_start_ms);

	join_action res;

	res = check.Poll(second_start_ms + JOIN_TIMEOUT_MS - 1, false, no_ip, no_reason);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiJoinTest, JoinCheckStartAfterOkClearsIp) {
	const uint32_t exp_ip = 0;

	const uint32_t start_ms = 5000;
	const uint32_t ip = 0x0100A8C0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + 3000, true, ip, no_reason);
	check.SaveFinished(true);

	check.Start(start_ms + 30000);

	EXPECT_EQ(exp_ip, check.Ip());
	EXPECT_EQ(join_state::checking, check.State());
}

TEST(WiFiJoinTest, JoinCheckStartAfterSaveFailedGivesFreshCheck) {
	const join_reason exp_reason = join_reason::timeout;

	const uint32_t start_ms = 5000;
	const uint32_t no_ip = 0;
	const uint32_t ip = 0x0100A8C0;
	const uint8_t no_reason = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + 3000, true, ip, no_reason);
	check.SaveFinished(false);
	const uint32_t second_start_ms = start_ms + 30000;
	check.Start(second_start_ms);
	check.Poll(second_start_ms + JOIN_TIMEOUT_MS, false, no_ip, no_reason);

	join_reason res;

	res = check.Reason();

	EXPECT_EQ(exp_reason, res);
}

// ---- Disconnect streak ----

TEST(WiFiJoinTest, AddDisconnectEventFirstWrongPasswordStartsStreakOfOne) {
	const uint32_t exp_streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | 1;

	const uint32_t empty_streak = 0;
	const uint8_t auth_fail = 202;

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(empty_streak, auth_fail);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, AddDisconnectEventFirstNoApFoundStartsStreakOfOne) {
	const uint32_t exp_streak = (static_cast<uint32_t>(join_reason::no_network) << 8) | 1;

	const uint32_t empty_streak = 0;
	const uint8_t no_ap_found = 201;

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(empty_streak, no_ap_found);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, AddDisconnectEvent4WayHandshakeTimeoutStartsWrongPasswordStreak) {
	const uint32_t exp_streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | 1;

	const uint32_t empty_streak = 0;
	const uint8_t handshake_timeout_4way = 15;

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(empty_streak, handshake_timeout_4way);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, AddDisconnectEventHandshakeTimeoutStartsWrongPasswordStreak) {
	const uint32_t exp_streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | 1;

	const uint32_t empty_streak = 0;
	const uint8_t handshake_timeout = 204;

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(empty_streak, handshake_timeout);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, AddDisconnectEvent8021xAuthFailedStartsWrongPasswordStreak) {
	const uint32_t exp_streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | 1;

	const uint32_t empty_streak = 0;
	const uint8_t auth_failed_8021x = 23;

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(empty_streak, auth_failed_8021x);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, AddDisconnectEventMicFailureStartsWrongPasswordStreak) {
	const uint32_t exp_streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | 1;

	const uint32_t empty_streak = 0;
	const uint8_t mic_failure = 14;

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(empty_streak, mic_failure);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, AddDisconnectEventSameCodeTwiceGivesStreakOfTwo) {
	const uint32_t exp_streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | 2;

	const uint8_t auth_fail = 202;
	const uint32_t streak = HAQuDA_AddDisconnectEvent(0, auth_fail);

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(streak, auth_fail);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, AddDisconnectEventDifferentCodesOfSameClassExtendStreak) {
	const uint32_t exp_streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | 2;

	const uint8_t handshake_timeout_4way = 15;
	const uint8_t auth_fail = 202;
	const uint32_t streak = HAQuDA_AddDisconnectEvent(0, handshake_timeout_4way);

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(streak, auth_fail);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, AddDisconnectEventNoApFoundTwiceGivesStreakOfTwo) {
	const uint32_t exp_streak = (static_cast<uint32_t>(join_reason::no_network) << 8) | 2;

	const uint8_t no_ap_found = 201;
	const uint32_t streak = HAQuDA_AddDisconnectEvent(0, no_ap_found);

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(streak, no_ap_found);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, AddDisconnectEventClassSwitchRestartsStreakWithNewClass) {
	const uint32_t exp_streak = (static_cast<uint32_t>(join_reason::no_network) << 8) | 1;

	const uint8_t handshake_timeout_4way = 15;
	const uint8_t no_ap_found = 201;
	const uint32_t streak = HAQuDA_AddDisconnectEvent(0, handshake_timeout_4way);

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(streak, no_ap_found);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, AddDisconnectEventNoNetworkToWrongPasswordSwitchRestartsStreak) {
	const uint32_t exp_streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | 1;

	const uint8_t no_ap_found = 201;
	const uint8_t auth_fail = 202;
	uint32_t streak = HAQuDA_AddDisconnectEvent(0, no_ap_found);
	streak = HAQuDA_AddDisconnectEvent(streak, no_ap_found);

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(streak, auth_fail);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, AddDisconnectEventAfterClassSwitchSecondNoApFoundGivesStreakOfTwo) {
	const uint32_t exp_streak = (static_cast<uint32_t>(join_reason::no_network) << 8) | 2;

	const uint8_t handshake_timeout_4way = 15;
	const uint8_t no_ap_found = 201;
	uint32_t streak = HAQuDA_AddDisconnectEvent(0, handshake_timeout_4way);
	streak = HAQuDA_AddDisconnectEvent(streak, no_ap_found);

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(streak, no_ap_found);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, AddDisconnectEventAssocLeaveKeepsStreakUnchanged) {
	const uint32_t exp_streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | 1;

	const uint8_t auth_fail = 202;
	const uint8_t assoc_leave = 8;
	const uint32_t streak = HAQuDA_AddDisconnectEvent(0, auth_fail);

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(streak, assoc_leave);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, AddDisconnectEventAssocLeaveOnEmptyStreakStaysEmpty) {
	const uint32_t exp_streak = 0;

	const uint32_t empty_streak = 0;
	const uint8_t assoc_leave = 8;

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(empty_streak, assoc_leave);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, AddDisconnectEventAssocLeaveBetweenEventsDoesNotBreakStreak) {
	const uint32_t exp_streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | 2;

	const uint8_t auth_fail = 202;
	const uint8_t assoc_leave = 8;
	uint32_t streak = HAQuDA_AddDisconnectEvent(0, auth_fail);
	streak = HAQuDA_AddDisconnectEvent(streak, assoc_leave);

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(streak, auth_fail);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, AddDisconnectEventConnectionFailEndsStreak) {
	const uint32_t exp_streak = 0;

	const uint8_t auth_fail = 202;
	const uint8_t connection_fail = 205; // tells nothing about the cause
	const uint32_t streak = HAQuDA_AddDisconnectEvent(0, auth_fail);

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(streak, connection_fail);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, AddDisconnectEventAuthExpireEndsStreak) {
	const uint32_t exp_streak = 0;

	const uint8_t no_ap_found = 201;
	const uint8_t auth_expire = 2;
	const uint32_t streak = HAQuDA_AddDisconnectEvent(0, no_ap_found);

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(streak, auth_expire);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, AddDisconnectEventUnknownCodeEndsStreak) {
	const uint32_t exp_streak = 0;

	const uint8_t auth_fail = 202;
	const uint8_t unknown_code = 255;
	const uint32_t streak = HAQuDA_AddDisconnectEvent(0, auth_fail);

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(streak, unknown_code);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, AddDisconnectEventZeroCodeEndsStreak) {
	const uint32_t exp_streak = 0;

	const uint8_t auth_fail = 202;
	const uint8_t zero_code = 0;
	const uint32_t streak = HAQuDA_AddDisconnectEvent(0, auth_fail);

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(streak, zero_code);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, AddDisconnectEventUninformativeCodeOnEmptyStreakStaysEmpty) {
	const uint32_t exp_streak = 0;

	const uint32_t empty_streak = 0;
	const uint8_t connection_fail = 205;

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(empty_streak, connection_fail);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, AddDisconnectEventStreakRestartsAfterReset) {
	const uint32_t exp_streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | 1;

	const uint8_t auth_fail = 202;
	const uint8_t connection_fail = 205;
	uint32_t streak = HAQuDA_AddDisconnectEvent(0, auth_fail);
	streak = HAQuDA_AddDisconnectEvent(streak, connection_fail);

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(streak, auth_fail);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, AddDisconnectEventCountBelowMaximumIncrements) {
	const uint32_t exp_streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | 0xFF;

	const uint8_t auth_fail = 202;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | 0xFE;

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(streak, auth_fail);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, AddDisconnectEventCountSaturatesAtMaximum) {
	const uint32_t exp_streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | 0xFF;

	const uint8_t auth_fail = 202;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | 0xFF;

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(streak, auth_fail);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, AddDisconnectEventSaturatedStreakStillSwitchesClass) {
	const uint32_t exp_streak = (static_cast<uint32_t>(join_reason::no_network) << 8) | 1;

	const uint8_t no_ap_found = 201;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | 0xFF;

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(streak, no_ap_found);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, AddDisconnectEventClassWithZeroCountStartsFreshStreak) {
	const uint32_t exp_streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | 1;

	const uint8_t auth_fail = 202;
	const uint32_t streak = static_cast<uint32_t>(join_reason::wrong_password) << 8; // class set, no events

	uint32_t res;

	res = HAQuDA_AddDisconnectEvent(streak, auth_fail);

	EXPECT_EQ(exp_streak, res);
}

TEST(WiFiJoinTest, StreakReasonEmptyStreakIsNone) {
	const join_reason exp_reason = join_reason::none;

	const uint32_t empty_streak = 0;

	join_reason res;

	res = HAQuDA_StreakReason(empty_streak);

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, StreakReasonOneWrongPasswordEventIsNone) {
	const join_reason exp_reason = join_reason::none;

	const uint32_t streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | 1;

	join_reason res;

	res = HAQuDA_StreakReason(streak);

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, StreakReasonOneNoNetworkEventIsNone) {
	const join_reason exp_reason = join_reason::none;

	const uint32_t streak = (static_cast<uint32_t>(join_reason::no_network) << 8) | 1;

	join_reason res;

	res = HAQuDA_StreakReason(streak);

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, StreakReasonTwoWrongPasswordEventsDecideWrongPassword) {
	const join_reason exp_reason = join_reason::wrong_password;

	const uint32_t streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | JOIN_EARLY_FAIL_EVENTS;

	join_reason res;

	res = HAQuDA_StreakReason(streak);

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, StreakReasonTwoNoNetworkEventsDecideNoNetwork) {
	const join_reason exp_reason = join_reason::no_network;

	const uint32_t streak = (static_cast<uint32_t>(join_reason::no_network) << 8) | JOIN_EARLY_FAIL_EVENTS;

	join_reason res;

	res = HAQuDA_StreakReason(streak);

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, StreakReasonLongStreakStillDecides) {
	const join_reason exp_reason = join_reason::wrong_password;

	const uint32_t streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | 0xFF;

	join_reason res;

	res = HAQuDA_StreakReason(streak);

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, StreakReasonTimeoutClassNeverDecides) {
	const join_reason exp_reason = join_reason::none;

	const uint32_t streak = (static_cast<uint32_t>(join_reason::timeout) << 8) | 5;

	join_reason res;

	res = HAQuDA_StreakReason(streak);

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, StreakReasonSaveFailedClassNeverDecides) {
	const join_reason exp_reason = join_reason::none;

	const uint32_t streak = (static_cast<uint32_t>(join_reason::save_failed) << 8) | 5;

	join_reason res;

	res = HAQuDA_StreakReason(streak);

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, StreakReasonNoneClassNeverDecides) {
	const join_reason exp_reason = join_reason::none;

	const uint32_t streak = (static_cast<uint32_t>(join_reason::none) << 8) | 5;

	join_reason res;

	res = HAQuDA_StreakReason(streak);

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, StreakReasonUnknownClassNeverDecides) {
	const join_reason exp_reason = join_reason::none;

	const uint32_t streak = (0x7Fu << 8) | 5;

	join_reason res;

	res = HAQuDA_StreakReason(streak);

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, StreakReasonWrongPasswordThenNoNetworkTwiceDecidesNoNetwork) {
	const join_reason exp_reason = join_reason::no_network;

	const uint8_t handshake_timeout_4way = 15;
	const uint8_t no_ap_found = 201;
	uint32_t streak = HAQuDA_AddDisconnectEvent(0, handshake_timeout_4way);
	streak = HAQuDA_AddDisconnectEvent(streak, no_ap_found);
	streak = HAQuDA_AddDisconnectEvent(streak, no_ap_found);

	join_reason res;

	res = HAQuDA_StreakReason(streak);

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, StreakReasonWrongPasswordThenNoNetworkOnceIsNone) {
	const join_reason exp_reason = join_reason::none;

	const uint8_t handshake_timeout_4way = 15;
	const uint8_t no_ap_found = 201;
	uint32_t streak = HAQuDA_AddDisconnectEvent(0, handshake_timeout_4way);
	streak = HAQuDA_AddDisconnectEvent(streak, no_ap_found);

	join_reason res;

	res = HAQuDA_StreakReason(streak);

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, StreakReasonTwoAuthFailEventsWithAssocLeaveBetweenDecide) {
	const join_reason exp_reason = join_reason::wrong_password;

	const uint8_t auth_fail = 202;
	const uint8_t assoc_leave = 8;
	uint32_t streak = HAQuDA_AddDisconnectEvent(0, auth_fail);
	streak = HAQuDA_AddDisconnectEvent(streak, assoc_leave);
	streak = HAQuDA_AddDisconnectEvent(streak, auth_fail);

	join_reason res;

	res = HAQuDA_StreakReason(streak);

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, StreakReasonAuthFailInterruptedByOtherCodeIsNone) {
	const join_reason exp_reason = join_reason::none;

	const uint8_t auth_fail = 202;
	const uint8_t connection_fail = 205;
	uint32_t streak = HAQuDA_AddDisconnectEvent(0, auth_fail);
	streak = HAQuDA_AddDisconnectEvent(streak, connection_fail);
	streak = HAQuDA_AddDisconnectEvent(streak, auth_fail);

	join_reason res;

	res = HAQuDA_StreakReason(streak);

	EXPECT_EQ(exp_reason, res);
}

TEST(WiFiJoinTest, StreakReasonAssocLeaveAloneIsNone) {
	const join_reason exp_reason = join_reason::none;

	const uint8_t assoc_leave = 8;
	uint32_t streak = HAQuDA_AddDisconnectEvent(0, assoc_leave);
	streak = HAQuDA_AddDisconnectEvent(streak, assoc_leave);

	join_reason res;

	res = HAQuDA_StreakReason(streak);

	EXPECT_EQ(exp_reason, res);
}

// ---- JoinCheck with a disconnect streak ----

TEST(WiFiJoinTest, JoinCheckPollDecidedWrongPasswordStreakReturnsLeaveStation) {
	const join_action exp_action = join_action::leave_station;

	const uint32_t start_ms = 1000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | JOIN_EARLY_FAIL_EVENTS;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	join_action res;

	res = check.Poll(start_ms + 6000, false, no_ip, no_reason, streak);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiJoinTest, JoinCheckPollDecidedWrongPasswordStreakFailsWithWrongPassword) {
	const join_state exp_state = join_state::fail;
	const join_reason exp_reason = join_reason::wrong_password;

	const uint32_t start_ms = 1000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | JOIN_EARLY_FAIL_EVENTS;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	check.Poll(start_ms + 6000, false, no_ip, no_reason, streak);

	EXPECT_EQ(exp_state, check.State());
	EXPECT_EQ(exp_reason, check.Reason());
}

TEST(WiFiJoinTest, JoinCheckPollDecidedNoNetworkStreakFailsWithNoNetwork) {
	const join_state exp_state = join_state::fail;
	const join_reason exp_reason = join_reason::no_network;

	const uint32_t start_ms = 1000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::no_network) << 8) | JOIN_EARLY_FAIL_EVENTS;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	check.Poll(start_ms + 6000, false, no_ip, no_reason, streak);

	EXPECT_EQ(exp_state, check.State());
	EXPECT_EQ(exp_reason, check.Reason());
}

TEST(WiFiJoinTest, JoinCheckPollDecidedStreakFailsWellBeforeTheTimeout) {
	const join_action exp_action = join_action::leave_station;

	const uint32_t start_ms = 1000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | JOIN_EARLY_FAIL_EVENTS;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	join_action res;

	res = check.Poll(start_ms + 1, false, no_ip, no_reason, streak); // far below JOIN_TIMEOUT_MS

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiJoinTest, JoinCheckPollDecidedStreakAtStartTimeFailsAtOnce) {
	const join_action exp_action = join_action::leave_station;

	const uint32_t start_ms = 1000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::no_network) << 8) | JOIN_EARLY_FAIL_EVENTS;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	join_action res;

	res = check.Poll(start_ms, false, no_ip, no_reason, streak);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiJoinTest, JoinCheckPollDecidedStreakBeatsLastDisconnectReason) {
	const join_reason exp_reason = join_reason::no_network;

	const uint32_t start_ms = 1000;
	const uint32_t no_ip = 0;
	const uint8_t auth_fail = 202; // would map to wrong_password if the timeout decided
	const uint32_t streak = (static_cast<uint32_t>(join_reason::no_network) << 8) | JOIN_EARLY_FAIL_EVENTS;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	check.Poll(start_ms + 6000, false, no_ip, auth_fail, streak);

	EXPECT_EQ(exp_reason, check.Reason());
}

TEST(WiFiJoinTest, JoinCheckPollLeaveStationOfDecidedStreakIsReturnedOnce) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | JOIN_EARLY_FAIL_EVENTS;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + 6000, false, no_ip, no_reason, streak);

	join_action res;

	res = check.Poll(start_ms + 6010, false, no_ip, no_reason, streak);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiJoinTest, JoinCheckPollDecidedStreakFailKeepsReasonOnLaterPoll) {
	const join_reason exp_reason = join_reason::wrong_password;

	const uint32_t start_ms = 1000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | JOIN_EARLY_FAIL_EVENTS;
	const uint32_t empty_streak = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + 6000, false, no_ip, no_reason, streak);

	check.Poll(start_ms + JOIN_TIMEOUT_MS, false, no_ip, no_reason, empty_streak);

	EXPECT_EQ(exp_reason, check.Reason());
}

TEST(WiFiJoinTest, JoinCheckPollStreakOfOneEventKeepsChecking) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | 1;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	join_action res;

	res = check.Poll(start_ms + 6000, false, no_ip, no_reason, streak);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(join_state::checking, check.State());
}

TEST(WiFiJoinTest, JoinCheckPollEmptyStreakKeepsChecking) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	const uint32_t empty_streak = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	join_action res;

	res = check.Poll(start_ms + 6000, false, no_ip, no_reason, empty_streak);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiJoinTest, JoinCheckPollConnectedWithIpWinsOverDecidedStreak) {
	const join_action exp_action = join_action::save_network;

	const uint32_t start_ms = 1000;
	const uint32_t ip = 0x0100A8C0;
	const uint8_t auth_fail = 202;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | JOIN_EARLY_FAIL_EVENTS;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	join_action res;

	res = check.Poll(start_ms + 6000, true, ip, auth_fail, streak);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiJoinTest, JoinCheckPollConnectedWithIpAndDecidedStreakKeepsCheckingWithIp) {
	const join_state exp_state = join_state::checking;

	const uint32_t start_ms = 1000;
	const uint32_t ip = 0x0100A8C0;
	const uint8_t no_reason = 0;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::no_network) << 8) | JOIN_EARLY_FAIL_EVENTS;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	check.Poll(start_ms + 6000, true, ip, no_reason, streak);

	EXPECT_EQ(exp_state, check.State());
	EXPECT_EQ(ip, check.Ip());
	EXPECT_EQ(join_reason::none, check.Reason());
}

TEST(WiFiJoinTest, JoinCheckPollConnectedWithZeroIpDoesNotWinOverDecidedStreak) {
	const join_action exp_action = join_action::leave_station;

	const uint32_t start_ms = 1000;
	const uint32_t zero_ip = 0;
	const uint8_t no_reason = 0;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | JOIN_EARLY_FAIL_EVENTS;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	join_action res;

	res = check.Poll(start_ms + 6000, true, zero_ip, no_reason, streak);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiJoinTest, JoinCheckPollNotConnectedWithIpDoesNotWinOverDecidedStreak) {
	const join_action exp_action = join_action::leave_station;

	const uint32_t start_ms = 1000;
	const uint32_t ip = 0x0100A8C0;
	const uint8_t no_reason = 0;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | JOIN_EARLY_FAIL_EVENTS;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	join_action res;

	res = check.Poll(start_ms + 6000, false, ip, no_reason, streak);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiJoinTest, JoinCheckPollWhileSavingIgnoresDecidedStreak) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t ip = 0x0100A8C0;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | JOIN_EARLY_FAIL_EVENTS;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + 3000, true, ip, no_reason);

	join_action res;

	res = check.Poll(start_ms + 3010, false, no_ip, no_reason, streak);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(join_state::checking, check.State());
}

TEST(WiFiJoinTest, JoinCheckPollBeforeStartIgnoresDecidedStreak) {
	const join_action exp_action = join_action::none;

	const uint32_t now_ms = 1000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | JOIN_EARLY_FAIL_EVENTS;
	HAQuDA_JoinCheck check;

	join_action res;

	res = check.Poll(now_ms, false, no_ip, no_reason, streak);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(join_state::idle, check.State());
}

TEST(WiFiJoinTest, JoinCheckPollAfterOkIgnoresDecidedStreak) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t ip = 0x0100A8C0;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | JOIN_EARLY_FAIL_EVENTS;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + 3000, true, ip, no_reason);
	check.SaveFinished(true);

	join_action res;

	res = check.Poll(start_ms + 3010, false, no_ip, no_reason, streak);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(join_state::ok, check.State());
}

TEST(WiFiJoinTest, JoinCheckPollUndecidedStreakStillTimesOutAtDeadline) {
	const join_action exp_action = join_action::leave_station;

	const uint32_t start_ms = 1000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | 1;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	join_action res;

	res = check.Poll(start_ms + JOIN_TIMEOUT_MS, false, no_ip, no_reason, streak);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiJoinTest, JoinCheckPollUndecidedStreakTimeoutUsesLastDisconnectReason) {
	const join_reason exp_reason = join_reason::wrong_password;

	const uint32_t start_ms = 1000;
	const uint32_t no_ip = 0;
	const uint8_t auth_fail = 202;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | 1;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	check.Poll(start_ms + JOIN_TIMEOUT_MS, false, no_ip, auth_fail, streak);

	EXPECT_EQ(exp_reason, check.Reason());
}

TEST(WiFiJoinTest, JoinCheckPollEmptyStreakTimeoutWithoutReasonIsTimeout) {
	const join_reason exp_reason = join_reason::timeout;

	const uint32_t start_ms = 1000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	const uint32_t empty_streak = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	check.Poll(start_ms + JOIN_TIMEOUT_MS, false, no_ip, no_reason, empty_streak);

	EXPECT_EQ(exp_reason, check.Reason());
}

TEST(WiFiJoinTest, JoinCheckPollUndecidedStreakJustBeforeTimeoutReturnsNone) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::no_network) << 8) | 1;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	join_action res;

	res = check.Poll(start_ms + JOIN_TIMEOUT_MS - 1, false, no_ip, no_reason, streak);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiJoinTest, JoinCheckStartAfterStreakFailReturnsTrue) {
	const bool exp_res = true;

	const uint32_t start_ms = 1000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | JOIN_EARLY_FAIL_EVENTS;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + 6000, false, no_ip, no_reason, streak);

	bool res;

	res = check.Start(start_ms + 7000);

	EXPECT_EQ(exp_res, res);
}

TEST(WiFiJoinTest, JoinCheckStartAfterStreakFailClearsReasonAndChecks) {
	const join_state exp_state = join_state::checking;
	const join_reason exp_reason = join_reason::none;

	const uint32_t start_ms = 1000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | JOIN_EARLY_FAIL_EVENTS;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + 6000, false, no_ip, no_reason, streak);

	check.Start(start_ms + 7000);

	EXPECT_EQ(exp_state, check.State());
	EXPECT_EQ(exp_reason, check.Reason());
}

TEST(WiFiJoinTest, JoinCheckPollAfterRestartWaitsForNewDeadline) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | JOIN_EARLY_FAIL_EVENTS;
	const uint32_t empty_streak = 0;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + 6000, false, no_ip, no_reason, streak);
	const uint32_t second_start_ms = start_ms + 7000;
	check.Start(second_start_ms);

	join_action res;

	res = check.Poll(second_start_ms + JOIN_TIMEOUT_MS - 1, false, no_ip, no_reason, empty_streak);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiJoinTest, JoinCheckPollAfterRestartDecidesOnNewStreak) {
	const join_reason exp_reason = join_reason::no_network;

	const uint32_t start_ms = 1000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	const uint32_t first_streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | JOIN_EARLY_FAIL_EVENTS;
	const uint32_t second_streak = (static_cast<uint32_t>(join_reason::no_network) << 8) | JOIN_EARLY_FAIL_EVENTS;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + 6000, false, no_ip, no_reason, first_streak);
	const uint32_t second_start_ms = start_ms + 7000;
	check.Start(second_start_ms);

	check.Poll(second_start_ms + 6000, false, no_ip, no_reason, second_streak);

	EXPECT_EQ(exp_reason, check.Reason());
}

TEST(WiFiJoinTest, JoinCheckPollDecidedStreakAcrossMillisWrapFailsAtOnce) {
	const join_action exp_action = join_action::leave_station;

	const uint32_t start_ms = 0xFFFFFFFFu - 1000; // the clock wraps during the check
	const uint32_t after_wrap_ms = 5000;
	const uint32_t no_ip = 0;
	const uint8_t no_reason = 0;
	const uint32_t streak = (static_cast<uint32_t>(join_reason::wrong_password) << 8) | JOIN_EARLY_FAIL_EVENTS;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	join_action res;

	res = check.Poll(after_wrap_ms, false, no_ip, no_reason, streak);

	EXPECT_EQ(exp_action, res);
}

// ---- JSON of the ok state with restart_ms ----

TEST(WiFiJoinTest, BuildJoinJsonOkWithRestartMsShowsTheValue) {
	const char exp_json[] = "{\"state\":\"ok\",\"ssid\":\"Home\",\"ip\":\"192.168.1.50\",\"restart_ms\":15000,\"host\":\"haquda.local\"}";

	const char *host = "haquda.local";
	join_view view;
	view.state = join_state::ok;
	view.ip = 192u | (168u << 8) | (1u << 16) | (50u << 24);
	view.restartMs = 15000;
	HAQuDA_SetJoinSsid(view, "Home", JOIN_SSID_BUF_LEN);
	char buf[128];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonOkWithRestartMsOfHoldTimeShowsTheValue) {
	const char exp_json[] = "{\"state\":\"ok\",\"ssid\":\"Home\",\"ip\":\"192.168.1.50\",\"restart_ms\":20000,\"host\":\"haquda.local\"}";

	const char *host = "haquda.local";
	join_view view;
	view.state = join_state::ok;
	view.ip = 192u | (168u << 8) | (1u << 16) | (50u << 24);
	view.restartMs = JOIN_OK_HOLD_MS;
	HAQuDA_SetJoinSsid(view, "Home", JOIN_SSID_BUF_LEN);
	char buf[128];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonOkWithRestartMsOfOneShowsOne) {
	const char exp_json[] = "{\"state\":\"ok\",\"ssid\":\"Home\",\"ip\":\"192.168.1.50\",\"restart_ms\":1,\"host\":\"haquda.local\"}";

	const char *host = "haquda.local";
	join_view view;
	view.state = join_state::ok;
	view.ip = 192u | (168u << 8) | (1u << 16) | (50u << 24);
	view.restartMs = 1;
	HAQuDA_SetJoinSsid(view, "Home", JOIN_SSID_BUF_LEN);
	char buf[128];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonOkWithLargestRestartMsIsUnsigned) {
	const char exp_json[] = "{\"state\":\"ok\",\"ssid\":\"Home\",\"ip\":\"192.168.1.50\",\"restart_ms\":4294967295,\"host\":\"haquda.local\"}";

	const char *host = "haquda.local";
	join_view view;
	view.state = join_state::ok;
	view.ip = 192u | (168u << 8) | (1u << 16) | (50u << 24);
	view.restartMs = 0xFFFFFFFFu;
	HAQuDA_SetJoinSsid(view, "Home", JOIN_SSID_BUF_LEN);
	char buf[128];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonOkWithZeroRestartMsShowsZero) {
	const char exp_json[] = "{\"state\":\"ok\",\"ssid\":\"Home\",\"ip\":\"192.168.1.50\",\"restart_ms\":0,\"host\":\"haquda.local\"}";

	const char *host = "haquda.local";
	join_view view;
	view.state = join_state::ok;
	view.ip = 192u | (168u << 8) | (1u << 16) | (50u << 24);
	view.restartMs = 0;
	HAQuDA_SetJoinSsid(view, "Home", JOIN_SSID_BUF_LEN);
	char buf[128];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonOkBufferOfExactSizeWithRestartMsFits) {
	const char exp_json[] = "{\"state\":\"ok\",\"ssid\":\"Home\",\"ip\":\"192.168.1.50\",\"restart_ms\":4294967295,\"host\":\"haquda.local\"}";

	const char *host = "haquda.local";
	join_view view;
	view.state = join_state::ok;
	view.ip = 192u | (168u << 8) | (1u << 16) | (50u << 24);
	view.restartMs = 0xFFFFFFFFu;
	HAQuDA_SetJoinSsid(view, "Home", JOIN_SSID_BUF_LEN);
	char buf[sizeof(exp_json)]; // text plus terminator

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonOkBufferOneByteTooSmallForRestartMsReturnsFalseAndEmpty) {
	const char exp_json[] = "{\"state\":\"ok\",\"ssid\":\"Home\",\"ip\":\"192.168.1.50\",\"restart_ms\":4294967295,\"host\":\"haquda.local\"}";

	const char *host = "haquda.local";
	join_view view;
	view.state = join_state::ok;
	view.ip = 192u | (168u << 8) | (1u << 16) | (50u << 24);
	view.restartMs = 0xFFFFFFFFu;
	HAQuDA_SetJoinSsid(view, "Home", JOIN_SSID_BUF_LEN);
	char buf[sizeof(exp_json) - 1]; // no room for the terminator
	memset(buf, 'x', sizeof(buf));

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_FALSE(res);
	EXPECT_EQ('\0', buf[0]);
}

TEST(WiFiJoinTest, BuildJoinJsonOkBufferCutInsideRestartMsReturnsFalseAndEmpty) {
	const char *host = "haquda.local";
	join_view view;
	view.state = join_state::ok;
	view.ip = 192u | (168u << 8) | (1u << 16) | (50u << 24);
	view.restartMs = 15000;
	HAQuDA_SetJoinSsid(view, "Home", JOIN_SSID_BUF_LEN);
	char buf[64]; // the text up to the number is 61 characters, so the number is cut
	memset(buf, 'x', sizeof(buf));

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_FALSE(res);
	EXPECT_EQ('\0', buf[0]);
}

TEST(WiFiJoinTest, BuildJoinJsonCheckingHidesRestartMs) {
	const char exp_json[] = "{\"state\":\"checking\",\"ssid\":\"Home\",\"host\":\"haquda.local\"}";

	const char *host = "haquda.local";
	join_view view;
	view.state = join_state::checking;
	view.restartMs = 15000; // stale value, must not be shown
	HAQuDA_SetJoinSsid(view, "Home", JOIN_SSID_BUF_LEN);
	char buf[128];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonFailHidesRestartMs) {
	const char exp_json[] = "{\"state\":\"fail\",\"ssid\":\"Home\",\"host\":\"haquda.local\",\"reason\":\"no_network\"}";

	const char *host = "haquda.local";
	join_view view;
	view.state = join_state::fail;
	view.reason = join_reason::no_network;
	view.restartMs = 15000; // stale value, must not be shown
	HAQuDA_SetJoinSsid(view, "Home", JOIN_SSID_BUF_LEN);
	char buf[128];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}
