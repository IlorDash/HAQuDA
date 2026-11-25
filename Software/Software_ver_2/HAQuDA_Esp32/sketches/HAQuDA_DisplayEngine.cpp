#include "HAQuDA_DisplayEngine.h"

namespace {
	constexpr uint8_t MAX_PERCENT = 100;
	constexpr uint32_t MIN_SLEEP_MS = 1;

	bool isMeasAnim(anim_id id) {
		return (id == anim_id::measStandard) || (id == anim_id::measMulti) || (id == anim_id::measNight) || (id == anim_id::measTotal);
	}

	bool sameColor(const rgb_color &a, const rgb_color &b) {
		return (a.r == b.r) && (a.g == b.g) && (a.b == b.b);
	}
} // namespace

HAQuDA_DisplayEngine::HAQuDA_DisplayEngine(HAQuDA_ILEDs &leds, HAQuDA_IMeasSource &measSource, uint8_t brightPercent)
	: leds(leds), measSource(measSource), animations(leds), brightPercent((brightPercent > MAX_PERCENT) ? MAX_PERCENT : brightPercent) {
	shown.item = DISPLAY_ITEM_NONE;
	shown.anim = HAQuDA_MeasParams(anim_id::none);
}

bool HAQuDA_SensorsErrorActiveAfter(bool active, const display_message &message) {
	if ((message.kind == display_msg::error) && (message.value == static_cast<uint8_t>(display_error::sensorsFailed))) {
		return true;
	}
	if ((message.kind == display_msg::end) && (message.value == static_cast<uint8_t>(display_end::sensorsRecovered))) {
		return false;
	}
	return active;
}

void HAQuDA_DisplayEngine::Begin() {
	leds.SetBright(brightPercent);
}

void HAQuDA_DisplayEngine::Apply(const display_message &message) {
	switch (message.kind) {
		case display_msg::event:
			scheduler.Post(static_cast<display_event>(message.value));
			break;
		case display_msg::error:
			scheduler.Post(static_cast<display_error>(message.value));
			break;
		case display_msg::end:
			if (message.value < DISPLAY_END_COUNT) {
				scheduler.End(static_cast<display_end>(message.value));
			}
			break;
		case display_msg::background:
			if (message.value <= ANIM_COUNT) {
				scheduler.SetBackground(static_cast<anim_id>(message.value), message.extra);
				measRedrawPending = true;
			}
			break;
		case display_msg::measEnabled:
			// Enabling what is already enabled must not send a shown picture back to the waiting one
			if ((message.value != 0) != measEnabled) {
				measEnabled = (message.value != 0);
				sampleSeen = false;
				scheduler.MeasReady(false);
			}
			break;
		case display_msg::brightness:
			setBrightness(message.value);
			break;
		case display_msg::lampLight:
			solidColor = message.color;
			scheduler.SetSolid(false);
			break;
		case display_msg::lampOff:
			solidColor = ANIM_COLOR_OFF;
			scheduler.SetSolid(true);
			break;
	}
}

uint32_t HAQuDA_DisplayEngine::Run(uint32_t nowMs) {
	if (measEnabled && !sampleSeen) {
		probeSample();
	}
	const display_pick pick = scheduler.Next(nowMs);
	if (pick.changed) {
		wanted = pick;
		switchPending = true;
		measPeriodStartMs = nowMs;
	}
	if (switchPending) {
		applyWanted(nowMs);
	} else if (wanted.source == display_source::meas) {
		refreshMeas(nowMs);
	}

	uint32_t sleepMs = ANIM_IDLE_MS;
	if (active != nullptr) {
		// A post wakes ShowTask before the step is due, and stepping early would run the animation faster
		if (static_cast<int32_t>(nowMs - nextStepMs) >= 0) {
			nextStepMs = nowMs + active->Step(nowMs);
			if (isMeasAnim(active->Id())) {
				recordMeasDrawn(static_cast<HAQuDA_MeasAnim *>(active)->Drawn(), nowMs);
			}
		}
		sleepMs = nextStepMs - nowMs;
	}
	if (pick.nextMs < sleepMs) {
		sleepMs = pick.nextMs;
	}
	if (switchPending && (sleepMs > ANIM_IDLE_MS)) {
		sleepMs = ANIM_IDLE_MS;
	}
	if ((wanted.source == display_source::meas) && !switchPending) {
		const uint32_t periodLeftMs = DISPLAY_MEAS_PERIOD_MS - static_cast<uint32_t>(nowMs - measPeriodStartMs);
		if (periodLeftMs < sleepMs) {
			sleepMs = periodLeftMs;
		}
	}
	return (sleepMs < MIN_SLEEP_MS) ? MIN_SLEEP_MS : sleepMs;
}

anim_id HAQuDA_DisplayEngine::ActiveAnim() const {
	return (active != nullptr) ? active->Id() : anim_id::none;
}

uint8_t HAQuDA_DisplayEngine::BrightPercent() const {
	return brightPercent;
}

const display_shown &HAQuDA_DisplayEngine::Shown() const {
	return shown;
}

void HAQuDA_DisplayEngine::probeSample() {
	anim_id anim = anim_id::none;
	meas_anim_data data = {};
	if (measSource.ReadAverages(anim, data)) {
		sampleSeen = true;
		scheduler.MeasReady(true);
	}
}

void HAQuDA_DisplayEngine::applyWanted(uint32_t nowMs) {
	if (wanted.source == display_source::meas) {
		if (startMeas(nowMs)) {
			switchPending = false;
		} else if (measPeriodOver(nowMs)) {
			// Bounds the sums of the parameters that do have samples while another one has none
			measSource.RestartAveraging();
			measPeriodStartMs = nowMs;
		}
		return;
	}
	stopActive();
	switchPending = false;
	if (wanted.source == display_source::solid) {
		startSolid(nowMs);
	} else if (wanted.source == display_source::none) {
		// The matrix keeps its last frame
		beginPicture(display_source::none, DISPLAY_ITEM_NONE, HAQuDA_MeasParams(anim_id::none), 0);
		markChanged(nowMs);
	} else {
		startItem(nowMs);
	}
}

void HAQuDA_DisplayEngine::startSolid(uint32_t nowMs) {
	const anim_params params = HAQuDA_StaticColorParams(solidColor);
	const anim_context context = makeContext(nowMs);
	HAQuDA_Animation *next = animations.Get(anim_id::staticColor);
	if ((next != nullptr) && next->Begin(params, context)) {
		active = next;
		nextStepMs = nowMs;
		beginPicture(display_source::solid, DISPLAY_ITEM_NONE, params, context.seed);
		markChanged(nowMs);
	}
}

void HAQuDA_DisplayEngine::startItem(uint32_t nowMs) {
	const display_entry *entry = HAQuDA_GetDisplayEntry(wanted.item);
	if (entry == nullptr) {
		return;
	}
	const anim_context context = makeContext(nowMs);
	HAQuDA_Animation *next = animations.Get(entry->anim.anim);
	if ((next != nullptr) && next->Begin(entry->anim, context)) {
		active = next;
		nextStepMs = nowMs;
		beginPicture(wanted.source, wanted.item, entry->anim, context.seed);
		markChanged(nowMs);
	}
}

bool HAQuDA_DisplayEngine::startMeas(uint32_t nowMs) {
	anim_id anim = anim_id::none;
	anim_context context = makeContext(nowMs);
	if (!measSource.ReadAverages(anim, context.meas)) {
		return false;
	}
	HAQuDA_Animation *next = animations.Get(anim);
	if (next == nullptr) {
		return false;
	}
	// A rejected start must leave the running picture alone, or the matrix freezes on its last frame
	if (!next->Begin(HAQuDA_MeasParams(anim), context)) {
		if (active == next) {
			active = nullptr;
		}
		return false;
	}
	if (active != next) {
		stopActive();
	}
	active = next;
	nextStepMs = nowMs;
	if ((shown.source != display_source::meas) || (shown.anim.anim != anim)) {
		// Not marked: the draw that follows in this Run() raises seq, and a redraw of the same animation must not
		beginPicture(display_source::meas, DISPLAY_ITEM_NONE, HAQuDA_MeasParams(anim), 0);
	}
	measSource.RestartAveraging();
	measPeriodStartMs = nowMs;
	measRedrawPending = false;
	return true;
}

void HAQuDA_DisplayEngine::refreshMeas(uint32_t nowMs) {
	// A changed setting redraws at once; without a sample the running picture stays until one arrives
	if (measRedrawPending && startMeas(nowMs)) {
		return;
	}
	if (!measPeriodOver(nowMs)) {
		return;
	}
	if (!startMeas(nowMs)) {
		measSource.RestartAveraging();
		measPeriodStartMs = nowMs;
	}
}

bool HAQuDA_DisplayEngine::measPeriodOver(uint32_t nowMs) const {
	return static_cast<uint32_t>(nowMs - measPeriodStartMs) >= DISPLAY_MEAS_PERIOD_MS;
}

void HAQuDA_DisplayEngine::stopActive() {
	if (active != nullptr) {
		active->Stop();
		active = nullptr;
	}
}

void HAQuDA_DisplayEngine::setBrightness(uint8_t percent) {
	const uint8_t limited = (percent > MAX_PERCENT) ? MAX_PERCENT : percent;
	if (limited == brightPercent) {
		return;
	}
	// Restarting a meas picture re-reads the averages, which a quick second change finds empty: the matrix would stay dark
	const bool keepMeas = (active != nullptr) && isMeasAnim(active->Id());
	if (!keepMeas) {
		// A fade puts back the brightness it saw at Begin(), which must not undo this one
		stopActive();
	}
	brightPercent = limited;
	leds.SetBright(brightPercent);
	if (!keepMeas) {
		switchPending = (wanted.source != display_source::none);
	}
}

anim_context HAQuDA_DisplayEngine::makeContext(uint32_t nowMs) {
	seedState ^= nowMs;
	if (seedState == 0) {
		seedState = 1;
	}
	anim_context context = {};
	context.lampBrightPercent = brightPercent;
	context.seed = HAQuDA_XorShift32(seedState);
	return context;
}

void HAQuDA_DisplayEngine::beginPicture(display_source source, display_item item, const anim_params &anim, uint32_t seed) {
	shown.source = source;
	shown.item = item;
	shown.anim = anim;
	shown.seed = seed;
	shown.measDrawn = false;
	shown.colorCount = 0;
	shown.nightLitCells = 0;
	shown.white = rgb_color{0, 0, 0};
	for (uint8_t i = 0; i < MEAS_ANIM_KEYS; i++) {
		shown.keys[i] = total;
		shown.colors[i] = rgb_color{0, 0, 0};
	}
}

void HAQuDA_DisplayEngine::markChanged(uint32_t nowMs) {
	shown.seq++;
	shown.startMs = nowMs;
}

void HAQuDA_DisplayEngine::recordMeasDrawn(const meas_drawn &drawn, uint32_t nowMs) {
	bool changed
		= !shown.measDrawn || (shown.colorCount != drawn.colorCount) || (shown.nightLitCells != drawn.nightLitCells) || !sameColor(shown.white, drawn.white);
	for (uint8_t i = 0; i < MEAS_ANIM_KEYS; i++) {
		changed = changed || !sameColor(shown.colors[i], drawn.colors[i]) || (shown.keys[i] != drawn.keys[i]);
		shown.colors[i] = drawn.colors[i];
		shown.keys[i] = drawn.keys[i];
	}
	shown.colorCount = drawn.colorCount;
	shown.nightLitCells = drawn.nightLitCells;
	shown.white = drawn.white;
	shown.measDrawn = true;
	if (changed) {
		markChanged(nowMs);
	}
}
