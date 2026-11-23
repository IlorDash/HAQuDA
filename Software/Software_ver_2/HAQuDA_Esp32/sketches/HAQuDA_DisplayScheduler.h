#pragma once

#include <cstdint>

#include "HAQuDA_DisplayCatalog.h"

constexpr uint8_t DISPLAY_QUEUE_CAPACITY = 8;
constexpr uint32_t DISPLAY_NO_DEADLINE = 0xFFFFFFFF;

enum class display_source : uint8_t { none, event, error, waiting, meas, solid };

struct display_pick {
	bool changed;
	display_source source;
	display_item item;
	anim_id anim;
	uint32_t nextMs;
};

// ShowTask only: not thread-safe. Time comes in as nowMs and is compared with unsigned subtraction to survive the 49-day millis() wrap
class HAQuDA_DisplayScheduler {
  public:
	HAQuDA_DisplayScheduler();

	bool Post(display_event event);
	bool Post(display_error error);

	void End(display_end end);

	void SetBackground(anim_id measAnim, display_item waiting);

	// The lamp's own picture (light colour or off) replaces the measurements as the background and counts as the end of the waiting
	// items. While it is off only errors show over it: events are not queued and the ones in the queue or playing are dropped.
	// Every call restarts the solid picture (a new colour), the item playing over it is left alone.
	void SetSolid(bool off);

	void MeasReady(bool ready);

	display_pick Next(uint32_t nowMs);

	uint8_t QueuedCount() const;
	uint8_t DroppedCount() const;

  private:
	enum class entry_state : uint8_t { fresh, resumed, rotated };

	struct queue_entry {
		display_item id;
		entry_state state;
		uint32_t seq;
	};

	struct current_item {
		bool valid;
		display_item id;
		uint32_t startMs;
		bool ended;
	};

	bool post(display_item item);
	void raise(uint16_t mask);
	bool isActive(display_item item) const;
	bool persists(const current_item &item) const;
	bool shouldPreempt(const display_entry &incoming) const;
	void preemptCurrent();
	bool makeRoom();
	void insertAt(uint8_t index, display_item item, entry_state state);
	void removeAt(uint8_t index);
	void dropEvents();
	int8_t pickIndex() const;
	uint8_t tierOf(const queue_entry &entry) const;
	void startCurrent(display_item item, uint32_t nowMs);
	display_pick describe(uint32_t nowMs);

	queue_entry queue[DISPLAY_QUEUE_CAPACITY];
	uint8_t queueCount = 0;
	uint32_t nextSeq = 0;
	current_item current;
	display_item lastPosted = DISPLAY_ITEM_NONE;
	bool measReadyNow = false;
	bool solidActive = false;
	bool eventsSuppressed = false;
	uint16_t solidSerial = 0;
	uint16_t shownSolidSerial = 0;
	anim_id measAnim = anim_id::none;
	display_item waitingItem = DISPLAY_ITEM_NONE;
	uint8_t droppedCount = 0;

	display_source shownSource = display_source::none;
	display_item shownItem = DISPLAY_ITEM_NONE;
	anim_id shownAnim = anim_id::none;
	// A restart of the same item must read as a change
	uint16_t startSerial = 0;
	uint16_t shownSerial = 0;
};
