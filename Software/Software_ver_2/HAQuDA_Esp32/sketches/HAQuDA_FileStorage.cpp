#include "HAQuDA_FileStorage.h"

HAQuDA_FileStorage::HAQuDA_FileStorage() {
}

bool HAQuDA_FileStorage::Start(void) {
	return SPIFFS.begin(true);
}

void HAQuDA_FileStorage::Stop() {
	SPIFFS.end();
}

void HAQuDA_FileStorage::ListDir(const char *dirname, uint8_t levels) {
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

bool HAQuDA_FileStorage::WriteFile(const char *path, const uint8_t *data, size_t len) {
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

bool HAQuDA_FileStorage::WriteFile(const char *path, const uint16_t fromPos, const uint8_t *data, size_t len) {
	File file = SPIFFS.open(path, FILE_WRITE);
	if (!file) {
		return false;
	}
	bool res = false;
	file.seek(fromPos);
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

bool HAQuDA_FileStorage::AppendFile(const char *path, const uint8_t *data, size_t len) {
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

bool HAQuDA_FileStorage::ReadFileFrom(const char *path, const int fromPos, uint8_t *data, size_t len) {
	File file = SPIFFS.open(path, FILE_READ);

	if (!file) {
		return false;
	}
	size_t readSize = len + fromPos;
	size_t fSize = file.size();
	if (readSize <= fSize) {

		size_t fileReadRes = file.read(data, readSize);

		if (fileReadRes != readSize) {
			return false;
		}
	} else {
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

size_t HAQuDA_FileStorage::FileSize(const char *path) {
	File f = SPIFFS.open(path, FILE_READ);
	int fSize = f.size();
	f.close();
	return fSize;
}

bool HAQuDA_FileStorage::checkStoredWiFiCredsCntLimit() {
	int fSize = FileSize(FILE_NAME_WIFI_NET);
	uint8_t storedWiFiCredsCnt = fSize / sizeof(TWiFiCreds);
	if (storedWiFiCredsCnt >= MAX_WIFI_CREDS_NUM) {
		return false;
	}
	return true;
}

bool HAQuDA_FileStorage::getWiFiCredsWriteIndex(uint16_t *index, const TWiFiCreds newWiFiCreds) {
	File file = SPIFFS.open(FILE_NAME_WIFI_NET, FILE_READ);

	if (!file) {
		return false;
	}

	*index = 0;
	uint8_t storedWiFiCredsCnt = file.size() / sizeof(TWiFiCreds);
	TWiFiCreds readWiFiCreds;
	bool newWiFiCredsExists = false;

	while (((*index) < storedWiFiCredsCnt) && !newWiFiCredsExists) {
		size_t fileReadRes = file.read((uint8_t *)&readWiFiCreds, sizeof(TWiFiCreds));
		if (fileReadRes != sizeof(TWiFiCreds)) {
			return false;
		}
		if (readWiFiCreds.ssid == newWiFiCreds.ssid) {
			newWiFiCredsExists = true;
		} else {
			(*index)++;
		}
	}
	file.close();
	return true;
}

saveNewWiFiCredsReturnMsgs HAQuDA_FileStorage::SaveNew_WiFiCreds(TWiFiCreds newWiFiCreds) {

	if (!checkStoredWiFiCredsCntLimit()) {
		return too_many_WiFi;
	}

	bool newWiFiCredsExists = false;
	int index = 0;
	int fSize = FileSize(FILE_NAME_WIFI_NET);
	if (fSize <= 0) {
		bool writeCredsRes = WriteFile(FILE_NAME_WIFI_NET, (uint8_t *)&newWiFiCreds, sizeof(TWiFiCreds));
		if (!writeCredsRes) {
			return error_saving_new_WiFi_creds;
		}
	} else {
		uint16_t index;
		bool getIndexRes = getWiFiCredsWriteIndex(&index, newWiFiCreds);
		if (!getIndexRes) {
			return error_reading_stored_WiFi_creds;
		}

		bool writeCredsRes = WriteFile(FILE_NAME_WIFI_NET, index * sizeof(TWiFiCreds), (uint8_t *)&newWiFiCreds, sizeof(TWiFiCreds));
		if (!writeCredsRes) {
			return error_saving_new_WiFi_creds;
		}
	}

	return saved_new_WiFi_creds;
}

TWiFiCreds HAQuDA_FileStorage::GetStored_WiFi(int num) {
	TWiFiCreds WiFiCreds;

	File f = SPIFFS.open(FILE_NAME_WIFI_NET, FILE_READ);
	int fSize = f.size();
	int count = fSize / sizeof(TWiFiCreds);
	if (num > count) {
		f.close();
		return WiFiCreds;
	}
	f.seek(num * sizeof(TWiFiCreds));
	if (f.read((uint8_t *)&WiFiCreds, sizeof(TWiFiCreds)) != sizeof(TWiFiCreds)) {
		TWiFiCreds WiFiCredsError;
		f.close();
		return WiFiCredsError;
	}
	f.close();
	return WiFiCreds;
}

int HAQuDA_FileStorage::GetStored_WiFiCredsNum() {
	return FileSize(FILE_NAME_WIFI_NET) / sizeof(TWiFiCreds);
}

const String HAQuDA_FileStorage::Read_WiFiCreds() {
	//	char buff[MAX_SHOW_WIFI_CREDS_BUFF_LEN];
	//	char *header = "Reading file : /WiFiNetworks\r\n";
	//	strncpy(buff, header, strlen(header));
	//
	//	int WiFiCredsNum = GetStored_WiFiCredsNum();
	//	TWiFiCreds readWiFiCred;
	//	for (int i = 0; i < WiFiCredsNum; i++) {
	//	}
	//
	//	return String(buff);
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