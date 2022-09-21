#include "HAQuDA_Logger.h"

#include "HAQuDA_WebServer.h"
#include "HAQuDA_FileStorage.h"

#define MSG_MAX_LEN 57
#define MSG_TIMESTAMP_LEN 20
#define MSG_PREFIX_LEN 5
#define MSG_DATA_MAX_LEN 32

#define ERROR_MSG_PREFIX "ERROR: "
#define INFO_MSG_PREFIX "Info: "

#define CANT_GET_TIME_PREFIX "Can't get time"

typedef struct {
	char timestamp[MSG_TIMESTAMP_LEN] = {0};
	char prefix[MSG_PREFIX_LEN] = {0};
	char data[MSG_DATA_MAX_LEN] = {0};
} LogMsgStruct;

//HAQuDA_Logger::HAQuDA_Logger(HAQuDA_TimeHelper *currTimeHelper) {
//	this->timeHelper = currTimeHelper;
//}

void HAQuDA_Logger::LogInfo(const char *msg) {
	LogMsgStruct infoLog;
	if (!timeHelper->GetDateTime(infoLog.timestamp)) {
		strncpy(infoLog.timestamp, CANT_GET_TIME_PREFIX, strlen(CANT_GET_TIME_PREFIX));
	}
	strncpy(infoLog.prefix, INFO_MSG_PREFIX, sizeof(INFO_MSG_PREFIX));
	strncpy(infoLog.data, msg, MSG_DATA_MAX_LEN);
	log_i("%s%s%s", infoLog.timestamp, infoLog.prefix, infolog.data);
	char info_msg[MSG_MAX_LEN] = {0};
	snprintf(info_msg, MSG_MAX_LEN, "%s%s%s", infoLog.timestamp, infoLog.prefix, infoLog.data);
	if (HAQuDA_WebServer::GetWebServerStarted()) {
		HAQuDA_WebServer::WebSerialPrint(info_msg);
	}
}
void HAQuDA_Logger::LogError(const char *msg) {
	LogMsgStruct infoLog;
	if (!timeHelper->GetDateTime(infoLog.timestamp)) {
		strncpy(infoLog.timestamp, CANT_GET_TIME_PREFIX, strlen(CANT_GET_TIME_PREFIX));
	}
	strncpy(infoLog.prefix, ERROR_MSG_PREFIX, sizeof(ERROR_MSG_PREFIX));
	strncpy(infoLog.data, msg, MSG_DATA_MAX_LEN);
	log_i("%s%s%s", infoLog.timestamp, infoLog.prefix, infolog.data);
	char error_msg[MSG_MAX_LEN] = {0};
	snprintf(error_msg, MSG_MAX_LEN, "%s%s%s", infoLog.timestamp, infoLog.prefix, infoLog.data);
	if (HAQuDA_WebServer::GetWebServerStarted()) {
		HAQuDA_WebServer::WebSerialPrint(error_msg);
	}
}

HAQuDA_Logger::~HAQuDA_Logger() {
}