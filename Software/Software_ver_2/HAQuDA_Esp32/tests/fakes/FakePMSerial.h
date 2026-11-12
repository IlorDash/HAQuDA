#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "HAQuDA_IPMSerial.h"

// Scripted stand-in for the ZH03B UART. The test fills `rx` (the bytes already waiting in the RX buffer) and scripts what the
// blocking fallback returns: `findResult` for Find(), `readBytesData` for ReadBytes(). The public counters record what the
// code under test did. No timing: Find()/ReadBytes() return at once with the scripted values.
class FakePMSerial : public HAQuDA_IPMSerial {
  public:
	std::vector<uint8_t> rx;
	size_t pos = 0; // next byte of rx to hand out; rx.size() - pos bytes are still waiting

	bool findResult = false;
	std::vector<uint8_t> readBytesData;

	int availableCalls = 0;
	int readCalls = 0;
	int findCalls = 0;
	int readBytesCalls = 0;
	uint8_t lastFindMarker[2] = {0, 0};
	size_t lastFindLen = 0;
	size_t lastReadBytesLen = 0;

	int Available() override {
		availableCalls++;
		return (int)(rx.size() - pos);
	}

	int Read() override {
		readCalls++;
		return (pos < rx.size()) ? rx[pos++] : -1;
	}

	bool Find(const uint8_t *marker, size_t len) override {
		findCalls++;
		lastFindLen = len;
		lastFindMarker[0] = marker[0];
		lastFindMarker[1] = marker[1];
		return findResult;
	}

	size_t ReadBytes(uint8_t *buf, size_t len) override {
		readBytesCalls++;
		lastReadBytesLen = len;
		const size_t n = std::min(len, readBytesData.size());
		std::copy(readBytesData.begin(), readBytesData.begin() + n, buf);
		return n;
	}
};
