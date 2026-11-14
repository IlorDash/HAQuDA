#include "HAQuDA_JoinLast.h"

#include <cstddef>
#include <cstring>

namespace {
	// FNV-1a over the bytes before the checksum field
	uint32_t recordChecksum(const join_last_record &record) {
		const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&record);
		uint32_t hash = 2166136261u;
		for (size_t i = 0; i < offsetof(join_last_record, checksum); i++) {
			hash ^= bytes[i];
			hash *= 16777619u;
		}
		return hash;
	}
} // namespace

void HAQuDA_EncodeJoinLast(join_last_record &record, const join_view &view) {
	memset(&record, 0, sizeof(record));
	if (view.lastResult == join_last_result::none) {
		return;
	}
	record.magic = JOIN_LAST_MAGIC;
	record.result = static_cast<uint8_t>(view.lastResult);
	record.reason = static_cast<uint8_t>(view.lastReason);
	size_t len = strnlen(view.lastSsid, sizeof(view.lastSsid));
	if (len >= sizeof(record.ssid)) {
		len = sizeof(record.ssid) - 1;
	}
	memcpy(record.ssid, view.lastSsid, len);
	record.checksum = recordChecksum(record);
}

bool HAQuDA_DecodeJoinLast(const join_last_record &record, join_view &view) {
	if ((record.magic != JOIN_LAST_MAGIC) || (record.checksum != recordChecksum(record))) {
		return false;
	}
	bool isOk = record.result == static_cast<uint8_t>(join_last_result::ok);
	bool isFail = record.result == static_cast<uint8_t>(join_last_result::fail);
	if (!isOk && !isFail) {
		return false;
	}
	if (record.reason > static_cast<uint8_t>(join_reason::scan_failed)) {
		return false;
	}
	bool reasonSet = record.reason != static_cast<uint8_t>(join_reason::none);
	if (isOk == reasonSet) {
		// ok carries no reason, fail carries one
		return false;
	}
	if ((record.ssid[JOIN_SSID_BUF_LEN - 1] != '\0') || (record.ssid[0] == '\0')) {
		return false;
	}
	HAQuDA_SetJoinLast(view, static_cast<join_last_result>(record.result), static_cast<join_reason>(record.reason), record.ssid, sizeof(record.ssid));
	return true;
}

void HAQuDA_ClearJoinLastRecord(join_last_record &record) {
	memset(&record, 0, sizeof(record));
}
