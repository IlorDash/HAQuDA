#include "HAQuDA_FileStorage.h"
#include "HAQuDA_FlashLock.h"

// An LED frame holds the lock for ~3.3 ms, so this wait is normally far shorter. On timeout the operation runs anyway:
// failing a credential or log write is worse than a rare one-frame LED glitch.
constexpr uint32_t FLASH_LOCK_TIMEOUT_MS = 100;

// Every public function below takes the flash lock once, in the outermost call, and calls only unlocked helpers or the SPIFFS
// API under it (the lock is not recursive). Reads are locked too: spi_flash_read stops the cache on both cores like a write.

HAQuDA_FileStorage::HAQuDA_FileStorage() {
}

bool HAQuDA_FileStorage::Start(void) {
	HAQuDA_FlashLockGuard guard(FLASH_LOCK_TIMEOUT_MS);
	return SPIFFS.begin(true);
}

void HAQuDA_FileStorage::Stop() {
	HAQuDA_FlashLockGuard guard(FLASH_LOCK_TIMEOUT_MS);
	SPIFFS.end();
}

void HAQuDA_FileStorage::ListDir(const char *dirname, const uint8_t levels) {
	HAQuDA_FlashLockGuard guard(FLASH_LOCK_TIMEOUT_MS);
	listDirUnlocked(dirname, levels);
}

void HAQuDA_FileStorage::listDirUnlocked(const char *dirname, const uint8_t levels) {
	Serial.printf("Listing directory: %s\r\n", dirname);

	File root = SPIFFS.open(dirname, FILE_READ);
	if (!root) {
		Serial.println("- failed to open directory");
		return;
	}
	if (!root.isDirectory()) {
		Serial.println(" - not a directory");
		return;
	}

	File file = root.openNextFile();
	while (file) {
		if (file.isDirectory()) {
			Serial.print("  DIR : ");
			Serial.println(file.name());
			if (levels) {
				listDirUnlocked(file.name(), levels - 1);
			}
		} else {
			Serial.print("  FILE: ");
			Serial.print(file.name());
			Serial.print("\tSIZE: ");
			Serial.println(file.size());
		}
		file = root.openNextFile();
	}
}

bool HAQuDA_FileStorage::DeleteFile(const char *path) {
	HAQuDA_FlashLockGuard guard(FLASH_LOCK_TIMEOUT_MS);
	DisableWdt();
	bool res = SPIFFS.remove(path);
	EnableWdt();
	return res;
}

bool HAQuDA_FileStorage::Format() {
	HAQuDA_FlashLockGuard guard(FLASH_LOCK_TIMEOUT_MS);
	DisableWdt();
	bool res = SPIFFS.format();
	EnableWdt();
	return res;
}

bool HAQuDA_FileStorage::RenameFile(const char *from, const char *to) {
	if ((from == nullptr) || (to == nullptr)) {
		return false;
	}
	HAQuDA_FlashLockGuard guard(FLASH_LOCK_TIMEOUT_MS);
	DisableWdt();
	bool res = SPIFFS.rename(from, to);
	EnableWdt();
	return res;
}

bool HAQuDA_FileStorage::WriteFile(const char *path, const uint8_t *data, const size_t len) {
	if (data == nullptr) {
		return false;
	}
	if (len <= 0) {
		return false;
	}

	HAQuDA_FlashLockGuard guard(FLASH_LOCK_TIMEOUT_MS);
	File file = SPIFFS.open(path, FILE_WRITE);
	if (!file) {
		return false;
	}
	if (file.write(data, len) != len) {
		file.close();
		return false;
	}

	file.close();
	return true;
}

bool HAQuDA_FileStorage::AppendFile(const char *path, const uint8_t *data, size_t len) {
	if (data == nullptr) {
		return false;
	}

	if (len <= 0) {
		return false;
	}

	HAQuDA_FlashLockGuard guard(FLASH_LOCK_TIMEOUT_MS);
	File file = SPIFFS.open(path, FILE_APPEND);
	if (!file) {
		return false;
	}
	if (file.write(data, len) != len) {
		file.close();
		return false;
	}
	file.close();
	return true;
}

bool HAQuDA_FileStorage::ReadFile(const char *path, uint8_t *data, size_t len) {
	if (data == nullptr) {
		return false;
	}

	HAQuDA_FlashLockGuard guard(FLASH_LOCK_TIMEOUT_MS);
	File file = SPIFFS.open(path, FILE_READ);

	if (!file) {
		return false;
	}
	size_t fSize = file.size();
	if (len > fSize) {
		file.close();
		return false;
	}

	size_t fileReadRes = file.read(data, len);

	if (fileReadRes != len) {
		file.close();
		return false;
	}
	file.close();
	return true;
}

bool HAQuDA_FileStorage::ReadFile(const char *path, const int fromPos, uint8_t *data, size_t len) {
	if (data == nullptr) {
		return false;
	}

	HAQuDA_FlashLockGuard guard(FLASH_LOCK_TIMEOUT_MS);
	File file = SPIFFS.open(path, FILE_READ);

	if (!file) {
		return false;
	}
	size_t readSize = len + fromPos;
	size_t fSize = file.size();
	file.seek(fromPos);
	if (readSize > fSize) {
		file.close();
		return false;
	}

	size_t fileReadRes = file.read(data, len);

	if (fileReadRes != len) {
		file.close();
		return false;
	}

	file.close();
	return true;
}

bool HAQuDA_FileStorage::Exists(const String &path) {
	return Exists(path.c_str());
}

bool HAQuDA_FileStorage::Exists(const char *path) {
	HAQuDA_FlashLockGuard guard(FLASH_LOCK_TIMEOUT_MS);
	return SPIFFS.exists(path);
}

int HAQuDA_FileStorage::FileSize(const char *path) {
	HAQuDA_FlashLockGuard guard(FLASH_LOCK_TIMEOUT_MS);
	File f = SPIFFS.open(path, FILE_READ);
	if (!f) {
		return -1;
	}
	int fSize = f.size();
	f.close();
	return fSize;
}

void HAQuDA_FileStorage::ReadFileInSerial(const char *path) {
	Serial.printf("Reading file: %s\r\n", path);

	HAQuDA_FlashLockGuard guard(FLASH_LOCK_TIMEOUT_MS);
	File file = SPIFFS.open(path, FILE_READ);
	if (!file || !file.isDirectory()) {
		Serial.println("- failed to open file for reading");
		return;
	}

	Serial.println("- read from file:");
	if (!file.available()) {
		Serial.write("File empty");
	} else {
		while (file.available()) {
			Serial.write(file.read());
		}
	}
	file.close();
}

HAQuDA_FileStorage::~HAQuDA_FileStorage() {
	SPIFFS.end();
}
