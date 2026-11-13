#include "HAQuDA_WiFiJoin.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace {
	// A name byte is escaped to at most 6 characters (\u00XX)
	constexpr size_t ESCAPED_SSID_LEN = JOIN_SSID_BUF_LEN * 6 + 1;

	// Appends formatted text at pos; returns false when it does not fit
	bool appendFormat(char *buf, size_t size, size_t &pos, const char *fmt, ...) {
		if (pos >= size) {
			return false;
		}
		va_list args;
		va_start(args, fmt);
		int written = vsnprintf(buf + pos, size - pos, fmt, args);
		va_end(args);
		if ((written < 0) || (static_cast<size_t>(written) >= size - pos)) {
			return false;
		}
		pos += static_cast<size_t>(written);
		return true;
	}
} // namespace

join_reason HAQuDA_MapDisconnectReason(uint8_t code) {
	switch (code) {
		case 14:  // MIC_FAILURE
		case 15:  // 4WAY_HANDSHAKE_TIMEOUT
		case 23:  // 802_1X_AUTH_FAILED
		case 202: // AUTH_FAIL
		case 204: // HANDSHAKE_TIMEOUT
			return join_reason::wrong_password;
		case 201: // NO_AP_FOUND
			return join_reason::no_network;
		default:
			return join_reason::timeout;
	}
}

namespace {
	int reasonRank(uint8_t code) {
		switch (HAQuDA_MapDisconnectReason(code)) {
			case join_reason::wrong_password:
				return 2;
			case join_reason::no_network:
				return 1;
			default:
				return 0;
		}
	}
} // namespace

uint8_t HAQuDA_MergeDisconnectReason(uint8_t current, uint8_t incoming) {
	int incomingRank = reasonRank(incoming);
	if (incomingRank == 0) {
		return (reasonRank(current) == 0) ? 0 : current;
	}
	// The same rank keeps the newer code; a lower rank never replaces a higher one
	return (incomingRank >= reasonRank(current)) ? incoming : current;
}

uint32_t HAQuDA_AddDisconnectEvent(uint32_t streak, uint8_t code) {
	// ESP-IDF WIFI_REASON_ASSOC_LEAVE
	if (code == 8) {
		return streak;
	}
	join_reason eventClass = HAQuDA_MapDisconnectReason(code);
	if ((eventClass != join_reason::wrong_password) && (eventClass != join_reason::no_network)) {
		return 0;
	}
	uint32_t count = streak & 0xFF;
	uint32_t streakClass = (streak >> 8) & 0xFF;
	if ((count > 0) && (streakClass == static_cast<uint32_t>(eventClass))) {
		if (count < 0xFF) {
			count++;
		}
	} else {
		count = 1;
	}
	return (static_cast<uint32_t>(eventClass) << 8) | count;
}

join_reason HAQuDA_StreakReason(uint32_t streak) {
	if ((streak & 0xFF) < JOIN_EARLY_FAIL_EVENTS) {
		return join_reason::none;
	}
	join_reason streakClass = static_cast<join_reason>((streak >> 8) & 0xFF);
	if ((streakClass == join_reason::wrong_password) || (streakClass == join_reason::no_network)) {
		return streakClass;
	}
	return join_reason::none;
}

join_scan_status HAQuDA_MapScanResult(int16_t scanComplete) {
	// WIFI_SCAN_RUNNING is -1, WIFI_SCAN_FAILED is -2
	if (scanComplete == -1) {
		return join_scan_status::running;
	}
	if (scanComplete < 0) {
		return join_scan_status::failed;
	}
	return (scanComplete == 0) ? join_scan_status::not_found : join_scan_status::found;
}

const char *HAQuDA_JoinStateName(join_state state) {
	switch (state) {
		case join_state::checking:
			return "checking";
		case join_state::switching:
			return "switching";
		case join_state::ok:
			return "ok";
		case join_state::fail:
			return "fail";
		case join_state::idle:
		default:
			return "idle";
	}
}

const char *HAQuDA_JoinReasonName(join_reason reason) {
	switch (reason) {
		case join_reason::wrong_password:
			return "wrong_password";
		case join_reason::no_network:
			return "no_network";
		case join_reason::timeout:
			return "timeout";
		case join_reason::save_failed:
			return "save_failed";
		case join_reason::scan_failed:
			return "scan_failed";
		case join_reason::none:
		default:
			return "";
	}
}

const char *HAQuDA_JoinLastResultName(join_last_result result) {
	switch (result) {
		case join_last_result::ok:
			return "ok";
		case join_last_result::fail:
			return "fail";
		case join_last_result::none:
		default:
			return "";
	}
}

namespace {
	// Copies at most outSize - 1 bytes of the NUL-terminated (or maxLen-bounded) text into out, terminated
	void copyName(char *out, size_t outSize, const char *text, size_t maxLen) {
		memset(out, 0, outSize);
		if (text == nullptr) {
			return;
		}
		size_t len = strnlen(text, maxLen);
		if (len >= outSize) {
			len = outSize - 1;
		}
		memcpy(out, text, len);
	}
} // namespace

void HAQuDA_SetJoinSsid(join_view &view, const char *ssid, size_t maxLen) {
	copyName(view.ssid, sizeof(view.ssid), ssid, maxLen);
}

void HAQuDA_SetJoinLast(join_view &view, join_last_result result, join_reason reason, const char *ssid, size_t maxLen) {
	if (result == join_last_result::none) {
		view.lastResult = join_last_result::none;
		view.lastReason = join_reason::none;
		memset(view.lastSsid, 0, sizeof(view.lastSsid));
		return;
	}
	view.lastResult = result;
	view.lastReason = (result == join_last_result::fail) ? reason : join_reason::none;
	copyName(view.lastSsid, sizeof(view.lastSsid), ssid, maxLen);
}

void HAQuDA_EscapeJson(const char *text, size_t maxLen, char *out, size_t outSize) {
	if ((out == nullptr) || (outSize == 0)) {
		return;
	}
	size_t pos = 0;
	size_t textLen = (text == nullptr) ? 0 : strnlen(text, maxLen);
	for (size_t i = 0; i < textLen; i++) {
		unsigned char c = static_cast<unsigned char>(text[i]);
		int written = 0;
		if ((c == '"') || (c == '\\')) {
			written = snprintf(out + pos, outSize - pos, "\\%c", c);
		} else if (c < 0x20) {
			written = snprintf(out + pos, outSize - pos, "\\u%04x", c);
		} else {
			written = snprintf(out + pos, outSize - pos, "%c", c);
		}
		if ((written < 0) || (static_cast<size_t>(written) >= outSize - pos)) {
			break;
		}
		pos += static_cast<size_t>(written);
	}
	out[pos] = '\0';
}

bool HAQuDA_BuildJoinJson(char *buf, size_t size, const join_view &view, const char *host) {
	if ((buf == nullptr) || (size == 0)) {
		return false;
	}
	buf[0] = '\0';
	if (host == nullptr) {
		return false;
	}

	char escapedSsid[ESCAPED_SSID_LEN];
	HAQuDA_EscapeJson(view.ssid, sizeof(view.ssid), escapedSsid, sizeof(escapedSsid));

	size_t pos = 0;
	bool fits = appendFormat(buf, size, pos, "{\"state\":\"%s\",\"ssid\":\"%s\"", HAQuDA_JoinStateName(view.state), escapedSsid);
	if (fits && (view.state == join_state::ok)) {
		fits = appendFormat(buf, size, pos, ",\"ip\":\"%u.%u.%u.%u\",\"restart_ms\":%lu", static_cast<unsigned>(view.ip & 0xFF),
							static_cast<unsigned>((view.ip >> 8) & 0xFF), static_cast<unsigned>((view.ip >> 16) & 0xFF),
							static_cast<unsigned>((view.ip >> 24) & 0xFF), static_cast<unsigned long>(view.restartMs));
	}
	fits = fits && appendFormat(buf, size, pos, ",\"host\":\"%s\"", host);
	if (fits && (view.state == join_state::fail)) {
		fits = appendFormat(buf, size, pos, ",\"reason\":\"%s\"", HAQuDA_JoinReasonName(view.reason));
	}
	if (fits && (view.lastResult != join_last_result::none)) {
		char escapedLastSsid[ESCAPED_SSID_LEN];
		HAQuDA_EscapeJson(view.lastSsid, sizeof(view.lastSsid), escapedLastSsid, sizeof(escapedLastSsid));
		fits = appendFormat(buf, size, pos, ",\"last\":{\"result\":\"%s\"", HAQuDA_JoinLastResultName(view.lastResult));
		if (fits && (view.lastResult == join_last_result::fail)) {
			fits = appendFormat(buf, size, pos, ",\"reason\":\"%s\"", HAQuDA_JoinReasonName(view.lastReason));
		}
		fits = fits && appendFormat(buf, size, pos, ",\"ssid\":\"%s\"}", escapedLastSsid);
	}
	fits = fits && appendFormat(buf, size, pos, "}");
	if (!fits) {
		buf[0] = '\0';
		return false;
	}
	return true;
}

bool HAQuDA_JoinCheck::Start(uint32_t nowMs) {
	if (Active()) {
		return false;
	}
	current = phase::checking;
	stationFlow = false;
	failReason = join_reason::none;
	startMs = nowMs;
	joinedIp = 0;
	return true;
}

bool HAQuDA_JoinCheck::StartSwitch(uint32_t nowMs) {
	if (Active()) {
		return false;
	}
	current = phase::scanning;
	stationFlow = true;
	failReason = join_reason::none;
	startMs = nowMs;
	joinedIp = 0;
	return true;
}

join_action HAQuDA_JoinCheck::PollSwitch(uint32_t nowMs, join_scan_status scan, bool stationUp) {
	// Unsigned difference survives the millis() wrap
	uint32_t phaseMs = static_cast<uint32_t>(nowMs - startMs);
	switch (current) {
		case phase::scanning:
			if (scan == join_scan_status::found) {
				current = phase::announcing;
				startMs = nowMs;
			} else if (scan == join_scan_status::not_found) {
				failReason = join_reason::no_network;
				current = phase::fail;
			} else if ((scan == join_scan_status::failed) || (phaseMs >= SWITCH_SCAN_TIMEOUT_MS)) {
				// The scan tells nothing about the network: the lamp stays where it is and reports that it could not search
				failReason = join_reason::scan_failed;
				current = phase::fail;
			}
			return join_action::none;
		case phase::announcing:
			if (phaseMs >= SWITCH_ANNOUNCE_MS) {
				current = phase::leaving;
				startMs = nowMs;
				return join_action::disconnect_old;
			}
			return join_action::none;
		case phase::leaving:
			// The connected state is cleared by the disconnect event, which arrives after the disconnect call returned.
			// Joining before that would take the stale state of the old network for a join of the new one. A station that
			// still reports connected after the wait did not disconnect; joining goes on, begin() disconnects again.
			if (!stationUp || (phaseMs >= SWITCH_LEAVE_TIMEOUT_MS)) {
				current = phase::checking;
				startMs = nowMs;
				return join_action::begin_join;
			}
			return join_action::none;
		default:
			return join_action::none;
	}
}

join_action HAQuDA_JoinCheck::Poll(uint32_t nowMs, bool connected, uint32_t ip, uint8_t lastDisconnectReason, uint32_t disconnectStreak) {
	if (current != phase::checking) {
		return join_action::none;
	}
	// A join that completes on the deadline pass still counts
	if (connected && (ip != 0)) {
		joinedIp = ip;
		current = phase::saving;
		return join_action::save_network;
	}
	join_reason early = HAQuDA_StreakReason(disconnectStreak);
	if (early != join_reason::none) {
		failReason = early;
		current = phase::fail;
		return join_action::leave_station;
	}
	// Unsigned difference survives the millis() wrap
	if (static_cast<uint32_t>(nowMs - startMs) >= JOIN_TIMEOUT_MS) {
		failReason = HAQuDA_MapDisconnectReason(lastDisconnectReason);
		current = phase::fail;
		return join_action::leave_station;
	}
	return join_action::none;
}

join_action HAQuDA_JoinCheck::SaveFinished(bool saved) {
	if (current != phase::saving) {
		return join_action::none;
	}
	if (saved) {
		current = phase::ok;
		return join_action::none;
	}
	failReason = join_reason::save_failed;
	joinedIp = 0;
	current = phase::fail;
	return join_action::leave_station;
}

join_state HAQuDA_JoinCheck::State() const {
	switch (current) {
		case phase::scanning:
			return join_state::checking;
		case phase::announcing:
		case phase::leaving:
			return join_state::switching;
		case phase::checking:
		case phase::saving:
			return stationFlow ? join_state::switching : join_state::checking;
		case phase::ok:
			return join_state::ok;
		case phase::fail:
			return join_state::fail;
		case phase::idle:
		default:
			return join_state::idle;
	}
}

join_reason HAQuDA_JoinCheck::Reason() const {
	return (current == phase::fail) ? failReason : join_reason::none;
}

uint32_t HAQuDA_JoinCheck::Ip() const {
	return joinedIp;
}

bool HAQuDA_JoinCheck::Scanning() const {
	return current == phase::scanning;
}

bool HAQuDA_JoinCheck::Joining() const {
	return current == phase::checking;
}

bool HAQuDA_JoinCheck::Active() const {
	return (current == phase::checking) || (current == phase::saving) || (current == phase::scanning) || (current == phase::announcing)
		   || (current == phase::leaving);
}
