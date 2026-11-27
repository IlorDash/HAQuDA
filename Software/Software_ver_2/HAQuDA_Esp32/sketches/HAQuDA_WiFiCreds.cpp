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

size_t HAQuDA_RemoveCredsByName(const TWiFiCreds *stored, size_t storedCount, const char *ssid, TWiFiCreds *out, size_t outCapacity, bool *found) {
	if (found != nullptr) {
		*found = false;
	}
	if (out == nullptr) {
		return 0;
	}
	if ((stored == nullptr) && (storedCount > 0)) {
		storedCount = 0;
	}
	size_t maxNum = static_cast<size_t>(MAX_WIFI_CREDS_NUM);
	size_t limit = (outCapacity < maxNum) ? outCapacity : maxNum;
	// A name of WIFI_SSID_LEN bytes or more cannot be a stored one: a record keeps at most WIFI_SSID_LEN - 1 plus the terminator
	bool nameUsable = (ssid != nullptr) && (ssid[0] != '\0') && (strnlen(ssid, WIFI_SSID_LEN) < WIFI_SSID_LEN);

	bool matched = false;
	size_t count = 0;
	for (size_t i = 0; i < storedCount; i++) {
		if (stored[i].ssid[0] == '\0') {
			continue;
		}
		if (nameUsable && (strncmp(stored[i].ssid, ssid, WIFI_SSID_LEN) == 0)) {
			matched = true;
			continue;
		}
		if (count < limit) {
			memcpy(&out[count], &stored[i], sizeof(TWiFiCreds));
			count++;
		}
	}
	if (found != nullptr) {
		*found = matched;
	}
	return count;
}

size_t HAQuDA_FindVisibleCreds(const TWiFiCreds *stored, size_t storedCount, const wifi_scan_name *scan, size_t scanCount, uint8_t *outIndices,
							   size_t outCapacity) {
	if (outIndices == nullptr) {
		return 0;
	}
	if (stored == nullptr) {
		storedCount = 0;
	}
	if (scan == nullptr) {
		scanCount = 0;
	}
	if (storedCount > static_cast<size_t>(MAX_WIFI_CREDS_NUM)) {
		storedCount = static_cast<size_t>(MAX_WIFI_CREDS_NUM);
	}

	size_t count = 0;
	for (size_t i = 0; (i < storedCount) && (count < outCapacity); i++) {
		size_t storedLen = strnlen(stored[i].ssid, WIFI_SSID_LEN);
		if ((storedLen == 0) || (storedLen >= WIFI_SSID_LEN)) {
			continue;
		}
		for (size_t j = 0; j < scanCount; j++) {
			size_t scanLen = (scan[j].len > WIFI_SSID_LEN) ? WIFI_SSID_LEN : scan[j].len;
			if ((scanLen == storedLen) && (memcmp(stored[i].ssid, scan[j].ssid, storedLen) == 0)) {
				outIndices[count] = static_cast<uint8_t>(i);
				count++;
				break;
			}
		}
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
