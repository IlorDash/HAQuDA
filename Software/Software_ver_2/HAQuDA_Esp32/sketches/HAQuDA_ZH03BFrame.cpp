#include "HAQuDA_ZH03BFrame.h"

#include <cstring>

constexpr size_t PM1_0_OFFSET = 8;
constexpr size_t PM2_5_OFFSET = 10;
constexpr size_t PM10_OFFSET = 12;
constexpr size_t CHECKSUM_OFFSET = ZH03B_PAYLOAD_LEN - 2;

static uint16_t readWord(const uint8_t *payload, size_t offset) {
	return (uint16_t)((payload[offset] << 8) + payload[offset + 1]);
}

bool HAQuDA_IsZH03BPayloadValid(const uint8_t *payload) {
	if (payload == nullptr) {
		return false;
	}
	if ((payload[0] != ZH03B_LEN_FIELD_HI) || (payload[1] != ZH03B_LEN_FIELD_LO)) {
		return false;
	}

	int sum = ZH03B_MARKER_FIRST + ZH03B_MARKER_SECOND;
	for (size_t i = 0; i < CHECKSUM_OFFSET; i++) {
		sum += payload[i];
	}
	return sum == readWord(payload, CHECKSUM_OFFSET);
}

bool HAQuDA_FindNewestZH03BFrame(const uint8_t *raw, size_t len, uint8_t *payloadOut) {
	if ((raw == nullptr) || (payloadOut == nullptr) || (len < ZH03B_FRAME_LEN)) {
		return false;
	}

	for (size_t i = len - ZH03B_FRAME_LEN + 1; i-- > 0;) {
		if ((raw[i] == ZH03B_MARKER_FIRST) && (raw[i + 1] == ZH03B_MARKER_SECOND) && HAQuDA_IsZH03BPayloadValid(&raw[i + 2])) {
			memcpy(payloadOut, &raw[i + 2], ZH03B_PAYLOAD_LEN);
			return true;
		}
	}
	return false;
}

uint16_t HAQuDA_GetZH03BPM1_0(const uint8_t *payload) {
	if (payload == nullptr) {
		return 0;
	}
	return readWord(payload, PM1_0_OFFSET);
}

uint16_t HAQuDA_GetZH03BPM2_5(const uint8_t *payload) {
	if (payload == nullptr) {
		return 0;
	}
	return readWord(payload, PM2_5_OFFSET);
}

uint16_t HAQuDA_GetZH03BPM10(const uint8_t *payload) {
	if (payload == nullptr) {
		return 0;
	}
	return readWord(payload, PM10_OFFSET);
}
