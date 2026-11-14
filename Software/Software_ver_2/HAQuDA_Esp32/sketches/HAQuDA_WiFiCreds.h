#pragma once

#include <cstddef>
#include <cstdint>

// Hardware-free part of the saved WiFi network list: the record layout of /WiFiNetworks, the list order and the recovery
// decision after an interrupted save.

constexpr int MAX_WIFI_CREDS_NUM = 10;
constexpr size_t WIFI_SSID_LEN = 32;
constexpr size_t WIFI_PASS_LEN = 32;

// One record of /WiFiNetworks. Two char arrays, so the file layout is the same with or without a packing attribute.
struct TWiFiCreds {
	char ssid[WIFI_SSID_LEN] = "";
	char pass[WIFI_PASS_LEN] = "";
};
static_assert(sizeof(TWiFiCreds) == WIFI_SSID_LEN + WIFI_PASS_LEN, "a record is written to the file as it is: no padding");

// Builds the saved list after a new network joined: newCreds first, then the stored records in their order. A stored
// record with the same name as newCreds (its password is replaced) or with an empty name is dropped. At most
// min(outCapacity, MAX_WIFI_CREDS_NUM) records are written; the oldest ones are cut. stored may be nullptr when
// storedCount is 0. Returns the number of records written to out; 0 (out untouched) when newCreds has no name or out
// is nullptr. Any task.
size_t HAQuDA_PlaceCredsFirst(const TWiFiCreds *stored, size_t storedCount, const TWiFiCreds &newCreds, TWiFiCreds *out, size_t outCapacity);

// A save writes the whole list to a temporary file, deletes /WiFiNetworks and renames the temporary file over it. A power
// loss between the delete and the rename leaves only the temporary file.
enum class creds_recovery : uint8_t { none, restore_tmp, delete_tmp };

// At most this many records of the temporary file are read at boot; anything beyond is ignored (the list is written
// newest first, and a list never holds more than MAX_WIFI_CREDS_NUM records anyway)
constexpr size_t CREDS_RECOVERY_READ_MAX = 16;

// What to do at boot with the files an interrupted save may have left. Only a temporary file that holds whole records
// (tmpSize > 0 and a multiple of the record size) is ever used: the save finishes writing it before it touches the
// list, so a complete one without the list means the power failed after the delete.
//  - list missing, temporary file whole: restore_tmp (read its records, keep the valid ones with
//    HAQuDA_FilterRecoveredCreds(), write them as the list; none valid: drop the temporary file)
//  - temporary file present otherwise (list present: the power failed while writing it; or not whole): delete_tmp
//  - no temporary file: none
// Sizes are bytes; a negative size is treated as 0. Any task.
creds_recovery HAQuDA_DecideCredsRecovery(bool listExists, bool tmpExists, int tmpSize);

// Picks the records worth restoring from the records read out of the temporary file: in file order, only those with a
// non-empty name that is terminated inside its field and a password terminated inside its field (an empty password is
// valid), at most min(outCapacity, MAX_WIFI_CREDS_NUM). The file is written newest first, so the first ones are the
// most recent. records may be nullptr when count is 0. Returns the number written to out; 0 when out is nullptr. Any task.
size_t HAQuDA_FilterRecoveredCreds(const TWiFiCreds *records, size_t count, TWiFiCreds *out, size_t outCapacity);

// Overwrites the bytes in a way the compiler does not drop as a dead store; for buffers that held passwords. Any task.
void HAQuDA_WipeBytes(void *data, size_t len);
