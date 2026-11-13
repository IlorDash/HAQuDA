#pragma once

#include <cstddef>
#include <cstdint>

#include "HAQuDA_WiFiJoin.h"

// Hardware-free part of the "result of the last network switch survives the restart" feature. The station mode switch ends
// in a restart, and /api/join must still tell the page what happened. The record lives in RTC memory that is not
// initialised at boot, so after a power-on it holds noise: a magic number and a checksum tell a record we wrote from noise.

constexpr uint32_t JOIN_LAST_MAGIC = 0x4A4C4152;

// Plain data with no padding, so the checksum covers every byte. No default member values: the variable that holds it sits
// in memory the start-up code must not touch.
struct join_last_record {
	uint32_t magic;
	// join_last_result and join_reason as numbers
	uint8_t result;
	uint8_t reason;
	// Always 0
	uint16_t reserved;
	char ssid[JOIN_SSID_BUF_LEN];
	// Checksum of every byte before this field
	uint32_t checksum;
};
static_assert(sizeof(join_last_record) == 4 + 1 + 1 + 2 + JOIN_SSID_BUF_LEN + 4, "the record has no padding: the checksum covers all of it");

// Writes the record from view.lastResult, view.lastReason and view.lastSsid. A view with lastResult none gives an empty
// record that HAQuDA_DecodeJoinLast() rejects. Any task.
void HAQuDA_EncodeJoinLast(join_last_record &record, const join_view &view);

// Fills view.lastResult, view.lastReason and view.lastSsid from the record and returns true when the record is valid: right
// magic and checksum, a result of ok or fail, a known reason (none for ok, not none for fail), a terminated non-empty name.
// Returns false and leaves the view untouched otherwise. Any task.
bool HAQuDA_DecodeJoinLast(const join_last_record &record, join_view &view);

// Makes the record invalid (all zero). Any task.
void HAQuDA_ClearJoinLastRecord(join_last_record &record);
