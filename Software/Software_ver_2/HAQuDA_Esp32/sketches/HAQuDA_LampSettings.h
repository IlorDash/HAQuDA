#pragma once

#include <cstddef>
#include <cstdint>

enum class lamp_mode : uint8_t { standard, multi, night, light };

// In the order of the page's keys; total is for the standard mode only
enum class lamp_param : uint8_t { total, temp, humid, eco2, tvoc, pm25 };

constexpr uint8_t LAMP_MULTI_NUM = 3;
constexpr uint8_t LAMP_COLORS_NUM = 6;
constexpr uint8_t LAMP_BRIGHT_MIN = 5;
constexpr uint8_t LAMP_BRIGHT_MAX = 100;
constexpr uint8_t LAMP_BRIGHT_STEP = 5;

// The longest JSON HAQuDA_BuildLampJson() writes is about 190 bytes
constexpr size_t LAMP_JSON_BUF_LEN = 256;
constexpr size_t LAMP_RECORD_LEN = 20;

struct lamp_settings {
	bool on;
	uint8_t brightPercent;
	lamp_mode mode;
	lamp_param standardParam;
	lamp_param multi[LAMP_MULTI_NUM]; // top, middle, bottom of the page's grid
	lamp_param nightParam;
	uint16_t nightFromMin;
	uint16_t nightToMin;
	uint8_t color;
};

// Raw text of the form fields on, bright, mode, std, multi, night_param, night_from, night_to, color; nullptr means the field is not in
// the request. multi is the three keys top to bottom separated by commas, the times are HH:MM.
struct lamp_request {
	const char *on;
	const char *bright;
	const char *mode;
	const char *standardParam;
	const char *multi;
	const char *nightParam;
	const char *nightFrom;
	const char *nightTo;
	const char *color;
};

enum class lamp_update_result : uint8_t {
	ok,
	nothing,
	bad_on,
	bad_bright,
	bad_mode,
	bad_standard,
	bad_multi,
	bad_night_param,
	bad_time,
	bad_color,
	night_window_empty,
	night_needs_time
};

lamp_settings HAQuDA_LampDefaults();

bool HAQuDA_LampSettingsValid(const lamp_settings &settings);

// All or nothing: any bad field leaves `settings` unchanged and gives the reason. Selecting the night mode needs timeKnown (the
// clock is set from the internet). A field longer than 32 characters is bad.
lamp_update_result HAQuDA_LampUpdate(lamp_settings &settings, const lamp_request &request, bool timeKnown);

// Plain text shown to the user as it is; empty for ok
const char *HAQuDA_LampUpdateText(lamp_update_result result);

// The lamp block of /api/status, also the reply of a successful update. Returns the length, 0 when it does not fit. The clock "now" is
// written only when timeKnown.
size_t HAQuDA_BuildLampJson(const lamp_settings &settings, bool timeKnown, uint16_t minuteOfDay, char *buf, size_t size);

// 0 for an index out of range
uint32_t HAQuDA_LampColor(uint8_t index);

// Decode fails (false, `out` untouched) when the length, magic, version, checksum or any field is wrong, so a damaged or foreign file
// gives the defaults. Encode fails when `settings` is not valid or `size` is too small.
bool HAQuDA_EncodeLampRecord(const lamp_settings &settings, uint8_t *out, size_t size);
bool HAQuDA_DecodeLampRecord(const uint8_t *data, size_t len, lamp_settings &out);
