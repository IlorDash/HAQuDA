#pragma once

#include <cstdint>

#include "HAQuDA_Animation.h"

// Enum order is the row order of the catalog: events first, then errors
enum class display_event : uint8_t { bootSnake, wifiJoined, wifiReconnecting, apRunning, count };
enum class display_error : uint8_t { wifiConnectFailed, wifiLost, apFailed, fsFailed, sensorsFailed, ntpFailed, count };

constexpr uint8_t DISPLAY_EVENT_COUNT = static_cast<uint8_t>(display_event::count);
constexpr uint8_t DISPLAY_ERROR_COUNT = static_cast<uint8_t>(display_error::count);
constexpr uint8_t DISPLAY_ITEM_COUNT = DISPLAY_EVENT_COUNT + DISPLAY_ERROR_COUNT;

using display_item = uint8_t;
constexpr display_item DISPLAY_ITEM_NONE = 0xFF;

constexpr display_item HAQuDA_ItemOf(display_event event) {
	return static_cast<display_item>(event);
}

constexpr display_item HAQuDA_ItemOf(display_error error) {
	return static_cast<display_item>(DISPLAY_EVENT_COUNT + static_cast<uint8_t>(error));
}

enum class display_class : uint8_t { event, error, fatal };

enum class display_end_cond : uint8_t { timed, untilEvent };

enum class display_end : uint8_t { measReady, bootResult, wifiJoined, reconnectStarted, sensorsRecovered, ntpSynced, count };
constexpr uint8_t DISPLAY_END_COUNT = static_cast<uint8_t>(display_end::count);

constexpr uint16_t HAQuDA_EndBit(display_end end) {
	return static_cast<uint16_t>(1u << static_cast<uint8_t>(end));
}

constexpr uint32_t DISPLAY_EVENT_MIN_MS = 5 * 1000;
constexpr uint32_t DISPLAY_ERROR_MIN_MS = 10 * 1000;

struct display_entry {
	// Must equal the row index, the host test checks it
	display_item id;
	const char *name;
	anim_params anim;
	display_class cls;
	display_end_cond end;
	uint16_t endsOn;
	uint16_t raises;
	uint32_t minMs;
};

const display_entry *HAQuDA_GetDisplayEntry(display_item item);

const char *HAQuDA_DisplayItemName(display_item item);

const char *HAQuDA_DisplayEndName(display_end end);
