#pragma once

#include "Arduino.h"

#include <atomic>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "HAQuDA_DisplayEngine.h"
#include "HAQuDA_WebPageDisplayManip.h"

constexpr uint8_t DISPLAY_QUEUE_LEN = 16;

// The post functions are for any task except an ISR: they never block, a post that finds the queue full is dropped and counted.
// The engine and the LEDs belong to ShowTask from RunTask() on.
class HAQuDA_DisplayService {
  public:
	HAQuDA_DisplayService(HAQuDA_ILEDs &leds, HAQuDA_IMeasSource &measSource, HAQuDA_WebPageDisplayManip &webDisp, uint8_t brightPercent);

	// loop task, once in setup() after the LEDs are started and before ShowTask exists
	void Begin();

	bool Post(display_event event);
	bool Post(display_error error);
	bool End(display_end end);
	bool SetBackground(anim_id measAnim, display_item waiting);
	bool SetMeasEnabled(bool enabled);
	bool SetBrightness(uint8_t percent);
	// The lamp's own picture replaces the measurements as the background: a colour that events and errors show over, or dark that
	// only errors show over
	bool SetLampLight(uint32_t rgb);
	bool SetLampOff();

	uint16_t DroppedPostCount() const;

	// Posted and not yet ended, whether it plays now or waits in the rotation. Only the loop task writes the flag: it is the only sender of that error and its
	// end.
	bool SensorsErrorActive() const;

	// ShowTask only, never returns
	void RunTask();

  private:
	bool send(const display_message &message);
	void publishShown();

	HAQuDA_DisplayEngine engine;
	HAQuDA_WebPageDisplayManip &webDisp;
	uint32_t publishedSeq = 0;
	std::atomic<bool> sensorsErrorActive{false};
	// Static storage: no heap use, and the queue exists before the first post
	StaticQueue_t queueControl;
	uint8_t queueStorage[DISPLAY_QUEUE_LEN * sizeof(display_message)];
	QueueHandle_t queue = nullptr;
	std::atomic<uint16_t> droppedPosts{0};
};
