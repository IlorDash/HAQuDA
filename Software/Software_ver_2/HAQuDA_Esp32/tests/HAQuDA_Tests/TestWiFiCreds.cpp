#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "HAQuDA_WiFiCreds.h"

// ---------------------------------------------------------------- HAQuDA_PlaceCredsFirst

TEST(WiFiCredsTest, PlaceCredsFirstEmptyListGivesOnlyNewRecord) {
	const size_t exp_count = 1;

	const TWiFiCreds new_creds = {"Home", "secret"};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_PlaceCredsFirst(nullptr, 0, new_creds, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("Home", out[0].ssid);
	EXPECT_STREQ("secret", out[0].pass);
}

TEST(WiFiCredsTest, PlaceCredsFirstPutsNewBeforeStoredInTheirOrder) {
	const size_t exp_count = 3;

	const TWiFiCreds stored[] = {{"Old1", "p1"}, {"Old2", "p2"}};
	const TWiFiCreds new_creds = {"New", "pn"};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_PlaceCredsFirst(stored, 2, new_creds, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("New", out[0].ssid);
	EXPECT_STREQ("Old1", out[1].ssid);
	EXPECT_STREQ("p1", out[1].pass);
	EXPECT_STREQ("Old2", out[2].ssid);
	EXPECT_STREQ("p2", out[2].pass);
}

TEST(WiFiCredsTest, PlaceCredsFirstSameNameAtEndIsReplacedNotDuplicated) {
	const size_t exp_count = 3;

	const TWiFiCreds stored[] = {{"A", "pa"}, {"B", "pb"}, {"C", "old"}};
	const TWiFiCreds new_creds = {"C", "fresh"};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_PlaceCredsFirst(stored, 3, new_creds, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("C", out[0].ssid);
	EXPECT_STREQ("fresh", out[0].pass);
	EXPECT_STREQ("A", out[1].ssid);
	EXPECT_STREQ("B", out[2].ssid);
}

TEST(WiFiCredsTest, PlaceCredsFirstSameNameAtStartIsReplacedNotDuplicated) {
	const size_t exp_count = 3;

	const TWiFiCreds stored[] = {{"A", "old"}, {"B", "pb"}, {"C", "pc"}};
	const TWiFiCreds new_creds = {"A", "fresh"};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_PlaceCredsFirst(stored, 3, new_creds, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("A", out[0].ssid);
	EXPECT_STREQ("fresh", out[0].pass);
	EXPECT_STREQ("B", out[1].ssid);
	EXPECT_STREQ("C", out[2].ssid);
}

TEST(WiFiCredsTest, PlaceCredsFirstSameNameInTheMiddleIsReplaced) {
	const size_t exp_count = 3;

	const TWiFiCreds stored[] = {{"A", "pa"}, {"B", "old"}, {"C", "pc"}};
	const TWiFiCreds new_creds = {"B", "fresh"};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_PlaceCredsFirst(stored, 3, new_creds, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("B", out[0].ssid);
	EXPECT_STREQ("fresh", out[0].pass);
	EXPECT_STREQ("A", out[1].ssid);
	EXPECT_STREQ("C", out[2].ssid);
}

TEST(WiFiCredsTest, PlaceCredsFirstSameNameStoredTwiceDropsBoth) {
	const size_t exp_count = 2;

	const TWiFiCreds stored[] = {{"A", "p1"}, {"B", "pb"}, {"A", "p2"}};
	const TWiFiCreds new_creds = {"A", "fresh"};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_PlaceCredsFirst(stored, 3, new_creds, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("A", out[0].ssid);
	EXPECT_STREQ("B", out[1].ssid);
}

TEST(WiFiCredsTest, PlaceCredsFirstNamesDifferingInCaseAreDifferentNetworks) {
	const size_t exp_count = 2;

	const TWiFiCreds stored[] = {{"home", "p1"}};
	const TWiFiCreds new_creds = {"Home", "p2"};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_PlaceCredsFirst(stored, 1, new_creds, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("Home", out[0].ssid);
	EXPECT_STREQ("home", out[1].ssid);
}

TEST(WiFiCredsTest, PlaceCredsFirstEmptyStoredRecordsAreDropped) {
	const size_t exp_count = 3;

	const TWiFiCreds stored[] = {{"", "junk"}, {"A", "pa"}, {"", ""}, {"B", "pb"}};
	const TWiFiCreds new_creds = {"New", "pn"};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_PlaceCredsFirst(stored, 4, new_creds, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("New", out[0].ssid);
	EXPECT_STREQ("A", out[1].ssid);
	EXPECT_STREQ("B", out[2].ssid);
}

TEST(WiFiCredsTest, PlaceCredsFirstOnlyEmptyStoredRecordsGiveOnlyNew) {
	const size_t exp_count = 1;

	const TWiFiCreds stored[] = {{"", ""}, {"", "x"}};
	const TWiFiCreds new_creds = {"New", "pn"};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_PlaceCredsFirst(stored, 2, new_creds, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("New", out[0].ssid);
}

TEST(WiFiCredsTest, PlaceCredsFirstNineStoredGiveTen) {
	const size_t exp_count = 10;

	const TWiFiCreds stored[] = {{"S1", "p"}, {"S2", "p"}, {"S3", "p"}, {"S4", "p"}, {"S5", "p"}, {"S6", "p"}, {"S7", "p"}, {"S8", "p"}, {"S9", "p"}};
	const TWiFiCreds new_creds = {"New", "pn"};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_PlaceCredsFirst(stored, 9, new_creds, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("New", out[0].ssid);
	EXPECT_STREQ("S1", out[1].ssid);
	EXPECT_STREQ("S9", out[9].ssid);
}

TEST(WiFiCredsTest, PlaceCredsFirstTenStoredKeepFirstNineInOrder) {
	const size_t exp_count = 10;

	const TWiFiCreds stored[]
		= {{"S1", "p"}, {"S2", "p"}, {"S3", "p"}, {"S4", "p"}, {"S5", "p"}, {"S6", "p"}, {"S7", "p"}, {"S8", "p"}, {"S9", "p"}, {"S10", "p"}};
	const TWiFiCreds new_creds = {"New", "pn"};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_PlaceCredsFirst(stored, 10, new_creds, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("New", out[0].ssid);
	EXPECT_STREQ("S1", out[1].ssid);
	EXPECT_STREQ("S8", out[8].ssid);
	EXPECT_STREQ("S9", out[9].ssid);
}

TEST(WiFiCredsTest, PlaceCredsFirstTenStoredWithNewNameOfTheLastKeepsAllOthers) {
	const size_t exp_count = 10;

	const TWiFiCreds stored[]
		= {{"S1", "p"}, {"S2", "p"}, {"S3", "p"}, {"S4", "p"}, {"S5", "p"}, {"S6", "p"}, {"S7", "p"}, {"S8", "p"}, {"S9", "p"}, {"S10", "old"}};
	const TWiFiCreds new_creds = {"S10", "fresh"};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_PlaceCredsFirst(stored, 10, new_creds, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("S10", out[0].ssid);
	EXPECT_STREQ("fresh", out[0].pass);
	EXPECT_STREQ("S1", out[1].ssid);
	EXPECT_STREQ("S9", out[9].ssid);
}

TEST(WiFiCredsTest, PlaceCredsFirstMoreThanTenStoredStillGiveTen) {
	const size_t exp_count = 10;

	const TWiFiCreds stored[] = {{"S1", "p"}, {"S2", "p"}, {"S3", "p"}, {"S4", "p"},  {"S5", "p"},	{"S6", "p"},
								 {"S7", "p"}, {"S8", "p"}, {"S9", "p"}, {"S10", "p"}, {"S11", "p"}, {"S12", "p"}};
	const TWiFiCreds new_creds = {"New", "pn"};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_PlaceCredsFirst(stored, 12, new_creds, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("S9", out[9].ssid);
}

TEST(WiFiCredsTest, PlaceCredsFirstLargeOutBufferIsNotWrittenPastTheMaximum) {
	const size_t exp_count = 10;
	const size_t out_capacity = 12;

	const TWiFiCreds stored[]
		= {{"S1", "p"}, {"S2", "p"}, {"S3", "p"}, {"S4", "p"}, {"S5", "p"}, {"S6", "p"}, {"S7", "p"}, {"S8", "p"}, {"S9", "p"}, {"S10", "p"}, {"S11", "p"}};
	const TWiFiCreds new_creds = {"New", "pn"};
	TWiFiCreds out[out_capacity] = {};
	memset(&out[10], 0xAB, sizeof(out[10]));
	memset(&out[11], 0xAB, sizeof(out[11]));
	TWiFiCreds exp_untouched;
	memset(&exp_untouched, 0xAB, sizeof(exp_untouched));

	size_t res;

	res = HAQuDA_PlaceCredsFirst(stored, 11, new_creds, out, out_capacity);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(0, memcmp(&exp_untouched, &out[10], sizeof(exp_untouched)));
	EXPECT_EQ(0, memcmp(&exp_untouched, &out[11], sizeof(exp_untouched)));
}

TEST(WiFiCredsTest, PlaceCredsFirstSmallOutBufferCutsOldestAndStaysInBounds) {
	const size_t exp_count = 3;
	const size_t out_capacity = 3;

	const TWiFiCreds stored[] = {{"S1", "p"}, {"S2", "p"}, {"S3", "p"}, {"S4", "p"}};
	const TWiFiCreds new_creds = {"New", "pn"};
	TWiFiCreds out[4] = {};
	memset(&out[3], 0xAB, sizeof(out[3]));
	TWiFiCreds exp_untouched;
	memset(&exp_untouched, 0xAB, sizeof(exp_untouched));

	size_t res;

	res = HAQuDA_PlaceCredsFirst(stored, 4, new_creds, out, out_capacity);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("New", out[0].ssid);
	EXPECT_STREQ("S1", out[1].ssid);
	EXPECT_STREQ("S2", out[2].ssid);
	EXPECT_EQ(0, memcmp(&exp_untouched, &out[3], sizeof(exp_untouched)));
}

TEST(WiFiCredsTest, PlaceCredsFirstOutBufferOfOneHoldsOnlyNew) {
	const size_t exp_count = 1;
	const size_t out_capacity = 1;

	const TWiFiCreds stored[] = {{"S1", "p"}};
	const TWiFiCreds new_creds = {"New", "pn"};
	TWiFiCreds out[2] = {};
	memset(&out[1], 0xAB, sizeof(out[1]));
	TWiFiCreds exp_untouched;
	memset(&exp_untouched, 0xAB, sizeof(exp_untouched));

	size_t res;

	res = HAQuDA_PlaceCredsFirst(stored, 1, new_creds, out, out_capacity);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("New", out[0].ssid);
	EXPECT_EQ(0, memcmp(&exp_untouched, &out[1], sizeof(exp_untouched)));
}

TEST(WiFiCredsTest, PlaceCredsFirstOutCapacityZeroWritesNothing) {
	const size_t exp_count = 0;

	const TWiFiCreds stored[] = {{"S1", "p"}};
	const TWiFiCreds new_creds = {"New", "pn"};
	TWiFiCreds out[1];
	memset(&out[0], 0xAB, sizeof(out[0]));
	TWiFiCreds exp_untouched;
	memset(&exp_untouched, 0xAB, sizeof(exp_untouched));

	size_t res;

	res = HAQuDA_PlaceCredsFirst(stored, 1, new_creds, out, 0);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(0, memcmp(&exp_untouched, &out[0], sizeof(exp_untouched)));
}

TEST(WiFiCredsTest, PlaceCredsFirstEmptyNewNameReturnsZeroAndLeavesOutUntouched) {
	const size_t exp_count = 0;

	const TWiFiCreds stored[] = {{"S1", "p"}};
	const TWiFiCreds new_creds = {"", "pn"};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];
	memset(&out[0], 0xAB, sizeof(out[0]));
	TWiFiCreds exp_untouched;
	memset(&exp_untouched, 0xAB, sizeof(exp_untouched));

	size_t res;

	res = HAQuDA_PlaceCredsFirst(stored, 1, new_creds, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(0, memcmp(&exp_untouched, &out[0], sizeof(exp_untouched)));
}

TEST(WiFiCredsTest, PlaceCredsFirstNullOutReturnsZero) {
	const size_t exp_count = 0;

	const TWiFiCreds stored[] = {{"S1", "p"}};
	const TWiFiCreds new_creds = {"New", "pn"};

	size_t res;

	res = HAQuDA_PlaceCredsFirst(stored, 1, new_creds, nullptr, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
}

TEST(WiFiCredsTest, PlaceCredsFirstNullStoredWithCountGivesOnlyNew) {
	const size_t exp_count = 1;
	const size_t claimed_stored_count = 3;

	const TWiFiCreds new_creds = {"New", "pn"};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_PlaceCredsFirst(nullptr, claimed_stored_count, new_creds, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("New", out[0].ssid);
}

TEST(WiFiCredsTest, PlaceCredsFirstNameOfExactly31BytesIsKeptWhole) {
	const size_t exp_count = 1;
	const char exp_ssid[] = "0123456789012345678901234567890";

	const TWiFiCreds new_creds = {"0123456789012345678901234567890", "pn"};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_PlaceCredsFirst(nullptr, 0, new_creds, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ(exp_ssid, out[0].ssid);
}

TEST(WiFiCredsTest, PlaceCredsFirstSame31ByteNameIsReplaced) {
	const size_t exp_count = 1;

	const TWiFiCreds stored[] = {{"0123456789012345678901234567890", "old"}};
	const TWiFiCreds new_creds = {"0123456789012345678901234567890", "fresh"};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_PlaceCredsFirst(stored, 1, new_creds, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("fresh", out[0].pass);
}

TEST(WiFiCredsTest, PlaceCredsFirst31ByteNamesDifferingInLastByteAreBothKept) {
	const size_t exp_count = 2;

	const TWiFiCreds stored[] = {{"0123456789012345678901234567890", "p1"}};
	const TWiFiCreds new_creds = {"0123456789012345678901234567891", "p2"};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_PlaceCredsFirst(stored, 1, new_creds, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("0123456789012345678901234567891", out[0].ssid);
	EXPECT_STREQ("0123456789012345678901234567890", out[1].ssid);
}

TEST(WiFiCredsTest, PlaceCredsFirstUnterminatedNewNameGetsTerminatorInOutput) {
	const size_t exp_count = 1;

	TWiFiCreds new_creds = {"", "pn"};
	memset(new_creds.ssid, 'x', WIFI_SSID_LEN);
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];
	memset(out, 0xAB, sizeof(out));

	size_t res;

	res = HAQuDA_PlaceCredsFirst(nullptr, 0, new_creds, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ('\0', out[0].ssid[WIFI_SSID_LEN - 1]);
	EXPECT_EQ('x', out[0].ssid[WIFI_SSID_LEN - 2]);
}

TEST(WiFiCredsTest, PlaceCredsFirstUnterminatedStoredNameDoesNotReadPastTheField) {
	const size_t exp_count = 2;

	TWiFiCreds stored[1] = {{"", "p1"}};
	memset(stored[0].ssid, 'y', WIFI_SSID_LEN);
	const TWiFiCreds new_creds = {"New", "pn"};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_PlaceCredsFirst(stored, 1, new_creds, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("New", out[0].ssid);
	EXPECT_EQ('y', out[1].ssid[WIFI_SSID_LEN - 1]);
}

TEST(WiFiCredsTest, PlaceCredsFirstPasswordOf31BytesIsCopiedWhole) {
	const size_t exp_count = 2;
	const char exp_pass[] = "abcdefghijklmnopqrstuvwxyz01234";

	const TWiFiCreds stored[] = {{"Old", "abcdefghijklmnopqrstuvwxyz01234"}};
	const TWiFiCreds new_creds = {"New", "abcdefghijklmnopqrstuvwxyz01234"};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_PlaceCredsFirst(stored, 1, new_creds, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ(exp_pass, out[0].pass);
	EXPECT_STREQ(exp_pass, out[1].pass);
}

TEST(WiFiCredsTest, PlaceCredsFirstEmptyPasswordOfOpenNetworkIsKept) {
	const size_t exp_count = 1;

	const TWiFiCreds new_creds = {"Open", ""};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_PlaceCredsFirst(nullptr, 0, new_creds, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("Open", out[0].ssid);
	EXPECT_STREQ("", out[0].pass);
}

// ---------------------------------------------------------------- HAQuDA_DecideCredsRecovery

TEST(WiFiCredsTest, DecideCredsRecoveryNoTmpAndListPresentIsNone) {
	const creds_recovery exp_action = creds_recovery::none;

	const bool list_exists = true;
	const bool tmp_exists = false;
	const int tmp_size = 0;

	creds_recovery res;

	res = HAQuDA_DecideCredsRecovery(list_exists, tmp_exists, tmp_size);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiCredsTest, DecideCredsRecoveryNoTmpAndNoListIsNone) {
	const creds_recovery exp_action = creds_recovery::none;

	const bool list_exists = false;
	const bool tmp_exists = false;
	const int tmp_size = 0;

	creds_recovery res;

	res = HAQuDA_DecideCredsRecovery(list_exists, tmp_exists, tmp_size);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiCredsTest, DecideCredsRecoveryNoTmpIgnoresItsSize) {
	const creds_recovery exp_action = creds_recovery::none;

	const bool list_exists = false;
	const bool tmp_exists = false;
	const int tmp_size = 64;

	creds_recovery res;

	res = HAQuDA_DecideCredsRecovery(list_exists, tmp_exists, tmp_size);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiCredsTest, DecideCredsRecoveryNoListAndOneWholeRecordRestoresTmp) {
	const creds_recovery exp_action = creds_recovery::restore_tmp;

	const bool list_exists = false;
	const bool tmp_exists = true;
	const int tmp_size = static_cast<int>(sizeof(TWiFiCreds));

	creds_recovery res;

	res = HAQuDA_DecideCredsRecovery(list_exists, tmp_exists, tmp_size);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiCredsTest, DecideCredsRecoveryNoListAndTenWholeRecordsRestoresTmp) {
	const creds_recovery exp_action = creds_recovery::restore_tmp;

	const bool list_exists = false;
	const bool tmp_exists = true;
	const int tmp_size = static_cast<int>(MAX_WIFI_CREDS_NUM * sizeof(TWiFiCreds));

	creds_recovery res;

	res = HAQuDA_DecideCredsRecovery(list_exists, tmp_exists, tmp_size);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiCredsTest, DecideCredsRecoveryNoListAndEmptyTmpDeletesTmp) {
	const creds_recovery exp_action = creds_recovery::delete_tmp;

	const bool list_exists = false;
	const bool tmp_exists = true;
	const int tmp_size = 0;

	creds_recovery res;

	res = HAQuDA_DecideCredsRecovery(list_exists, tmp_exists, tmp_size);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiCredsTest, DecideCredsRecoveryNoListAndOneByteOverARecordDeletesTmp) {
	const creds_recovery exp_action = creds_recovery::delete_tmp;

	const bool list_exists = false;
	const bool tmp_exists = true;
	const int tmp_size = static_cast<int>(sizeof(TWiFiCreds)) + 1;

	creds_recovery res;

	res = HAQuDA_DecideCredsRecovery(list_exists, tmp_exists, tmp_size);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiCredsTest, DecideCredsRecoveryNoListAndOneByteShortOfARecordDeletesTmp) {
	const creds_recovery exp_action = creds_recovery::delete_tmp;

	const bool list_exists = false;
	const bool tmp_exists = true;
	const int tmp_size = static_cast<int>(sizeof(TWiFiCreds)) - 1;

	creds_recovery res;

	res = HAQuDA_DecideCredsRecovery(list_exists, tmp_exists, tmp_size);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiCredsTest, DecideCredsRecoveryNoListAndTmpOf700BytesDeletesTmp) {
	const creds_recovery exp_action = creds_recovery::delete_tmp;

	const bool list_exists = false;
	const bool tmp_exists = true;
	const int tmp_size = 700;

	creds_recovery res;

	res = HAQuDA_DecideCredsRecovery(list_exists, tmp_exists, tmp_size);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiCredsTest, DecideCredsRecoveryNoListAndNegativeSizeDeletesTmp) {
	const creds_recovery exp_action = creds_recovery::delete_tmp;

	const bool list_exists = false;
	const bool tmp_exists = true;
	const int tmp_size = -1;

	creds_recovery res;

	res = HAQuDA_DecideCredsRecovery(list_exists, tmp_exists, tmp_size);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiCredsTest, DecideCredsRecoveryNoListAndWholeRecordsBeyondTheMaximumRestoresTmp) {
	// More than MAX_WIFI_CREDS_NUM whole records are still restored: HAQuDA_FilterRecoveredCreds keeps the first ones
	const creds_recovery exp_action = creds_recovery::restore_tmp;

	const bool list_exists = false;
	const bool tmp_exists = true;
	const int tmp_size = static_cast<int>((MAX_WIFI_CREDS_NUM + 1) * sizeof(TWiFiCreds));

	creds_recovery res;

	res = HAQuDA_DecideCredsRecovery(list_exists, tmp_exists, tmp_size);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiCredsTest, DecideCredsRecoveryListPresentAndWholeTmpDeletesTmp) {
	const creds_recovery exp_action = creds_recovery::delete_tmp;

	const bool list_exists = true;
	const bool tmp_exists = true;
	const int tmp_size = static_cast<int>(sizeof(TWiFiCreds));

	creds_recovery res;

	res = HAQuDA_DecideCredsRecovery(list_exists, tmp_exists, tmp_size);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiCredsTest, DecideCredsRecoveryListPresentAndTenRecordTmpDeletesTmp) {
	const creds_recovery exp_action = creds_recovery::delete_tmp;

	const bool list_exists = true;
	const bool tmp_exists = true;
	const int tmp_size = static_cast<int>(MAX_WIFI_CREDS_NUM * sizeof(TWiFiCreds));

	creds_recovery res;

	res = HAQuDA_DecideCredsRecovery(list_exists, tmp_exists, tmp_size);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiCredsTest, DecideCredsRecoveryListPresentAndEmptyTmpDeletesTmp) {
	const creds_recovery exp_action = creds_recovery::delete_tmp;

	const bool list_exists = true;
	const bool tmp_exists = true;
	const int tmp_size = 0;

	creds_recovery res;

	res = HAQuDA_DecideCredsRecovery(list_exists, tmp_exists, tmp_size);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiCredsTest, DecideCredsRecoveryListPresentAndPartialTmpDeletesTmp) {
	const creds_recovery exp_action = creds_recovery::delete_tmp;

	const bool list_exists = true;
	const bool tmp_exists = true;
	const int tmp_size = 65;

	creds_recovery res;

	res = HAQuDA_DecideCredsRecovery(list_exists, tmp_exists, tmp_size);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiCredsTest, RecordLayoutIs64Bytes) {
	const size_t exp_size = 64;

	size_t res;

	res = sizeof(TWiFiCreds);

	EXPECT_EQ(exp_size, res);
}

// ---------------------------------------------------------------- HAQuDA_FilterRecoveredCreds

TEST(WiFiCredsTest, FilterRecoveredCredsAllValidFewAreKeptInFileOrder) {
	const size_t exp_count = 3;

	const TWiFiCreds records[] = {{"A", "pa"}, {"B", "pb"}, {"C", "pc"}};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_FilterRecoveredCreds(records, 3, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("A", out[0].ssid);
	EXPECT_STREQ("pa", out[0].pass);
	EXPECT_STREQ("B", out[1].ssid);
	EXPECT_STREQ("C", out[2].ssid);
	EXPECT_STREQ("pc", out[2].pass);
}

TEST(WiFiCredsTest, FilterRecoveredCredsTenValidAreAllKept) {
	const size_t exp_count = 10;

	const TWiFiCreds records[]
		= {{"S1", "p"}, {"S2", "p"}, {"S3", "p"}, {"S4", "p"}, {"S5", "p"}, {"S6", "p"}, {"S7", "p"}, {"S8", "p"}, {"S9", "p"}, {"S10", "p"}};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_FilterRecoveredCreds(records, 10, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("S1", out[0].ssid);
	EXPECT_STREQ("S10", out[9].ssid);
}

TEST(WiFiCredsTest, FilterRecoveredCredsTwelveValidKeepFirstTenInOrder) {
	const size_t exp_count = 10;

	const TWiFiCreds records[] = {{"S1", "p"}, {"S2", "p"}, {"S3", "p"}, {"S4", "p"},  {"S5", "p"},	 {"S6", "p"},
								  {"S7", "p"}, {"S8", "p"}, {"S9", "p"}, {"S10", "p"}, {"S11", "p"}, {"S12", "p"}};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_FilterRecoveredCreds(records, 12, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("S1", out[0].ssid);
	EXPECT_STREQ("S9", out[8].ssid);
	EXPECT_STREQ("S10", out[9].ssid);
}

TEST(WiFiCredsTest, FilterRecoveredCredsReadLimitOfValidRecordsKeepsFirstTenInOrder) {
	const size_t exp_count = 10;

	const TWiFiCreds records[CREDS_RECOVERY_READ_MAX]
		= {{"S1", "p"}, {"S2", "p"},  {"S3", "p"},	{"S4", "p"},  {"S5", "p"},	{"S6", "p"},  {"S7", "p"},	{"S8", "p"},
		   {"S9", "p"}, {"S10", "p"}, {"S11", "p"}, {"S12", "p"}, {"S13", "p"}, {"S14", "p"}, {"S15", "p"}, {"S16", "p"}};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_FilterRecoveredCreds(records, CREDS_RECOVERY_READ_MAX, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("S1", out[0].ssid);
	EXPECT_STREQ("S10", out[9].ssid);
}

TEST(WiFiCredsTest, FilterRecoveredCredsEmptySsidIsSkippedAndLaterRecordMovesUp) {
	const size_t exp_count = 2;

	const TWiFiCreds records[] = {{"A", "pa"}, {"", "junk"}, {"B", "pb"}};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_FilterRecoveredCreds(records, 3, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("A", out[0].ssid);
	EXPECT_STREQ("B", out[1].ssid);
}

TEST(WiFiCredsTest, FilterRecoveredCredsUnterminatedSsidIsSkipped) {
	const size_t exp_count = 2;

	TWiFiCreds records[3] = {{"A", "pa"}, {"", "p1"}, {"B", "pb"}};
	memset(records[1].ssid, 'x', WIFI_SSID_LEN);
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_FilterRecoveredCreds(records, 3, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("A", out[0].ssid);
	EXPECT_STREQ("B", out[1].ssid);
}

TEST(WiFiCredsTest, FilterRecoveredCredsUnterminatedPasswordIsSkipped) {
	const size_t exp_count = 2;

	TWiFiCreds records[3] = {{"A", "pa"}, {"Bad", ""}, {"B", "pb"}};
	memset(records[1].pass, 'x', WIFI_PASS_LEN);
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_FilterRecoveredCreds(records, 3, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("A", out[0].ssid);
	EXPECT_STREQ("B", out[1].ssid);
}

TEST(WiFiCredsTest, FilterRecoveredCredsInterleavedInvalidRecordsAreSkippedInOrder) {
	const size_t exp_count = 4;

	TWiFiCreds records[7] = {{"V1", "p1"}, {"", "junk"}, {"V2", "p2"}, {"", ""}, {"V3", ""}, {"Bad", ""}, {"V4", "p4"}};
	memset(records[3].ssid, 'x', WIFI_SSID_LEN);
	memset(records[5].pass, 'y', WIFI_PASS_LEN);
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_FilterRecoveredCreds(records, 7, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("V1", out[0].ssid);
	EXPECT_STREQ("V2", out[1].ssid);
	EXPECT_STREQ("V3", out[2].ssid);
	EXPECT_STREQ("V4", out[3].ssid);
}

TEST(WiFiCredsTest, FilterRecoveredCredsInvalidRecordsDoNotTakeSlotsOfTheTen) {
	const size_t exp_count = 10;

	const TWiFiCreds records[] = {{"", "x"},   {"", "y"},	{"S1", "p"}, {"S2", "p"}, {"S3", "p"},	{"S4", "p"},  {"S5", "p"},
								  {"S6", "p"}, {"S7", "p"}, {"S8", "p"}, {"S9", "p"}, {"S10", "p"}, {"S11", "p"}, {"S12", "p"}};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_FilterRecoveredCreds(records, 14, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("S1", out[0].ssid);
	EXPECT_STREQ("S10", out[9].ssid);
}

TEST(WiFiCredsTest, FilterRecoveredCredsAllInvalidGivesZeroAndLeavesOutUntouched) {
	const size_t exp_count = 0;

	TWiFiCreds records[2] = {{"", "junk"}, {"Bad", ""}};
	memset(records[1].pass, 'x', WIFI_PASS_LEN);
	TWiFiCreds out[2];
	memset(out, 0xAB, sizeof(out));
	TWiFiCreds exp_untouched[2];
	memset(exp_untouched, 0xAB, sizeof(exp_untouched));

	size_t res;

	res = HAQuDA_FilterRecoveredCreds(records, 2, out, 2);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(0, memcmp(exp_untouched, out, sizeof(out)));
}

TEST(WiFiCredsTest, FilterRecoveredCredsZeroRecordsGiveZero) {
	const size_t exp_count = 0;

	const TWiFiCreds records[1] = {{"A", "pa"}};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_FilterRecoveredCreds(records, 0, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
}

TEST(WiFiCredsTest, FilterRecoveredCredsNullRecordsWithZeroCountGiveZero) {
	const size_t exp_count = 0;

	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_FilterRecoveredCreds(nullptr, 0, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
}

TEST(WiFiCredsTest, FilterRecoveredCredsNullRecordsWithCountGiveZeroAndLeaveOutUntouched) {
	const size_t exp_count = 0;
	const size_t claimed_count = 3;

	TWiFiCreds out[1];
	memset(out, 0xAB, sizeof(out));
	TWiFiCreds exp_untouched[1];
	memset(exp_untouched, 0xAB, sizeof(exp_untouched));

	size_t res;

	res = HAQuDA_FilterRecoveredCreds(nullptr, claimed_count, out, 1);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(0, memcmp(exp_untouched, out, sizeof(out)));
}

TEST(WiFiCredsTest, FilterRecoveredCredsNullOutGivesZero) {
	const size_t exp_count = 0;

	const TWiFiCreds records[] = {{"A", "pa"}};

	size_t res;

	res = HAQuDA_FilterRecoveredCreds(records, 1, nullptr, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
}

TEST(WiFiCredsTest, FilterRecoveredCredsEmptyPasswordIsValid) {
	const size_t exp_count = 1;

	const TWiFiCreds records[] = {{"Open", ""}};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_FilterRecoveredCreds(records, 1, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("Open", out[0].ssid);
	EXPECT_STREQ("", out[0].pass);
}

TEST(WiFiCredsTest, FilterRecoveredCredsNameAndPasswordOf31BytesAreValid) {
	const size_t exp_count = 1;
	const char exp_ssid[] = "0123456789012345678901234567890";
	const char exp_pass[] = "abcdefghijklmnopqrstuvwxyz01234";

	const TWiFiCreds records[] = {{"0123456789012345678901234567890", "abcdefghijklmnopqrstuvwxyz01234"}};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_FilterRecoveredCreds(records, 1, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ(exp_ssid, out[0].ssid);
	EXPECT_STREQ(exp_pass, out[0].pass);
}

TEST(WiFiCredsTest, FilterRecoveredCredsSameNameTwiceIsNotMerged) {
	// The filter checks the records one by one; removing duplicates is not its job
	const size_t exp_count = 2;

	const TWiFiCreds records[] = {{"A", "p1"}, {"A", "p2"}};
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	size_t res;

	res = HAQuDA_FilterRecoveredCreds(records, 2, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(exp_count, res);
}

TEST(WiFiCredsTest, FilterRecoveredCredsSmallOutBufferStopsAtItsCapacity) {
	const size_t exp_count = 3;
	const size_t out_capacity = 3;

	const TWiFiCreds records[] = {{"S1", "p"}, {"S2", "p"}, {"S3", "p"}, {"S4", "p"}};
	TWiFiCreds out[4] = {};
	memset(&out[3], 0xAB, sizeof(out[3]));
	TWiFiCreds exp_untouched;
	memset(&exp_untouched, 0xAB, sizeof(exp_untouched));

	size_t res;

	res = HAQuDA_FilterRecoveredCreds(records, 4, out, out_capacity);

	EXPECT_EQ(exp_count, res);
	EXPECT_STREQ("S3", out[2].ssid);
	EXPECT_EQ(0, memcmp(&exp_untouched, &out[3], sizeof(exp_untouched)));
}

TEST(WiFiCredsTest, FilterRecoveredCredsLargeOutBufferIsNotWrittenPastTheMaximum) {
	const size_t exp_count = 10;
	const size_t out_capacity = 12;

	const TWiFiCreds records[] = {{"S1", "p"}, {"S2", "p"}, {"S3", "p"}, {"S4", "p"},  {"S5", "p"},	 {"S6", "p"},
								  {"S7", "p"}, {"S8", "p"}, {"S9", "p"}, {"S10", "p"}, {"S11", "p"}, {"S12", "p"}};
	TWiFiCreds out[out_capacity] = {};
	memset(&out[10], 0xAB, sizeof(out[10]));
	memset(&out[11], 0xAB, sizeof(out[11]));
	TWiFiCreds exp_untouched;
	memset(&exp_untouched, 0xAB, sizeof(exp_untouched));

	size_t res;

	res = HAQuDA_FilterRecoveredCreds(records, 12, out, out_capacity);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(0, memcmp(&exp_untouched, &out[10], sizeof(exp_untouched)));
	EXPECT_EQ(0, memcmp(&exp_untouched, &out[11], sizeof(exp_untouched)));
}

TEST(WiFiCredsTest, FilterRecoveredCredsOutCapacityZeroWritesNothing) {
	const size_t exp_count = 0;

	const TWiFiCreds records[] = {{"A", "pa"}};
	TWiFiCreds out[1];
	memset(&out[0], 0xAB, sizeof(out[0]));
	TWiFiCreds exp_untouched;
	memset(&exp_untouched, 0xAB, sizeof(exp_untouched));

	size_t res;

	res = HAQuDA_FilterRecoveredCreds(records, 1, out, 0);

	EXPECT_EQ(exp_count, res);
	EXPECT_EQ(0, memcmp(&exp_untouched, &out[0], sizeof(exp_untouched)));
}

TEST(WiFiCredsTest, FilterRecoveredCredsDoesNotModifyTheInput) {
	TWiFiCreds records[3] = {{"A", "pa"}, {"", "junk"}, {"Bad", ""}};
	memset(records[2].pass, 'x', WIFI_PASS_LEN);
	TWiFiCreds exp_records[3];
	memcpy(exp_records, records, sizeof(records));
	TWiFiCreds out[MAX_WIFI_CREDS_NUM];

	HAQuDA_FilterRecoveredCreds(records, 3, out, MAX_WIFI_CREDS_NUM);

	EXPECT_EQ(0, memcmp(exp_records, records, sizeof(records)));
}

// ---------------------------------------------------------------- HAQuDA_WipeBytes

TEST(WiFiCredsTest, WipeBytesZeroesExactlyTheRequestedBytes) {
	const size_t wipe_len = 5;
	const uint8_t exp_bytes[8] = {0, 0, 0, 0, 0, 0xAA, 0xAA, 0xAA};

	uint8_t data[8];
	memset(data, 0xAA, sizeof(data));

	HAQuDA_WipeBytes(data, wipe_len);

	EXPECT_EQ(0, memcmp(exp_bytes, data, sizeof(data)));
}

TEST(WiFiCredsTest, WipeBytesOfZeroLengthChangesNothing) {
	const uint8_t exp_bytes[4] = {0xAA, 0xAA, 0xAA, 0xAA};

	uint8_t data[4];
	memset(data, 0xAA, sizeof(data));

	HAQuDA_WipeBytes(data, 0);

	EXPECT_EQ(0, memcmp(exp_bytes, data, sizeof(data)));
}

TEST(WiFiCredsTest, WipeBytesOfWholeBufferZeroesAll) {
	const uint8_t exp_bytes[4] = {0, 0, 0, 0};

	uint8_t data[4];
	memset(data, 0xAA, sizeof(data));

	HAQuDA_WipeBytes(data, sizeof(data));

	EXPECT_EQ(0, memcmp(exp_bytes, data, sizeof(data)));
}

TEST(WiFiCredsTest, WipeBytesOfOneByteLeavesTheNextOne) {
	const uint8_t exp_bytes[2] = {0, 0xAA};

	uint8_t data[2];
	memset(data, 0xAA, sizeof(data));

	HAQuDA_WipeBytes(data, 1);

	EXPECT_EQ(0, memcmp(exp_bytes, data, sizeof(data)));
}

TEST(WiFiCredsTest, WipeBytesClearsAPasswordField) {
	TWiFiCreds creds = {"Home", "secret"};

	HAQuDA_WipeBytes(creds.pass, sizeof(creds.pass));

	EXPECT_STREQ("Home", creds.ssid);
	EXPECT_STREQ("", creds.pass);
}

TEST(WiFiCredsTest, WipeBytesNullPointerDoesNotCrash) {
	const size_t len = 16;

	HAQuDA_WipeBytes(nullptr, len);

	SUCCEED();
}
