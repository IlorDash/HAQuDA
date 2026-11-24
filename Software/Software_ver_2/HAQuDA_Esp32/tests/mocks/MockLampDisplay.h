#pragma once

#include <gmock/gmock.h>

#include "HAQuDA_ILampDisplay.h"

using lamp_multi_ref = const lamp_param (&)[LAMP_MULTI_NUM];

class MockLampDisplay : public HAQuDA_ILampDisplay {
  public:
	MOCK_METHOD(bool, ErrorShown, (), (override));
	MOCK_METHOD(void, ClearError, (), (override));
	MOCK_METHOD(void, ShowSensorsError, (), (override));
	MOCK_METHOD(void, SetBrightnessPercent, (uint8_t brightPercent), (override));
	MOCK_METHOD(void, ShowColor, (uint32_t rgb), (override));
	MOCK_METHOD(void, ShowStandard, (lamp_param param), (override));
	MOCK_METHOD(void, ShowMulti, (lamp_multi_ref bottomToTop), (override));
	MOCK_METHOD(void, ShowNight, (lamp_param param, uint16_t fromMin, uint16_t toMin), (override));
	MOCK_METHOD(void, ShowFlash, (lamp_flash kind), (override));
};
