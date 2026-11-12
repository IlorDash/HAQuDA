#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>

#include "HAQuDA_ZH03BFrame.h"

// Frames in these tests follow the ZH03B datasheet layout: 42 4D, length field 00 14, nine data words, checksum word.
// The checksum is the sum of every byte before it (42 + 4D + length bytes + data bytes), big-endian.
// Reference frame used in most tests (checksum worked out by hand):
//   PM1.0 = 0x000A = 10, PM2.5 = 0x012C = 300, PM10 = 0x0032 = 50, all other data bytes 0
//   sum = 0x42 + 0x4D + 0x14 + 0x0A + (0x01 + 0x2C) + 0x32 = 268 = 0x010C
// Frame B differs by PM1.0 = 0x0014 = 20, so its checksum is 0x0116.

// ---------------------------------------------------------------- HAQuDA_IsZH03BPayloadValid

TEST(ZH03BFrameTest, IsPayloadValidAcceptsLength0014WithMatchingChecksum) {
	const uint8_t frame[ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C};

	const bool exp_valid = true;

	const uint8_t *payload = &frame[2]; // the payload starts after 42 4D

	bool res;

	res = HAQuDA_IsZH03BPayloadValid(payload);

	EXPECT_EQ(exp_valid, res);
}

TEST(ZH03BFrameTest, IsPayloadValidRejectsWrongChecksum) {
	// Checksum is 0x010D, the data sums to 0x010C
	const uint8_t frame[ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0D};

	const bool exp_valid = false;

	const uint8_t *payload = &frame[2];

	bool res;

	res = HAQuDA_IsZH03BPayloadValid(payload);

	EXPECT_EQ(exp_valid, res);
}

TEST(ZH03BFrameTest, IsPayloadValidRejectsSwappedChecksumBytes) {
	// Checksum word bytes swapped: 0x0C01 instead of 0x010C (a little-endian reader would accept it)
	const uint8_t frame[ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x01};

	const bool exp_valid = false;

	const uint8_t *payload = &frame[2];

	bool res;

	res = HAQuDA_IsZH03BPayloadValid(payload);

	EXPECT_EQ(exp_valid, res);
}

TEST(ZH03BFrameTest, IsPayloadValidRejectsLength0013WithMatchingChecksum) {
	// Only the length field is wrong (00 13); the checksum 0x010B matches the bytes
	const uint8_t frame[ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0B};

	const bool exp_valid = false;

	const uint8_t *payload = &frame[2];

	bool res;

	res = HAQuDA_IsZH03BPayloadValid(payload);

	EXPECT_EQ(exp_valid, res);
}

TEST(ZH03BFrameTest, IsPayloadValidRejectsLength0114WithMatchingChecksum) {
	// Only the length field is wrong (01 14); the checksum 0x010D matches the bytes
	const uint8_t frame[ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x01, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0D};

	const bool exp_valid = false;

	const uint8_t *payload = &frame[2];

	bool res;

	res = HAQuDA_IsZH03BPayloadValid(payload);

	EXPECT_EQ(exp_valid, res);
}

TEST(ZH03BFrameTest, IsPayloadValidRejectsSwappedLength1400WithMatchingChecksum) {
	// Length bytes swapped (14 00): same byte sum as 00 14, so the checksum 0x010C still matches
	const uint8_t frame[ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C};

	const bool exp_valid = false;

	const uint8_t *payload = &frame[2];

	bool res;

	res = HAQuDA_IsZH03BPayloadValid(payload);

	EXPECT_EQ(exp_valid, res);
}

TEST(ZH03BFrameTest, IsPayloadValidRejectsNullptr) {
	const bool exp_valid = false;

	const uint8_t *payload = nullptr;

	bool res;

	res = HAQuDA_IsZH03BPayloadValid(payload);

	EXPECT_EQ(exp_valid, res);
}

// ---------------------------------------------------------------- HAQuDA_FindNewestZH03BFrame

TEST(ZH03BFrameTest, FindNewestFrameFillingWholeBufferAtIndexZero) {
	const uint8_t raw[ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C};

	const bool exp_found = true;
	const uint8_t *exp_payload = &raw[2];

	uint8_t payloadOut[ZH03B_PAYLOAD_LEN] = {};

	bool res;

	res = HAQuDA_FindNewestZH03BFrame(raw, sizeof(raw), payloadOut);

	EXPECT_EQ(exp_found, res);
	EXPECT_EQ(0, memcmp(exp_payload, payloadOut, ZH03B_PAYLOAD_LEN));
}

TEST(ZH03BFrameTest, FindNewestSingleValidFrameAfterGarbage) {
	// 5 garbage bytes (one of them 0x42, not followed by 0x4D), then the reference frame
	const uint8_t raw[5 + ZH03B_FRAME_LEN] = {0x11, 0x42, 0x22, 0x33, 0x44, 0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											  0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C};

	const bool exp_found = true;
	const uint8_t *exp_payload = &raw[5 + 2];

	uint8_t payloadOut[ZH03B_PAYLOAD_LEN] = {};

	bool res;

	res = HAQuDA_FindNewestZH03BFrame(raw, sizeof(raw), payloadOut);

	EXPECT_EQ(exp_found, res);
	EXPECT_EQ(0, memcmp(exp_payload, payloadOut, ZH03B_PAYLOAD_LEN));
}

TEST(ZH03BFrameTest, FindNewestTwoFramesPicksNewer) {
	// Frame A (PM1.0 = 10) followed by frame B (PM1.0 = 20)
	const uint8_t raw[2 * ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C,
		   0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x16};

	const bool exp_found = true;
	const uint8_t *exp_payload = &raw[ZH03B_FRAME_LEN + 2]; // payload of frame B

	uint8_t payloadOut[ZH03B_PAYLOAD_LEN] = {};

	bool res;

	res = HAQuDA_FindNewestZH03BFrame(raw, sizeof(raw), payloadOut);

	EXPECT_EQ(exp_found, res);
	EXPECT_EQ(0, memcmp(exp_payload, payloadOut, ZH03B_PAYLOAD_LEN));
}

TEST(ZH03BFrameTest, FindNewestPartialTailFallsBackToOlderFrame) {
	// Frame A, then only the first 10 bytes of frame B
	const uint8_t raw[ZH03B_FRAME_LEN + 10] = {0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00,
											   0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C, 0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	const bool exp_found = true;
	const uint8_t *exp_payload = &raw[2]; // payload of frame A

	uint8_t payloadOut[ZH03B_PAYLOAD_LEN] = {};

	bool res;

	res = HAQuDA_FindNewestZH03BFrame(raw, sizeof(raw), payloadOut);

	EXPECT_EQ(exp_found, res);
	EXPECT_EQ(0, memcmp(exp_payload, payloadOut, ZH03B_PAYLOAD_LEN));
}

TEST(ZH03BFrameTest, FindNewestCorruptedNewestChecksumPicksOlder) {
	// Frame A valid, then frame B with checksum 0x0117 instead of 0x0116
	const uint8_t raw[2 * ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C,
		   0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x17};

	const bool exp_found = true;
	const uint8_t *exp_payload = &raw[2]; // payload of frame A

	uint8_t payloadOut[ZH03B_PAYLOAD_LEN] = {};

	bool res;

	res = HAQuDA_FindNewestZH03BFrame(raw, sizeof(raw), payloadOut);

	EXPECT_EQ(exp_found, res);
	EXPECT_EQ(0, memcmp(exp_payload, payloadOut, ZH03B_PAYLOAD_LEN));
}

TEST(ZH03BFrameTest, FindNewestSkipsNewestFrameWithWrongLengthPicksOlder) {
	// Frame A valid, then a frame with length field 00 13 whose checksum 0x010B matches its bytes
	const uint8_t raw[2 * ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C,
		   0x42, 0x4D, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0B};

	const bool exp_found = true;
	const uint8_t *exp_payload = &raw[2]; // payload of frame A

	uint8_t payloadOut[ZH03B_PAYLOAD_LEN] = {};

	bool res;

	res = HAQuDA_FindNewestZH03BFrame(raw, sizeof(raw), payloadOut);

	EXPECT_EQ(exp_found, res);
	EXPECT_EQ(0, memcmp(exp_payload, payloadOut, ZH03B_PAYLOAD_LEN));
}

TEST(ZH03BFrameTest, FindNewestBufferOfExactly256BytesFrameAtEnd) {
	const size_t rawLen = 256;

	const uint8_t frame[ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C};

	const bool exp_found = true;
	const uint8_t *exp_payload = &frame[2];

	uint8_t raw[rawLen] = {};
	memcpy(&raw[rawLen - ZH03B_FRAME_LEN], frame, ZH03B_FRAME_LEN); // the frame ends on the last byte
	uint8_t payloadOut[ZH03B_PAYLOAD_LEN] = {};

	bool res;

	res = HAQuDA_FindNewestZH03BFrame(raw, rawLen, payloadOut);

	EXPECT_EQ(exp_found, res);
	EXPECT_EQ(0, memcmp(exp_payload, payloadOut, ZH03B_PAYLOAD_LEN));
}

TEST(ZH03BFrameTest, FindNewestBufferOf256BytesFrameAtIndexZero) {
	const size_t rawLen = 256;

	const uint8_t frame[ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C};

	const bool exp_found = true;
	const uint8_t *exp_payload = &frame[2];

	uint8_t raw[rawLen] = {};
	memcpy(raw, frame, ZH03B_FRAME_LEN); // the frame starts on the first byte, zeros after it
	uint8_t payloadOut[ZH03B_PAYLOAD_LEN] = {};

	bool res;

	res = HAQuDA_FindNewestZH03BFrame(raw, rawLen, payloadOut);

	EXPECT_EQ(exp_found, res);
	EXPECT_EQ(0, memcmp(exp_payload, payloadOut, ZH03B_PAYLOAD_LEN));
}

TEST(ZH03BFrameTest, FindNewestNoMarkerReturnsFalse) {
	const uint8_t raw[40] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
							 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};

	const bool exp_found = false;

	uint8_t payloadOut[ZH03B_PAYLOAD_LEN] = {};

	bool res;

	res = HAQuDA_FindNewestZH03BFrame(raw, sizeof(raw), payloadOut);

	EXPECT_EQ(exp_found, res);
}

TEST(ZH03BFrameTest, FindNewestShorterThanFrameReturnsFalse) {
	// A valid frame, but the length passed is one byte short of ZH03B_FRAME_LEN
	const uint8_t raw[ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C};

	const size_t len = ZH03B_FRAME_LEN - 1;

	const bool exp_found = false;

	uint8_t payloadOut[ZH03B_PAYLOAD_LEN] = {};

	bool res;

	res = HAQuDA_FindNewestZH03BFrame(raw, len, payloadOut);

	EXPECT_EQ(exp_found, res);
}

TEST(ZH03BFrameTest, FindNewestZeroLengthReturnsFalse) {
	const uint8_t raw[ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C};

	const size_t len = 0;

	const bool exp_found = false;

	uint8_t payloadOut[ZH03B_PAYLOAD_LEN] = {};

	bool res;

	res = HAQuDA_FindNewestZH03BFrame(raw, len, payloadOut);

	EXPECT_EQ(exp_found, res);
}

TEST(ZH03BFrameTest, FindNewestMarkerInsidePayloadIsNotTakenAsFrame) {
	// The valid frame carries 42 4D in its data (payload bytes 2-3) and 22 zero bytes follow it, so the scan also
	// tries the inner marker with a full frame length behind it; its "length field" is 00 00 and it is rejected.
	// Checksum of the outer frame: 268 + 0x42 + 0x4D = 411 = 0x019B.
	const uint8_t raw[ZH03B_FRAME_LEN + 22]
		= {0x42, 0x4D, 0x00, 0x14, 0x42, 0x4D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x9B};

	const bool exp_found = true;
	const uint8_t *exp_payload = &raw[2]; // payload of the outer frame

	uint8_t payloadOut[ZH03B_PAYLOAD_LEN] = {};

	bool res;

	res = HAQuDA_FindNewestZH03BFrame(raw, sizeof(raw), payloadOut);

	EXPECT_EQ(exp_found, res);
	EXPECT_EQ(0, memcmp(exp_payload, payloadOut, ZH03B_PAYLOAD_LEN));
}

TEST(ZH03BFrameTest, FindNewestLeavesOutputUntouchedOnFailure) {
	const uint8_t raw[40] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
							 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11};

	const uint8_t exp_payloadOut[ZH03B_PAYLOAD_LEN]
		= {0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE};

	uint8_t payloadOut[ZH03B_PAYLOAD_LEN]
		= {0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE};

	bool res;

	res = HAQuDA_FindNewestZH03BFrame(raw, sizeof(raw), payloadOut);

	EXPECT_FALSE(res);
	EXPECT_EQ(0, memcmp(exp_payloadOut, payloadOut, ZH03B_PAYLOAD_LEN));
}

TEST(ZH03BFrameTest, FindNewestNullptrRawReturnsFalse) {
	const uint8_t *raw = nullptr;
	const size_t len = ZH03B_FRAME_LEN;

	const bool exp_found = false;

	uint8_t payloadOut[ZH03B_PAYLOAD_LEN] = {};

	bool res;

	res = HAQuDA_FindNewestZH03BFrame(raw, len, payloadOut);

	EXPECT_EQ(exp_found, res);
}

TEST(ZH03BFrameTest, FindNewestNullptrOutputReturnsFalse) {
	const uint8_t raw[ZH03B_FRAME_LEN]
		= {0x42, 0x4D, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C};

	const bool exp_found = false;

	uint8_t *payloadOut = nullptr;

	bool res;

	res = HAQuDA_FindNewestZH03BFrame(raw, sizeof(raw), payloadOut);

	EXPECT_EQ(exp_found, res);
}

// ---------------------------------------------------------------- PM getters (no validation inside, no nullptr guard)

TEST(ZH03BFrameTest, GetPM1Dot0DecodesPayloadBytes8And9) {
	// Payload bytes: length field, then PM1.0 = 0x000A at bytes 8-9; PM2.5 and PM10 differ so a wrong offset shows
	const uint8_t payload[ZH03B_PAYLOAD_LEN]
		= {0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C};

	const uint16_t exp_pm = 10;

	uint16_t res;

	res = HAQuDA_GetZH03BPM1_0(payload);

	EXPECT_EQ(exp_pm, res);
}

TEST(ZH03BFrameTest, GetPM2Dot5DecodesPayloadBytes10And11) {
	// PM2.5 = 0x012C = 300 at bytes 10-11: the value above 255 shows the high byte comes first
	const uint8_t payload[ZH03B_PAYLOAD_LEN]
		= {0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C};

	const uint16_t exp_pm = 300;

	uint16_t res;

	res = HAQuDA_GetZH03BPM2_5(payload);

	EXPECT_EQ(exp_pm, res);
}

TEST(ZH03BFrameTest, GetPM10DecodesPayloadBytes12And13) {
	const uint8_t payload[ZH03B_PAYLOAD_LEN]
		= {0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x01, 0x2C, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x0C};

	const uint16_t exp_pm = 50;

	uint16_t res;

	res = HAQuDA_GetZH03BPM10(payload);

	EXPECT_EQ(exp_pm, res);
}

TEST(ZH03BFrameTest, GetPM1Dot0HighByteIsMostSignificant) {
	// PM1.0 = 0x0102 = 258: swapped bytes would give 513
	const uint8_t payload[ZH03B_PAYLOAD_LEN]
		= {0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	const uint16_t exp_pm = 258;

	uint16_t res;

	res = HAQuDA_GetZH03BPM1_0(payload);

	EXPECT_EQ(exp_pm, res);
}

TEST(ZH03BFrameTest, GetPM10HighByteIsMostSignificant) {
	// PM10 = 0x0102 = 258: swapped bytes would give 513
	const uint8_t payload[ZH03B_PAYLOAD_LEN]
		= {0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	const uint16_t exp_pm = 258;

	uint16_t res;

	res = HAQuDA_GetZH03BPM10(payload);

	EXPECT_EQ(exp_pm, res);
}

TEST(ZH03BFrameTest, GetPM2Dot5MaxWordIs65535) {
	// 0xFFFF must not turn negative through int promotion
	const uint8_t payload[ZH03B_PAYLOAD_LEN]
		= {0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	const uint16_t exp_pm = 65535;

	uint16_t res;

	res = HAQuDA_GetZH03BPM2_5(payload);

	EXPECT_EQ(exp_pm, res);
}
