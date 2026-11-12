#pragma once

#include <cstddef>
#include <cstdint>

// Hardware seam: the ZH03B UART, only the calls the PM read flow uses. The firmware implements it over SoftwareSerial,
// the tests with a fake. All methods: loop task only.
class HAQuDA_IPMSerial {
  public:
	virtual ~HAQuDA_IPMSerial() = default;

	// Number of bytes waiting in the RX buffer. Does not block.
	virtual int Available() = 0;
	// Next byte from the RX buffer, -1 when empty. Does not block.
	virtual int Read() = 0;
	// Reads and discards bytes until marker (len bytes) has been read. False on timeout (1 s stream timeout per step).
	virtual bool Find(const uint8_t *marker, size_t len) = 0;
	// Reads up to len bytes into buf, waits up to the stream timeout. Returns the number of bytes read.
	virtual size_t ReadBytes(uint8_t *buf, size_t len) = 0;
};
