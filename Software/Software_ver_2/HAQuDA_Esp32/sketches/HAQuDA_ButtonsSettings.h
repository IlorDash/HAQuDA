#pragma once

#include <cstddef>
#include <cstdint>

#include "HAQuDA_LampSettings.h"
#include "HAQuDA_TouchGestures.h"

enum class button_action : uint8_t { none, power, up, down, next, mode, show };

// `mode` is meaningful for the mode action only and `param` for the show action only; any other action keeps them at standard / total
// so that equal bindings compare equal.
struct button_binding {
	button_action action;
	lamp_mode mode;
	lamp_param param;
};

// Pad order 2, 6, 8, 9: the index is the index of the touch buttons and of the gesture events
struct buttons_settings {
	button_binding buttons[TOUCH_BUTTONS_NUM];
	bool locked;
};

// update HAQuDA_ButtonsSame when a field is added
static_assert(sizeof(button_binding) == 3 && sizeof(buttons_settings) == 3 * TOUCH_BUTTONS_NUM + 1, "update HAQuDA_ButtonsSame");

bool HAQuDA_ButtonsSame(const buttons_settings &a, const buttons_settings &b);

struct buttons_request {
	const char *action[TOUCH_BUTTONS_NUM];
	const char *arg[TOUCH_BUTTONS_NUM];
	const char *locked;
	const char *reset;
};

// The longest JSON is 185 bytes: 164 with every button on the mode action with the standard mode and the lock off, plus 21 for the
// saveErr field at its largest value (4294967295); the rest is spare
constexpr size_t BUTTONS_JSON_BUF_LEN = 192;
constexpr size_t BUTTONS_RECORD_LEN = 16;

// Stores the pointer, not a copy: `value` must outlive the request
bool HAQuDA_SetButtonsField(buttons_request &request, const char *name, const char *value);

enum class buttons_update_result : uint8_t { ok, nothing, field_too_long, bad_action, bad_arg, arg_not_allowed, bad_locked, bad_reset, reset_with_fields };

buttons_settings HAQuDA_ButtonsDefaults();

bool HAQuDA_ButtonsSettingsValid(const buttons_settings &settings);

bool HAQuDA_ButtonRepeatable(const button_binding &binding);

// All or nothing: any bad field leaves `settings` unchanged. Switching to mode or show needs the argument in the same request; reset=1
// restores the bindings, keeps the lock and cannot be combined with another field.
buttons_update_result HAQuDA_ButtonsUpdate(buttons_settings &settings, const buttons_request &request);

const char *HAQuDA_ButtonsUpdateText(buttons_update_result result);

// Returns the length, 0 when it does not fit or `settings` is not valid
size_t HAQuDA_BuildButtonsJson(const buttons_settings &settings, uint32_t saveErrors, char *buf, size_t size);

// Decode leaves `out` untouched when the length, magic, version, checksum or any field is wrong
bool HAQuDA_EncodeButtonsRecord(const buttons_settings &settings, uint8_t *out, size_t size);
bool HAQuDA_DecodeButtonsRecord(const uint8_t *data, size_t len, buttons_settings &out);
