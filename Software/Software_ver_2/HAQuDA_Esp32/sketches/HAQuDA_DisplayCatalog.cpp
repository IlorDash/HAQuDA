#include "HAQuDA_DisplayCatalog.h"

namespace {
	constexpr uint32_t WIFI_CONNECT_FAILED_MIN_MS = 5 * 1000;
	constexpr uint16_t NO_END = 0;
	constexpr uint16_t NO_RAISE = 0;
	constexpr bool STAYS = false;
	constexpr bool YIELDS = true;

	constexpr display_entry CATALOG[] = {
		{HAQuDA_ItemOf(display_event::bootSnake), "boot", HAQuDA_SnakeParams(ANIM_COLOR_LIME, 200, 5), display_class::event, display_end_cond::untilEvent,
		 HAQuDA_EndBit(display_end::bootResult), NO_RAISE, DISPLAY_EVENT_MIN_MS, STAYS},
		{HAQuDA_ItemOf(display_event::wifiJoined), "wifi joined", HAQuDA_StaticColorParams(ANIM_COLOR_GREEN), display_class::event, display_end_cond::timed,
		 NO_END, HAQuDA_EndBit(display_end::bootResult) | HAQuDA_EndBit(display_end::wifiJoined), DISPLAY_EVENT_MIN_MS, STAYS},
		// Also posted when a deliberate network switch starts, which ends with a restart
		{HAQuDA_ItemOf(display_event::wifiReconnecting), "wifi reconnecting", HAQuDA_SnakeParams(ANIM_COLOR_LIME, 200, 5), display_class::event,
		 display_end_cond::untilEvent, HAQuDA_EndBit(display_end::wifiJoined), NO_RAISE, DISPLAY_EVENT_MIN_MS, STAYS},
		{HAQuDA_ItemOf(display_event::apRunning), "ap running", HAQuDA_FadingParams(ANIM_COLOR_AQUA, 10, 100, 0, 5), display_class::event,
		 display_end_cond::untilEvent, HAQuDA_EndBit(display_end::measReady), HAQuDA_EndBit(display_end::bootResult), DISPLAY_EVENT_MIN_MS, STAYS},

		{HAQuDA_ItemOf(display_error::wifiConnectFailed), "wifi connect failed", HAQuDA_GrowingParams(ANIM_COLOR_RED, 100), display_class::error,
		 display_end_cond::timed, NO_END, HAQuDA_EndBit(display_end::bootResult), WIFI_CONNECT_FAILED_MIN_MS, STAYS},
		{HAQuDA_ItemOf(display_error::wifiLost), "wifi lost", HAQuDA_StaticColorParams(ANIM_COLOR_RED), display_class::error, display_end_cond::timed, NO_END,
		 HAQuDA_EndBit(display_end::bootResult), DISPLAY_ERROR_MIN_MS, STAYS},
		{HAQuDA_ItemOf(display_error::apFailed), "ap failed", HAQuDA_SnakeParams(ANIM_COLOR_RED, 100, 1), display_class::fatal, display_end_cond::untilEvent,
		 NO_END, HAQuDA_EndBit(display_end::bootResult), DISPLAY_ERROR_MIN_MS, STAYS},
		{HAQuDA_ItemOf(display_error::fsFailed), "fs failed", HAQuDA_FadingParams(ANIM_COLOR_RED, 10, 100, 0, 5), display_class::fatal,
		 display_end_cond::untilEvent, NO_END, HAQuDA_EndBit(display_end::bootResult), DISPLAY_ERROR_MIN_MS, STAYS},
		{HAQuDA_ItemOf(display_error::sensorsFailed), "sensors failed", HAQuDA_RandomPixelsParams(50, 500), display_class::error, display_end_cond::untilEvent,
		 HAQuDA_EndBit(display_end::sensorsRecovered), HAQuDA_EndBit(display_end::bootResult), DISPLAY_ERROR_MIN_MS, STAYS},
		// Without internet it never ends, so between its slots it gives the screen back to the measurements
		{HAQuDA_ItemOf(display_error::ntpFailed), "ntp failed", HAQuDA_FadingParams(ANIM_COLOR_YELLOW, 10, 0, 100, 10), display_class::error,
		 display_end_cond::untilEvent, HAQuDA_EndBit(display_end::ntpSynced), HAQuDA_EndBit(display_end::bootResult), DISPLAY_ERROR_MIN_MS, YIELDS},
	};

	static_assert(sizeof(CATALOG) / sizeof(CATALOG[0]) == DISPLAY_ITEM_COUNT, "one catalog row per display_event and display_error value");

	const char *const END_NAMES[DISPLAY_END_COUNT] = {"meas ready", "boot result", "wifi joined", "sensors recovered", "ntp synced"};
} // namespace

const display_entry *HAQuDA_GetDisplayEntry(display_item item) {
	return (item < DISPLAY_ITEM_COUNT) ? &CATALOG[item] : nullptr;
}

const char *HAQuDA_DisplayItemName(display_item item) {
	const display_entry *entry = HAQuDA_GetDisplayEntry(item);
	return (entry != nullptr) ? entry->name : "?";
}

const char *HAQuDA_DisplayEndName(display_end end) {
	const uint8_t index = static_cast<uint8_t>(end);
	return (index < DISPLAY_END_COUNT) ? END_NAMES[index] : "?";
}
