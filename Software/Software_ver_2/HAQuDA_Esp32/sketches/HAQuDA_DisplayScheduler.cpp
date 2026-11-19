#include "HAQuDA_DisplayScheduler.h"

namespace {
	// Order the next item is chosen in, lowest first (see the class rules).
	constexpr uint8_t TIER_NEW_FATAL = 0;
	constexpr uint8_t TIER_NEW_ERROR = 1;
	constexpr uint8_t TIER_RESUMED = 2;
	constexpr uint8_t TIER_NEW_EVENT = 3;
	constexpr uint8_t TIER_ROTATION = 4;
	constexpr uint8_t TIER_NONE = 0xFF;

	bool isUntilEvent(const display_entry &entry) {
		return entry.end == display_end_cond::untilEvent;
	}
} // namespace

HAQuDA_DisplayScheduler::HAQuDA_DisplayScheduler() {
	for (uint8_t i = 0; i < DISPLAY_QUEUE_CAPACITY; i++) {
		queue[i] = queue_entry{DISPLAY_ITEM_NONE, entry_state::fresh, 0};
	}
	current = current_item{false, DISPLAY_ITEM_NONE, 0, false};
}

bool HAQuDA_DisplayScheduler::Post(display_event event) {
	if (event >= display_event::count) {
		return false;
	}
	return post(HAQuDA_ItemOf(event));
}

bool HAQuDA_DisplayScheduler::Post(display_error error) {
	if (error >= display_error::count) {
		return false;
	}
	return post(HAQuDA_ItemOf(error));
}

bool HAQuDA_DisplayScheduler::post(display_item item) {
	const display_entry *entry = HAQuDA_GetDisplayEntry(item);
	if (entry == nullptr) {
		return false;
	}
	// Signals first, so a post never ends itself and ends what it replaces (a result ends the boot snake)
	raise(entry->raises);

	if (isUntilEvent(*entry)) {
		// Its end already happened: the measurements are ready before the "access point running" picture was posted
		if (measReadyNow && ((entry->endsOn & HAQuDA_EndBit(display_end::measReady)) != 0)) {
			return true;
		}
		if (isActive(item)) {
			return true;
		}
	}
	if ((lastPosted == item) && isActive(item)) {
		return true;
	}

	const bool preempt = shouldPreempt(*entry);
	if (!makeRoom()) {
		return false;
	}
	insertAt(queueCount, item, entry_state::fresh);
	if (preempt) {
		preemptCurrent();
	}
	lastPosted = item;
	return true;
}

void HAQuDA_DisplayScheduler::End(display_end end) {
	raise(HAQuDA_EndBit(end));
}

void HAQuDA_DisplayScheduler::raise(uint16_t mask) {
	for (uint8_t i = queueCount; i > 0; i--) {
		const display_entry *entry = HAQuDA_GetDisplayEntry(queue[i - 1].id);
		if ((entry != nullptr) && isUntilEvent(*entry) && ((entry->endsOn & mask) != 0)) {
			removeAt(i - 1);
		}
	}
	if (current.valid) {
		const display_entry *entry = HAQuDA_GetDisplayEntry(current.id);
		if ((entry != nullptr) && isUntilEvent(*entry) && ((entry->endsOn & mask) != 0)) {
			current.ended = true;
		}
	}
}

void HAQuDA_DisplayScheduler::SetBackground(anim_id newMeasAnim, display_item newWaiting) {
	measAnim = newMeasAnim;
	waitingItem = (HAQuDA_GetDisplayEntry(newWaiting) != nullptr) ? newWaiting : DISPLAY_ITEM_NONE;
}

void HAQuDA_DisplayScheduler::MeasReady(bool ready) {
	measReadyNow = ready;
	if (ready) {
		raise(HAQuDA_EndBit(display_end::measReady));
	}
}

// Waiting or playing and not over: an item whose end signal arrived while it plays is over, a new post of it is a new occurrence.
bool HAQuDA_DisplayScheduler::isActive(display_item item) const {
	if (current.valid && !current.ended && (current.id == item)) {
		return true;
	}
	for (uint8_t i = 0; i < queueCount; i++) {
		if (queue[i].id == item) {
			return true;
		}
	}
	return false;
}

bool HAQuDA_DisplayScheduler::persists(const current_item &item) const {
	const display_entry *entry = HAQuDA_GetDisplayEntry(item.id);
	return (entry != nullptr) && isUntilEvent(*entry) && !item.ended;
}

bool HAQuDA_DisplayScheduler::shouldPreempt(const display_entry &incoming) const {
	if (!current.valid) {
		return false;
	}
	const display_entry *playing = HAQuDA_GetDisplayEntry(current.id);
	if ((playing == nullptr) || (playing->cls == display_class::fatal)) {
		return false;
	}
	if (incoming.cls == display_class::fatal) {
		return true;
	}
	return (incoming.cls == display_class::error) && (playing->cls == display_class::event);
}

// The interrupted item goes back to the head and starts over with its full minimum time. An item whose end signal already arrived
// is over and is not kept. A queue full of items that cannot be dropped also loses the interrupted item (not reachable with the
// shipped catalog, capacity 8).
void HAQuDA_DisplayScheduler::preemptCurrent() {
	if (!current.ended && (queueCount < DISPLAY_QUEUE_CAPACITY || makeRoom())) {
		insertAt(0, current.id, entry_state::resumed);
	}
	current.valid = false;
}

// Frees one slot by dropping the oldest timed event, if the queue is full. false: full and nothing may be dropped.
bool HAQuDA_DisplayScheduler::makeRoom() {
	if (queueCount < DISPLAY_QUEUE_CAPACITY) {
		return true;
	}
	int8_t oldest = -1;
	for (uint8_t i = 0; i < queueCount; i++) {
		const display_entry *entry = HAQuDA_GetDisplayEntry(queue[i].id);
		const bool droppable = (entry != nullptr) && (entry->cls == display_class::event) && !isUntilEvent(*entry);
		// seq differences, not values: nextSeq may have wrapped
		if (droppable && ((oldest < 0) || (static_cast<uint32_t>(queue[oldest].seq - queue[i].seq) < 0x80000000u))) {
			oldest = static_cast<int8_t>(i);
		}
	}
	if (oldest < 0) {
		return false;
	}
	removeAt(static_cast<uint8_t>(oldest));
	droppedCount++;
	return true;
}

void HAQuDA_DisplayScheduler::insertAt(uint8_t index, display_item item, entry_state state) {
	for (uint8_t i = queueCount; i > index; i--) {
		queue[i] = queue[i - 1];
	}
	queue[index] = queue_entry{item, state, nextSeq++};
	queueCount++;
}

void HAQuDA_DisplayScheduler::removeAt(uint8_t index) {
	for (uint8_t i = index; i + 1 < queueCount; i++) {
		queue[i] = queue[i + 1];
	}
	queueCount--;
}

uint8_t HAQuDA_DisplayScheduler::tierOf(const queue_entry &entry) const {
	const display_entry *row = HAQuDA_GetDisplayEntry(entry.id);
	if (row == nullptr) {
		return TIER_NONE;
	}
	switch (entry.state) {
		case entry_state::rotated:
			return TIER_ROTATION;
		case entry_state::resumed:
			return TIER_RESUMED;
		default:
			break;
	}
	switch (row->cls) {
		case display_class::fatal:
			return TIER_NEW_FATAL;
		case display_class::error:
			return TIER_NEW_ERROR;
		default:
			return TIER_NEW_EVENT;
	}
}

int8_t HAQuDA_DisplayScheduler::pickIndex() const {
	int8_t best = -1;
	uint8_t bestTier = TIER_NONE;
	for (uint8_t i = 0; i < queueCount; i++) {
		const uint8_t tier = tierOf(queue[i]);
		if (tier < bestTier) {
			best = static_cast<int8_t>(i);
			bestTier = tier;
		}
	}
	return best;
}

void HAQuDA_DisplayScheduler::startCurrent(display_item item, uint32_t nowMs) {
	current = current_item{true, item, nowMs, false};
	startSerial++;
}

display_pick HAQuDA_DisplayScheduler::Next(uint32_t nowMs) {
	if (current.valid) {
		const display_entry *playing = HAQuDA_GetDisplayEntry(current.id);
		// a fatal error never leaves
		const bool slotOver
			= (playing != nullptr) && (playing->cls != display_class::fatal) && (static_cast<uint32_t>(nowMs - current.startMs) >= playing->minMs);
		if (slotOver) {
			const int8_t next = pickIndex();
			if (next >= 0) {
				const display_item nextItem = queue[next].id;
				const bool keepPlaying = persists(current);
				const display_item playedItem = current.id;
				removeAt(static_cast<uint8_t>(next));
				if (keepPlaying) {
					// the slot freed by removeAt() is the one the rotation needs
					insertAt(queueCount, playedItem, entry_state::rotated);
				}
				startCurrent(nextItem, nowMs);
			} else if (!persists(current)) {
				current.valid = false;
			}
			// else: alone in the rotation, plays on; startMs stays, so an item posted later replaces it at the next call
		}
	}
	if (!current.valid) {
		const int8_t next = pickIndex();
		if (next >= 0) {
			const display_item nextItem = queue[next].id;
			removeAt(static_cast<uint8_t>(next));
			startCurrent(nextItem, nowMs);
		}
	}
	return describe(nowMs);
}

display_pick HAQuDA_DisplayScheduler::describe(uint32_t nowMs) {
	display_pick pick = {false, display_source::none, DISPLAY_ITEM_NONE, anim_id::none, DISPLAY_NO_DEADLINE};

	const display_entry *playing = current.valid ? HAQuDA_GetDisplayEntry(current.id) : nullptr;
	if (playing != nullptr) {
		pick.source = (playing->cls == display_class::event) ? display_source::event : display_source::error;
		pick.item = current.id;
		pick.anim = playing->anim.anim;
		const bool waitsForever = (playing->cls == display_class::fatal) || (persists(current) && (queueCount == 0));
		if (!waitsForever) {
			const uint32_t elapsedMs = nowMs - current.startMs;
			pick.nextMs = (elapsedMs < playing->minMs) ? (playing->minMs - elapsedMs) : 0;
		}
	} else if (measReadyNow && (measAnim != anim_id::none)) {
		pick.source = display_source::meas;
		pick.anim = measAnim;
	} else {
		const display_entry *waiting = HAQuDA_GetDisplayEntry(waitingItem);
		if (waiting != nullptr) {
			pick.source = display_source::waiting;
			pick.item = waitingItem;
			pick.anim = waiting->anim.anim;
		}
	}

	pick.changed = (pick.source != shownSource) || (pick.item != shownItem) || (pick.anim != shownAnim) || (startSerial != shownSerial);
	shownSource = pick.source;
	shownItem = pick.item;
	shownAnim = pick.anim;
	shownSerial = startSerial;
	return pick;
}

uint8_t HAQuDA_DisplayScheduler::QueuedCount() const {
	return queueCount;
}

uint8_t HAQuDA_DisplayScheduler::DroppedCount() const {
	return droppedCount;
}
