#include "HAQuDA_DisplayEngine.h"

namespace {
	constexpr uint8_t MAX_PERCENT = 100;
	constexpr uint32_t MIN_SLEEP_MS = 1;

	bool isMeasAnim(anim_id id) {
		return (id == anim_id::measStandard) || (id == anim_id::measMulti) || (id == anim_id::measNight) || (id == anim_id::measTotal);
	}
} // namespace

HAQuDA_DisplayEngine::HAQuDA_DisplayEngine(HAQuDA_ILEDs &leds, HAQuDA_IMeasSource &measSource, uint8_t brightPercent)
	: leds(leds), measSource(measSource), animations(leds), brightPercent((brightPercent > MAX_PERCENT) ? MAX_PERCENT : brightPercent) {
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
			}
			break;
		case display_msg::measEnabled:
			measEnabled = (message.value != 0);
			sampleSeen = false;
			scheduler.MeasReady(false);
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
	} else {
		startItem(nowMs);
	}
}

void HAQuDA_DisplayEngine::startSolid(uint32_t nowMs) {
	HAQuDA_Animation *next = animations.Get(anim_id::staticColor);
	if ((next != nullptr) && next->Begin(HAQuDA_StaticColorParams(solidColor), makeContext(nowMs))) {
		active = next;
		nextStepMs = nowMs;
	}
}

void HAQuDA_DisplayEngine::startItem(uint32_t nowMs) {
	const display_entry *entry = HAQuDA_GetDisplayEntry(wanted.item);
	if (entry == nullptr) {
		return;
	}
	HAQuDA_Animation *next = animations.Get(entry->anim.anim);
	if ((next != nullptr) && next->Begin(entry->anim, makeContext(nowMs))) {
		active = next;
		nextStepMs = nowMs;
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
	measSource.RestartAveraging();
	measPeriodStartMs = nowMs;
	return true;
}

void HAQuDA_DisplayEngine::refreshMeas(uint32_t nowMs) {
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
