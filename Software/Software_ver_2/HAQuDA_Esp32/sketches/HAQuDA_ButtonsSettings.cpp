#include "HAQuDA_ButtonsSettings.h"

#include <cstdio>
#include <cstring>

#include "HAQuDA_RecordCodec.h"

namespace {
	constexpr size_t MAX_FIELD_LEN = 16;
	constexpr size_t ACTION_NUM = 7;
	constexpr uint32_t BUTTONS_RECORD_MAGIC = 0x54544248; // "HBTT", little endian
	constexpr uint8_t BUTTONS_RECORD_VERSION = 1;
	constexpr size_t RECORD_BODY_LEN = BUTTONS_RECORD_LEN - 2;
	constexpr size_t RECORD_BUTTONS_OFFSET = 6;
	static_assert(RECORD_BUTTONS_OFFSET + 2 * TOUCH_BUTTONS_NUM == RECORD_BODY_LEN,
				  "the record holds the version, the lock and an action with an argument per button");

	const char *const ACTION_NAMES[ACTION_NUM] = {"none", "power", "up", "down", "next", "mode", "show"};
	const char *const BUTTON_NAMES[TOUCH_BUTTONS_NUM] = {"2", "6", "8", "9"};

	enum class field_kind : uint8_t { action, arg, locked, reset };

	struct form_field {
		const char *name;
		field_kind kind;
		uint8_t index;
	};

	const form_field FORM_FIELDS[]
		= {{"a2", field_kind::action, 0},	  {"a6", field_kind::action, 1},  {"a8", field_kind::action, 2}, {"a9", field_kind::action, 3},
		   {"arg2", field_kind::arg, 0},	  {"arg6", field_kind::arg, 1},	  {"arg8", field_kind::arg, 2},	 {"arg9", field_kind::arg, 3},
		   {"locked", field_kind::locked, 0}, {"reset", field_kind::reset, 0}};

	const char *&slotOf(buttons_request &request, const form_field &field) {
		switch (field.kind) {
			case field_kind::action:
				return request.action[field.index];
			case field_kind::arg:
				return request.arg[field.index];
			case field_kind::locked:
				return request.locked;
			case field_kind::reset:
			default:
				return request.reset;
		}
	}

	bool parseAction(const char *text, button_action &out) {
		for (size_t i = 0; i < ACTION_NUM; i++) {
			if (strcmp(text, ACTION_NAMES[i]) == 0) {
				out = static_cast<button_action>(i);
				return true;
			}
		}
		return false;
	}

	bool takesArg(button_action action) {
		return (action == button_action::mode) || (action == button_action::show);
	}

	bool fieldFits(const char *text) {
		return (text == nullptr) || (strnlen(text, MAX_FIELD_LEN + 1) <= MAX_FIELD_LEN);
	}

	bool parseArg(button_binding &binding, const char *text) {
		if (binding.action == button_action::mode) {
			return HAQuDA_ParseLampMode(text, binding.mode);
		}
		return HAQuDA_ParseLampParam(text, binding.param);
	}

	bool bindingValid(const button_binding &b) {
		if (static_cast<size_t>(b.action) >= ACTION_NUM) {
			return false;
		}
		if (b.action == button_action::mode) {
			return (static_cast<size_t>(b.mode) < LAMP_MODE_NUM) && (b.param == lamp_param::total);
		}
		if (b.action == button_action::show) {
			return (static_cast<size_t>(b.param) < LAMP_PARAM_NUM) && (b.mode == lamp_mode::standard);
		}
		return (b.mode == lamp_mode::standard) && (b.param == lamp_param::total);
	}

	uint8_t argByte(const button_binding &b) {
		if (b.action == button_action::mode) {
			return static_cast<uint8_t>(b.mode);
		}
		if (b.action == button_action::show) {
			return static_cast<uint8_t>(b.param);
		}
		return 0;
	}

	button_binding binding(button_action action) {
		return {action, lamp_mode::standard, lamp_param::total};
	}
} // namespace

bool HAQuDA_SetButtonsField(buttons_request &request, const char *name, const char *value) {
	if ((name == nullptr) || (value == nullptr)) {
		return false;
	}
	for (const form_field &field : FORM_FIELDS) {
		if (strcmp(name, field.name) != 0) {
			continue;
		}
		const char *&slot = slotOf(request, field);
		if (slot != nullptr) {
			return false;
		}
		slot = value;
		return true;
	}
	return false;
}

buttons_settings HAQuDA_ButtonsDefaults() {
	buttons_settings s{};
	s.buttons[0] = binding(button_action::power);
	s.buttons[1] = binding(button_action::up);
	s.buttons[2] = binding(button_action::next);
	s.buttons[3] = binding(button_action::down);
	s.locked = false;
	return s;
}

bool HAQuDA_ButtonsSettingsValid(const buttons_settings &settings) {
	for (const button_binding &b : settings.buttons) {
		if (!bindingValid(b)) {
			return false;
		}
	}
	return true;
}

bool HAQuDA_ButtonRepeatable(const button_binding &b) {
	return (b.action == button_action::up) || (b.action == button_action::down);
}

buttons_update_result HAQuDA_ButtonsUpdate(buttons_settings &settings, const buttons_request &request) {
	bool any = false;
	for (size_t i = 0; i < TOUCH_BUTTONS_NUM; i++) {
		if (!fieldFits(request.action[i]) || !fieldFits(request.arg[i])) {
			return buttons_update_result::field_too_long;
		}
		any = any || (request.action[i] != nullptr) || (request.arg[i] != nullptr);
	}
	if (!fieldFits(request.locked) || !fieldFits(request.reset)) {
		return buttons_update_result::field_too_long;
	}
	if (!any && (request.locked == nullptr) && (request.reset == nullptr)) {
		return buttons_update_result::nothing;
	}

	if (request.reset != nullptr) {
		if (strcmp(request.reset, "1") != 0) {
			return buttons_update_result::bad_reset;
		}
		if (any || (request.locked != nullptr)) {
			return buttons_update_result::reset_with_fields;
		}
		const buttons_settings defaults = HAQuDA_ButtonsDefaults();
		for (size_t i = 0; i < TOUCH_BUTTONS_NUM; i++) {
			settings.buttons[i] = defaults.buttons[i];
		}
		return buttons_update_result::ok;
	}

	buttons_settings next = settings;
	if (request.locked != nullptr) {
		if (strcmp(request.locked, "1") == 0) {
			next.locked = true;
		} else if (strcmp(request.locked, "0") == 0) {
			next.locked = false;
		} else {
			return buttons_update_result::bad_locked;
		}
	}
	for (size_t i = 0; i < TOUCH_BUTTONS_NUM; i++) {
		const char *const actionText = request.action[i];
		const char *const argText = request.arg[i];
		if (actionText != nullptr) {
			button_action action = button_action::none;
			if (!parseAction(actionText, action)) {
				return buttons_update_result::bad_action;
			}
			next.buttons[i] = binding(action);
			if (takesArg(action)) {
				if ((argText == nullptr) || !parseArg(next.buttons[i], argText)) {
					return buttons_update_result::bad_arg;
				}
			} else if (argText != nullptr) {
				return buttons_update_result::arg_not_allowed;
			}
		} else if (argText != nullptr) {
			if (!takesArg(next.buttons[i].action)) {
				return buttons_update_result::arg_not_allowed;
			}
			if (!parseArg(next.buttons[i], argText)) {
				return buttons_update_result::bad_arg;
			}
		}
	}
	settings = next;
	return buttons_update_result::ok;
}

const char *HAQuDA_ButtonsUpdateText(buttons_update_result result) {
	switch (result) {
		case buttons_update_result::nothing:
			return "Nothing to change.";
		case buttons_update_result::field_too_long:
			return "A value is too long.";
		case buttons_update_result::bad_action:
			return "Unknown button action.";
		case buttons_update_result::bad_arg:
			return "The mode and show actions need a known mode or reading.";
		case buttons_update_result::arg_not_allowed:
			return "Only the mode and show actions take an argument.";
		case buttons_update_result::bad_locked:
			return "Locked must be 1 or 0.";
		case buttons_update_result::bad_reset:
			return "Reset must be 1.";
		case buttons_update_result::reset_with_fields:
			return "Reset cannot be combined with other fields.";
		case buttons_update_result::ok:
		default:
			return "";
	}
}

size_t HAQuDA_BuildButtonsJson(const buttons_settings &s, char *buf, size_t size) {
	if ((buf == nullptr) || (size == 0)) {
		return 0;
	}
	buf[0] = '\0';
	if (!HAQuDA_ButtonsSettingsValid(s)) {
		return 0;
	}
	size_t len = 0;
	const int head = snprintf(buf, size, "{\"locked\":%s,\"buttons\":{", s.locked ? "true" : "false");
	if ((head < 0) || (static_cast<size_t>(head) >= size)) {
		buf[0] = '\0';
		return 0;
	}
	len = static_cast<size_t>(head);
	for (size_t i = 0; i < TOUCH_BUTTONS_NUM; i++) {
		const button_binding &b = s.buttons[i];
		const char *const actionName = ACTION_NAMES[static_cast<size_t>(b.action)];
		int written = 0;
		if (b.action == button_action::mode) {
			written = snprintf(buf + len, size - len, "%s\"%s\":{\"a\":\"%s\",\"arg\":\"%s\"}", (i == 0) ? "" : ",", BUTTON_NAMES[i], actionName,
							   HAQuDA_LampModeName(b.mode));
		} else if (b.action == button_action::show) {
			written = snprintf(buf + len, size - len, "%s\"%s\":{\"a\":\"%s\",\"arg\":\"%s\"}", (i == 0) ? "" : ",", BUTTON_NAMES[i], actionName,
							   HAQuDA_LampParamName(b.param));
		} else {
			written = snprintf(buf + len, size - len, "%s\"%s\":{\"a\":\"%s\"}", (i == 0) ? "" : ",", BUTTON_NAMES[i], actionName);
		}
		if ((written < 0) || (static_cast<size_t>(written) >= size - len)) {
			buf[0] = '\0';
			return 0;
		}
		len += static_cast<size_t>(written);
	}
	const int tail = snprintf(buf + len, size - len, "}}");
	if ((tail < 0) || (static_cast<size_t>(tail) >= size - len)) {
		buf[0] = '\0';
		return 0;
	}
	return len + static_cast<size_t>(tail);
}

bool HAQuDA_EncodeButtonsRecord(const buttons_settings &s, uint8_t *out, size_t size) {
	if ((out == nullptr) || (size < BUTTONS_RECORD_LEN) || !HAQuDA_ButtonsSettingsValid(s)) {
		return false;
	}
	HAQuDA_PutU32(out, BUTTONS_RECORD_MAGIC);
	out[4] = BUTTONS_RECORD_VERSION;
	out[5] = s.locked ? 1 : 0;
	for (size_t i = 0; i < TOUCH_BUTTONS_NUM; i++) {
		out[RECORD_BUTTONS_OFFSET + 2 * i] = static_cast<uint8_t>(s.buttons[i].action);
		out[RECORD_BUTTONS_OFFSET + 2 * i + 1] = argByte(s.buttons[i]);
	}
	HAQuDA_PutU16(out + RECORD_BODY_LEN, HAQuDA_Fletcher16(out, RECORD_BODY_LEN));
	return true;
}

bool HAQuDA_DecodeButtonsRecord(const uint8_t *data, size_t len, buttons_settings &out) {
	if ((data == nullptr) || (len != BUTTONS_RECORD_LEN)) {
		return false;
	}
	if ((HAQuDA_GetU32(data) != BUTTONS_RECORD_MAGIC) || (data[4] != BUTTONS_RECORD_VERSION)
		|| (HAQuDA_GetU16(data + RECORD_BODY_LEN) != HAQuDA_Fletcher16(data, RECORD_BODY_LEN)) || (data[5] > 1)) {
		return false;
	}
	buttons_settings s{};
	s.locked = data[5] == 1;
	for (size_t i = 0; i < TOUCH_BUTTONS_NUM; i++) {
		const uint8_t actionByte = data[RECORD_BUTTONS_OFFSET + 2 * i];
		const uint8_t arg = data[RECORD_BUTTONS_OFFSET + 2 * i + 1];
		if (actionByte >= ACTION_NUM) {
			return false;
		}
		button_binding b = binding(static_cast<button_action>(actionByte));
		if (b.action == button_action::mode) {
			b.mode = static_cast<lamp_mode>(arg);
		} else if (b.action == button_action::show) {
			b.param = static_cast<lamp_param>(arg);
		} else if (arg != 0) {
			return false;
		}
		s.buttons[i] = b;
	}
	if (!HAQuDA_ButtonsSettingsValid(s)) {
		return false;
	}
	out = s;
	return true;
}
