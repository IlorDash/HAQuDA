#pragma once

#include <cstddef>
#include <cstdint>

class HAQuDA_IFileStorage {
  public:
	virtual ~HAQuDA_IFileStorage() = default;

	virtual bool Exists(const char *path) = 0;
	virtual int FileSize(const char *path) = 0;
	virtual bool ReadFile(const char *path, uint8_t *data, size_t len) = 0;
	virtual bool WriteFile(const char *path, const uint8_t *data, size_t len) = 0;
};
