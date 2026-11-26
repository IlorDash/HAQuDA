#pragma once

#include <gmock/gmock.h>

#include "HAQuDA_ILampActions.h"

class MockLampActions : public HAQuDA_ILampActions {
  public:
	MOCK_METHOD(void, TogglePower, (), (override));
	MOCK_METHOD(bool, StepBrightness, (bool up), (override));
	MOCK_METHOD(void, CommitBrightness, (), (override));
	MOCK_METHOD(void, NextMode, (), (override));
	MOCK_METHOD(bool, SetMode, (lamp_mode mode), (override));
	MOCK_METHOD(bool, ShowReading, (lamp_param param), (override));
	MOCK_METHOD(void, Flash, (lamp_flash kind), (override));
};
