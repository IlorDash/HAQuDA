#pragma once

#include <cstddef>
#include <cstdint>

// Hardware-free logic of the "check a new WiFi network before saving it" feature. Two flows share it:
//  - access point mode: the station joins the new network next to the running access point, the result is saved and the
//    lamp restarts after a hold (Start / Poll / SaveFinished);
//  - station mode: the lamp scans for the network while it stays connected, announces the switch, leaves its current
//    network, joins the new one, saves it as the first record and restarts at once (StartSwitch / PollSwitch / Poll /
//    SaveFinished). Every outcome after the lamp has left its network ends in a restart.
// The WiFi handler feeds the state machine with the station state and acts on the actions it returns; the web server shows
// the result.

// How long the station may take to join before the check fails
constexpr uint32_t JOIN_TIMEOUT_MS = 20 * 1000;
// After a successful check the lamp keeps its access point up this long, so the phone can fetch the result, then restarts
constexpr uint32_t JOIN_OK_HOLD_MS = 20 * 1000;
// Station mode: the scan for the new network gives up after this long. The core ends a scan by itself after 6 s; this is the
// state machine's own limit for a scan that never reports.
constexpr uint32_t SWITCH_SCAN_TIMEOUT_MS = 10 * 1000;
// Station mode: /api/join reports "switching" this long before the lamp leaves its network, so a page that polls every 2 s
// sees it and can warn the user
constexpr uint32_t SWITCH_ANNOUNCE_MS = 2500;
// Station mode: the lamp waits this long for its old connection to drop before it joins the new network anyway
constexpr uint32_t SWITCH_LEAVE_TIMEOUT_MS = 3000;
// Station mode: delay between the end of the switch (passed or failed) and the restart. The phone cannot reach the lamp in
// the new network anyway, so there is no hold.
constexpr uint32_t SWITCH_RESTART_DELAY_MS = 1000;
// Bytes of the network name the check keeps, terminator included (the lamp stores 31 + terminator)
constexpr size_t JOIN_SSID_BUF_LEN = 32;

// switching: station mode only, from the moment the scan found the network until the check ends
enum class join_state : uint8_t { idle, checking, ok, fail, switching };
// scan_failed: station mode only, the scan itself failed (the lamp could not search for networks), which tells nothing
// about the network
enum class join_reason : uint8_t { none, wrong_password, no_network, timeout, save_failed, scan_failed };
// disconnect_old and begin_join: station mode only
enum class join_action : uint8_t { none, save_network, leave_station, disconnect_old, begin_join };
// Outcome of an earlier switch that survived the restart: none = nothing to report
enum class join_last_result : uint8_t { none, ok, fail };
// What the asynchronous scan reports (see HAQuDA_MapScanResult)
enum class join_scan_status : uint8_t { running, found, not_found, failed };

// What /api/join reports. Written by the loop task and copied by the web server task under a lock.
struct join_view {
	join_state state = join_state::idle;
	join_reason reason = join_reason::none;
	// Station address of the joined network, first octet in the lowest byte (the value lwIP and IPAddress use)
	uint32_t ip = 0;
	// Milliseconds until the lamp restarts, filled in by the web server when it answers; only shown in state ok
	uint32_t restartMs = 0;
	char ssid[JOIN_SSID_BUF_LEN] = {0};
	// The last station-mode switch that ended in a restart. none = nothing to report. reason is set only for fail.
	join_last_result lastResult = join_last_result::none;
	join_reason lastReason = join_reason::none;
	char lastSsid[JOIN_SSID_BUF_LEN] = {0};
};

// What /api/join answers to a request that cannot start a check
enum class join_queue_result : uint8_t { queued, checking, restarting };

// Maps the reason code of a station disconnect event (wifi_err_reason_t of ESP-IDF 4.4) to what the user can act on:
// the password was refused, the network was not found, or anything else, which is reported as a timeout.
// AUTH_EXPIRE (2) counts as anything else: routers send it for reasons that have nothing to do with the password.
join_reason HAQuDA_MapDisconnectReason(uint8_t code);

// Picks the disconnect reason code to keep when a new one arrives during a check. A refused password outranks a missing
// network (the router answers a retry with a late NO_AP_FOUND after the handshake failed), a missing network outranks
// anything else, and a code that tells nothing never replaces one that does. current 0 = nothing kept yet.
// Returns the code to keep (0 when neither tells anything).
uint8_t HAQuDA_MergeDisconnectReason(uint8_t current, uint8_t incoming);

// A check fails at once after this many consecutive disconnect events of the same telling class: the core retries a join
// every ~3 s, and a refused password or a missing network answers the same way every time
constexpr uint8_t JOIN_EARLY_FAIL_EVENTS = 2;

// Streak of consecutive disconnect events of the same class, packed into one word so the WiFi event task can keep it in
// a single std::atomic (one writer): class in bits 8..15, count in bits 0..7; 0 = empty.
// Returns the streak after one more event with reason code. A code of the wrong_password or no_network class extends
// the streak of that class or starts one; any other code ends the streak (returns 0), except ASSOC_LEAVE (8), the
// voluntary disconnect the core issues before each retry and on our own disconnect, which changes nothing.
uint32_t HAQuDA_AddDisconnectEvent(uint32_t streak, uint8_t code);

// The reason the streak decides: wrong_password or no_network once it holds JOIN_EARLY_FAIL_EVENTS events, else none
join_reason HAQuDA_StreakReason(uint32_t streak);

// Maps what WiFi.scanComplete() returns (-1 still running, -2 failed or never started, 0 or more = number of networks the
// scan found) to a scan status. The scan is asked for the one network name, so any network found is the one wanted.
join_scan_status HAQuDA_MapScanResult(int16_t scanComplete);

// "idle", "checking", "ok", "fail", "switching"
const char *HAQuDA_JoinStateName(join_state state);
// "wrong_password", "no_network", "timeout", "save_failed", "scan_failed"; an empty string for none
const char *HAQuDA_JoinReasonName(join_reason reason);
// "ok", "fail"; an empty string for none
const char *HAQuDA_JoinLastResultName(join_last_result result);

// Copies at most JOIN_SSID_BUF_LEN - 1 bytes of the NUL-terminated (or maxLen-bounded) name into the view, terminated
void HAQuDA_SetJoinSsid(join_view &view, const char *ssid, size_t maxLen);

// Sets the outcome of the last switch in the view (the name is copied like HAQuDA_SetJoinSsid does). reason is kept only
// for fail. Passing none clears it.
void HAQuDA_SetJoinLast(join_view &view, join_last_result result, join_reason reason, const char *ssid, size_t maxLen);

// Writes the JSON string body (no quotes) of a possibly unterminated text of at most maxLen bytes into out. Quote and
// backslash get a backslash, control characters become \u00XX. Stops when out is full; out is always terminated.
void HAQuDA_EscapeJson(const char *text, size_t maxLen, char *out, size_t outSize);

// Writes {"state":"..","ssid":"..","ip":"..","restart_ms":N,"host":"..","reason":"..","last":{..}} into buf; "ip" and
// "restart_ms" only in state ok, "reason" only in state fail. "last" only when view.lastResult is not none:
// {"result":"ok"|"fail","reason":"..","ssid":".."}, with "reason" only for fail. host is written as it is, so it must hold no quote,
// backslash or control character. Returns false (buf terminated and empty) when the text does not fit.
bool HAQuDA_BuildJoinJson(char *buf, size_t size, const join_view &view, const char *host);

// One check of a network: joining, saving, then the result. Time is passed in, nothing is read from hardware.
// All methods: loop task only.
class HAQuDA_JoinCheck {
  public:
	// Begins an access point mode check (the join starts at once) at nowMs. Returns false and changes nothing while a
	// check or a switch is running.
	bool Start(uint32_t nowMs);

	// Begins a station mode switch at nowMs; the caller has started the scan for the network. Returns false and changes
	// nothing while a check or a switch is running.
	bool StartSwitch(uint32_t nowMs);

	// Station mode, call on every pass while Scanning() or in the announce and leave phases; elsewhere it does nothing.
	// scan = what the scan reports (used while scanning), stationUp = the station is connected (used while leaving).
	// A scan that finds no network or fails ends the switch in state fail and returns none: the lamp never left its
	// network, so nothing is to be undone and no restart is needed. A found network moves on to the announce phase
	// (State() becomes switching). After SWITCH_ANNOUNCE_MS disconnect_old is returned once: disconnect from the current
	// network. When the old connection is gone (or after SWITCH_LEAVE_TIMEOUT_MS) begin_join is returned once: start
	// joining the new network; the join timeout counts from this call.
	join_action PollSwitch(uint32_t nowMs, join_scan_status scan, bool stationUp);

	// Call on every pass while Joining() (the join phase of either flow). connected = the station is joined to the
	// network under test and ip is its address (non-zero), lastDisconnectReason = the most telling disconnect code of the
	// check (0 = none), used when the timeout ends it. disconnectStreak = the streak of HAQuDA_AddDisconnectEvent();
	// once it decides a reason the check fails at once.
	// save_network: the network works, save it and call SaveFinished(). leave_station: the check failed (access point
	// mode: switch the station off; station mode: restart). Both are returned once.
	join_action Poll(uint32_t nowMs, bool connected, uint32_t ip, uint8_t lastDisconnectReason, uint32_t disconnectStreak = 0);

	// Reports the result of the save requested by Poll(). Returns leave_station when the save failed.
	join_action SaveFinished(bool saved);

	// Access point mode: checking covers both joining and saving. Station mode: checking while the scan runs, switching
	// from the found network until the end, then ok or fail.
	join_state State() const;
	// Reason of the failed check, none otherwise
	join_reason Reason() const;
	// Station address of the joined network, 0 until the join succeeded
	uint32_t Ip() const;
	// The scan of a station mode switch is under way
	bool Scanning() const;
	// The join phase: the station is trying the network under test
	bool Joining() const;
	// A check or a switch is running, saving included
	bool Active() const;

  private:
	enum class phase : uint8_t { idle, checking, saving, ok, fail, scanning, announcing, leaving };

	phase current = phase::idle;
	// Station mode switch (set by StartSwitch): the join and save phases report switching instead of checking
	bool stationFlow = false;
	join_reason failReason = join_reason::none;
	// Entry time of the current phase
	uint32_t startMs = 0;
	uint32_t joinedIp = 0;
};
