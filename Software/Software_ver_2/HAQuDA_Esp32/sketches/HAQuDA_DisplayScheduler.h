#pragma once

#include <cstdint>

#include "HAQuDA_DisplayCatalog.h"

// Layer 2 of the display system: decides what the matrix shows now. Pure (no Arduino.h, no clock of its own): every call that needs
// time gets nowMs, compared with unsigned subtraction so it survives the 49-day millis() wrap. Not thread-safe: the display
// service in ShowTask owns it and feeds it from its own queue; other tasks never call it.
//
// Rules (all from the catalog rows):
// - An item plays at least its row's minimum time. A timed item then leaves, an untilEvent item stays in the rotation until one of
//   its end signals arrived; a signal that arrives earlier only keeps it from rotating on after the minimum time.
// - Choice of the next item: new fatal, new error, items an error interrupted (back at the head, full minimum time again), new
//   event, then the rotation of untilEvent items that already played once (errors and events alike, in turn).
// - An error interrupts a running event at once; an error never interrupts another error inside its minimum time; a fatal error
//   interrupts everything and never leaves.
// - Queue of DISPLAY_QUEUE_CAPACITY waiting items. When it is full the oldest timed event is dropped, never an error or an
//   untilEvent item. Of two posts in a row only the first is kept (a post equal to the one before, still waiting or playing, is
//   dropped); an untilEvent item that is already waiting or playing is never queued twice.
// - Nothing to show: the measurement picture once MeasReady(true), else the waiting item, else nothing (the LEDs keep their frame).

constexpr uint8_t DISPLAY_QUEUE_CAPACITY = 8;
// display_pick::nextMs when only a new Post(), End() or MeasReady() can change the picture
constexpr uint32_t DISPLAY_NO_DEADLINE = 0xFFFFFFFF;

enum class display_source : uint8_t {
	none,	 // nothing to draw: the LEDs keep their last frame
	event,	 // a catalog event
	error,	 // a catalog error (also fatal)
	waiting, // the waiting item of SetBackground(), nothing else is active and the measurements are not ready
	meas	 // the measurement animation of SetBackground(), the measurements are ready
};

// What to draw now. Returned by every Next(), also when nothing changed.
struct display_pick {
	// true when this differs from what the previous Next() returned (including a restart of the same item): stop the running
	// animation, Begin() the new one, tell the web mirror
	bool changed;
	display_source source;
	// catalog item for event, error and waiting; DISPLAY_ITEM_NONE for meas and none
	display_item item;
	// animation to run; anim_id::none for display_source::none. The catalog gives its parameters
	anim_id anim;
	// ms until Next() has to be called at the latest because the minimum time of the current item runs out;
	// DISPLAY_NO_DEADLINE when nothing time-based is pending
	uint32_t nextMs;
};

class HAQuDA_DisplayScheduler {
  public:
	HAQuDA_DisplayScheduler();

	// Queues an event or an error. Returns false when the id is invalid or the queue is full of items that must not be dropped;
	// true also when the post was coalesced or ignored as a duplicate. Raises the end signals of its catalog row first. ShowTask only.
	bool Post(display_event event);
	bool Post(display_error error);

	// An end signal arrived: waiting untilEvent items that end on it are removed, the playing one will not rotate on after its
	// minimum time. ShowTask only.
	void End(display_end end);

	// What plays when no item is active: measAnim (a measurement animation) once MeasReady(true), until then the catalog item
	// `waiting` (DISPLAY_ITEM_NONE: nothing). anim_id::none: no measurement picture, the waiting item (or nothing) stays. ShowTask only.
	void SetBackground(anim_id measAnim, display_item waiting);

	// Every parameter the shown measurement mode needs has a valid sample (true) or not (false). true raises
	// display_end::measReady and, like the signal, ends items waiting for it; an item that ends on it and is posted while
	// the measurements are ready is not queued. ShowTask only.
	void MeasReady(bool ready);

	// Advances the schedule to nowMs and returns what to draw. Call after every Post()/End()/MeasReady()/SetBackground() and when
	// the previous result's nextMs has passed. ShowTask only.
	display_pick Next(uint32_t nowMs);

	// Items waiting (not the playing one). ShowTask only.
	uint8_t QueuedCount() const;
	// Timed events dropped for lack of room so far. ShowTask only.
	uint8_t DroppedCount() const;

  private:
	enum class entry_state : uint8_t { fresh, resumed, rotated };

	struct queue_entry {
		display_item id;
		entry_state state;
		// post order, to find the oldest timed event; survives the wrap of 32 bits for 4 billion posts
		uint32_t seq;
	};

	struct current_item {
		bool valid;
		display_item id;
		uint32_t startMs;
		// an end signal arrived while it played
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
	anim_id measAnim = anim_id::none;
	display_item waitingItem = DISPLAY_ITEM_NONE;
	uint8_t droppedCount = 0;

	// what the previous Next() reported
	display_source shownSource = display_source::none;
	display_item shownItem = DISPLAY_ITEM_NONE;
	anim_id shownAnim = anim_id::none;
	// counts the starts of the playing item, so a restart of the same item reads as a change
	uint16_t startSerial = 0;
	uint16_t shownSerial = 0;
};
