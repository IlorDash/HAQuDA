#pragma once

#include <cstdint>

#include "HAQuDA_Animation.h"

// Layer 1b of the display system: the one table that says which animation each event and each error plays, for how long and what
// ends it. Hardware-free (no Arduino.h), built on the PC. Adding an event or an error = one enum value + one row in
// HAQuDA_DisplayCatalog.cpp (a static_assert and the host test fail when the two disagree).

// Results of the lamp's own life that are shown for a few seconds or until something changes. Order = row order in the table.
enum class display_event : uint8_t { bootSnake, wifiJoined, wifiReconnecting, apRunning, count };
// Failures the user must see. Order = row order after the events.
enum class display_error : uint8_t { wifiConnectFailed, wifiLost, apFailed, fsFailed, sensorsFailed, ntpFailed, count };

constexpr uint8_t DISPLAY_EVENT_COUNT = static_cast<uint8_t>(display_event::count);
constexpr uint8_t DISPLAY_ERROR_COUNT = static_cast<uint8_t>(display_error::count);
constexpr uint8_t DISPLAY_ITEM_COUNT = DISPLAY_EVENT_COUNT + DISPLAY_ERROR_COUNT;

// One number for any catalog row: events first, then errors. DISPLAY_ITEM_NONE is "no item".
using display_item = uint8_t;
constexpr display_item DISPLAY_ITEM_NONE = 0xFF;

constexpr display_item HAQuDA_ItemOf(display_event event) {
	return static_cast<display_item>(event);
}

constexpr display_item HAQuDA_ItemOf(display_error error) {
	return static_cast<display_item>(DISPLAY_EVENT_COUNT + static_cast<uint8_t>(error));
}

// event: result of an action or a state change, 5 s. error: a failure, 10 s, takes precedence and interrupts a running event.
// fatal: an error that never ends (dead stop) and interrupts everything, also another error inside its minimum time.
enum class display_class : uint8_t { event, error, fatal };

// timed: plays its minimum time and leaves. untilEvent: plays until one of its end signals arrives, never shorter than the
// minimum time, and until then takes part in the rotation.
enum class display_end_cond : uint8_t { timed, untilEvent };

// Signals that end untilEvent items. Posted by the code that knows (measurement start, WiFi, NTP) or raised by posting an item
// (the `raises` column).
enum class display_end : uint8_t {
	measReady,		  // every parameter of the shown measurement mode has a valid sample (scheduler MeasReady(true))
	bootResult,		  // the boot ended: WiFi joined, access point running or any error
	wifiJoined,		  // the lamp has an IP
	reconnectStarted, // HandleConnection() entered the reconnect loop
	sensorsRecovered, // the sensors run again
	ntpSynced,		  // the first NTP sync
	count
};
constexpr uint8_t DISPLAY_END_COUNT = static_cast<uint8_t>(display_end::count);

// The bit of an end signal in the endsOn / raises masks.
constexpr uint16_t HAQuDA_EndBit(display_end end) {
	return static_cast<uint16_t>(1u << static_cast<uint8_t>(end));
}

constexpr uint32_t DISPLAY_EVENT_MIN_MS = 5 * 1000;
constexpr uint32_t DISPLAY_ERROR_MIN_MS = 10 * 1000;

struct display_entry {
	// the row's own item number; the host test checks it equals the row index
	display_item id;
	const char *name;
	// what plays and its constant parameters
	anim_params anim;
	display_class cls;
	display_end_cond end;
	// HAQuDA_EndBit() mask of the signals that end the item; 0 = never ends (untilEvent only)
	uint16_t endsOn;
	// signals this item raises the moment it is posted
	uint16_t raises;
	// shortest time it is shown, whatever ends it
	uint32_t minMs;
};

// The row of an item, nullptr when item is not a catalog item. Any task.
const display_entry *HAQuDA_GetDisplayEntry(display_item item);

// Name of an item, "?" when invalid. Any task.
const char *HAQuDA_DisplayItemName(display_item item);

// Name of an end signal, "?" when invalid. Any task.
const char *HAQuDA_DisplayEndName(display_end end);
