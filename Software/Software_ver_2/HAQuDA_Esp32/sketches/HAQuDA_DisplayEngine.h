#pragma once

#include <cstdint>

#include "HAQuDA_AnimationSet.h"
#include "HAQuDA_DisplayScheduler.h"
#include "HAQuDA_IMeasSource.h"

constexpr uint32_t DISPLAY_MEAS_PERIOD_MS = 60 * 1000;

enum class display_msg : uint8_t { event, error, end, background, measEnabled, brightness };

// What a producer task sends to ShowTask; value and extra depend on kind: event/error/end carry the enum value, background carries
// the measurement animation and the waiting item, measEnabled the flag, brightness the percent
// The engine decides itself when the measurements are ready: the first sample ends the waiting item
struct display_message {
	display_msg kind;
	uint8_t value;
	uint8_t extra;
};

// ShowTask only: not thread-safe. The FreeRTOS queue and the task loop live in HAQuDA_DisplayService; time comes in as nowMs
class HAQuDA_DisplayEngine {
  public:
	HAQuDA_DisplayEngine(HAQuDA_ILEDs &leds, HAQuDA_IMeasSource &measSource, uint8_t brightPercent);

	// Applies the brightness to the LEDs. Before ShowTask exists
	void Begin();

	void Apply(const display_message &message);

	// Returns how long ShowTask may sleep: an animation step is due or the shown item's time is over by then; never 0
	uint32_t Run(uint32_t nowMs);

	anim_id ActiveAnim() const;
	uint8_t BrightPercent() const;

  private:
	void probeSample();
	void applyWanted(uint32_t nowMs);
	void startItem(uint32_t nowMs);
	bool startMeas(uint32_t nowMs);
	void refreshMeas(uint32_t nowMs);
	bool measPeriodOver(uint32_t nowMs) const;
	void stopActive();
	void setBrightness(uint8_t percent);
	anim_context makeContext(uint32_t nowMs);

	HAQuDA_ILEDs &leds;
	HAQuDA_IMeasSource &measSource;
	HAQuDA_AnimationSet animations;
	HAQuDA_DisplayScheduler scheduler;

	uint8_t brightPercent;
	uint32_t seedState = 0x9E3779B9;

	HAQuDA_Animation *active = nullptr;
	display_pick wanted = {false, display_source::none, DISPLAY_ITEM_NONE, anim_id::none, DISPLAY_NO_DEADLINE};
	// The scheduler's choice is not shown yet: the measurements lost their sample when the averaging restarted (a brightness change
	// right after a redraw), the previous animation plays meanwhile
	bool switchPending = false;
	bool measEnabled = false;
	bool sampleSeen = false;
	uint32_t measPeriodStartMs = 0;
	uint32_t nextStepMs = 0;
};
