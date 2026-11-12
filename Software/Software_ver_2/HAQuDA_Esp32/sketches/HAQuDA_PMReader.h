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

  private:
	bool readNewestFrame(uint8_t *payloadOut);
	void drain();

	HAQuDA_IPMSerial &serial;
	HAQuDA_PMWarmup &warmup;
};
