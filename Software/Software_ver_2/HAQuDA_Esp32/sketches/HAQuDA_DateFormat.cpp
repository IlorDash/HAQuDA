#include "HAQuDA_DateFormat.h"

#include <cstdio>

bool HAQuDA_FormatDateTime(char *out, size_t outSize, int date, int month, int year, int hour, int min, int sec) {
	if ((out == nullptr) || (outSize == 0)) {
		return false;
	}
	out[0] = '\0';

	// A negative field prints with a minus sign: "-1" is still two characters, so the length check below would accept it.
	if ((date < 0) || (month < 0) || (year < 0) || (hour < 0) || (min < 0) || (sec < 0)) {
		return false;
	}

	// snprintf returns the length it wanted to write, without the terminator: negative on an error, >= outSize on truncation.
	// Anything but the exact fixed length means a field did not fit its digits (the 0-padded width is a minimum, not a limit).
	const int written = snprintf(out, outSize, "%02d.%02d.%04d %02d:%02d:%02d", date, month, year, hour, min, sec);
	if ((written < 0) || ((size_t)written >= outSize) || ((size_t)written != DATE_TIME_TEXT_LEN)) {
		out[0] = '\0';
		return false;
	}
	return true;
}
