#include "HAQuDA_LogDedup.h"

#include <cstdio>
#include <cstring>

void HAQuDA_FormatUptime(char *out, size_t outSize, uint32_t ms) {
	if ((out == nullptr) || (outSize == 0)) {
		return;
	}
	snprintf(out, outSize, "up %lu.%03lu s", (unsigned long)(ms / 1000), (unsigned long)(ms % 1000));
}

void HAQuDA_FormatRepeatSummary(char *out, size_t outSize, const char *text, uint32_t repeats) {
	if ((out == nullptr) || (outSize == 0)) {
		return;
	}
	snprintf(out, outSize, "+%lu %s", (unsigned long)repeats, (text != nullptr) ? text : "");
}

HAQuDA_LogDedup::HAQuDA_LogDedup() {
	for (size_t i = 0; i < LOG_DEDUP_TABLE_SIZE; i++) {
		table[i].text[0] = '\0';
		table[i].level = log_dedup_level::info;
		table[i].repeats = 0;
		table[i].lastSeenMs = 0;
		table[i].used = false;
	}
}

void HAQuDA_LogDedup::copyText(char *dst, const char *src) {
	size_t i = 0;
	while ((i < (LOG_DEDUP_TEXT_SIZE - 1)) && (src[i] != '\0')) {
		dst[i] = src[i];
		i++;
	}
	dst[i] = '\0';
}

size_t HAQuDA_LogDedup::findEntry(log_dedup_level level, const char *text) const {
	for (size_t i = 0; i < LOG_DEDUP_TABLE_SIZE; i++) {
		if (table[i].used && (table[i].level == level) && (strncmp(table[i].text, text, LOG_DEDUP_TEXT_SIZE - 1) == 0)) {
			return i;
		}
	}
	return LOG_DEDUP_TABLE_SIZE;
}

size_t HAQuDA_LogDedup::pickSlot(uint32_t nowMs) const {
	size_t oldest = 0;
	uint32_t oldestAgeMs = 0;
	for (size_t i = 0; i < LOG_DEDUP_TABLE_SIZE; i++) {
		if (!table[i].used) {
			return i;
		}
		const uint32_t ageMs = nowMs - table[i].lastSeenMs;
		if (ageMs >= oldestAgeMs) {
			oldestAgeMs = ageMs;
			oldest = i;
		}
	}
	return oldest;
}

log_dedup_result HAQuDA_LogDedup::Check(log_dedup_level level, const char *text, uint32_t nowMs) {
	log_dedup_result result;
	result.write = true;
	result.hasEvicted = false;
	result.evicted.text[0] = '\0';
	result.evicted.level = log_dedup_level::info;
	result.evicted.repeats = 0;
	result.evicted.lastSeenMs = 0;

	if (text == nullptr) {
		return result;
	}

	const size_t known = findEntry(level, text);
	if (known < LOG_DEDUP_TABLE_SIZE) {
		if (table[known].repeats < UINT32_MAX) {
			table[known].repeats++;
		}
		table[known].lastSeenMs = nowMs;
		result.write = false;
		return result;
	}

	const size_t slot = pickSlot(nowMs);
	if (table[slot].used && (table[slot].repeats > 0)) {
		copyText(result.evicted.text, table[slot].text);
		result.evicted.level = table[slot].level;
		result.evicted.repeats = table[slot].repeats;
		result.evicted.lastSeenMs = table[slot].lastSeenMs;
		result.hasEvicted = true;
	}

	copyText(table[slot].text, text);
	table[slot].level = level;
	table[slot].repeats = 0;
	table[slot].lastSeenMs = nowMs;
	table[slot].used = true;
	return result;
}

bool HAQuDA_LogDedup::PopExpired(uint32_t nowMs, log_dedup_summary &summary) {
	for (size_t i = 0; i < LOG_DEDUP_TABLE_SIZE; i++) {
		if (!table[i].used || ((nowMs - table[i].lastSeenMs) < LOG_DEDUP_EXPIRY_MS)) {
			continue;
		}
		table[i].used = false;
		if (table[i].repeats > 0) {
			copyText(summary.text, table[i].text);
			summary.level = table[i].level;
			summary.repeats = table[i].repeats;
			summary.lastSeenMs = table[i].lastSeenMs;
			return true;
		}
	}
	return false;
}
