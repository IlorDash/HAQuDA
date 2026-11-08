#include "HAQuDA_Logger.h"

#include "HAQuDA_WebServer.h"
#include "HAQuDA_FileStorage.h"

#define LOG_PRINT_T "%s - %s : %s"

static HAQuDA_ErrorHandler *errHandler;
static HAQuDA_LogDedup *dedup;

static int recordsInFile(const char *path) {
	const int size = HAQuDA_FileStorage::FileSize(path);
	if (size < 0) {
		return 0;
	}
	// A torn last record (power loss in the middle of a write) is ignored.
	return size / (int)sizeof(LogMsgStruct);
}

// SPIFFS logs "does not exists" as an error for every remove() of a missing file, so check first.
static void deleteIfExists(const char *path) {
	if (HAQuDA_FileStorage::Exists(path)) {
		HAQuDA_FileStorage::DeleteFile(path);
	}
}

void HAQuDA_Logger::SetErrHandler(HAQuDA_ErrorHandler *_err_handler) {
	errHandler = _err_handler;
}

void HAQuDA_Logger::SetDedup(HAQuDA_LogDedup *_dedup) {
	dedup = _dedup;
}

void HAQuDA_Logger::PrepareLogsFile() {
	deleteIfExists(FILE_NAME_LOGS_OLD);
	deleteIfExists(FILE_NAME_LOGS);
}

void HAQuDA_Logger::fillTimestamp(char *timestamp, size_t timestampSize) {
	if (HAQuDA_TimeService::GetTimeClientStarted()) {
		if (HAQuDA_TimeService::GetDateTime(timestamp)) {
			return;
		}
		// NTP runs but gave no time: a real failure. The handler only sets the error effect (ShowTask draws it), so the log call stays fast.
		if (errHandler != nullptr) {
			errHandler->FailedToUpdateNTP();
		}
	}
	// Without NTP (access point mode, before the first WiFi connection) the time is simply unknown, not an error.
	// The longest stamp "up 4294967.295 s" is 16 chars, fits the 21-byte field and keeps the stored record size.
	HAQuDA_FormatUptime(timestamp, timestampSize, millis());
}

void HAQuDA_Logger::rotateLogsIfFull() {
	const int size = HAQuDA_FileStorage::FileSize(FILE_NAME_LOGS);
	if ((size < 0) || ((size_t)size + sizeof(LogMsgStruct) <= (size_t)LOG_FILE_MAX_RECORDS * sizeof(LogMsgStruct))) {
		return;
	}
	// The newest records survive in /logs.old, the oldest ones are dropped.
	deleteIfExists(FILE_NAME_LOGS_OLD);
	if (!HAQuDA_FileStorage::RenameFile(FILE_NAME_LOGS, FILE_NAME_LOGS_OLD)) {
		// Not LogError(): it would store a record and rotate again. Delete /logs so it cannot grow without a limit.
		log_e("Log rotation failed, deleting " FILE_NAME_LOGS);
		deleteIfExists(FILE_NAME_LOGS);
	}
}

void HAQuDA_Logger::emitRecord(const LogMsgStruct &record) {
	if (HAQuDA_WebServer::GetWebServerStarted()) {
		char text[MSG_MAX_LEN] = {0};
		snprintf(text, sizeof(text), LOG_PRINT_T, record.timestamp, record.prefix, record.data);
		HAQuDA_WebServer::WebSerialPrint(text);
	}

	rotateLogsIfFull();
	if (!HAQuDA_FileStorage::AppendFile(FILE_NAME_LOGS, (const uint8_t *)&record, sizeof(record))) {
		log_e("Cannot append to " FILE_NAME_LOGS);
	}
}

void HAQuDA_Logger::emitSummary(const log_dedup_summary &summary) {
	LogMsgStruct record;
	// The stamp is the last occurrence, not the time of writing: it tells when the problem stopped.
	HAQuDA_FormatUptime(record.timestamp, sizeof(record.timestamp), summary.lastSeenMs);
	const char *prefix = (summary.level == log_dedup_level::error) ? REPEAT_ERROR_MSG_PREFIX : REPEAT_MSG_PREFIX;
	strncpy(record.prefix, prefix, sizeof(record.prefix) - 1);
	HAQuDA_FormatRepeatSummary(record.data, sizeof(record.data), summary.text, summary.repeats);
	emitRecord(record);
}

void HAQuDA_Logger::logMessage(log_dedup_level level, const char *msg) {
	LogMsgStruct record;
	char text[MSG_MAX_LEN] = {0};

	if (msg == nullptr) {
		msg = "";
	}
	fillTimestamp(record.timestamp, sizeof(record.timestamp));
	const char *prefix = (level == log_dedup_level::error) ? ERROR_MSG_PREFIX : INFO_MSG_PREFIX;
	strncpy(record.prefix, prefix, sizeof(record.prefix) - 1);
	strncpy(record.data, msg, sizeof(record.data) - 1);

	// The UART gets every line, also the repeats: a disconnected sensor stays visible there.
	// log_e/log_i are macros that compile to nothing below their core debug level.
	snprintf(text, sizeof(text), LOG_PRINT_T, record.timestamp, record.prefix, record.data);
	if (level == log_dedup_level::error) {
		log_e("%s", text);
	} else {
		log_i("%s", text);
	}

	if (dedup == nullptr) {
		emitRecord(record);
		return;
	}

	const log_dedup_result result = dedup->Check(level, record.data, millis());
	if (result.hasEvicted) {
		emitSummary(result.evicted);
	}
	if (result.write) {
		emitRecord(record);
	}
}

void HAQuDA_Logger::LogInfo(const char *msg_to_show) {
	logMessage(log_dedup_level::info, msg_to_show);
}

void HAQuDA_Logger::LogError(const char *msg_to_show) {
	logMessage(log_dedup_level::error, msg_to_show);
}

void HAQuDA_Logger::FlushExpiredRepeats() {
	if (dedup == nullptr) {
		return;
	}
	const uint32_t nowMs = millis();
	log_dedup_summary summary = {};
	while (dedup->PopExpired(nowMs, summary)) {
		emitSummary(summary);
	}
}

LogMsgStruct HAQuDA_Logger::GetStored_Log(int num) {
	LogMsgStruct logMsg;

	if (num < 0) {
		return logMsg;
	}

	const int oldNum = recordsInFile(FILE_NAME_LOGS_OLD);
	if (num < oldNum) {
		HAQuDA_FileStorage::ReadFile(FILE_NAME_LOGS_OLD, num * (int)sizeof(LogMsgStruct), (uint8_t *)&logMsg, sizeof(LogMsgStruct));
	} else {
		HAQuDA_FileStorage::ReadFile(FILE_NAME_LOGS, (num - oldNum) * (int)sizeof(LogMsgStruct), (uint8_t *)&logMsg, sizeof(LogMsgStruct));
	}
	return logMsg;
}

void HAQuDA_Logger::GetStored_Log(int num, char *text) {
	LogMsgStruct logMsg = HAQuDA_Logger::GetStored_Log(num);
	snprintf(text, MSG_MAX_LEN, LOG_PRINT_T, logMsg.timestamp, logMsg.prefix, logMsg.data);
}

int HAQuDA_Logger::GetStored_LogsNum() {
	return recordsInFile(FILE_NAME_LOGS_OLD) + recordsInFile(FILE_NAME_LOGS);
}

HAQuDA_Logger::~HAQuDA_Logger() {
}
