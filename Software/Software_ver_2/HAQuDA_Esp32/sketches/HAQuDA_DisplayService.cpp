#include "HAQuDA_DisplayService.h"

HAQuDA_DisplayService::HAQuDA_DisplayService(HAQuDA_ILEDs &leds, HAQuDA_IMeasSource &measSource, uint8_t brightPercent)
	: engine(leds, measSource, brightPercent) {
}

void HAQuDA_DisplayService::Begin() {
	queue = xQueueCreateStatic(DISPLAY_QUEUE_LEN, sizeof(display_message), queueStorage, &queueControl);
	engine.Begin();
}

bool HAQuDA_DisplayService::Post(display_event event) {
	return send(display_message{display_msg::event, static_cast<uint8_t>(event), 0});
}

bool HAQuDA_DisplayService::Post(display_error error) {
	return send(display_message{display_msg::error, static_cast<uint8_t>(error), 0});
}

bool HAQuDA_DisplayService::End(display_end end) {
	return send(display_message{display_msg::end, static_cast<uint8_t>(end), 0});
}

bool HAQuDA_DisplayService::SetBackground(anim_id measAnim, display_item waiting) {
	return send(display_message{display_msg::background, static_cast<uint8_t>(measAnim), waiting});
}

bool HAQuDA_DisplayService::SetMeasEnabled(bool enabled) {
	return send(display_message{display_msg::measEnabled, static_cast<uint8_t>(enabled ? 1 : 0), 0});
}

bool HAQuDA_DisplayService::SetBrightness(uint8_t percent) {
	return send(display_message{display_msg::brightness, percent, 0});
}

bool HAQuDA_DisplayService::SetLampLight(uint32_t rgb) {
	return send(display_message{display_msg::lampLight, 0, 0, rgb});
}

bool HAQuDA_DisplayService::SetLampOff() {
	return send(display_message{display_msg::lampOff, 0, 0, 0});
}

uint16_t HAQuDA_DisplayService::DroppedPostCount() const {
	return droppedPosts.load();
}

bool HAQuDA_DisplayService::send(const display_message &message) {
	if ((queue == nullptr) || (xQueueSend(queue, &message, 0) != pdTRUE)) {
		droppedPosts++;
		return false;
	}
	return true;
}

void HAQuDA_DisplayService::RunTask() {
	while (true) {
		const uint32_t sleepMs = engine.Run(millis());
		// A post wakes the task at once, so an error interrupts the running animation without waiting for its next step
		display_message message;
		if (xQueueReceive(queue, &message, pdMS_TO_TICKS(sleepMs)) == pdTRUE) {
			do {
				engine.Apply(message);
			} while (xQueueReceive(queue, &message, 0) == pdTRUE);
		}
	}
}
