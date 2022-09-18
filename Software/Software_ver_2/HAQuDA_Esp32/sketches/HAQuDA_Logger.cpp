#include "HAQuDA_Logger.h"

#include "HAQuDA_WebServer.h"
#include "HAQuDA_FileStorage.h"

#define MSG_MAX_LEN 64

#define ERROR_MSG_PREFIX "ERROR: "
#define INFO_MSG_PREFIX "Info: "

HAQuDA_Logger::HAQuDA_Logger() {
}

void HAQuDA_Logger::LogInfo(const char *msg) {
	log_i("%s%s", INFO_MSG_PREFIX, msg);
	char info_msg[MSG_MAX_LEN] = INFO_MSG_PREFIX;
	strcat(info_msg, msg);
	if (HAQuDA_WebServer::GetWebServerStarted()) {
		//HAQuDA_WebServer::WebSerialPrint(info_msg);
	}
}
void HAQuDA_Logger::LogError(const char *msg) {
	log_e("%s%s", ERROR_MSG_PREFIX, msg);
	char error_msg[MSG_MAX_LEN] = ERROR_MSG_PREFIX;
	strcat(error_msg, msg);
	if (HAQuDA_WebServer::GetWebServerStarted()) {
		//HAQuDA_WebServer::WebSerialPrint(error_msg);
	}
}

HAQuDA_Logger::~HAQuDA_Logger() {
}