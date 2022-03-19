#include "Board.h"
#include "FileStorage.h"

FileStorage fileStorage;

FileStorage::FileStorage() {
}

FileStorage::~FileStorage() {
}

bool FileStorage::Start(void) {
	return SPIFFS.begin();
}

void FileStorage::Stop(void) {
	SPIFFS.end();
}

void FileStorage::ListDir(const char *dirname, uint8_t levels, FileStorage::TListDirFunction fn) {
	File root = SPIFFS.open(dirname, FILE_READ);
	if (!root) {
		return;
	}
	if (!root.isDirectory()) {
		return;
	}

	File file = root.openNextFile();
	while (file) {
		if (file.isDirectory()) {
			if (levels) {
				ListDir(file.name(), levels - 1, fn);
			}
		} else {
		}
		if (fn) {
			fn(file.name(), file.size());
		}
		file = root.openNextFile();
	}
}

bool FileStorage::DeleteFile(const char *path) {
	DisableWdt();
	bool res = SPIFFS.remove(path);
	EnableWdt();
	return res;
}

bool FileStorage::Format() {
	DisableWdt();
	bool res = SPIFFS.format();
	EnableWdt();
	return res;
}

bool FileStorage::WriteFile(const char *path, const uint8_t *data, size_t len) {
	File file = SPIFFS.open(path, FILE_WRITE);
	if (!file) {
		return false;
	}
	bool res = false;
	if (len > 0) {
		if (file.write(data, len) == len) {
			res = true;
		} else {
		}
	} else {
		res = true;
	}
	file.close();
	return res;
}

bool FileStorage::AppendFile(const char *path, const uint8_t *data, size_t len) {
	File file = SPIFFS.open(path, FILE_APPEND);
	if (!file) {
		return false;
	}
	bool res = false;
	if (file.write(data, len) == len) {
		res = true;
	} else {
	}
	file.close();
	return res;
}

bool FileStorage::Exists(const String &path) {
	return SPIFFS.exists(path);
}

bool FileStorage::Exists(const char *path) {
	return SPIFFS.exists(path);
}

File FileStorage::FileOpen(const String &path, const char *mode = FILE_READ) {
	return SPIFFS.open(path, mode);
}

bool FileStorage::ReadFile(const char *path, uint8_t *data, size_t len) {
	File file = SPIFFS.open(path, FILE_READ);
	if (!file) {
		return false;
	}
	bool res = false;
	if (len == file.size()) {
		if (file.read(data, len) == len) {
			res = true;
		}
	} else {
		res = false;
	}
	file.close();
	return res;
}

bool FileStorage::write(File &file, String &s) {
	return file.write((uint8_t *)s.c_str(), s.length()) == s.length();
}

size_t FileStorage::FileSize(const char *path) {
	return SPIFFS.open(path, "r").size();
}

bool FileStorage::DeleteLogData() {
	return DeleteFile(FILE_NAME_MEAS_LOG) && DeleteFile(FILE_CHECK_COUNT);
}

saveNewWiFiCredsReturnMsgs FileStorage::SaveNewWiFiCreds(TWiFiCreds newWiFiCreds) {
	File f = SPIFFS.open(FILE_NAME_WIFI_NET, FILE_APPEND);
	int fSize = f.size();
	uint8_t storedWiFiCredsCnt = fSize / sizeof(TWiFiCreds);

	if (storedWiFiCredsCnt >= MAX_WIFI_CREDS_NUM) {
		f.close();
		return too_many_WiFi;
	} else {
		bool newWiFiCredsExists = false;
		int index = 0;
		if (fSize <= 0) {
			f.close();
			f = SPIFFS.open(FILE_NAME_WIFI_NET, FILE_WRITE);
		} else {
			f.close();
			f = SPIFFS.open(FILE_NAME_WIFI_NET, FILE_READ);
			int count = fSize / sizeof(TWiFiCreds);
			TWiFiCreds readWiFiNet;
			while (index < count && !newWiFiCredsExists) {
				if (f.read((uint8_t *)&readWiFiNet, sizeof(TWiFiCreds)) != sizeof(TWiFiCreds)) {
					return error_reading_stored_WiFi_creds;
				}
				if (readWiFiNet.ssid == newWiFiCreds.ssid) {
					newWiFiCredsExists = true;
				} else {
					index++;
				}
			}
			f.close();
			if (newWiFiCredsExists) {
				f = SPIFFS.open(FILE_NAME_WIFI_NET, FILE_WRITE);
			} else {
				f = SPIFFS.open(FILE_NAME_WIFI_NET, FILE_APPEND);
			}
		}
		if (newWiFiCredsExists) {
			f.seek(index * sizeof(TWiFiCreds));
			//Serial.println("Rewriting WiFi " + String(newWiFiCreds.ssid) + " found at " + String(index));
		}
		if (f.write((uint8_t *)&newWiFiCreds, sizeof(TWiFiCreds)) != sizeof(TWiFiCreds)) {
			return error_saving_new_WiFi_creds;
		}
		f.close();
	}
	return saved_new_WiFi_creds;
}