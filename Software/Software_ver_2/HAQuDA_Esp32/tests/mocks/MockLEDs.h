#pragma once

#include <gmock/gmock.h>

#include "HAQuDA_ILEDs.h"

class MockLEDs : public HAQuDA_ILEDs {
  public:
	MOCK_METHOD(void, SetBright, (uint8_t brightPerCent), (override));
	MOCK_METHOD(uint8_t, GetBright, (), (override));
	MOCK_METHOD(void, Clear, (), (override));
	MOCK_METHOD(void, Show, (), (override));
	MOCK_METHOD(void, Show, (uint16_t delayMs), (override));
	MOCK_METHOD(void, SetPixelColor, (uint16_t pixelNum, uint32_t color), (override));
	MOCK_METHOD(void, FillColor, (uint32_t color), (override));
	MOCK_METHOD(void, FillColor, (uint8_t r, uint8_t g, uint8_t b), (override));
	MOCK_METHOD(void, FillColorFrom, (uint32_t color, uint8_t startPixel, uint8_t numPixels), (override));
	MOCK_METHOD(void, FillColorFrom, (uint8_t r, uint8_t g, uint8_t b, uint8_t startPixel, uint8_t numPixels), (override));
	MOCK_METHOD(uint32_t, GetColor, (uint8_t r, uint8_t g, uint8_t b), (override));
};
