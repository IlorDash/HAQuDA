#pragma once

#include <gmock/gmock.h>

#include <vector>

#include "HAQuDA_IFileStorage.h"

class MockFileStorage : public HAQuDA_IFileStorage {
  public:
	MOCK_METHOD(bool, Exists, (const char *path), (override));
	MOCK_METHOD(int, FileSize, (const char *path), (override));
	MOCK_METHOD(bool, ReadFile, (const char *path, uint8_t *data, size_t len), (override));
	MOCK_METHOD(bool, WriteFileBytes, (const char *path, const std::vector<uint8_t> &data));

	// The caller's buffer is gone after the call, so the written bytes are copied for the matchers
	bool WriteFile(const char *path, const uint8_t *data, size_t len) override {
		return WriteFileBytes(path, std::vector<uint8_t>(data, data + len));
	}
};
