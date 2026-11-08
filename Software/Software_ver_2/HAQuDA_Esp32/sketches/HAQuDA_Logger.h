#pragma once

#include "HAQuDA_TimeHelper.h"
#include "HAQuDA_ErrorHandler.h"
#include "HAQuDA_LogDedup.h"

#define MSG_MAX_LEN 64
#define MSG_PREFIX_LEN 8
#define MSG_DATA_MAX_LEN 32

#define ERROR_MSG_PREFIX "ERROR"
#define INFO_MSG_PREFIX "Info"
// Summary of repeated info lines and of repeated errors; "RepeatE" fills the 8-byte prefix field, so the record size stays 61 B.
#define REPEAT_MSG_PREFIX "Repeat"
#define REPEAT_ERROR_MSG_PREFIX "RepeatE"

// Each of the two log files (/logs.old and /logs) holds at most this many records: 512 * 61 B = 31232 B, under 32 KB.
// Rotation keeps the SPIFFS share of the logs near 62 KB, the rest is for the WiFi credentials.
constexpr size_t LOG_FILE_MAX_RECORDS = 512;

typedef struct {
	char timestamp[DATE_TIME_STR_LEN + 1] = {0};
	char prefix[MSG_PREFIX_LEN] = {0};
	char data[MSG_DATA_MAX_LEN] = {0};
} LogMsgStruct;

// The dedup table compares texts of the size the record keeps.
static_assert(LOG_DEDUP_TEXT_SIZE == sizeof(LogMsgStruct::data), "LOG_DEDUP_TEXT_SIZE must equal the record data field size");

// All public functions run in the loop task (setup() and loop() are the only callers: ErrorHandler methods and the sensor checks are called from there),
// so the dedup table and the rotation need no lock. Calling them from ShowTask or a callback requires adding one.
class HAQuDA_Logger {
	// Fills the stamp with the date, or with the uptime when the date is not available.
	// Does not block on the time when NTP is not started (access point mode). In station mode GetFormattedDate() can still call forceUpdate() up to twice.
	static void fillTimestamp(char *timestamp, size_t timestampSize);
	// Prints to the UART (errors with log_e, info with log_i), dedups, stores. The level picks the record prefix and the UART function.
	static void logMessage(log_dedup_level level, const char *msg);
	// Prints the record to WebSerial (when it runs) and appends it to /logs, rotating first when the file is full.
	static void emitRecord(const LogMsgStruct &record);
	static void emitSummary(const log_dedup_summary &summary);
	static void rotateLogsIfFull();
	static LogMsgStruct GetStored_Log(int num);

  public:
	explicit HAQuDA_Logger();

	static void SetErrHandler(HAQuDA_ErrorHandler *_err_handler);
	// Without a dedup table every message is stored and printed. Call once in setup().
	static void SetDedup(HAQuDA_LogDedup *_dedup);
	// Deletes both log files. setup() only.
	static void PrepareLogsFile();

	// Every message goes to the UART, the file and WebSerial get it only when it is not a repeat of a recent message.
	// Info lines reach the UART only when the core debug level is info or higher (the shipped level is error), errors at error level and above.
	static void LogInfo(const char *msg);
	static void LogError(const char *msg);

	// Writes the summary of every message that stopped repeating (not seen for 10 s). Call on every loop() pass, it only scans 8 entries.
	static void FlushExpiredRepeats();

	// Records of /logs.old come first, then /logs, so index order is time order.
	static void GetStored_Log(int num, char *text);
	static int GetStored_LogsNum();
	~HAQuDA_Logger();
};
