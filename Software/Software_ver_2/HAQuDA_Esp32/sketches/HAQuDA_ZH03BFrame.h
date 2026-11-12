#pragma once

#include <cstddef>
#include <cstdint>

// Frame start marker 42 4D (the two bytes before the payload)
constexpr uint8_t ZH03B_MARKER_FIRST = 0x42;
constexpr uint8_t ZH03B_MARKER_SECOND = 0x4D;
// Payload = the bytes after the marker: length field (2) + 9 data words (18) + checksum (2)
constexpr size_t ZH03B_PAYLOAD_LEN = 22;
// 42 4D + payload
constexpr size_t ZH03B_FRAME_LEN = ZH03B_PAYLOAD_LEN + 2;
// Length field (payload bytes 0-1, big-endian): the 9 data words + checksum that follow it = 20 bytes
constexpr uint8_t ZH03B_LEN_FIELD_HI = 0x00;
constexpr uint8_t ZH03B_LEN_FIELD_LO = 0x14;

// True when the payload (ZH03B_PAYLOAD_LEN bytes after 42 4D) has the expected length field and its checksum matches
// (sum of the marker and payload bytes 0-19 = the big-endian word in bytes 20-21). False for nullptr. Any task.
bool HAQuDA_IsZH03BPayloadValid(const uint8_t *payload);

// Scans raw backwards for the newest frame whose payload is valid and copies the ZH03B_PAYLOAD_LEN payload bytes into payloadOut.
// Needs no frame sync: a frame cut by a buffer overflow or a partial tail just fails the checksum. Returns false when raw holds
// no valid frame (payloadOut is then untouched), or for nullptr. Any task.
bool HAQuDA_FindNewestZH03BFrame(const uint8_t *raw, size_t len, uint8_t *payloadOut);

// Concentrations in ug/m3 from a valid payload (ZH03B_PAYLOAD_LEN bytes); the payload is not validated here. 0 for nullptr. Any task.
uint16_t HAQuDA_GetZH03BPM1_0(const uint8_t *payload);
uint16_t HAQuDA_GetZH03BPM2_5(const uint8_t *payload);
uint16_t HAQuDA_GetZH03BPM10(const uint8_t *payload);
