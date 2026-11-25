#pragma once

#include <cstdint>

#include "HAQuDA_ButtonsControl.h"
#include "HAQuDA_ButtonsSettings.h"
#include "HAQuDA_IApRequest.h"
#include "HAQuDA_ILampActions.h"
#include "HAQuDA_ILogger.h"
#include "HAQuDA_TouchGestures.h"

constexpr uint8_t TOUCH_BUTTON_PADS[TOUCH_BUTTONS_NUM] = {2, 6, 8, 9};

uint8_t HAQuDA_RepeatableMask(const buttons_settings &settings);

// Loop task only
class HAQuDA_ButtonActions {
  public:
	HAQuDA_ButtonActions(HAQuDA_ILampActions &lampActions, HAQuDA_ButtonsControl &buttonsControl, HAQuDA_ILogger &actionsLogger,
						 HAQuDA_IApRequest &accessPoint);

	// `events` is everything the loop drained in one pass, `settings` the copy taken for that pass. A run of repeat events of one button
	// does one step: after a long block of the loop the held button must not jump the brightness.
	void Handle(const touch_gesture_event *events, uint8_t count, const buttons_settings &settings);

  private:
	void handleEvent(const touch_gesture_event &event, buttons_settings &settings);
	void tap(uint8_t button, const button_binding &binding);
	void logAction(uint8_t button, const char *text);

	HAQuDA_ILampActions &lamp;
	HAQuDA_ButtonsControl &buttons;
	HAQuDA_ILogger &logger;
	HAQuDA_IApRequest &apRequest;
};
