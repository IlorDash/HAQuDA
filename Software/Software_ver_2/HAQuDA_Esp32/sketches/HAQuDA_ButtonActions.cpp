#include "HAQuDA_ButtonActions.h"

#include <cstdio>

namespace {
	const char *actionText(button_action action) {
		switch (action) {
			case button_action::power:
				return "power";
			case button_action::up:
				return "brighter";
			case button_action::down:
				return "dimmer";
			case button_action::next:
				return "next mode";
			case button_action::mode:
				return "mode";
			case button_action::show:
				return "show reading";
			case button_action::none:
				break;
		}
		return "none";
	}

	bool isBrightness(button_action action) {
		return (action == button_action::up) || (action == button_action::down);
	}
} // namespace

uint8_t HAQuDA_RepeatableMask(const buttons_settings &settings) {
	uint8_t mask = 0;
	for (uint8_t i = 0; i < TOUCH_BUTTONS_NUM; i++) {
		if (HAQuDA_ButtonRepeatable(settings.buttons[i])) {
			mask |= static_cast<uint8_t>(1 << i);
		}
	}
	return mask;
}

HAQuDA_ButtonActions::HAQuDA_ButtonActions(HAQuDA_ILampActions &lampActions, HAQuDA_ButtonsControl &buttonsControl, HAQuDA_ILogger &actionsLogger)
	: lamp(lampActions), buttons(buttonsControl), logger(actionsLogger) {
}

void HAQuDA_ButtonActions::Handle(const touch_gesture_event *events, uint8_t count, const buttons_settings &settings) {
	if (events == nullptr) {
		return;
	}
	buttons_settings current = settings;
	for (uint8_t i = 0; i < count; i++) {
		const bool repeatFollows = (events[i].kind == touch_gesture_kind::repeat) && (i + 1 < count) && (events[i + 1].kind == touch_gesture_kind::repeat)
								   && (events[i + 1].button == events[i].button);
		if (repeatFollows) {
			continue;
		}
		handleEvent(events[i], current);
	}
}

void HAQuDA_ButtonActions::handleEvent(const touch_gesture_event &event, buttons_settings &settings) {
	if (event.kind == touch_gesture_kind::lock_chord) {
		settings.locked = buttons.ToggleLocked();
		lamp.Flash(settings.locked ? lamp_flash::locked : lamp_flash::unlocked);
		logger.LogInfo(settings.locked ? "Touch buttons locked" : "Touch buttons unlocked");
		return;
	}
	if (event.kind == touch_gesture_kind::ap_chord) {
		logger.LogInfo("Touch buttons: access point requested");
		return;
	}
	if (event.kind == touch_gesture_kind::repeat_end) {
		// The held button may have been rebound or locked from the page since the steps: the steps still have to be saved
		lamp.CommitBrightness();
		if ((event.button < TOUCH_BUTTONS_NUM) && !settings.locked && isBrightness(settings.buttons[event.button].action)) {
			char text[24] = {};
			snprintf(text, sizeof(text), "%s, held", actionText(settings.buttons[event.button].action));
			logAction(event.button, text);
		}
		return;
	}
	if (settings.locked || (event.button >= TOUCH_BUTTONS_NUM)) {
		return;
	}

	const button_binding &binding = settings.buttons[event.button];
	switch (event.kind) {
		case touch_gesture_kind::tap:
			tap(event.button, binding);
			break;
		case touch_gesture_kind::repeat:
			if (isBrightness(binding.action)) {
				lamp.StepBrightness(binding.action == button_action::up);
			}
			break;
		case touch_gesture_kind::repeat_end:
		case touch_gesture_kind::lock_chord:
		case touch_gesture_kind::ap_chord:
			break;
	}
}

void HAQuDA_ButtonActions::tap(uint8_t button, const button_binding &binding) {
	bool refused = false;
	switch (binding.action) {
		case button_action::power:
			lamp.TogglePower();
			break;
		case button_action::up:
		case button_action::down:
			lamp.StepBrightness(binding.action == button_action::up);
			lamp.CommitBrightness();
			break;
		case button_action::next:
			lamp.NextMode();
			break;
		case button_action::mode:
			refused = !lamp.SetMode(binding.mode);
			break;
		case button_action::show:
			refused = !lamp.ShowReading(binding.param);
			break;
		case button_action::none:
			return;
	}
	if (refused) {
		lamp.Flash(lamp_flash::refused);
		char text[24] = {};
		snprintf(text, sizeof(text), "%s refused", actionText(binding.action));
		logAction(button, text);
		return;
	}
	logAction(button, actionText(binding.action));
}

void HAQuDA_ButtonActions::logAction(uint8_t button, const char *text) {
	char msg[48] = {};
	snprintf(msg, sizeof(msg), "Touch %u: %s", static_cast<unsigned>(TOUCH_BUTTON_PADS[button]), text);
	logger.LogInfo(msg);
}
