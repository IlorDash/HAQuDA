#pragma once

#include <cstdint>

constexpr uint16_t MINUTES_PER_DAY = 24 * 60;
constexpr uint8_t NIGHT_LIT_CELLS_MAX = 18;

constexpr uint16_t NIGHT_DEFAULT_FROM_MIN = 21 * 60;
constexpr uint16_t NIGHT_DEFAULT_TO_MIN = 9 * 60;

// The light is 18 cells: six stripes of three, filled from the bottom up, a stripe left to right; each cell is two lamp columns wide and
// two rows high. The window from `fromMin` to `toMin` (may cross midnight) lights them one by one over its length: ceil(18 * elapsed /
// span). Outside the window and for an empty window all cells are lit, the bar the lamp showed before the window was set in minutes.
// Returns 0 when a minute is above 1439 (no valid time).
uint8_t HAQuDA_NightLitCells(uint16_t fromMin, uint16_t toMin, uint16_t nowMin);

// The measurement colour is in lamp columns 0, 3 and 6, the night light in the other six
bool HAQuDA_NightColumnIsColor(uint8_t column);

// Cell n is in stripe n / 3 and in the column pair n % 3, so the pairs (1 and 2, 4 and 5, 7 and 8) have the same height.
// column must be one where HAQuDA_NightColumnIsColor() is false.
uint8_t HAQuDA_NightLightRows(uint8_t litCells, uint8_t column);
