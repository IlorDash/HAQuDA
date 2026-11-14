#include "HAQuDA_WebPageDisplayManip_Translator.h"

#include <cstdarg>
#include <cstdio>

namespace {

	// Lamp geometry and timing, mirrors HAQuDA_LEDsParams.h (LED_NUM_PIXELS, LED_ROW_NUM), the 100 ms pause
	// HAQuDA_DisplayManip::Show adds after every draw and the 200 ms HAQuDA_LEDs::Show() blocks without an argument (so do
	// Clear() and SetBright(), which call it). Not included from there: that header pulls in Arduino.h.
	constexpr uint32_t LAMP_PIXELS = 109;
	constexpr uint32_t LAMP_ROWS = 12;
	constexpr uint32_t SHOW_PAUSE_MS = 100;
	constexpr uint32_t LEDS_SHOW_MS = 200;
	// Every effect starts with Clear(): the matrix is dark for LEDS_SHOW_MS before the first frame.
	constexpr uint32_t CLEAR_DARK_MS = LEDS_SHOW_MS;
	// show_fading() is dark for Clear() and for SetBright(start) (still dark, the pixels are cleared).
	constexpr uint32_t FADING_DARK_MS = 2 * LEDS_SHOW_MS;

	// Page grid: 4 columns x 6 rows (the lamp is 9 columns x 12 rows).
	constexpr uint32_t PAGE_CELLS = 24;
	constexpr uint32_t PAGE_ROWS = 6;

	constexpr uint32_t MAX_PERCENT = 100;
	constexpr uint32_t MULTI_KEYS_NUM = 3;

	const char *const SOURCE_NAMES[] = {"none", "effect", "meas", "error", "event"};
	const char *const ERROR_NAMES[] = {"none", "wifi", "ap", "fs", "sensors", "ntp"};
	const char *const KEY_NAMES[] = {"total", "eco2", "tvoc", "pm25", "temp", "humid"};

	template <typename T, size_t N> constexpr size_t countOf(const T (&)[N]) {
		return N;
	}

	// Appends formatted text; once something does not fit, every later append is ignored and ok stays false.
	class JsonWriter {
	  public:
		JsonWriter(char *buf, size_t size) : buf(buf), size(size) {
		}

		void Append(const char *fmt, ...) {
			if (!ok) {
				return;
			}
			va_list args;
			va_start(args, fmt);
			const int n = vsnprintf(buf + len, size - len, fmt, args);
			va_end(args);
			if ((n < 0) || (static_cast<size_t>(n) >= size - len)) {
				ok = false;
				return;
			}
			len += static_cast<size_t>(n);
		}

		bool Ok() const {
			return ok;
		}

		size_t Length() const {
			return len;
		}

	  private:
		char *buf;
		size_t size;
		size_t len = 0;
		bool ok = true;
	};

	uint32_t clampU32(uint32_t value, uint32_t minValue, uint32_t maxValue) {
		if (value < minValue) {
			return minValue;
		}
		return (value > maxValue) ? maxValue : value;
	}

	// Integer division rounded to the nearest, halves up. Callers keep numerator + denominator / 2 inside uint32.
	uint32_t divRound(uint32_t numerator, uint32_t denominator) {
		return (numerator + denominator / 2) / denominator;
	}

	uint32_t divCeil(uint32_t numerator, uint32_t denominator) {
		return (numerator + denominator - 1) / denominator;
	}

	// Colour as lower-case "#rrggbb" from the 24-bit 0xRRGGBB value, higher bits are ignored.
	void appendColor(JsonWriter &w, uint32_t color) {
		w.Append(",\"color\":\"#%02x%02x%02x\"", static_cast<unsigned>((color >> 16) & 0xFF), static_cast<unsigned>((color >> 8) & 0xFF),
				 static_cast<unsigned>(color & 0xFF));
	}

	// Each channel scaled by percent (0..100), rounded.
	uint32_t scaleColor(uint32_t color, uint32_t percent) {
		uint32_t scaled = 0;
		for (uint32_t shift = 0; shift <= 16; shift += 8) {
			const uint32_t channel = (color >> shift) & 0xFF;
			scaled |= divRound(channel * percent, MAX_PERCENT) << shift;
		}
		return scaled;
	}

	const char *keyName(web_meas_key key) {
		const size_t index = static_cast<size_t>(key);
		return KEY_NAMES[(index < countOf(KEY_NAMES)) ? index : 0];
	}

	// Lamp: Clear() (dark), then one lap = LAMP_PIXELS draws of movePauseMs plus the 100 ms pause. Page: "dark" ms, then the same
	// lap over PAGE_CELLS cells.
	void appendSnake(JsonWriter &w, const web_snake_params &p) {
		const uint32_t lapMs = LAMP_PIXELS * p.movePauseMs + SHOW_PAUSE_MS;
		const uint32_t stepMs = divRound(lapMs, PAGE_CELLS);
		const uint32_t tail = clampU32(divRound(static_cast<uint32_t>(p.tailLen) * PAGE_CELLS, LAMP_PIXELS), 1, PAGE_CELLS);
		w.Append(",\"fx\":\"snake\"");
		appendColor(w, p.color);
		w.Append(",\"dark\":%lu,\"step\":%lu,\"tail\":%lu", static_cast<unsigned long>(CLEAR_DARK_MS), static_cast<unsigned long>(stepMs),
				 static_cast<unsigned long>(tail));
	}

	// Lamp: Clear() (dark), LAMP_ROWS rows, one per growPauseMs, then the full matrix for the 100 ms pause. Page: "dark" ms, then
	// PAGE_ROWS rows, so each page row takes two lamp rows.
	void appendGrowing(JsonWriter &w, const web_growing_params &p) {
		const uint32_t stepMs = (LAMP_ROWS / PAGE_ROWS) * p.growPauseMs;
		w.Append(",\"fx\":\"growing\"");
		appendColor(w, p.color);
		w.Append(",\"dark\":%lu,\"step\":%lu,\"end\":%lu", static_cast<unsigned long>(CLEAR_DARK_MS), static_cast<unsigned long>(stepMs),
				 static_cast<unsigned long>(SHOW_PAUSE_MS));
	}

	// Lamp: Clear() (dark), LAMP_PIXELS pixels, one per pauseMs, then endPauseMs and the 100 ms pause. Page: "dark" ms, then
	// PAGE_CELLS cells over the same time.
	void appendRandom(JsonWriter &w, const web_random_params &p) {
		const uint32_t stepMs = divRound(LAMP_PIXELS * p.pauseMs, PAGE_CELLS);
		const uint32_t endMs = static_cast<uint32_t>(p.endPauseMs) + SHOW_PAUSE_MS;
		w.Append(",\"fx\":\"random\",\"dark\":%lu,\"step\":%lu,\"end\":%lu", static_cast<unsigned long>(CLEAR_DARK_MS), static_cast<unsigned long>(stepMs),
				 static_cast<unsigned long>(endMs));
	}

	// Lamp: Clear() and SetBright(start) are dark (2 x 200 ms), the fill shows startBright for 200 ms, then brightness counts down
	// from startBright to stopBright in `step` decrements: every one is SetBright() = 200 ms plus fadePauseMs, the first is
	// startBright again. The last level stays for the 100 ms pause, then the cycle restarts. So the page gets: "dark" ms, the
	// first level for "first" ms, every further level for "step" ms (steps levels in all), the last level "end" ms longer.
	// When startBright <= stopBright the countdown never runs: the lamp is dark for 400 ms, lit at startBright for 200 ms plus the
	// 100 ms pause, and starts over, so it flashes. The page gets one level (steps 1, from = to = startBright, first 200 ms),
	// a cycle of 700 ms; with startBright 0 the lamp stays dark for the whole cycle and so does the page. Only step == 0 with
	// startBright > stopBright is steady: the lamp loops at startBright until the effect is stopped, the page shows a static colour.
	void appendFading(JsonWriter &w, const web_fading_params &p) {
		const uint32_t from = clampU32(p.startBright, 0, MAX_PERCENT);
		const uint32_t to = clampU32(p.stopBright, 0, MAX_PERCENT);
		const bool counts = from > to;
		if (counts && (p.step == 0)) {
			w.Append(",\"fx\":\"static\"");
			appendColor(w, scaleColor(p.color, from));
			return;
		}
		const uint32_t steps = counts ? divCeil(from - to, p.step) : 1;
		const uint32_t stepMs = LEDS_SHOW_MS + p.fadePauseMs;
		const uint32_t firstMs = counts ? (LEDS_SHOW_MS + stepMs) : LEDS_SHOW_MS;
		w.Append(",\"fx\":\"fading\"");
		appendColor(w, p.color);
		w.Append(",\"from\":%lu,\"to\":%lu,\"steps\":%lu,\"dark\":%lu,\"first\":%lu,\"step\":%lu,\"end\":%lu", static_cast<unsigned long>(from),
				 static_cast<unsigned long>(counts ? to : from), static_cast<unsigned long>(steps), static_cast<unsigned long>(FADING_DARK_MS),
				 static_cast<unsigned long>(firstMs), static_cast<unsigned long>(stepMs), static_cast<unsigned long>(SHOW_PAUSE_MS));
	}

	void appendEffect(JsonWriter &w, const web_display_state &s) {
		switch (s.effect) {
			case web_effect::snake:
				appendSnake(w, s.snake);
				break;
			case web_effect::growing:
				appendGrowing(w, s.growing);
				break;
			case web_effect::random_led:
				appendRandom(w, s.randomLed);
				break;
			case web_effect::fading:
				appendFading(w, s.fading);
				break;
			case web_effect::static_color:
				w.Append(",\"fx\":\"static\"");
				appendColor(w, s.staticColor);
				break;
			default:
				w.Append(",\"fx\":\"hold\"");
				break;
		}
	}

	// Night bar: white page rows = ceil(hour * PAGE_ROWS / 12), 12 (outside the window) fills all of them.
	uint32_t nightWhiteRows(uint8_t hour) {
		const uint32_t h = clampU32(hour, 0, 12);
		return clampU32(divCeil(h * PAGE_ROWS, 12), 0, PAGE_ROWS);
	}

	void appendMeas(JsonWriter &w, const web_display_state &s) {
		const char *layoutName = nullptr;
		size_t keysNum = 1;
		switch (s.layout) {
			case web_meas_layout::standard:
				layoutName = "standard";
				break;
			case web_meas_layout::multi:
				layoutName = "multi";
				keysNum = MULTI_KEYS_NUM;
				break;
			case web_meas_layout::night:
				layoutName = "night";
				break;
			default:
				w.Append(",\"fx\":\"hold\"");
				return;
		}
		// The LEDs keep their previous frame until the lamp draws this picture
		if (!s.measDrawn) {
			w.Append(",\"fx\":\"hold\"");
			return;
		}
		w.Append(",\"fx\":\"meas\",\"layout\":\"%s\",\"keys\":[", layoutName);
		for (size_t i = 0; i < keysNum; i++) {
			w.Append("%s\"%s\"", (i == 0) ? "" : ",", keyName(s.keys[i]));
		}
		w.Append("],\"colors\":[");
		for (size_t i = 0; i < keysNum; i++) {
			w.Append("%s\"#%06lx\"", (i == 0) ? "" : ",", static_cast<unsigned long>(s.measColors[i] & 0xFFFFFF));
		}
		w.Append("]");
		if (s.layout == web_meas_layout::night) {
			w.Append(",\"white\":%lu,\"whiteColor\":\"#%06lx\"", static_cast<unsigned long>(nightWhiteRows(s.nightHour)),
					 static_cast<unsigned long>(s.measWhiteColor & 0xFFFFFF));
		}
	}

} // namespace

uint8_t HAQuDA_NightHour(uint8_t start, uint8_t finish, uint8_t hour) {
	constexpr uint8_t OUTSIDE_WINDOW = 12;
	constexpr int HOURS_PER_DAY = 24;

	if (hour >= HOURS_PER_DAY) {
		return OUTSIDE_WINDOW;
	}
	const bool crossesMidnight = finish < start;
	const bool inWindow = crossesMidnight ? ((hour >= start) || (hour <= finish)) : ((hour >= start) && (hour <= finish));
	if (!inWindow) {
		return OUTSIDE_WINDOW;
	}
	int hoursIn = static_cast<int>(hour) - static_cast<int>(start);
	if (hoursIn < 0) {
		hoursIn += HOURS_PER_DAY;
	}
	return static_cast<uint8_t>(hoursIn);
}

size_t HAQuDA_TranslateDisplayToJson(const web_display_state &state, char *buf, size_t size) {
	if ((buf == nullptr) || (size == 0)) {
		return 0;
	}
	buf[0] = '\0';

	const size_t sourceIndex = static_cast<size_t>(state.source);
	if (sourceIndex >= countOf(SOURCE_NAMES)) {
		return 0;
	}
	// The error field is meaningful only for the error source, any value is ignored for the others.
	const size_t errorIndex = static_cast<size_t>(state.error);
	if ((state.source == web_source::error) && (errorIndex >= countOf(ERROR_NAMES))) {
		return 0;
	}

	JsonWriter w(buf, size);
	w.Append("{\"seq\":%lu,\"age\":%lu,\"src\":\"%s\"", static_cast<unsigned long>(state.seq), static_cast<unsigned long>(state.ageMs),
			 SOURCE_NAMES[sourceIndex]);
	if (state.source == web_source::error) {
		w.Append(",\"err\":\"%s\"", ERROR_NAMES[errorIndex]);
	}
	w.Append(",\"bright\":%lu", static_cast<unsigned long>(clampU32(state.brightPercent, 0, MAX_PERCENT)));

	switch (state.source) {
		case web_source::effect:
		case web_source::error:
			appendEffect(w, state);
			break;
		case web_source::meas:
			appendMeas(w, state);
			break;
		default:
			w.Append(",\"fx\":\"hold\"");
			break;
	}
	w.Append("}");

	if (!w.Ok()) {
		buf[0] = '\0';
		return 0;
	}
	return w.Length();
}
