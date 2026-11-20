#pragma once

#include <gmock/gmock.h>

#include "HAQuDA_ILogger.h"

class MockLogger : public HAQuDA_ILogger {
  public:
	MOCK_METHOD(void, LogInfo, (const char *msg), (override));
	MOCK_METHOD(void, LogError, (const char *msg), (override));
};
