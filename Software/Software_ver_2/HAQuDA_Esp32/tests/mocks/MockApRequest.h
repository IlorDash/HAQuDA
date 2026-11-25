#pragma once

#include <gmock/gmock.h>

#include "HAQuDA_IApRequest.h"

class MockApRequest : public HAQuDA_IApRequest {
  public:
	MOCK_METHOD(bool, RequestAccessPoint, (), (override));
};
