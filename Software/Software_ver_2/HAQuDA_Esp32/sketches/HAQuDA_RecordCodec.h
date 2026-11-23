#pragma once

#include <cstddef>
#include <cstdint>

// Little endian helpers and the checksum of the flash records (/lamp, /buttons)
inline uint16_t HAQuDA_Fletcher16(const uint8_t *data, size_t len) {
	uint16_t sum1 = 0;
	uint16_t sum2 = 0;
	for (size_t i = 0; i < len; i++) {
		sum1 = static_cast<uint16_t>((sum1 + data[i]) % 255);
		sum2 = static_cast<uint16_t>((sum2 + sum1) % 255);
	}
	return static_cast<uint16_t>((sum2 << 8) | sum1);
}

inline void HAQuDA_PutU16(uint8_t *out, uint16_t value) {
	out[0] = static_cast<uint8_t>(value & 0xFF);
	out[1] = static_cast<uint8_t>(value >> 8);
}

inline uint16_t HAQuDA_GetU16(const uint8_t *in) {
	return static_cast<uint16_t>(in[0] | (in[1] << 8));
}

inline void HAQuDA_PutU32(uint8_t *out, uint32_t value) {
	out[0] = static_cast<uint8_t>(value & 0xFF);
	out[1] = static_cast<uint8_t>((value >> 8) & 0xFF);
	out[2] = static_cast<uint8_t>((value >> 16) & 0xFF);
	out[3] = static_cast<uint8_t>((value >> 24) & 0xFF);
}

inline uint32_t HAQuDA_GetU32(const uint8_t *in) {
	return static_cast<uint32_t>(in[0]) | (static_cast<uint32_t>(in[1]) << 8) | (static_cast<uint32_t>(in[2]) << 16) | (static_cast<uint32_t>(in[3]) << 24);
}
