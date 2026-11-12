#pragma once

#include <cstddef>
#include <cstdint>

// Hardware-free logic of the "check a new WiFi network before saving it" feature (access point mode). The WiFi handler
// feeds the state machine with the station state and acts on the actions it returns; the web server shows the result.

// How long the station may take to join before the check fails
constexpr uint32_t JOIN_TIMEOUT_MS = 20 * 1000;
// After a successful check the lamp keeps its access point up this long, so the phone can fetch the result, then restarts
constexpr uint32_t JOIN_OK_HOLD_MS = 20 * 1000;
// Bytes of the network name the check keeps, terminator included (the lamp stores 31 + terminator)
constexpr size_t JOIN_SSID_BUF_LEN = 32;

enum class join_state : uint8_t { idle, checking, ok, fail };
enum class join_reason : uint8_t { none, wrong_password, no_network, timeout, save_failed };
enum class join_action : uint8_t { none, save_network, leave_station };

// What /api/join reports. Written by the loop task and copied by the web server task under a lock.
struct join_view {
	join_state state = join_state::idle;
	join_reason reason = join_reason::none;
	// Station address of the joined network, first octet in the lowest byte (the value lwIP and IPAddress use)
	uint32_t ip = 0;
	// Milliseconds until the lamp restarts, filled in by the web server when it answers; only shown in state ok
	uint32_t restartMs = 0;
	char ssid[JOIN_SSID_BUF_LEN] = {0};
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

// "idle", "checking", "ok", "fail"
const char *HAQuDA_JoinStateName(join_state state);
// "wrong_password", "no_network", "timeout", "save_failed"; an empty string for none
const char *HAQuDA_JoinReasonName(join_reason reason);

// Copies at most JOIN_SSID_BUF_LEN - 1 bytes of the NUL-terminated (or maxLen-bounded) name into the view, terminated
void HAQuDA_SetJoinSsid(join_view &view, const char *ssid, size_t maxLen);

// Writes the JSON string body (no quotes) of a possibly unterminated text of at most maxLen bytes into out. Quote and
// backslash get a backslash, control characters become \u00XX. Stops when out is full; out is always terminated.
void HAQuDA_EscapeJson(const char *text, size_t maxLen, char *out, size_t outSize);

// Writes {"state":"..","ssid":"..","ip":"..","restart_ms":N,"host":"..","reason":".."} into buf; "ip" and "restart_ms"
// only in state ok, "reason" only in state fail. host is written as it is, so it must hold no quote, backslash or control character. Returns false (buf
// terminated and empty) when the text does not fit.
bool HAQuDA_BuildJoinJson(char *buf, size_t size, const join_view &view, const char *host);

// One check of a network: joining, saving, then the result. Time is passed in, nothing is read from hardware.
// All methods: loop task only.
class HAQuDA_JoinCheck {
  public:
	// Begins a check at nowMs. Returns false and changes nothing while a check is running.
	bool Start(uint32_t nowMs);

	// Call on every pass while State() is checking. connected = the station is joined and ip is its address (non-zero),
	// lastDisconnectReason = the most telling disconnect code of the check (0 = none), used when the timeout ends it.
	// disconnectStreak = the streak of HAQuDA_AddDisconnectEvent(); once it decides a reason the check fails at once.
	// save_network: the network works, save it and call SaveFinished(). leave_station: the check failed, switch the
	// station off. Both are returned once.
	join_action Poll(uint32_t nowMs, bool connected, uint32_t ip, uint8_t lastDisconnectReason, uint32_t disconnectStreak = 0);

	// Reports the result of the save requested by Poll(). Returns leave_station when the save failed.
	join_action SaveFinished(bool saved);

	// checking covers both joining and saving
	join_state State() const;
	// Reason of the failed check, none otherwise
	join_reason Reason() const;
	// Station address of the joined network, 0 until the join succeeded
	uint32_t Ip() const;

  private:
	enum class phase : uint8_t { idle, checking, saving, ok, fail };

	phase current = phase::idle;
	join_reason failReason = join_reason::none;
	uint32_t startMs = 0;
	uint32_t joinedIp = 0;
};
