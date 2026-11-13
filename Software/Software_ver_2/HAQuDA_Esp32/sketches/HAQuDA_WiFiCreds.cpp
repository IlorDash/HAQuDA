#include "HAQuDA_WiFiCreds.h"

#include <cstring>

size_t HAQuDA_PlaceCredsFirst(const TWiFiCreds *stored, size_t storedCount, const TWiFiCreds &newCreds, TWiFiCreds *out, size_t outCapacity) {
	if ((out == nullptr) || (newCreds.ssid[0] == '\0')) {
		return 0;
	}
	if ((stored == nullptr) && (storedCount > 0)) {
		storedCount = 0;
	}
	size_t maxNum = static_cast<size_t>(MAX_WIFI_CREDS_NUM);
	size_t limit = (outCapacity < maxNum) ? outCapacity : maxNum;
	if (limit == 0) {
		return 0;
	}

	size_t count = 0;
	memcpy(&out[count], &newCreds, sizeof(TWiFiCreds));
	// A record read from a damaged file may lack the terminator: the compare stays inside the field
	out[count].ssid[WIFI_SSID_LEN - 1] = '\0';
	count++;

	for (size_t i = 0; (i < storedCount) && (count < limit); i++) {
		if (stored[i].ssid[0] == '\0') {
			continue;
		}
		if (strncmp(stored[i].ssid, out[0].ssid, WIFI_SSID_LEN) == 0) {
			continue;
		}
		memcpy(&out[count], &stored[i], sizeof(TWiFiCreds));
		count++;
	}
	return count;
}

creds_recovery HAQuDA_DecideCredsRecovery(bool listExists, bool tmpExists, int tmpSize) {
	if (!tmpExists) {
		return creds_recovery::none;
	}
	bool whole = (tmpSize > 0) && (static_cast<size_t>(tmpSize) % sizeof(TWiFiCreds) == 0);
	return (!listExists && whole) ? creds_recovery::restore_tmp : creds_recovery::delete_tmp;
}

size_t HAQuDA_FilterRecoveredCreds(const TWiFiCreds *records, size_t count, TWiFiCreds *out, size_t outCapacity) {
	if (out == nullptr) {
		return 0;
	}
	if (records == nullptr) {
		return 0;
	}
	size_t maxNum = static_cast<size_t>(MAX_WIFI_CREDS_NUM);
	size_t limit = (outCapacity < maxNum) ? outCapacity : maxNum;
	size_t kept = 0;
	for (size_t i = 0; (i < count) && (kept < limit); i++) {
		bool ssidOk = (records[i].ssid[0] != '\0') && (memchr(records[i].ssid, '\0', WIFI_SSID_LEN) != nullptr);
		bool passOk = memchr(records[i].pass, '\0', WIFI_PASS_LEN) != nullptr;
		if (!ssidOk || !passOk) {
			continue;
		}
		memcpy(&out[kept], &records[i], sizeof(TWiFiCreds));
		kept++;
	}
	return kept;
}

void HAQuDA_WipeBytes(void *data, size_t len) {
	if (data == nullptr) {
		return;
	}
	volatile uint8_t *bytes = static_cast<volatile uint8_t *>(data);
	for (size_t i = 0; i < len; i++) {
		bytes[i] = 0;
	}
}
