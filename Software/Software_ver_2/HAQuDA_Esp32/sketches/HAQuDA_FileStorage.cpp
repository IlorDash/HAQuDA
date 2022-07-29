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

bool HAQuDA_FileStorage::ReadFileFrom(const char *path, const int bias, uint8_t *data, size_t len) {
	File file = SPIFFS.open(path, FILE_READ);
	bool res = false;
	if (!file) {
		res = false;
	} else {
		uint8_t *d;
		if (len <= file.size()) {
			int fileReadRes = file.read(d, bias + len);
			if (fileReadRes == (bias + len)) {
				res = true;
				memcpy(data, d + bias, len);
			} else {
				res = false;
			}
		} else {
			res = false;
		}
	}
	file.close();
	return res;
}

bool HAQuDA_FileStorage::Exists(const String &path) {
	return SPIFFS.exists(path);
}

bool HAQuDA_FileStorage::Exists(const char *path) {
	return SPIFFS.exists(path);
}

File HAQuDA_FileStorage::FileOpen(const String &path, const char *mode = FILE_READ) {
	return SPIFFS.open(path, mode);
}

bool HAQuDA_FileStorage::write(File &file, String &s) {
	return file.write((uint8_t *)s.c_str(), s.length()) == s.length();
}

size_t HAQuDA_FileStorage::FileSize(const char *path) {
	File f = SPIFFS.open(path, FILE_READ);
	int fSize = f.size();
	f.close();
	return fSize;
}

saveNewWiFiCredsReturnMsgs HAQuDA_FileStorage::SaveNew_WiFiCreds(TWiFiCreds newWiFiCreds) {
	saveNewWiFiCredsReturnMsgs returnMsg;

	int fSize = FileSize(FILE_NAME_WIFI_NET);
	uint8_t storedWiFiCredsCnt = fSize / sizeof(TWiFiCreds);

	File f = SPIFFS.open(FILE_NAME_WIFI_NET, FILE_APPEND);
	if (storedWiFiCredsCnt >= MAX_WIFI_CREDS_NUM) {
		returnMsg = too_many_WiFi;
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
			if (f.write((uint8_t *)&newWiFiCreds, sizeof(TWiFiCreds)) != sizeof(TWiFiCreds)) {
				f.close();
				return error_saving_new_WiFi_creds;
			}
			f.close();
			return re_writed_WiFi_creds;
		} else if (f.write((uint8_t *)&newWiFiCreds, sizeof(TWiFiCreds)) != sizeof(TWiFiCreds)) {
			f.close();
			return error_saving_new_WiFi_creds;
		}
	}
	f.close();
	return returnMsg;
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