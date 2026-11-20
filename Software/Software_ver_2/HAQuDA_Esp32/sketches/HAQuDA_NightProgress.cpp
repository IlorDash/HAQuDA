#include "HAQuDA_NightProgress.h"

namespace {
	constexpr uint8_t PAGE_ROWS = 6;
	constexpr uint8_t LAMP_ROWS_PER_CELL = 2;
	constexpr uint8_t COLUMNS_PER_GROUP = 3;
	constexpr uint8_t CELLS_PER_STRIPE = 3;
} // namespace

uint8_t HAQuDA_NightLitCells(uint16_t fromMin, uint16_t toMin, uint16_t nowMin) {
	if ((fromMin >= MINUTES_PER_DAY) || (toMin >= MINUTES_PER_DAY) || (nowMin >= MINUTES_PER_DAY)) {
		return 0;
	}
	const uint32_t span = (static_cast<uint32_t>(toMin) + MINUTES_PER_DAY - fromMin) % MINUTES_PER_DAY;
	const uint32_t elapsed = (static_cast<uint32_t>(nowMin) + MINUTES_PER_DAY - fromMin) % MINUTES_PER_DAY;
	if ((span == 0) || (elapsed >= span)) {
		return NIGHT_LIT_CELLS_MAX;
	}
	const uint32_t cells = (NIGHT_LIT_CELLS_MAX * elapsed + span - 1) / span;
	return static_cast<uint8_t>((cells < 1) ? 1 : cells);
}

bool HAQuDA_NightColumnIsColor(uint8_t column) {
	return (column % COLUMNS_PER_GROUP) == 0;
}

uint8_t HAQuDA_NightLightRows(uint8_t litCells, uint8_t column) {
	const uint8_t group = column / COLUMNS_PER_GROUP;
	if (litCells <= group) {
		return 0;
	}
	// The cells of this pair are group, group + 3, group + 6, ...: those below litCells are lit
	const uint16_t cellsHere = (static_cast<uint16_t>(litCells) - group + CELLS_PER_STRIPE - 1) / CELLS_PER_STRIPE;
	return static_cast<uint8_t>(((cellsHere > PAGE_ROWS) ? PAGE_ROWS : cellsHere) * LAMP_ROWS_PER_CELL);
}
