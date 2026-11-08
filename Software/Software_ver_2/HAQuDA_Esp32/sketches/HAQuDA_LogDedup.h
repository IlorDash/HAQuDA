#pragma once

#include <cstddef>
#include <cstdint>

// Hardware-free: no Arduino.h, time is passed in by the caller. The object has no lock, one task at a time (the logger calls it from loop() only).

// Same size as the data field of a stored log record, so a message is compared and kept the way the file keeps it.
constexpr size_t LOG_DEDUP_TEXT_SIZE = 32;
constexpr size_t LOG_DEDUP_TABLE_SIZE = 8;
// A message not seen for this long counts as gone: its repeats are summarised and the entry is freed.
constexpr uint32_t LOG_DEDUP_EXPIRY_MS = 10 * 1000;

// An info line and an error line with the same text are different messages: they do not merge and the summary keeps the level.
enum class log_dedup_level : uint8_t { info, error };

struct log_dedup_summary {
	char text[LOG_DEDUP_TEXT_SIZE];
	log_dedup_level level;
	uint32_t repeats;
	uint32_t lastSeenMs;
};

struct log_dedup_result {
	// true: the message is new, store and print it. false: it repeats a known one, suppress it.
	bool write;
	// true: the table was full and the evicted entry had repeats, "evicted" holds its summary. Store that summary before the message.
	bool hasEvicted;
	log_dedup_summary evicted;
};

// Formats an uptime stamp "up 412.345 s" (at most 16 chars plus the terminator for any uint32_t). Any task.
void HAQuDA_FormatUptime(char *out, size_t outSize, uint32_t ms);

// Formats a repeat summary "+150 <text>": the count comes first, so a too small buffer cuts the text, never the count. Any task.
void HAQuDA_FormatRepeatSummary(char *out, size_t outSize, const char *text, uint32_t repeats);

// Remembers the last messages so the same line is stored once while it keeps repeating.
class HAQuDA_LogDedup {
  public:
	HAQuDA_LogDedup();

	// Registers a message seen at nowMs (ms since boot, wraps after 49 days, handled). The text is cut to LOG_DEDUP_TEXT_SIZE - 1 chars like a stored record.
	// A message repeats only when level and text both match. A null text is always written. Task that owns the object (loop).
	log_dedup_result Check(log_dedup_level level, const char *text, uint32_t nowMs);

	// Takes out one entry not seen for LOG_DEDUP_EXPIRY_MS. Entries without repeats are freed silently.
	// Returns true and fills the summary for the first expired entry with repeats; call again until false. Task that owns the object (loop).
	bool PopExpired(uint32_t nowMs, log_dedup_summary &summary);

  private:
	struct entry {
		char text[LOG_DEDUP_TEXT_SIZE];
		log_dedup_level level;
		uint32_t repeats;
		uint32_t lastSeenMs;
		bool used;
	};

	static void copyText(char *dst, const char *src);
	size_t findEntry(log_dedup_level level, const char *text) const;
	size_t pickSlot(uint32_t nowMs) const;

	entry table[LOG_DEDUP_TABLE_SIZE];
};
