#include "HAQuDA_FileStorage.h"

HAQuDA_FileStorage::HAQuDA_FileStorage() {
}

bool HAQuDA_FileStorage::Start(void) {
	return SPIFFS.begin(true);
}

void HAQuDA_FileStorage::Stop() {
	SPIFFS.end();
}

void HAQuDA_FileStorage::ListDir(const char *dirname, const uint8_t levels) {
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
				ListDir(file.name(), levels - 1);
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
	DisableWdt();
	bool res = SPIFFS.remove(path);
	EnableWdt();
	return res;
}

bool HAQuDA_FileStorage::Format() {
	DisableWdt();
	bool res = SPIFFS.format();
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

bool HAQuDA_FileStorage::WriteFile(const char *path, const uint16_t fromPos, const uint8_t *data, size_t len) {
	if (data == nullptr) {
		return false;
	}

	if (len <= 0) {
		return false;
	}

	File file = SPIFFS.open(path, FILE_WRITE);
	if (!file) {
		return false;
	}
	file.seek(fromPos);

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

	File file = SPIFFS.open(path, FILE_APPEND);
	if (!file) {
		return false;
	}
	bool res = false;
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
	return SPIFFS.exists(path);
}

bool HAQuDA_FileStorage::Exists(const char *path) {
	return SPIFFS.exists(path);
}

int HAQuDA_FileStorage::FileSize(const char *path) {
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