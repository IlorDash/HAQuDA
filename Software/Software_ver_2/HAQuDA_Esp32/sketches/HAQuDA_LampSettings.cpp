#include "HAQuDA_LampSettings.h"

#include <cstdio>
#include <cstring>

#include "HAQuDA_LEDsParams.h"
#include "HAQuDA_NightProgress.h"

static_assert((DEFAULT_BRIGHTNESS >= LAMP_BRIGHT_MIN) && (DEFAULT_BRIGHTNESS <= LAMP_BRIGHT_MAX) && ((DEFAULT_BRIGHTNESS % LAMP_BRIGHT_STEP) == 0),
			  "the boot brightness must be a step of the lamp page's brightness scale");

namespace {
	constexpr size_t MAX_FIELD_LEN = 32;
	constexpr size_t PARAM_NUM = 6;
	constexpr size_t MODE_NUM = 4;
	constexpr uint32_t LAMP_RECORD_MAGIC = 0x504D4C48; // "HLMP", little endian
	constexpr uint8_t LAMP_RECORD_VERSION = 1;
	constexpr size_t RECORD_BODY_LEN = LAMP_RECORD_LEN - 2;
	// The page's key names, in the order of lamp_param
	const char *const PARAM_NAMES[PARAM_NUM] = {"total", "temp", "humid", "eco2", "tvoc", "pm25"};
	const char *const MODE_NAMES[MODE_NUM] = {"standard", "multi", "night", "light"};
	// Warm white, neutral, cool white, amber, lavender, teal
	const uint32_t COLORS[LAMP_COLORS_NUM] = {0xFFCF8F, 0xFFF1DC, 0xDCE9FF, 0xFF9F43, 0x8F8CFF, 0x36C9B8};

	bool sameText(const char *text, size_t len, const char *name) {
		return (strlen(name) == len) && (memcmp(text, name, len) == 0);
	}

	bool parseParam(const char *text, size_t len, bool allowTotal, lamp_param &out) {
		for (size_t i = allowTotal ? 0 : 1; i < PARAM_NUM; i++) {
			if (sameText(text, len, PARAM_NAMES[i])) {
				out = static_cast<lamp_param>(i);
				return true;
			}
		}
		return false;
	}

	// Decimal digits only, at most 3 of them (the largest value any field takes is 100)
	bool parseSmallNumber(const char *text, size_t len, uint32_t &out) {
		if ((len == 0) || (len > 3)) {
			return false;
		}
		uint32_t value = 0;
		for (size_t i = 0; i < len; i++) {
			if ((text[i] < '0') || (text[i] > '9')) {
				return false;
			}
			value = value * 10 + static_cast<uint32_t>(text[i] - '0');
		}
		out = value;
		return true;
	}

	bool parseTime(const char *text, size_t len, uint16_t &out) {
		if ((len != 5) || (text[2] != ':')) {
			return false;
		}
		uint32_t hours = 0;
		uint32_t minutes = 0;
		if (!parseSmallNumber(text, 2, hours) || !parseSmallNumber(text + 3, 2, minutes) || (hours > 23) || (minutes > 59)) {
			return false;
		}
		out = static_cast<uint16_t>(hours * 60 + minutes);
		return true;
	}

	bool parseBright(const char *text, size_t len, uint8_t &out) {
		uint32_t value = 0;
		if (!parseSmallNumber(text, len, value) || (value < LAMP_BRIGHT_MIN) || (value > LAMP_BRIGHT_MAX) || ((value % LAMP_BRIGHT_STEP) != 0)) {
			return false;
		}
		out = static_cast<uint8_t>(value);
		return true;
	}

	bool parseMulti(const char *text, size_t len, lamp_param (&out)[LAMP_MULTI_NUM]) {
		lamp_param parsed[LAMP_MULTI_NUM] = {lamp_param::temp, lamp_param::temp, lamp_param::temp};
		size_t start = 0;
		for (size_t i = 0; i < LAMP_MULTI_NUM; i++) {
			size_t end = start;
			while ((end < len) && (text[end] != ',')) {
				end++;
			}
			const bool last = (i == LAMP_MULTI_NUM - 1);
			// The last key must end the text, the others must be followed by a comma
			if ((last && (end != len)) || (!last && (end == len)) || !parseParam(text + start, end - start, false, parsed[i])) {
				return false;
			}
			start = end + 1;
		}
		for (size_t i = 0; i < LAMP_MULTI_NUM; i++) {
			out[i] = parsed[i];
		}
		return true;
	}

	bool fieldFits(const char *text) {
		return (text == nullptr) || (strnlen(text, MAX_FIELD_LEN + 1) <= MAX_FIELD_LEN);
	}

	struct field_ref {
		const char *text;
		lamp_update_result ifBad;
	};

	const char *paramName(lamp_param p) {
		const size_t index = static_cast<size_t>(p);
		return PARAM_NAMES[(index < PARAM_NUM) ? index : 0];
	}

	const char *modeName(lamp_mode m) {
		const size_t index = static_cast<size_t>(m);
		return MODE_NAMES[(index < MODE_NUM) ? index : 0];
	}

	bool paramInRange(lamp_param p, bool allowTotal) {
		const size_t index = static_cast<size_t>(p);
		return (index < PARAM_NUM) && (allowTotal || (index != 0));
	}

	uint16_t fletcher16(const uint8_t *data, size_t len) {
		uint16_t sum1 = 0;
		uint16_t sum2 = 0;
		for (size_t i = 0; i < len; i++) {
			sum1 = static_cast<uint16_t>((sum1 + data[i]) % 255);
			sum2 = static_cast<uint16_t>((sum2 + sum1) % 255);
		}
		return static_cast<uint16_t>((sum2 << 8) | sum1);
	}

	void putU16(uint8_t *out, uint16_t value) {
		out[0] = static_cast<uint8_t>(value & 0xFF);
		out[1] = static_cast<uint8_t>(value >> 8);
	}

	uint16_t getU16(const uint8_t *in) {
		return static_cast<uint16_t>(in[0] | (in[1] << 8));
	}
} // namespace

lamp_settings HAQuDA_LampDefaults() {
	lamp_settings s{};
	s.on = true;
	s.brightPercent = DEFAULT_BRIGHTNESS;
	s.mode = lamp_mode::multi;
	s.standardParam = lamp_param::total;
	s.multi[0] = lamp_param::pm25;
	s.multi[1] = lamp_param::eco2;
	s.multi[2] = lamp_param::temp;
	s.nightParam = lamp_param::eco2;
	s.nightFromMin = NIGHT_DEFAULT_FROM_MIN;
	s.nightToMin = NIGHT_DEFAULT_TO_MIN;
	s.color = 0;
	return s;
}

bool HAQuDA_LampSettingsValid(const lamp_settings &s) {
	if ((s.brightPercent < LAMP_BRIGHT_MIN) || (s.brightPercent > LAMP_BRIGHT_MAX) || ((s.brightPercent % LAMP_BRIGHT_STEP) != 0)) {
		return false;
	}
	if ((static_cast<size_t>(s.mode) >= MODE_NUM) || !paramInRange(s.standardParam, true) || !paramInRange(s.nightParam, false)) {
		return false;
	}
	for (size_t i = 0; i < LAMP_MULTI_NUM; i++) {
		if (!paramInRange(s.multi[i], false)) {
			return false;
		}
	}
	return (s.color < LAMP_COLORS_NUM) && (s.nightFromMin < MINUTES_PER_DAY) && (s.nightToMin < MINUTES_PER_DAY) && (s.nightFromMin != s.nightToMin);
}

lamp_update_result HAQuDA_LampUpdate(lamp_settings &settings, const lamp_request &request, bool timeKnown) {
	// A field that is too long is bad whatever it holds; the parsers below may assume at most MAX_FIELD_LEN characters
	const field_ref fields[] = {{request.on, lamp_update_result::bad_on},		   {request.bright, lamp_update_result::bad_bright},
								{request.mode, lamp_update_result::bad_mode},	   {request.standardParam, lamp_update_result::bad_standard},
								{request.multi, lamp_update_result::bad_multi},	   {request.nightParam, lamp_update_result::bad_night_param},
								{request.nightFrom, lamp_update_result::bad_time}, {request.nightTo, lamp_update_result::bad_time},
								{request.color, lamp_update_result::bad_color}};
	bool any = false;
	for (const field_ref &field : fields) {
		if (!fieldFits(field.text)) {
			return field.ifBad;
		}
		any = any || (field.text != nullptr);
	}
	if (!any) {
		return lamp_update_result::nothing;
	}

	lamp_settings next = settings;
	if (request.on != nullptr) {
		if (strcmp(request.on, "1") == 0) {
			next.on = true;
		} else if (strcmp(request.on, "0") == 0) {
			next.on = false;
		} else {
			return lamp_update_result::bad_on;
		}
	}
	if ((request.bright != nullptr) && !parseBright(request.bright, strlen(request.bright), next.brightPercent)) {
		return lamp_update_result::bad_bright;
	}
	if (request.mode != nullptr) {
		bool found = false;
		for (size_t i = 0; i < MODE_NUM; i++) {
			if (strcmp(request.mode, MODE_NAMES[i]) == 0) {
				next.mode = static_cast<lamp_mode>(i);
				found = true;
			}
		}
		if (!found) {
			return lamp_update_result::bad_mode;
		}
		if ((next.mode == lamp_mode::night) && !timeKnown) {
			return lamp_update_result::night_needs_time;
		}
	}
	if ((request.standardParam != nullptr) && !parseParam(request.standardParam, strlen(request.standardParam), true, next.standardParam)) {
		return lamp_update_result::bad_standard;
	}
	if ((request.multi != nullptr) && !parseMulti(request.multi, strlen(request.multi), next.multi)) {
		return lamp_update_result::bad_multi;
	}
	if ((request.nightParam != nullptr) && !parseParam(request.nightParam, strlen(request.nightParam), false, next.nightParam)) {
		return lamp_update_result::bad_night_param;
	}
	if (((request.nightFrom != nullptr) && !parseTime(request.nightFrom, strlen(request.nightFrom), next.nightFromMin))
		|| ((request.nightTo != nullptr) && !parseTime(request.nightTo, strlen(request.nightTo), next.nightToMin))) {
		return lamp_update_result::bad_time;
	}
	if (next.nightFromMin == next.nightToMin) {
		return lamp_update_result::night_window_empty;
	}
	if (request.color != nullptr) {
		uint32_t index = 0;
		if (!parseSmallNumber(request.color, strlen(request.color), index) || (index >= LAMP_COLORS_NUM)) {
			return lamp_update_result::bad_color;
		}
		next.color = static_cast<uint8_t>(index);
	}
	settings = next;
	return lamp_update_result::ok;
}

const char *HAQuDA_LampUpdateText(lamp_update_result result) {
	switch (result) {
		case lamp_update_result::nothing:
			return "Nothing to change.";
		case lamp_update_result::bad_on:
			return "Power must be 1 or 0.";
		case lamp_update_result::bad_bright:
			return "Brightness must be 5 to 100 in steps of 5.";
		case lamp_update_result::bad_mode:
			return "Unknown mode.";
		case lamp_update_result::bad_standard:
			return "Unknown reading for the standard mode.";
		case lamp_update_result::bad_multi:
			return "The multi mode needs three readings, top to bottom, separated by commas.";
		case lamp_update_result::bad_night_param:
			return "Unknown reading for the night mode.";
		case lamp_update_result::bad_time:
			return "The night time must look like 21:30.";
		case lamp_update_result::bad_color:
			return "The colour must be 0 to 5.";
		case lamp_update_result::night_window_empty:
			return "The night light must start and end at different times.";
		case lamp_update_result::night_needs_time:
			return "The night mode needs the time from the internet.";
		case lamp_update_result::ok:
		default:
			return "";
	}
}

size_t HAQuDA_BuildLampJson(const lamp_settings &s, bool timeKnown, uint16_t minuteOfDay, char *buf, size_t size) {
	if ((buf == nullptr) || (size == 0)) {
		return 0;
	}
	buf[0] = '\0';
	const int head = snprintf(buf, size,
							  "{\"on\":%s,\"bright\":%u,\"mode\":\"%s\",\"multi\":[\"%s\",\"%s\",\"%s\"],\"std\":\"%s\","
							  "\"night\":{\"param\":\"%s\",\"from\":\"%02u:%02u\",\"to\":\"%02u:%02u\"},\"color\":%u,\"time\":%s",
							  s.on ? "true" : "false", static_cast<unsigned>(s.brightPercent), modeName(s.mode), paramName(s.multi[0]), paramName(s.multi[1]),
							  paramName(s.multi[2]), paramName(s.standardParam), paramName(s.nightParam), static_cast<unsigned>(s.nightFromMin / 60),
							  static_cast<unsigned>(s.nightFromMin % 60), static_cast<unsigned>(s.nightToMin / 60), static_cast<unsigned>(s.nightToMin % 60),
							  static_cast<unsigned>(s.color), timeKnown ? "true" : "false");
	if ((head < 0) || (static_cast<size_t>(head) >= size)) {
		buf[0] = '\0';
		return 0;
	}
	size_t len = static_cast<size_t>(head);
	const int tail = timeKnown ? snprintf(buf + len, size - len, ",\"now\":\"%02u:%02u\"}", static_cast<unsigned>((minuteOfDay / 60) % 24),
										  static_cast<unsigned>(minuteOfDay % 60))
							   : snprintf(buf + len, size - len, "}");
	if ((tail < 0) || (static_cast<size_t>(tail) >= size - len)) {
		buf[0] = '\0';
		return 0;
	}
	return len + static_cast<size_t>(tail);
}

uint32_t HAQuDA_LampColor(uint8_t index) {
	return (index < LAMP_COLORS_NUM) ? COLORS[index] : 0;
}

bool HAQuDA_EncodeLampRecord(const lamp_settings &s, uint8_t *out, size_t size) {
	if ((out == nullptr) || (size < LAMP_RECORD_LEN) || !HAQuDA_LampSettingsValid(s)) {
		return false;
	}
	out[0] = static_cast<uint8_t>(LAMP_RECORD_MAGIC & 0xFF);
	out[1] = static_cast<uint8_t>((LAMP_RECORD_MAGIC >> 8) & 0xFF);
	out[2] = static_cast<uint8_t>((LAMP_RECORD_MAGIC >> 16) & 0xFF);
	out[3] = static_cast<uint8_t>((LAMP_RECORD_MAGIC >> 24) & 0xFF);
	out[4] = LAMP_RECORD_VERSION;
	out[5] = s.on ? 1 : 0;
	out[6] = s.brightPercent;
	out[7] = static_cast<uint8_t>(s.mode);
	out[8] = static_cast<uint8_t>(s.standardParam);
	out[9] = static_cast<uint8_t>(s.multi[0]);
	out[10] = static_cast<uint8_t>(s.multi[1]);
	out[11] = static_cast<uint8_t>(s.multi[2]);
	out[12] = static_cast<uint8_t>(s.nightParam);
	out[13] = s.color;
	putU16(out + 14, s.nightFromMin);
	putU16(out + 16, s.nightToMin);
	putU16(out + RECORD_BODY_LEN, fletcher16(out, RECORD_BODY_LEN));
	return true;
}

bool HAQuDA_DecodeLampRecord(const uint8_t *data, size_t len, lamp_settings &out) {
	if ((data == nullptr) || (len != LAMP_RECORD_LEN)) {
		return false;
	}
	const uint32_t magic = static_cast<uint32_t>(data[0]) | (static_cast<uint32_t>(data[1]) << 8) | (static_cast<uint32_t>(data[2]) << 16)
						   | (static_cast<uint32_t>(data[3]) << 24);
	if ((magic != LAMP_RECORD_MAGIC) || (data[4] != LAMP_RECORD_VERSION) || (getU16(data + RECORD_BODY_LEN) != fletcher16(data, RECORD_BODY_LEN))
		|| (data[5] > 1)) {
		return false;
	}
	lamp_settings s{};
	s.on = data[5] == 1;
	s.brightPercent = data[6];
	s.mode = static_cast<lamp_mode>(data[7]);
	s.standardParam = static_cast<lamp_param>(data[8]);
	s.multi[0] = static_cast<lamp_param>(data[9]);
	s.multi[1] = static_cast<lamp_param>(data[10]);
	s.multi[2] = static_cast<lamp_param>(data[11]);
	s.nightParam = static_cast<lamp_param>(data[12]);
	s.color = data[13];
	s.nightFromMin = getU16(data + 14);
	s.nightToMin = getU16(data + 16);
	if (!HAQuDA_LampSettingsValid(s)) {
		return false;
	}
	out = s;
	return true;
}
