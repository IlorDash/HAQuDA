#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>

#include "FakePMSerial.h"
#include "HAQuDA_PMReader.h"

// Frames follow the ZH03B datasheet layout: 42 4D, length field 00 14, nine data words, checksum word (big-endian sum of
// every byte before it). The serial port is FakePMSerial: `rx` = bytes waiting in the RX buffer, `findResult` /
// `readBytesData` = what the blocking fallback (Find + ReadBytes) returns. The warm-up object is the real one: a fresh
// HAQuDA_PMWarmup is not warming; PowerOn(0) with nowMs inside PM_WARMUP_MS is.
//
// Frame A: PM1.0 = 10, PM2.5 = 300, PM10 = 50, checksum 0x010C
//   42 4D 00 14 00 00 00 00 00 00 00 0A 01 2C 00 32 00 00 00 00 00 00 01 0C
// Frame B: same, PM1.0 = 20, checksum 0x0116
//   42 4D 00 14 00 00 00 00 00 00 00 14 01 2C 00 32 00 00 00 00 00 00 01 16

TEST(PMReaderTest, ReadNullPayloadFailed) {
	const uint32_t nowMs = 1000;

	const pm_meas_result exp_result = pm_meas_result::failed;

	const uint8_t frameA[ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C};
	FakePMSerial serial;
	serial.rx.assign(frameA, frameA + ZH03B_FRAME_LEN);
	HAQuDA_PMWarmup warmup;
	HAQuDA_PMReader reader(serial, warmup);

	pm_meas_result res;

	res = reader.Read(nowMs, nullptr);

	EXPECT_EQ(exp_result, res);
}

TEST(PMReaderTest, ReadWhileWarmingUpReturnsWarmingUp) {
	const uint32_t powerOnMs = 0;
	const uint32_t nowMs = 1000;

	const pm_meas_result exp_result = pm_meas_result::warming_up;

	FakePMSerial serial;
	HAQuDA_PMWarmup warmup;
	warmup.PowerOn(powerOnMs);
	HAQuDA_PMReader reader(serial, warmup);
	uint8_t payload[ZH03B_PAYLOAD_LEN] = {};

	pm_meas_result res;

	res = reader.Read(nowMs, payload);

	EXPECT_EQ(exp_result, res);
}

TEST(PMReaderTest, ReadWhileWarmingUpDrainsAllAvailableBytes) {
	// More bytes than the scan buffer holds: the drain must not stop at PM_RX_BUF_SIZE
	const uint32_t powerOnMs = 0;
	const uint32_t nowMs = 1000;
	const size_t rxBytes = 300;

	const size_t exp_consumed = rxBytes;

	FakePMSerial serial;
	serial.rx.assign(rxBytes, 0x00);
	HAQuDA_PMWarmup warmup;
	warmup.PowerOn(powerOnMs);
	HAQuDA_PMReader reader(serial, warmup);
	uint8_t payload[ZH03B_PAYLOAD_LEN] = {};

	reader.Read(nowMs, payload);

	EXPECT_EQ(exp_consumed, serial.pos);
}

TEST(PMReaderTest, ReadWhileWarmingUpNeverCallsFindOrReadBytes) {
	const uint32_t powerOnMs = 0;
	const uint32_t nowMs = 1000;

	const int exp_calls = 0;

	const uint8_t frameA[ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C};
	FakePMSerial serial;
	serial.rx.assign(frameA, frameA + ZH03B_FRAME_LEN);
	HAQuDA_PMWarmup warmup;
	warmup.PowerOn(powerOnMs);
	HAQuDA_PMReader reader(serial, warmup);
	uint8_t payload[ZH03B_PAYLOAD_LEN] = {};

	reader.Read(nowMs, payload);

	EXPECT_EQ(exp_calls, serial.findCalls);
	EXPECT_EQ(exp_calls, serial.readBytesCalls);
}

TEST(PMReaderTest, ReadWhileWarmingUpLeavesPayloadUntouched) {
	const uint32_t powerOnMs = 0;
	const uint32_t nowMs = 1000;

	const uint8_t exp_payload[ZH03B_PAYLOAD_LEN] = {};

	const uint8_t frameA[ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C};
	FakePMSerial serial;
	serial.rx.assign(frameA, frameA + ZH03B_FRAME_LEN);
	HAQuDA_PMWarmup warmup;
	warmup.PowerOn(powerOnMs);
	HAQuDA_PMReader reader(serial, warmup);
	uint8_t payload[ZH03B_PAYLOAD_LEN] = {};

	reader.Read(nowMs, payload);

	EXPECT_EQ(0, memcmp(exp_payload, payload, ZH03B_PAYLOAD_LEN));
}

TEST(PMReaderTest, ReadValidBufferedFrameReturnsOk) {
	const uint32_t nowMs = 1000;

	const pm_meas_result exp_result = pm_meas_result::ok;

	const uint8_t frameA[ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C};
	FakePMSerial serial;
	serial.rx.assign(frameA, frameA + ZH03B_FRAME_LEN);
	HAQuDA_PMWarmup warmup;
	HAQuDA_PMReader reader(serial, warmup);
	uint8_t payload[ZH03B_PAYLOAD_LEN] = {};

	pm_meas_result res;

	res = reader.Read(nowMs, payload);

	EXPECT_EQ(exp_result, res);
}

TEST(PMReaderTest, ReadValidBufferedFrameCopiesPayload) {
	const uint32_t nowMs = 1000;

	const uint8_t exp_payload[ZH03B_PAYLOAD_LEN]
		= {0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C};

	const uint8_t frameA[ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C};
	FakePMSerial serial;
	serial.rx.assign(frameA, frameA + ZH03B_FRAME_LEN);
	HAQuDA_PMWarmup warmup;
	HAQuDA_PMReader reader(serial, warmup);
	uint8_t payload[ZH03B_PAYLOAD_LEN] = {};

	reader.Read(nowMs, payload);

	EXPECT_EQ(0, memcmp(exp_payload, payload, ZH03B_PAYLOAD_LEN));
}

TEST(PMReaderTest, ReadValidBufferedFrameDoesNotCallFind) {
	const uint32_t nowMs = 1000;

	const int exp_calls = 0;

	const uint8_t frameA[ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C};
	FakePMSerial serial;
	serial.rx.assign(frameA, frameA + ZH03B_FRAME_LEN);
	HAQuDA_PMWarmup warmup;
	HAQuDA_PMReader reader(serial, warmup);
	uint8_t payload[ZH03B_PAYLOAD_LEN] = {};

	reader.Read(nowMs, payload);

	EXPECT_EQ(exp_calls, serial.findCalls);
	EXPECT_EQ(exp_calls, serial.readBytesCalls);
}

TEST(PMReaderTest, ReadTwoBufferedFramesCopiesNewest) {
	const uint32_t nowMs = 1000;

	// Frame B payload (PM1.0 = 20)
	const uint8_t exp_payload[ZH03B_PAYLOAD_LEN]
		= {0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x16};

	// Frame A followed by frame B
	const uint8_t twoFrames[2 * ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C,
		   0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x16};
	FakePMSerial serial;
	serial.rx.assign(twoFrames, twoFrames + sizeof(twoFrames));
	HAQuDA_PMWarmup warmup;
	HAQuDA_PMReader reader(serial, warmup);
	uint8_t payload[ZH03B_PAYLOAD_LEN] = {};

	reader.Read(nowMs, payload);

	EXPECT_EQ(0, memcmp(exp_payload, payload, ZH03B_PAYLOAD_LEN));
}

TEST(PMReaderTest, ReadBufferedGarbageOnlyFallsBackToFindAndReadBytes) {
	const uint32_t nowMs = 1000;
	const size_t garbageBytes = 10;

	const pm_meas_result exp_result = pm_meas_result::ok;

	// Payload of frame A, as the sensor stream delivers it after the 42 4D marker
	const uint8_t framePayload[ZH03B_PAYLOAD_LEN]
		= {0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C};
	FakePMSerial serial;
	serial.rx.assign(garbageBytes, 0x00);
	serial.findResult = true;
	serial.readBytesData.assign(framePayload, framePayload + ZH03B_PAYLOAD_LEN);
	HAQuDA_PMWarmup warmup;
	HAQuDA_PMReader reader(serial, warmup);
	uint8_t payload[ZH03B_PAYLOAD_LEN] = {};

	pm_meas_result res;

	res = reader.Read(nowMs, payload);

	EXPECT_EQ(exp_result, res);
}

TEST(PMReaderTest, ReadFallbackCopiesPayload) {
	const uint32_t nowMs = 1000;

	const uint8_t exp_payload[ZH03B_PAYLOAD_LEN]
		= {0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C};

	FakePMSerial serial;
	serial.findResult = true;
	serial.readBytesData.assign(exp_payload, exp_payload + ZH03B_PAYLOAD_LEN);
	HAQuDA_PMWarmup warmup;
	HAQuDA_PMReader reader(serial, warmup);
	uint8_t payload[ZH03B_PAYLOAD_LEN] = {};

	reader.Read(nowMs, payload);

	EXPECT_EQ(0, memcmp(exp_payload, payload, ZH03B_PAYLOAD_LEN));
}

TEST(PMReaderTest, ReadFallbackSearchesMarker424D) {
	const uint32_t nowMs = 1000;

	const size_t exp_markerLen = 2;
	const uint8_t exp_markerFirst = ZH03B_MARKER_FIRST;
	const uint8_t exp_markerSecond = ZH03B_MARKER_SECOND;

	FakePMSerial serial;
	HAQuDA_PMWarmup warmup;
	HAQuDA_PMReader reader(serial, warmup);
	uint8_t payload[ZH03B_PAYLOAD_LEN] = {};

	reader.Read(nowMs, payload);

	EXPECT_EQ(1, serial.findCalls);
	EXPECT_EQ(exp_markerLen, serial.lastFindLen);
	EXPECT_EQ(exp_markerFirst, serial.lastFindMarker[0]);
	EXPECT_EQ(exp_markerSecond, serial.lastFindMarker[1]);
}

TEST(PMReaderTest, ReadFallbackReadsExactlyOnePayload) {
	const uint32_t nowMs = 1000;

	const size_t exp_len = ZH03B_PAYLOAD_LEN;

	FakePMSerial serial;
	serial.findResult = true;
	HAQuDA_PMWarmup warmup;
	HAQuDA_PMReader reader(serial, warmup);
	uint8_t payload[ZH03B_PAYLOAD_LEN] = {};

	reader.Read(nowMs, payload);

	EXPECT_EQ(1, serial.readBytesCalls);
	EXPECT_EQ(exp_len, serial.lastReadBytesLen);
}

TEST(PMReaderTest, ReadFallbackFindFalseFailed) {
	const uint32_t nowMs = 1000;

	const pm_meas_result exp_result = pm_meas_result::failed;

	FakePMSerial serial;
	serial.findResult = false;
	HAQuDA_PMWarmup warmup;
	HAQuDA_PMReader reader(serial, warmup);
	uint8_t payload[ZH03B_PAYLOAD_LEN] = {};

	pm_meas_result res;

	res = reader.Read(nowMs, payload);

	EXPECT_EQ(exp_result, res);
}

TEST(PMReaderTest, ReadFallbackFindFalseDoesNotCallReadBytes) {
	const uint32_t nowMs = 1000;

	const int exp_calls = 0;

	FakePMSerial serial;
	serial.findResult = false;
	HAQuDA_PMWarmup warmup;
	HAQuDA_PMReader reader(serial, warmup);
	uint8_t payload[ZH03B_PAYLOAD_LEN] = {};

	reader.Read(nowMs, payload);

	EXPECT_EQ(exp_calls, serial.readBytesCalls);
}

TEST(PMReaderTest, ReadFallbackShortReadFailed) {
	// 21 bytes of a valid payload (the last checksum byte is missing)
	const uint32_t nowMs = 1000;

	const pm_meas_result exp_result = pm_meas_result::failed;

	const uint8_t shortPayload[ZH03B_PAYLOAD_LEN - 1]
		= {0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
	FakePMSerial serial;
	serial.findResult = true;
	serial.readBytesData.assign(shortPayload, shortPayload + sizeof(shortPayload));
	HAQuDA_PMWarmup warmup;
	HAQuDA_PMReader reader(serial, warmup);
	uint8_t payload[ZH03B_PAYLOAD_LEN] = {};

	pm_meas_result res;

	res = reader.Read(nowMs, payload);

	EXPECT_EQ(exp_result, res);
}

TEST(PMReaderTest, ReadFallbackShortReadLeavesPayloadUntouched) {
	const uint32_t nowMs = 1000;

	const uint8_t exp_payload[ZH03B_PAYLOAD_LEN] = {};

	const uint8_t shortPayload[ZH03B_PAYLOAD_LEN - 1]
		= {0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
	FakePMSerial serial;
	serial.findResult = true;
	serial.readBytesData.assign(shortPayload, shortPayload + sizeof(shortPayload));
	HAQuDA_PMWarmup warmup;
	HAQuDA_PMReader reader(serial, warmup);
	uint8_t payload[ZH03B_PAYLOAD_LEN] = {};

	reader.Read(nowMs, payload);

	EXPECT_EQ(0, memcmp(exp_payload, payload, ZH03B_PAYLOAD_LEN));
}

TEST(PMReaderTest, ReadFallbackBadChecksumFailed) {
	// Checksum 0x010D, the data sums to 0x010C
	const uint32_t nowMs = 1000;

	const pm_meas_result exp_result = pm_meas_result::failed;

	const uint8_t badPayload[ZH03B_PAYLOAD_LEN]
		= {0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0D};
	FakePMSerial serial;
	serial.findResult = true;
	serial.readBytesData.assign(badPayload, badPayload + ZH03B_PAYLOAD_LEN);
	HAQuDA_PMWarmup warmup;
	HAQuDA_PMReader reader(serial, warmup);
	uint8_t payload[ZH03B_PAYLOAD_LEN] = {};

	pm_meas_result res;

	res = reader.Read(nowMs, payload);

	EXPECT_EQ(exp_result, res);
}

TEST(PMReaderTest, ReadFallbackBadChecksumLeavesPayloadUntouched) {
	const uint32_t nowMs = 1000;

	const uint8_t exp_payload[ZH03B_PAYLOAD_LEN] = {};

	const uint8_t badPayload[ZH03B_PAYLOAD_LEN]
		= {0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0D};
	FakePMSerial serial;
	serial.findResult = true;
	serial.readBytesData.assign(badPayload, badPayload + ZH03B_PAYLOAD_LEN);
	HAQuDA_PMWarmup warmup;
	HAQuDA_PMReader reader(serial, warmup);
	uint8_t payload[ZH03B_PAYLOAD_LEN] = {};

	reader.Read(nowMs, payload);

	EXPECT_EQ(0, memcmp(exp_payload, payload, ZH03B_PAYLOAD_LEN));
}

TEST(PMReaderTest, ReadFallbackBadLengthFieldFailed) {
	// Length field 00 13 with a matching checksum 0x010B
	const uint32_t nowMs = 1000;

	const pm_meas_result exp_result = pm_meas_result::failed;

	const uint8_t badPayload[ZH03B_PAYLOAD_LEN]
		= {0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0B};
	FakePMSerial serial;
	serial.findResult = true;
	serial.readBytesData.assign(badPayload, badPayload + ZH03B_PAYLOAD_LEN);
	HAQuDA_PMWarmup warmup;
	HAQuDA_PMReader reader(serial, warmup);
	uint8_t payload[ZH03B_PAYLOAD_LEN] = {};

	pm_meas_result res;

	res = reader.Read(nowMs, payload);

	EXPECT_EQ(exp_result, res);
}

TEST(PMReaderTest, ReadFallbackBadLengthFieldLeavesPayloadUntouched) {
	const uint32_t nowMs = 1000;

	const uint8_t exp_payload[ZH03B_PAYLOAD_LEN] = {};

	const uint8_t badPayload[ZH03B_PAYLOAD_LEN]
		= {0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0B};
	FakePMSerial serial;
	serial.findResult = true;
	serial.readBytesData.assign(badPayload, badPayload + ZH03B_PAYLOAD_LEN);
	HAQuDA_PMWarmup warmup;
	HAQuDA_PMReader reader(serial, warmup);
	uint8_t payload[ZH03B_PAYLOAD_LEN] = {};

	reader.Read(nowMs, payload);

	EXPECT_EQ(0, memcmp(exp_payload, payload, ZH03B_PAYLOAD_LEN));
}

TEST(PMReaderTest, ReadBufferLargerThanScanBufferReadsAtMostPMRxBufSize) {
	const uint32_t nowMs = 1000;
	const size_t rxBytes = 300;

	const size_t exp_consumed = PM_RX_BUF_SIZE;

	FakePMSerial serial;
	serial.rx.assign(rxBytes, 0x00);
	HAQuDA_PMWarmup warmup;
	HAQuDA_PMReader reader(serial, warmup);
	uint8_t payload[ZH03B_PAYLOAD_LEN] = {};

	reader.Read(nowMs, payload);

	EXPECT_EQ(exp_consumed, serial.pos);
}

TEST(PMReaderTest, ReadNullPayloadDoesNotTouchSerial) {
	const uint32_t nowMs = 1000;

	const int exp_calls = 0;

	FakePMSerial serial;
	serial.rx.assign(10, 0x00);
	HAQuDA_PMWarmup warmup;
	HAQuDA_PMReader reader(serial, warmup);

	reader.Read(nowMs, nullptr);

	EXPECT_EQ(exp_calls, serial.readCalls);
	EXPECT_EQ(exp_calls, serial.findCalls);
}

TEST(PMReaderTest, HasValidFrameBufferedFrameTrue) {
	const bool exp_result = true;

	const uint8_t frameA[ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C};
	FakePMSerial serial;
	serial.rx.assign(frameA, frameA + ZH03B_FRAME_LEN);
	HAQuDA_PMWarmup warmup;
	HAQuDA_PMReader reader(serial, warmup);

	bool res;

	res = reader.HasValidFrame();

	EXPECT_EQ(exp_result, res);
}

TEST(PMReaderTest, HasValidFrameDuringWarmUpTrue) {
	// Frames in the warm-up window still prove the sensor is alive
	const bool exp_result = true;

	const uint8_t frameA[ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C};
	FakePMSerial serial;
	serial.rx.assign(frameA, frameA + ZH03B_FRAME_LEN);
	HAQuDA_PMWarmup warmup;
	warmup.PowerOn(0);
	HAQuDA_PMReader reader(serial, warmup);

	bool res;

	res = reader.HasValidFrame();

	EXPECT_EQ(exp_result, res);
}

TEST(PMReaderTest, HasValidFrameEmptyBufferFalseWithoutWaiting) {
	const bool exp_result = false;
	const int exp_blocking_calls = 0;

	FakePMSerial serial;
	serial.findResult = true;
	HAQuDA_PMWarmup warmup;
	HAQuDA_PMReader reader(serial, warmup);

	bool res;

	res = reader.HasValidFrame();

	EXPECT_EQ(exp_result, res);
	EXPECT_EQ(exp_blocking_calls, serial.findCalls);
}

TEST(PMReaderTest, HasValidFrameBadChecksumFalse) {
	const bool exp_result = false;

	const uint8_t badFrame[ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0D};
	FakePMSerial serial;
	serial.rx.assign(badFrame, badFrame + ZH03B_FRAME_LEN);
	HAQuDA_PMWarmup warmup;
	HAQuDA_PMReader reader(serial, warmup);

	bool res;

	res = reader.HasValidFrame();

	EXPECT_EQ(exp_result, res);
}

// Wrong behaviour reproduced: HasValidFrame() drains the RX buffer and throws away a partial frame, so a frame that straddles
// two polls is never recognised - its head is gone when the tail arrives. The second call should report the sensor alive.
TEST(PMReaderTest, DISABLED_HasValidFrameFrameSplitAcrossTwoCallsTrue) {
	const size_t firstPartLen = 10;

	const bool exp_result = true;

	const uint8_t frameA[ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C};
	FakePMSerial serial;
	serial.rx.assign(frameA, frameA + firstPartLen);
	HAQuDA_PMWarmup warmup;
	HAQuDA_PMReader reader(serial, warmup);
	// First poll sees only the head of the frame (setup of the state, not the action under test)
	reader.HasValidFrame();
	// The rest of the frame arrives before the next poll
	serial.rx.insert(serial.rx.end(), frameA + firstPartLen, frameA + ZH03B_FRAME_LEN);

	bool res;

	res = reader.HasValidFrame();

	EXPECT_EQ(exp_result, res);
}
