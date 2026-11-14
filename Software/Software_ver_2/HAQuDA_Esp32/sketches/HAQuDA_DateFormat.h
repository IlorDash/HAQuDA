#pragma once

#include <cstddef>

// Hardware-free: no Arduino.h, builds on the PC.

// Length of "DD.MM.YYYY HH:MM:SS" without the terminator.
constexpr size_t DATE_TIME_TEXT_LEN = 19;

// Formats "DD.MM.YYYY HH:MM:SS" into out (outSize must be at least DATE_TIME_TEXT_LEN + 1). Returns false and leaves an empty string when the buffer is
// too small, a field is negative or wider than its two (four for the year) digits, so a returned stamp always has the fixed length. Any task.
bool HAQuDA_FormatDateTime(char *out, size_t outSize, int date, int month, int year, int hour, int min, int sec);
