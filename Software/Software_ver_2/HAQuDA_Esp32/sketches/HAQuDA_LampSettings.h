#pragma once

#include <cstddef>
#include <cstdint>

// What the user chose for the lamp on the web page: power, brightness, the display mode and the parameters of each mode. The
// rules for changing it from a request, the JSON the page reads and the record kept in flash. Hardware-free: the firmware
// (HAQuDA_LampControl) maps these values onto the display, the tests run them on the PC.

// The four modes of the page. light is a solid colour.
enum class lamp_mode : uint8_t { standard, multi, night, light };

// Measured quantity of a mode, in the order of the page's keys. total (the worst or the average reading) is for standard only.
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
	uint8_t bright; // percent, 5..100 in steps of 5
	lamp_mode mode;
	lamp_param standardParam;
	lamp_param multi[LAMP_MULTI_NUM]; // top, middle, bottom of the page's grid
	lamp_param nightParam;
	uint16_t nightFrom; // minutes since midnight
	uint16_t nightTo;
	uint8_t color; // index into the lamp light colours, 0..5
};

// Raw text of the fields of an update request (form fields on, bright, mode, std, multi, night_param, night_from, night_to, color);
// nullptr means the field is not in the request. multi is the three keys top to bottom separated by commas, the times are HH:MM.
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

// Power on, DEFAULT_BRIGHTNESS (35 %), multi mode (PM2.5 on top, eCO2, temperature at the bottom), overall reading for standard, eCO2 for night,
// night window 21:00 to 09:00, the first light colour. Any task.
lamp_settings HAQuDA_LampDefaults();

// True when every field is in its range. Any task.
bool HAQuDA_LampSettingsValid(const lamp_settings &settings);

// Applies the fields present in `request` to `settings`, all or nothing: any bad field leaves `settings` unchanged and gives the
// reason. Brightness must be 5..100 in steps of 5, multi three parameters other than total, night times HH:MM (00:00..23:59,
// different from each other), colour 0..5. Selecting the night mode needs timeKnown (the lamp's clock is set from the internet).
// A field longer than 32 characters is bad. No heap. Any task.
lamp_update_result HAQuDA_LampUpdate(lamp_settings &settings, const lamp_request &request, bool timeKnown);

// Plain-text reply for a result other than ok (shown to the user as it is). Any task.
const char *HAQuDA_LampUpdateText(lamp_update_result result);

// Writes the lamp block of /api/status, which is also the reply of a successful update, into buf (NUL-terminated) and returns its
// length; 0 when it does not fit (use LAMP_JSON_BUF_LEN). time tells whether the lamp knows the time; the clock "now" (HH:MM, the
// lamp's local time) is written only then. No heap. Any task.
size_t HAQuDA_BuildLampJson(const lamp_settings &settings, bool timeKnown, uint16_t minuteOfDay, char *buf, size_t size);

// Lamp light colour 0..5 as 0xRRGGBB; 0 for an index out of range. Any task.
uint32_t HAQuDA_LampColor(uint8_t index);

// Settings record of LAMP_RECORD_LEN bytes: magic, version, fields, Fletcher-16 checksum. Encode fails (false) when `settings` is not
// valid or `size` is too small. Decode fails (false, `out` untouched) when the length, magic, version, checksum or any field is
// wrong, so a damaged or foreign file gives the defaults. Any task.
bool HAQuDA_EncodeLampRecord(const lamp_settings &settings, uint8_t *out, size_t size);
bool HAQuDA_DecodeLampRecord(const uint8_t *data, size_t len, lamp_settings &out);
