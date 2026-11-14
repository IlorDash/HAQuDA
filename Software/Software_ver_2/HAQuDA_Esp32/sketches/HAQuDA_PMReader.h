#pragma once

#include "HAQuDA_IPMSerial.h"
#include "HAQuDA_PMResult.h"
#include "HAQuDA_PMWarmup.h"
#include "HAQuDA_ZH03BFrame.h"

// RX buffer size of the ZH03B UART, also the size of the local scan buffer
constexpr size_t PM_RX_BUF_SIZE = 256;

// One ZH03B read: warm-up check, buffer drain, frame search. Takes no samples into sums, that stays with the caller.
class HAQuDA_PMReader {
  public:
	HAQuDA_PMReader(HAQuDA_IPMSerial &serial, HAQuDA_PMWarmup &warmup);

	// During the warm-up drains the RX buffer without waiting and returns warming_up. Otherwise copies the payload
	// (ZH03B_PAYLOAD_LEN bytes after 42 4D) of the newest checksum-valid buffered frame into payloadOut and returns ok; when
	// none is buffered waits for the next frame (up to the 1 s stream timeout per step) and returns ok if it is valid, else
	// failed. payloadOut is untouched unless ok; nullptr gives failed. loop task only.
	pm_meas_result Read(uint32_t nowMs, uint8_t *payloadOut);

	// True when the RX buffer holds a checksum-valid frame; never waits. With fewer than ZH03B_FRAME_LEN bytes waiting it returns
	// false and leaves them buffered, so a frame still arriving is completed by a later call; otherwise it consumes the buffered
	// bytes. A tail of one frame plus the head of the next drops that frame, usually only one (a later call finds the next
	// whole frame). Works during the warm-up too (frames carry the firmware version in PM2.5 then, but they prove the sensor is alive). loop task only.
	bool HasValidFrame();

	// Throws away every byte waiting in the RX buffer without waiting for more. The sensor keeps streaming into the buffer while
	// nobody reads it, so a presence check must start from an empty buffer or it accepts a frame sent before the sensor died. loop task only.
	void DiscardBuffered();

  private:
	bool readBufferedFrame(uint8_t *payloadOut);
	bool readNewestFrame(uint8_t *payloadOut);

	HAQuDA_IPMSerial &serial;
	HAQuDA_PMWarmup &warmup;
};
