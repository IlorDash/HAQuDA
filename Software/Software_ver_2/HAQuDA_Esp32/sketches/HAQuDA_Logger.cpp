#include "HAQuDA_Logger.h"

#include "HAQuDA_WebServer.h"
#include "HAQuDA_FileStorage.h"

void HAQuDA_Logger::prepareLogInfoText(LogMsgStruct *log_i_struct, char *log_i_text, const char *msg) {

	if (!HAQuDA_TimeHelper_Singleton::getInstance()->GetDateTime(log_i_struct->timestamp)) {
		strncpy(log_i_struct->timestamp, CANT_GET_TIME_PREFIX, strlen(CANT_GET_TIME_PREFIX));
	}
	strncpy(log_i_struct->prefix, INFO_MSG_PREFIX, sizeof(INFO_MSG_PREFIX));
	strncpy(log_i_struct->data, msg, MSG_DATA_MAX_LEN);

	snprintf(log_i_text, MSG_MAX_LEN, "%s%s%s", log_i_struct->timestamp, log_i_struct->prefix, log_i_struct->data);
}

void HAQuDA_Logger::LogInfo(const char *msg_to_show) {
	LogMsgStruct logInfoStruct;
	char logInfoText[MSG_MAX_LEN] = {0};

	HAQuDA_Logger::prepareLogInfoText(&logInfoStruct, logInfoText, msg_to_show);

	log_i("%s%s%s", logInfoText.timestamp, logInfoText.prefix, logInfoText.data);

	if (HAQuDA_WebServer::GetWebServerStarted()) {
		HAQuDA_WebServer::WebSerialPrint(logInfoText);
	}

	HAQuDA_FileStorage::WriteFile(FILE_NAME_LOGS, (uint8_t *)&logInfoStruct, sizeof(logInfoStruct));
}

void HAQuDA_Logger::prepareLogErrorText(LogMsgStruct *log_e_struct, char *log_e_text, const char *msg) {

	if (!HAQuDA_TimeHelper_Singleton::getInstance()->GetDateTime(log_e_struct->timestamp)) {
		strncpy(log_e_struct->timestamp, CANT_GET_TIME_PREFIX, strlen(CANT_GET_TIME_PREFIX));
	}
	strncpy(log_e_struct->prefix, ERROR_MSG_PREFIX, sizeof(ERROR_MSG_PREFIX));
	strncpy(log_e_struct->data, msg, MSG_DATA_MAX_LEN);

	snprintf(log_e_text, MSG_MAX_LEN, "%s%s%s", log_e_struct->timestamp, log_e_struct->prefix, log_e_struct->data);
}

void HAQuDA_Logger::LogError(const char *msg_to_show) {
	LogMsgStruct logErrorStruct;
	char logErrorText[MSG_MAX_LEN] = {0};

	HAQuDA_Logger::prepareLogErrorText(&logErrorStruct, logErrorText, msg_to_show);

	log_i("%s%s%s", logErrorStruct.timestamp, logErrorStruct.prefix, logErrorStruct.data);

	if (HAQuDA_WebServer::GetWebServerStarted()) {
		HAQuDA_WebServer::WebSerialPrint(logErrorText);
	}

	HAQuDA_FileStorage::WriteFile(FILE_NAME_LOGS, (uint8_t *)&logErrorStruct, sizeof(logErrorStruct));
}

LogMsgStruct HAQuDA_Logger::GetStored_Log(int num) {
	LogMsgStruct logMsg;

	uint8_t pos = num * sizeof(LogMsgStruct);

	HAQuDA_FileStorage::ReadFile(FILE_NAME_LOGS, pos, (uint8_t *)&logMsg, sizeof(LogMsgStruct));
	return logMsg;
}

void HAQuDA_Logger::GetStored_Log(int num, char *text) {
	LogMsgStruct logMsg = HAQuDA_Logger::GetStored_Log(num);
	snprintf(text, MSG_MAX_LEN, "%s%s%s", logMsg.timestamp, logMsg.prefix, logMsg.data);
}

int HAQuDA_Logger::GetStored_LogsNum() {
	return HAQuDA_FileStorage::FileSize(FILE_NAME_LOGS) / sizeof(LogMsgStruct);
}

HAQuDA_Logger::~HAQuDA_Logger() {
}