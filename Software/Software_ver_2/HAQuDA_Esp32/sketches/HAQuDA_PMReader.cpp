#include "HAQuDA_PMReader.h"

#include <cstring>

static const uint8_t PM_FRAME_MARKER[] = {ZH03B_MARKER_FIRST, ZH03B_MARKER_SECOND};

HAQuDA_PMReader::HAQuDA_PMReader(HAQuDA_IPMSerial &serial, HAQuDA_PMWarmup &warmup) : serial(serial), warmup(warmup) {
}

pm_meas_result HAQuDA_PMReader::Read(uint32_t nowMs, uint8_t *payloadOut) {
	if (payloadOut == nullptr) {
		return pm_meas_result::failed;
	}

	if (warmup.IsWarmingUp(nowMs)) {
		DiscardBuffered();
		return pm_meas_result::warming_up;
	}

	return readNewestFrame(payloadOut) ? pm_meas_result::ok : pm_meas_result::failed;
}

bool HAQuDA_PMReader::HasValidFrame() {
	// A frame takes ~25 ms on the wire, so a poll can catch one half-received. Reading that head out of the buffer would lose
	// the frame (the next poll sees only the tail); leave it buffered until a whole frame's worth of bytes is waiting.
	if (serial.Available() < (int)ZH03B_FRAME_LEN) {
		return false;
	}

	uint8_t payload[ZH03B_PAYLOAD_LEN] = {};
	return readBufferedFrame(payload);
}

// The ZH03B sends one frame per second while polling is slower, so older frames are stale and are dropped.
bool HAQuDA_PMReader::readBufferedFrame(uint8_t *payloadOut) {
	uint8_t raw[PM_RX_BUF_SIZE] = {};
	size_t rawLen = 0;
	while ((rawLen < sizeof(raw)) && (serial.Available() > 0)) {
		raw[rawLen++] = (uint8_t)serial.Read();
	}

	return HAQuDA_FindNewestZH03BFrame(raw, rawLen, payloadOut);
}

bool HAQuDA_PMReader::readNewestFrame(uint8_t *payloadOut) {
	if (readBufferedFrame(payloadOut)) {
		return true;
	}

	// Nothing usable was buffered (sensor just powered up, or only a partial frame): wait for the next frame, up to the 1 s stream timeout per step
	if (serial.Find(PM_FRAME_MARKER, sizeof(PM_FRAME_MARKER))) {
		// Read into a local buffer so payloadOut stays untouched when the frame is cut or fails the checksum
		uint8_t payload[ZH03B_PAYLOAD_LEN] = {};
		if ((serial.ReadBytes(payload, ZH03B_PAYLOAD_LEN) == ZH03B_PAYLOAD_LEN) && HAQuDA_IsZH03BPayloadValid(payload)) {
			memcpy(payloadOut, payload, ZH03B_PAYLOAD_LEN);
			return true;
		}
	}
	return false;
}

// Also keeps frames sent during the warm-up from piling up in the buffer
void HAQuDA_PMReader::DiscardBuffered() {
	while (serial.Available() > 0) {
		serial.Read();
	}
}
