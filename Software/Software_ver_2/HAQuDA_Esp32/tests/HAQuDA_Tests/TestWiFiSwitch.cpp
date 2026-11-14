#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>

#include "HAQuDA_WiFiJoin.h"

// Station mode switch: scan -> announce -> leave -> join. Names below: Switch = HAQuDA_JoinCheck::StartSwitch / PollSwitch.

// ---------------------------------------------------------------- HAQuDA_MapScanResult

TEST(WiFiJoinTest, MapScanResultMinusOneIsRunning) {
	const join_scan_status exp_status = join_scan_status::running;

	const int16_t scan_running = -1;

	join_scan_status res;

	res = HAQuDA_MapScanResult(scan_running);

	EXPECT_EQ(exp_status, res);
}

TEST(WiFiJoinTest, MapScanResultMinusTwoIsFailed) {
	const join_scan_status exp_status = join_scan_status::failed;

	const int16_t scan_failed = -2;

	join_scan_status res;

	res = HAQuDA_MapScanResult(scan_failed);

	EXPECT_EQ(exp_status, res);
}

TEST(WiFiJoinTest, MapScanResultOtherNegativeIsFailed) {
	const join_scan_status exp_status = join_scan_status::failed;

	const int16_t scan_unknown_negative = -3;

	join_scan_status res;

	res = HAQuDA_MapScanResult(scan_unknown_negative);

	EXPECT_EQ(exp_status, res);
}

TEST(WiFiJoinTest, MapScanResultLowestValueIsFailed) {
	const join_scan_status exp_status = join_scan_status::failed;

	const int16_t scan_lowest = INT16_MIN;

	join_scan_status res;

	res = HAQuDA_MapScanResult(scan_lowest);

	EXPECT_EQ(exp_status, res);
}

TEST(WiFiJoinTest, MapScanResultZeroIsNotFound) {
	const join_scan_status exp_status = join_scan_status::not_found;

	const int16_t scan_zero_networks = 0;

	join_scan_status res;

	res = HAQuDA_MapScanResult(scan_zero_networks);

	EXPECT_EQ(exp_status, res);
}

TEST(WiFiJoinTest, MapScanResultOneNetworkIsFound) {
	const join_scan_status exp_status = join_scan_status::found;

	const int16_t scan_one_network = 1;

	join_scan_status res;

	res = HAQuDA_MapScanResult(scan_one_network);

	EXPECT_EQ(exp_status, res);
}

TEST(WiFiJoinTest, MapScanResultSeveralNetworksIsFound) {
	const join_scan_status exp_status = join_scan_status::found;

	const int16_t scan_five_networks = 5;

	join_scan_status res;

	res = HAQuDA_MapScanResult(scan_five_networks);

	EXPECT_EQ(exp_status, res);
}

TEST(WiFiJoinTest, MapScanResultHighestValueIsFound) {
	const join_scan_status exp_status = join_scan_status::found;

	const int16_t scan_highest = INT16_MAX;

	join_scan_status res;

	res = HAQuDA_MapScanResult(scan_highest);

	EXPECT_EQ(exp_status, res);
}

// ---------------------------------------------------------------- names

TEST(WiFiJoinTest, JoinStateNameSwitching) {
	const char exp_name[] = "switching";

	const join_state state = join_state::switching;

	const char *res;

	res = HAQuDA_JoinStateName(state);

	EXPECT_STREQ(exp_name, res);
}

TEST(WiFiJoinTest, JoinReasonNameScanFailed) {
	const char exp_name[] = "scan_failed";

	const join_reason reason = join_reason::scan_failed;

	const char *res;

	res = HAQuDA_JoinReasonName(reason);

	EXPECT_STREQ(exp_name, res);
}

TEST(WiFiJoinTest, JoinLastResultNameOk) {
	const char exp_name[] = "ok";

	const join_last_result result = join_last_result::ok;

	const char *res;

	res = HAQuDA_JoinLastResultName(result);

	EXPECT_STREQ(exp_name, res);
}

TEST(WiFiJoinTest, JoinLastResultNameFail) {
	const char exp_name[] = "fail";

	const join_last_result result = join_last_result::fail;

	const char *res;

	res = HAQuDA_JoinLastResultName(result);

	EXPECT_STREQ(exp_name, res);
}

TEST(WiFiJoinTest, JoinLastResultNameNoneIsEmpty) {
	const char exp_name[] = "";

	const join_last_result result = join_last_result::none;

	const char *res;

	res = HAQuDA_JoinLastResultName(result);

	EXPECT_STREQ(exp_name, res);
}

// ---------------------------------------------------------------- HAQuDA_SetJoinLast

TEST(WiFiJoinTest, SetJoinLastFailKeepsReasonAndName) {
	const join_last_result exp_result = join_last_result::fail;
	const join_reason exp_reason = join_reason::no_network;

	join_view view;

	HAQuDA_SetJoinLast(view, join_last_result::fail, join_reason::no_network, "Home", JOIN_SSID_BUF_LEN);

	EXPECT_EQ(exp_result, view.lastResult);
	EXPECT_EQ(exp_reason, view.lastReason);
	EXPECT_STREQ("Home", view.lastSsid);
}

TEST(WiFiJoinTest, SetJoinLastOkDropsTheReason) {
	const join_last_result exp_result = join_last_result::ok;
	const join_reason exp_reason = join_reason::none;

	join_view view;

	HAQuDA_SetJoinLast(view, join_last_result::ok, join_reason::timeout, "Home", JOIN_SSID_BUF_LEN);

	EXPECT_EQ(exp_result, view.lastResult);
	EXPECT_EQ(exp_reason, view.lastReason);
}

TEST(WiFiJoinTest, SetJoinLastNoneClearsResultReasonAndName) {
	const join_last_result exp_result = join_last_result::none;
	const join_reason exp_reason = join_reason::none;

	join_view view;
	HAQuDA_SetJoinLast(view, join_last_result::fail, join_reason::timeout, "Home", JOIN_SSID_BUF_LEN);

	HAQuDA_SetJoinLast(view, join_last_result::none, join_reason::timeout, "Other", JOIN_SSID_BUF_LEN);

	EXPECT_EQ(exp_result, view.lastResult);
	EXPECT_EQ(exp_reason, view.lastReason);
	EXPECT_STREQ("", view.lastSsid);
}

TEST(WiFiJoinTest, SetJoinLastTruncatesLongNameToBufferMinusTerminator) {
	const char exp_ssid[] = "0123456789012345678901234567890";

	join_view view;

	HAQuDA_SetJoinLast(view, join_last_result::ok, join_reason::none, "01234567890123456789012345678901234567", 38);

	EXPECT_STREQ(exp_ssid, view.lastSsid);
}

TEST(WiFiJoinTest, SetJoinLastDoesNotTouchTheCurrentName) {
	join_view view;
	HAQuDA_SetJoinSsid(view, "Current", JOIN_SSID_BUF_LEN);

	HAQuDA_SetJoinLast(view, join_last_result::ok, join_reason::none, "Home", JOIN_SSID_BUF_LEN);

	EXPECT_STREQ("Current", view.ssid);
}

// ---------------------------------------------------------------- StartSwitch

TEST(WiFiJoinTest, SwitchStartReturnsTrue) {
	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;

	bool res;

	res = check.StartSwitch(start_ms);

	EXPECT_TRUE(res);
}

TEST(WiFiJoinTest, SwitchStartIsScanningAndReportsChecking) {
	const join_state exp_state = join_state::checking;

	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;

	check.StartSwitch(start_ms);

	EXPECT_EQ(exp_state, check.State());
	EXPECT_TRUE(check.Scanning());
	EXPECT_FALSE(check.Joining());
	EXPECT_TRUE(check.Active());
}

TEST(WiFiJoinTest, SwitchStartWhileScanningReturnsFalse) {
	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);

	bool res;

	res = check.StartSwitch(start_ms + 5);

	EXPECT_FALSE(res);
}

TEST(WiFiJoinTest, SwitchStartWhileScanningKeepsTheFirstStartTime) {
	const join_state exp_state = join_state::fail;
	const join_reason exp_reason = join_reason::scan_failed;

	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.StartSwitch(start_ms + 5000);

	check.PollSwitch(start_ms + SWITCH_SCAN_TIMEOUT_MS, join_scan_status::running, true);

	EXPECT_EQ(exp_state, check.State());
	EXPECT_EQ(exp_reason, check.Reason());
}

TEST(WiFiJoinTest, SwitchStartWhileAnnouncingReturnsFalse) {
	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(start_ms + 100, join_scan_status::found, true);

	bool res;

	res = check.StartSwitch(start_ms + 200);

	EXPECT_FALSE(res);
}

TEST(WiFiJoinTest, SwitchStartWhileLeavingReturnsFalse) {
	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(start_ms + 100, join_scan_status::found, true);
	check.PollSwitch(start_ms + 100 + SWITCH_ANNOUNCE_MS, join_scan_status::found, true);

	bool res;

	res = check.StartSwitch(start_ms + 100 + SWITCH_ANNOUNCE_MS + 10);

	EXPECT_FALSE(res);
}

TEST(WiFiJoinTest, SwitchStartWhileJoiningReturnsFalse) {
	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(start_ms + 100, join_scan_status::found, true);
	check.PollSwitch(start_ms + 100 + SWITCH_ANNOUNCE_MS, join_scan_status::found, true);
	check.PollSwitch(start_ms + 100 + SWITCH_ANNOUNCE_MS, join_scan_status::found, false);

	bool res;

	res = check.StartSwitch(start_ms + 100 + SWITCH_ANNOUNCE_MS + 10);

	EXPECT_FALSE(res);
}

TEST(WiFiJoinTest, SwitchStartWhileSavingReturnsFalse) {
	const uint32_t start_ms = 1000;
	const uint32_t ip = 0x3201A8C0;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(start_ms + 100, join_scan_status::found, true);
	check.PollSwitch(start_ms + 100 + SWITCH_ANNOUNCE_MS, join_scan_status::found, true);
	check.PollSwitch(start_ms + 100 + SWITCH_ANNOUNCE_MS, join_scan_status::found, false);
	check.Poll(start_ms + 100 + SWITCH_ANNOUNCE_MS + 500, true, ip, 0);

	bool res;

	res = check.StartSwitch(start_ms + 100 + SWITCH_ANNOUNCE_MS + 600);

	EXPECT_FALSE(res);
}

TEST(WiFiJoinTest, SwitchStartWhileAccessPointCheckRunsReturnsFalse) {
	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	bool res;

	res = check.StartSwitch(start_ms + 100);

	EXPECT_FALSE(res);
}

TEST(WiFiJoinTest, StartWhileSwitchScanRunsReturnsFalse) {
	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);

	bool res;

	res = check.Start(start_ms + 100);

	EXPECT_FALSE(res);
}

TEST(WiFiJoinTest, SwitchStartAfterFailedScanReturnsTrueAndScansAgain) {
	const join_state exp_state = join_state::checking;

	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(start_ms + 100, join_scan_status::not_found, true);

	bool res;

	res = check.StartSwitch(start_ms + 200);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_state, check.State());
	EXPECT_TRUE(check.Scanning());
}

TEST(WiFiJoinTest, SwitchStartAfterFailedScanClearsTheReason) {
	const join_reason exp_reason = join_reason::none;

	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(start_ms + 100, join_scan_status::not_found, true);

	check.StartSwitch(start_ms + 200);

	EXPECT_EQ(exp_reason, check.Reason());
}

TEST(WiFiJoinTest, SwitchStartAfterOkClearsTheIp) {
	const uint32_t exp_ip = 0;

	const uint32_t start_ms = 1000;
	const uint32_t ip = 0x3201A8C0;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(start_ms + 100, join_scan_status::found, true);
	check.PollSwitch(start_ms + 100 + SWITCH_ANNOUNCE_MS, join_scan_status::found, true);
	check.PollSwitch(start_ms + 100 + SWITCH_ANNOUNCE_MS, join_scan_status::found, false);
	check.Poll(start_ms + 3000, true, ip, 0);
	check.SaveFinished(true);

	bool res;

	res = check.StartSwitch(start_ms + 4000);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_ip, check.Ip());
}

TEST(WiFiJoinTest, StartAfterSwitchEndedReportsCheckingNotSwitching) {
	const join_state exp_state = join_state::checking;

	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(start_ms + 100, join_scan_status::not_found, true);

	bool res;

	res = check.Start(start_ms + 200);

	EXPECT_TRUE(res);
	EXPECT_EQ(exp_state, check.State());
	EXPECT_TRUE(check.Joining());
}

TEST(WiFiJoinTest, SwitchStartAfterAccessPointCheckEndedReportsSwitchingWhileJoining) {
	const join_state exp_state = join_state::switching;

	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);
	check.Poll(start_ms + JOIN_TIMEOUT_MS, false, 0, 0);
	check.StartSwitch(start_ms + 30000);
	check.PollSwitch(start_ms + 30100, join_scan_status::found, true);
	check.PollSwitch(start_ms + 30100 + SWITCH_ANNOUNCE_MS, join_scan_status::found, true);
	check.PollSwitch(start_ms + 30100 + SWITCH_ANNOUNCE_MS, join_scan_status::found, false);

	EXPECT_EQ(exp_state, check.State());
	EXPECT_TRUE(check.Joining());
}

// ---------------------------------------------------------------- PollSwitch: scanning

TEST(WiFiJoinTest, SwitchPollScanRunningKeepsScanning) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);

	join_action res;

	res = check.PollSwitch(start_ms + 500, join_scan_status::running, true);

	EXPECT_EQ(exp_action, res);
	EXPECT_TRUE(check.Scanning());
	EXPECT_EQ(join_state::checking, check.State());
}

TEST(WiFiJoinTest, SwitchPollScanFoundMovesToAnnouncingAndReportsSwitching) {
	const join_action exp_action = join_action::none;
	const join_state exp_state = join_state::switching;

	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);

	join_action res;

	res = check.PollSwitch(start_ms + 500, join_scan_status::found, true);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(exp_state, check.State());
	EXPECT_FALSE(check.Scanning());
	EXPECT_FALSE(check.Joining());
	EXPECT_TRUE(check.Active());
}

TEST(WiFiJoinTest, SwitchPollScanNotFoundFailsWithNoNetworkAndNoAction) {
	const join_action exp_action = join_action::none;
	const join_state exp_state = join_state::fail;
	const join_reason exp_reason = join_reason::no_network;

	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);

	join_action res;

	res = check.PollSwitch(start_ms + 500, join_scan_status::not_found, true);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(exp_state, check.State());
	EXPECT_EQ(exp_reason, check.Reason());
	EXPECT_FALSE(check.Active());
	EXPECT_FALSE(check.Scanning());
}

TEST(WiFiJoinTest, SwitchPollScanFailedFailsWithScanFailedAndNoAction) {
	const join_action exp_action = join_action::none;
	const join_state exp_state = join_state::fail;
	const join_reason exp_reason = join_reason::scan_failed;

	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);

	join_action res;

	res = check.PollSwitch(start_ms + 500, join_scan_status::failed, true);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(exp_state, check.State());
	EXPECT_EQ(exp_reason, check.Reason());
	EXPECT_FALSE(check.Active());
}

TEST(WiFiJoinTest, SwitchPollScanRunningJustBeforeTimeoutKeepsScanning) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);

	join_action res;

	res = check.PollSwitch(start_ms + SWITCH_SCAN_TIMEOUT_MS - 1, join_scan_status::running, true);

	EXPECT_EQ(exp_action, res);
	EXPECT_TRUE(check.Scanning());
}

TEST(WiFiJoinTest, SwitchPollScanRunningAtTimeoutFailsWithScanFailed) {
	const join_action exp_action = join_action::none;
	const join_state exp_state = join_state::fail;
	const join_reason exp_reason = join_reason::scan_failed;

	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);

	join_action res;

	res = check.PollSwitch(start_ms + SWITCH_SCAN_TIMEOUT_MS, join_scan_status::running, true);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(exp_state, check.State());
	EXPECT_EQ(exp_reason, check.Reason());
}

TEST(WiFiJoinTest, SwitchPollScanFoundOnTimeoutPassStillCounts) {
	const join_state exp_state = join_state::switching;

	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);

	check.PollSwitch(start_ms + SWITCH_SCAN_TIMEOUT_MS, join_scan_status::found, true);

	EXPECT_EQ(exp_state, check.State());
}

TEST(WiFiJoinTest, SwitchPollScanNotFoundOnTimeoutPassGivesNoNetwork) {
	const join_reason exp_reason = join_reason::no_network;

	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);

	check.PollSwitch(start_ms + SWITCH_SCAN_TIMEOUT_MS, join_scan_status::not_found, true);

	EXPECT_EQ(exp_reason, check.Reason());
}

TEST(WiFiJoinTest, SwitchPollScanTimeoutAcrossMillisWrapFails) {
	const join_state exp_state = join_state::fail;
	const join_reason exp_reason = join_reason::scan_failed;

	const uint32_t start_ms = UINT32_MAX - 4999;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);

	check.PollSwitch(start_ms + SWITCH_SCAN_TIMEOUT_MS, join_scan_status::running, true);

	EXPECT_EQ(exp_state, check.State());
	EXPECT_EQ(exp_reason, check.Reason());
}

TEST(WiFiJoinTest, SwitchPollScanRunningJustAfterMillisWrapKeepsScanning) {
	const uint32_t start_ms = UINT32_MAX - 4999;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);

	check.PollSwitch(start_ms + SWITCH_SCAN_TIMEOUT_MS - 1, join_scan_status::running, true);

	EXPECT_TRUE(check.Scanning());
}

TEST(WiFiJoinTest, SwitchPollAfterFailedScanDoesNothing) {
	const join_action exp_action = join_action::none;
	const join_reason exp_reason = join_reason::no_network;

	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(start_ms + 500, join_scan_status::not_found, true);

	join_action res;

	res = check.PollSwitch(start_ms + 600, join_scan_status::found, false);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(join_state::fail, check.State());
	EXPECT_EQ(exp_reason, check.Reason());
}

TEST(WiFiJoinTest, SwitchPollBeforeStartDoesNothing) {
	const join_action exp_action = join_action::none;
	const join_state exp_state = join_state::idle;

	HAQuDA_JoinCheck check;

	join_action res;

	res = check.PollSwitch(5000, join_scan_status::found, false);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(exp_state, check.State());
}

TEST(WiFiJoinTest, SwitchPollDuringAccessPointCheckDoesNothing) {
	const join_action exp_action = join_action::none;
	const join_state exp_state = join_state::checking;

	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.Start(start_ms);

	join_action res;

	res = check.PollSwitch(start_ms + 5000, join_scan_status::failed, false);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(exp_state, check.State());
	EXPECT_TRUE(check.Joining());
}

TEST(WiFiJoinTest, SwitchPollWhileJoiningDoesNothing) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(start_ms + 100, join_scan_status::found, true);
	check.PollSwitch(start_ms + 100 + SWITCH_ANNOUNCE_MS, join_scan_status::found, true);
	check.PollSwitch(start_ms + 100 + SWITCH_ANNOUNCE_MS, join_scan_status::found, false);

	join_action res;

	res = check.PollSwitch(start_ms + 100 + SWITCH_ANNOUNCE_MS + 500, join_scan_status::failed, false);

	EXPECT_EQ(exp_action, res);
	EXPECT_TRUE(check.Joining());
	EXPECT_EQ(join_state::switching, check.State());
}

// ---------------------------------------------------------------- PollSwitch: announcing

TEST(WiFiJoinTest, SwitchPollAnnounceJustBeforeBoundaryReturnsNone) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t found_ms = 2000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);

	join_action res;

	res = check.PollSwitch(found_ms + SWITCH_ANNOUNCE_MS - 1, join_scan_status::found, true);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(join_state::switching, check.State());
}

TEST(WiFiJoinTest, SwitchPollAnnounceAtBoundaryReturnsDisconnectOld) {
	const join_action exp_action = join_action::disconnect_old;

	const uint32_t start_ms = 1000;
	const uint32_t found_ms = 2000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);

	join_action res;

	res = check.PollSwitch(found_ms + SWITCH_ANNOUNCE_MS, join_scan_status::found, true);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(join_state::switching, check.State());
}

TEST(WiFiJoinTest, SwitchPollAnnounceCountsFromTheFoundPassNotFromTheStart) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t found_ms = 5000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);

	join_action res;

	res = check.PollSwitch(found_ms + SWITCH_ANNOUNCE_MS - 1, join_scan_status::found, true);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiJoinTest, SwitchPollAnnounceIgnoresStationStateBeforeBoundary) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t found_ms = 2000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);

	join_action res;

	res = check.PollSwitch(found_ms + 100, join_scan_status::found, false);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(join_state::switching, check.State());
	EXPECT_FALSE(check.Joining());
}

TEST(WiFiJoinTest, SwitchPollAnnounceIgnoresScanStatusAfterFound) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t found_ms = 2000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);

	join_action res;

	res = check.PollSwitch(found_ms + 100, join_scan_status::failed, true);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(join_state::switching, check.State());
}

TEST(WiFiJoinTest, SwitchPollAnnounceBoundaryAcrossMillisWrapReturnsDisconnectOld) {
	const join_action exp_action = join_action::disconnect_old;

	const uint32_t start_ms = UINT32_MAX - 2000;
	const uint32_t found_ms = UINT32_MAX - 1000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);

	join_action res;

	res = check.PollSwitch(found_ms + SWITCH_ANNOUNCE_MS, join_scan_status::found, true);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiJoinTest, SwitchPollAnnounceJustBeforeBoundaryAcrossMillisWrapReturnsNone) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = UINT32_MAX - 2000;
	const uint32_t found_ms = UINT32_MAX - 1000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);

	join_action res;

	res = check.PollSwitch(found_ms + SWITCH_ANNOUNCE_MS - 1, join_scan_status::found, true);

	EXPECT_EQ(exp_action, res);
}

// ---------------------------------------------------------------- PollSwitch: leaving

TEST(WiFiJoinTest, SwitchPollLeavingStationStillUpKeepsWaiting) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t found_ms = 2000;
	const uint32_t leave_ms = found_ms + SWITCH_ANNOUNCE_MS;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms, join_scan_status::found, true);

	join_action res;

	res = check.PollSwitch(leave_ms + 100, join_scan_status::found, true);

	EXPECT_EQ(exp_action, res);
	EXPECT_FALSE(check.Joining());
	EXPECT_EQ(join_state::switching, check.State());
}

TEST(WiFiJoinTest, SwitchPollLeavingStationDownReturnsBeginJoin) {
	const join_action exp_action = join_action::begin_join;

	const uint32_t start_ms = 1000;
	const uint32_t found_ms = 2000;
	const uint32_t leave_ms = found_ms + SWITCH_ANNOUNCE_MS;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms, join_scan_status::found, true);

	join_action res;

	res = check.PollSwitch(leave_ms + 100, join_scan_status::found, false);

	EXPECT_EQ(exp_action, res);
	EXPECT_TRUE(check.Joining());
	EXPECT_EQ(join_state::switching, check.State());
}

TEST(WiFiJoinTest, SwitchPollLeavingStationDownOnTheSamePassReturnsBeginJoin) {
	const join_action exp_action = join_action::begin_join;

	const uint32_t start_ms = 1000;
	const uint32_t found_ms = 2000;
	const uint32_t leave_ms = found_ms + SWITCH_ANNOUNCE_MS;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms, join_scan_status::found, true);

	join_action res;

	res = check.PollSwitch(leave_ms, join_scan_status::found, false);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiJoinTest, SwitchPollDisconnectOldIsReturnedOnce) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t found_ms = 2000;
	const uint32_t leave_ms = found_ms + SWITCH_ANNOUNCE_MS;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms, join_scan_status::found, true);

	join_action res;

	res = check.PollSwitch(leave_ms, join_scan_status::found, true);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiJoinTest, SwitchPollLeavingStationUpJustBeforeCapKeepsWaiting) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t found_ms = 2000;
	const uint32_t leave_ms = found_ms + SWITCH_ANNOUNCE_MS;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms, join_scan_status::found, true);

	join_action res;

	res = check.PollSwitch(leave_ms + SWITCH_LEAVE_TIMEOUT_MS - 1, join_scan_status::found, true);

	EXPECT_EQ(exp_action, res);
	EXPECT_FALSE(check.Joining());
}

TEST(WiFiJoinTest, SwitchPollLeavingStationUpAtCapReturnsBeginJoin) {
	const join_action exp_action = join_action::begin_join;

	const uint32_t start_ms = 1000;
	const uint32_t found_ms = 2000;
	const uint32_t leave_ms = found_ms + SWITCH_ANNOUNCE_MS;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms, join_scan_status::found, true);

	join_action res;

	res = check.PollSwitch(leave_ms + SWITCH_LEAVE_TIMEOUT_MS, join_scan_status::found, true);

	EXPECT_EQ(exp_action, res);
	EXPECT_TRUE(check.Joining());
}

TEST(WiFiJoinTest, SwitchPollLeavingCapAcrossMillisWrapReturnsBeginJoin) {
	const join_action exp_action = join_action::begin_join;

	const uint32_t start_ms = UINT32_MAX - 6000;
	const uint32_t found_ms = UINT32_MAX - 5000;
	const uint32_t leave_ms = found_ms + SWITCH_ANNOUNCE_MS;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms, join_scan_status::found, true);

	join_action res;

	res = check.PollSwitch(leave_ms + SWITCH_LEAVE_TIMEOUT_MS, join_scan_status::found, true);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiJoinTest, SwitchPollBeginJoinIsReturnedOnce) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t found_ms = 2000;
	const uint32_t leave_ms = found_ms + SWITCH_ANNOUNCE_MS;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms + 100, join_scan_status::found, false);

	join_action res;

	res = check.PollSwitch(leave_ms + 200, join_scan_status::found, false);

	EXPECT_EQ(exp_action, res);
}

// ---------------------------------------------------------------- Poll after begin_join

TEST(WiFiJoinTest, SwitchJoinTimeoutCountsFromBeginJoinJustBefore) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t found_ms = 2000;
	const uint32_t leave_ms = found_ms + SWITCH_ANNOUNCE_MS;
	const uint32_t join_ms = leave_ms + 700;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms, join_scan_status::found, true);
	check.PollSwitch(join_ms, join_scan_status::found, false);

	join_action res;

	res = check.Poll(join_ms + JOIN_TIMEOUT_MS - 1, false, 0, 0);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(join_state::switching, check.State());
}

TEST(WiFiJoinTest, SwitchJoinTimeoutAtDeadlineReturnsLeaveStationAndFailsWithTimeout) {
	const join_action exp_action = join_action::leave_station;
	const join_state exp_state = join_state::fail;
	const join_reason exp_reason = join_reason::timeout;

	const uint32_t start_ms = 1000;
	const uint32_t found_ms = 2000;
	const uint32_t leave_ms = found_ms + SWITCH_ANNOUNCE_MS;
	const uint32_t join_ms = leave_ms + 700;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms, join_scan_status::found, true);
	check.PollSwitch(join_ms, join_scan_status::found, false);

	join_action res;

	res = check.Poll(join_ms + JOIN_TIMEOUT_MS, false, 0, 0);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(exp_state, check.State());
	EXPECT_EQ(exp_reason, check.Reason());
}

TEST(WiFiJoinTest, SwitchJoinTimeoutDoesNotCountFromTheStartOfTheSwitch) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t found_ms = 2000;
	const uint32_t leave_ms = found_ms + SWITCH_ANNOUNCE_MS;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms + SWITCH_LEAVE_TIMEOUT_MS, join_scan_status::found, true);

	join_action res;

	res = check.Poll(start_ms + JOIN_TIMEOUT_MS, false, 0, 0);

	EXPECT_EQ(exp_action, res);
}

TEST(WiFiJoinTest, SwitchJoinTimeoutUsesTheLastDisconnectReason) {
	const join_reason exp_reason = join_reason::wrong_password;

	const uint32_t start_ms = 1000;
	const uint32_t found_ms = 2000;
	const uint32_t leave_ms = found_ms + SWITCH_ANNOUNCE_MS;
	const uint8_t code_auth_fail = 202;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms + 100, join_scan_status::found, false);

	check.Poll(leave_ms + 100 + JOIN_TIMEOUT_MS, false, 0, code_auth_fail);

	EXPECT_EQ(exp_reason, check.Reason());
}

TEST(WiFiJoinTest, SwitchJoinEarlyFailStreakReturnsLeaveStationAtOnce) {
	const join_action exp_action = join_action::leave_station;
	const join_state exp_state = join_state::fail;
	const join_reason exp_reason = join_reason::wrong_password;

	const uint32_t start_ms = 1000;
	const uint32_t found_ms = 2000;
	const uint32_t leave_ms = found_ms + SWITCH_ANNOUNCE_MS;
	const uint8_t code_auth_fail = 202;
	uint32_t streak = 0;
	streak = HAQuDA_AddDisconnectEvent(streak, code_auth_fail);
	streak = HAQuDA_AddDisconnectEvent(streak, code_auth_fail);
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms + 100, join_scan_status::found, false);

	join_action res;

	res = check.Poll(leave_ms + 200, false, 0, code_auth_fail, streak);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(exp_state, check.State());
	EXPECT_EQ(exp_reason, check.Reason());
}

TEST(WiFiJoinTest, SwitchJoinEarlyFailStreakOfNoNetworkFailsWithNoNetwork) {
	const join_reason exp_reason = join_reason::no_network;

	const uint32_t start_ms = 1000;
	const uint32_t found_ms = 2000;
	const uint32_t leave_ms = found_ms + SWITCH_ANNOUNCE_MS;
	const uint8_t code_no_ap_found = 201;
	uint32_t streak = 0;
	streak = HAQuDA_AddDisconnectEvent(streak, code_no_ap_found);
	streak = HAQuDA_AddDisconnectEvent(streak, code_no_ap_found);
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms + 100, join_scan_status::found, false);

	check.Poll(leave_ms + 200, false, 0, code_no_ap_found, streak);

	EXPECT_EQ(exp_reason, check.Reason());
}

TEST(WiFiJoinTest, SwitchJoinSingleDisconnectEventKeepsJoining) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t found_ms = 2000;
	const uint32_t leave_ms = found_ms + SWITCH_ANNOUNCE_MS;
	const uint8_t code_auth_fail = 202;
	uint32_t streak = 0;
	streak = HAQuDA_AddDisconnectEvent(streak, code_auth_fail);
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms + 100, join_scan_status::found, false);

	join_action res;

	res = check.Poll(leave_ms + 200, false, 0, code_auth_fail, streak);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(join_state::switching, check.State());
}

TEST(WiFiJoinTest, SwitchJoinConnectedWithIpReturnsSaveNetworkAndStaysSwitching) {
	const join_action exp_action = join_action::save_network;
	const join_state exp_state = join_state::switching;
	const uint32_t exp_ip = 0x3201A8C0;

	const uint32_t start_ms = 1000;
	const uint32_t found_ms = 2000;
	const uint32_t leave_ms = found_ms + SWITCH_ANNOUNCE_MS;
	const uint32_t ip = 0x3201A8C0;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms + 100, join_scan_status::found, false);

	join_action res;

	res = check.Poll(leave_ms + 3000, true, ip, 0);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(exp_state, check.State());
	EXPECT_EQ(exp_ip, check.Ip());
}

TEST(WiFiJoinTest, SwitchJoinSaveFinishedTrueGivesOkWithIp) {
	const join_state exp_state = join_state::ok;
	const uint32_t exp_ip = 0x3201A8C0;

	const uint32_t start_ms = 1000;
	const uint32_t found_ms = 2000;
	const uint32_t leave_ms = found_ms + SWITCH_ANNOUNCE_MS;
	const uint32_t ip = 0x3201A8C0;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms + 100, join_scan_status::found, false);
	check.Poll(leave_ms + 3000, true, ip, 0);

	check.SaveFinished(true);

	EXPECT_EQ(exp_state, check.State());
	EXPECT_EQ(exp_ip, check.Ip());
	EXPECT_FALSE(check.Active());
}

TEST(WiFiJoinTest, SwitchJoinSaveFinishedFalseReturnsLeaveStationAndFailsWithSaveFailed) {
	const join_action exp_action = join_action::leave_station;
	const join_state exp_state = join_state::fail;
	const join_reason exp_reason = join_reason::save_failed;

	const uint32_t start_ms = 1000;
	const uint32_t found_ms = 2000;
	const uint32_t leave_ms = found_ms + SWITCH_ANNOUNCE_MS;
	const uint32_t ip = 0x3201A8C0;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms + 100, join_scan_status::found, false);
	check.Poll(leave_ms + 3000, true, ip, 0);

	join_action res;

	res = check.SaveFinished(false);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(exp_state, check.State());
	EXPECT_EQ(exp_reason, check.Reason());
}

TEST(WiFiJoinTest, SwitchPollWhileScanningIgnoresAConnectedStation) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t ip = 0x3201A8C0;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);

	join_action res;

	res = check.Poll(start_ms + 100, true, ip, 0);

	EXPECT_EQ(exp_action, res);
	EXPECT_TRUE(check.Scanning());
}

TEST(WiFiJoinTest, SwitchPollWhileAnnouncingIgnoresAConnectedStation) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t found_ms = 2000;
	const uint32_t ip = 0x3201A8C0;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);

	join_action res;

	res = check.Poll(found_ms + 100, true, ip, 0);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(join_state::switching, check.State());
	EXPECT_EQ(0u, check.Ip());
}

TEST(WiFiJoinTest, SwitchPollWhileLeavingIgnoresAConnectedStation) {
	const join_action exp_action = join_action::none;

	const uint32_t start_ms = 1000;
	const uint32_t found_ms = 2000;
	const uint32_t leave_ms = found_ms + SWITCH_ANNOUNCE_MS;
	const uint32_t ip = 0x3201A8C0;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(found_ms, join_scan_status::found, true);
	check.PollSwitch(leave_ms, join_scan_status::found, true);

	join_action res;

	res = check.Poll(leave_ms + 100, true, ip, 0);

	EXPECT_EQ(exp_action, res);
	EXPECT_EQ(0u, check.Ip());
}

TEST(WiFiJoinTest, SwitchPollAfterTheNewSwitchStartedForgetsTheOldReason) {
	const join_reason exp_reason = join_reason::none;

	const uint32_t start_ms = 1000;
	HAQuDA_JoinCheck check;
	check.StartSwitch(start_ms);
	check.PollSwitch(start_ms + 100, join_scan_status::failed, true);
	check.StartSwitch(start_ms + 200);

	check.PollSwitch(start_ms + 300, join_scan_status::running, true);

	EXPECT_EQ(exp_reason, check.Reason());
	EXPECT_TRUE(check.Scanning());
}

// ---------------------------------------------------------------- HAQuDA_BuildJoinJson: switching and last

TEST(WiFiJoinTest, BuildJoinJsonSwitchingHasStateSsidAndHostOnly) {
	const char exp_json[] = "{\"state\":\"switching\",\"ssid\":\"New\",\"host\":\"haquda.local\"}";

	const char *host = "haquda.local";
	join_view view;
	view.state = join_state::switching;
	view.reason = join_reason::timeout; // stale value, must not be shown
	view.ip = 0x3201A8C0;				// stale value, must not be shown
	view.restartMs = 777;				// stale value, must not be shown
	HAQuDA_SetJoinSsid(view, "New", JOIN_SSID_BUF_LEN);
	char buf[256];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonLastOkHasResultAndSsidWithoutReason) {
	const char exp_json[] = "{\"state\":\"idle\",\"ssid\":\"\",\"host\":\"haquda.local\",\"last\":{\"result\":\"ok\",\"ssid\":\"Old\"}}";

	const char *host = "haquda.local";
	join_view view;
	HAQuDA_SetJoinLast(view, join_last_result::ok, join_reason::none, "Old", JOIN_SSID_BUF_LEN);
	char buf[256];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonLastOkHidesAStaleReason) {
	const char exp_json[] = "{\"state\":\"idle\",\"ssid\":\"\",\"host\":\"haquda.local\",\"last\":{\"result\":\"ok\",\"ssid\":\"Old\"}}";

	const char *host = "haquda.local";
	join_view view;
	view.lastResult = join_last_result::ok;
	view.lastReason = join_reason::timeout; // set directly, bypassing HAQuDA_SetJoinLast
	memcpy(view.lastSsid, "Old", sizeof("Old"));
	char buf[256];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonLastFailHasResultReasonAndSsid) {
	const char exp_json[]
		= "{\"state\":\"idle\",\"ssid\":\"\",\"host\":\"haquda.local\",\"last\":{\"result\":\"fail\",\"reason\":\"wrong_password\",\"ssid\":\"Old\"}}";

	const char *host = "haquda.local";
	join_view view;
	HAQuDA_SetJoinLast(view, join_last_result::fail, join_reason::wrong_password, "Old", JOIN_SSID_BUF_LEN);
	char buf[256];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonLastFailScanFailedReason) {
	const char exp_json[]
		= "{\"state\":\"idle\",\"ssid\":\"\",\"host\":\"haquda.local\",\"last\":{\"result\":\"fail\",\"reason\":\"scan_failed\",\"ssid\":\"Old\"}}";

	const char *host = "haquda.local";
	join_view view;
	HAQuDA_SetJoinLast(view, join_last_result::fail, join_reason::scan_failed, "Old", JOIN_SSID_BUF_LEN);
	char buf[256];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonLastNoneIsOmittedEvenWithStaleFields) {
	const char exp_json[] = "{\"state\":\"idle\",\"ssid\":\"\",\"host\":\"haquda.local\"}";

	const char *host = "haquda.local";
	join_view view;
	view.lastResult = join_last_result::none;
	view.lastReason = join_reason::timeout; // stale value, must not be shown
	memcpy(view.lastSsid, "Old", sizeof("Old"));
	char buf[256];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonLastSsidQuoteAndBackslashAreEscaped) {
	const char exp_json[] = "{\"state\":\"idle\",\"ssid\":\"\",\"host\":\"haquda.local\",\"last\":{\"result\":\"ok\",\"ssid\":\"a\\\"b\\\\c\"}}";

	const char *host = "haquda.local";
	join_view view;
	HAQuDA_SetJoinLast(view, join_last_result::ok, join_reason::none, "a\"b\\c", JOIN_SSID_BUF_LEN);
	char buf[256];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonLastSsidControlCharacterIsEscaped) {
	const char exp_json[] = "{\"state\":\"idle\",\"ssid\":\"\",\"host\":\"haquda.local\",\"last\":{\"result\":\"ok\",\"ssid\":\"a\\u000ab\"}}";

	const char *host = "haquda.local";
	join_view view;
	HAQuDA_SetJoinLast(view, join_last_result::ok, join_reason::none, "a\nb", JOIN_SSID_BUF_LEN);
	char buf[256];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonLastSsidOf31BytesFits) {
	const char exp_json[]
		= "{\"state\":\"idle\",\"ssid\":\"\",\"host\":\"haquda.local\",\"last\":{\"result\":\"ok\",\"ssid\":\"0123456789012345678901234567890\"}}";

	const char *host = "haquda.local";
	join_view view;
	HAQuDA_SetJoinLast(view, join_last_result::ok, join_reason::none, "0123456789012345678901234567890", JOIN_SSID_BUF_LEN);
	char buf[256];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonLastSsidOf31QuotesIsEscapedWhole) {
	const char exp_json[] = "{\"state\":\"idle\",\"ssid\":\"\",\"host\":\"haquda.local\",\"last\":{\"result\":\"ok\",\"ssid\":\""
							"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\\\"\"}}";

	const char *host = "haquda.local";
	join_view view;
	view.lastResult = join_last_result::ok;
	memset(view.lastSsid, '"', JOIN_SSID_BUF_LEN - 1);
	char buf[512];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonFailStateWithLastFailHasBothReasonsInOrder) {
	const char exp_json[] = "{\"state\":\"fail\",\"ssid\":\"New\",\"host\":\"haquda.local\",\"reason\":\"no_network\","
							"\"last\":{\"result\":\"fail\",\"reason\":\"timeout\",\"ssid\":\"Old\"}}";

	const char *host = "haquda.local";
	join_view view;
	view.state = join_state::fail;
	view.reason = join_reason::no_network;
	HAQuDA_SetJoinSsid(view, "New", JOIN_SSID_BUF_LEN);
	HAQuDA_SetJoinLast(view, join_last_result::fail, join_reason::timeout, "Old", JOIN_SSID_BUF_LEN);
	char buf[256];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonOkStateWithLastOkHasIpRestartMsThenLast) {
	const char exp_json[] = "{\"state\":\"ok\",\"ssid\":\"New\",\"ip\":\"192.168.1.50\",\"restart_ms\":1000,\"host\":\"haquda.local\","
							"\"last\":{\"result\":\"ok\",\"ssid\":\"Old\"}}";

	const char *host = "haquda.local";
	join_view view;
	view.state = join_state::ok;
	view.ip = 0x3201A8C0;
	view.restartMs = 1000;
	HAQuDA_SetJoinSsid(view, "New", JOIN_SSID_BUF_LEN);
	HAQuDA_SetJoinLast(view, join_last_result::ok, join_reason::none, "Old", JOIN_SSID_BUF_LEN);
	char buf[256];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonSwitchingWithLastHasLast) {
	const char exp_json[] = "{\"state\":\"switching\",\"ssid\":\"New\",\"host\":\"haquda.local\","
							"\"last\":{\"result\":\"ok\",\"ssid\":\"Old\"}}";

	const char *host = "haquda.local";
	join_view view;
	view.state = join_state::switching;
	HAQuDA_SetJoinSsid(view, "New", JOIN_SSID_BUF_LEN);
	HAQuDA_SetJoinLast(view, join_last_result::ok, join_reason::none, "Old", JOIN_SSID_BUF_LEN);
	char buf[256];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonFailStateScanFailedReasonIsShown) {
	const char exp_json[] = "{\"state\":\"fail\",\"ssid\":\"New\",\"host\":\"haquda.local\",\"reason\":\"scan_failed\"}";

	const char *host = "haquda.local";
	join_view view;
	view.state = join_state::fail;
	view.reason = join_reason::scan_failed;
	HAQuDA_SetJoinSsid(view, "New", JOIN_SSID_BUF_LEN);
	char buf[256];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonWithLastBufferOfExactSizeFits) {
	const char exp_json[] = "{\"state\":\"idle\",\"ssid\":\"\",\"host\":\"h\",\"last\":{\"result\":\"ok\",\"ssid\":\"Old\"}}";

	const char *host = "h";
	join_view view;
	HAQuDA_SetJoinLast(view, join_last_result::ok, join_reason::none, "Old", JOIN_SSID_BUF_LEN);
	char buf[sizeof(exp_json)];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_TRUE(res);
	EXPECT_STREQ(exp_json, buf);
}

TEST(WiFiJoinTest, BuildJoinJsonWithLastBufferOneByteTooSmallReturnsFalseAndEmpty) {
	const char exp_json[] = "{\"state\":\"idle\",\"ssid\":\"\",\"host\":\"h\",\"last\":{\"result\":\"ok\",\"ssid\":\"Old\"}}";

	const char *host = "h";
	join_view view;
	HAQuDA_SetJoinLast(view, join_last_result::ok, join_reason::none, "Old", JOIN_SSID_BUF_LEN);
	char buf[sizeof(exp_json) - 1];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_FALSE(res);
	EXPECT_STREQ("", buf);
}

TEST(WiFiJoinTest, BuildJoinJsonBufferCutInsideLastReturnsFalseAndEmpty) {
	const char *host = "h";
	join_view view;
	HAQuDA_SetJoinLast(view, join_last_result::fail, join_reason::timeout, "Old", JOIN_SSID_BUF_LEN);
	char buf[60];

	bool res;

	res = HAQuDA_BuildJoinJson(buf, sizeof(buf), view, host);

	EXPECT_FALSE(res);
	EXPECT_STREQ("", buf);
}
