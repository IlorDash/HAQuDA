#pragma once

#include <cstdint>

// Night light progress of the lamp's night mode. Hardware-free: the lamp (HAQuDA_MeasHandler), its web mirror and the web page
// (app.js, the same rules) all derive the picture from these numbers.
//
// The night picture has the measurement colour in every third lamp column (0, 3, 6) and the night light in the other six. The light
// is 18 cells: six stripes of three cells, each cell two lamp columns wide and two lamp rows high. The stripes fill from the
// bottom up, a stripe from left to right. The window between `from` and `to` (minutes since midnight, may cross midnight) lights the
// cells one by one over its length.

constexpr uint16_t MINUTES_PER_DAY = 24 * 60;
constexpr uint8_t NIGHT_LIT_CELLS_MAX = 18;

// Default night window, 21:00 to 09:00
constexpr uint16_t NIGHT_DEFAULT_FROM_MIN = 21 * 60;
constexpr uint16_t NIGHT_DEFAULT_TO_MIN = 9 * 60;

// Number of lit cells (1..NIGHT_LIT_CELLS_MAX) at minute nowMin of the day: ceil(18 * elapsed / span), where elapsed counts from
// `fromMin` and span is the window length. Outside the window (and for an empty window, fromMin == toMin) all cells are lit, the
// bar the lamp showed before the window was set in minutes. Minutes of a day above 1439 give 0 (no valid time). Any task.
uint8_t HAQuDA_NightLitCells(uint16_t fromMin, uint16_t toMin, uint16_t nowMin);

// True when lamp column 0..8 shows the measurement colour (columns 0, 3 and 6) and not the night light. Any task.
bool HAQuDA_NightColumnIsColor(uint8_t column);

// Lit pixel rows (0..12, counted from the bottom end of the column) of a night light column (one where HAQuDA_NightColumnIsColor()
// is false) when `litCells` cells are on. Cell n is in stripe n / 3 (from the bottom) and in the column pair n % 3, so a pair of
// columns (1 and 2, 4 and 5, 7 and 8, left to right) has the same height. Any task.
uint8_t HAQuDA_NightLightRows(uint8_t litCells, uint8_t column);
