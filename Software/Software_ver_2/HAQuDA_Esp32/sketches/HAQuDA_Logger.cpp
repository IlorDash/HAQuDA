#include "HAQuDA_Logger.h"

#include "HAQuDA_WebServer.h"

#define ERROR_MSG_PREFIX "ERROR: "
#define INFO_MSG_PREFIX "Info: "

HAQuDA_Logger::HAQuDA_Logger() {
}

void HAQuDA_Logger::LogInfo(const char *msg) {
	char *info_msg = strcat(INFO_MSG_PREFIX, msg);
	log_i("%s%s", INFO_MSG_PREFIX, msg);
	if (HAQuDA_WebServer::GetWebServerStarted()) {
		HAQuDA_WebServer::WebSerialPrint(info_msg);
	}
	
}
void HAQuDA_Logger::LogError(const char *msg) {
	char *error_msg = strcat(ERROR_MSG_PREFIX, msg);
	log_e("%s%s", ERROR_MSG_PREFIX, msg);
	if (HAQuDA_WebServer::GetWebServerStarted()) {
		HAQuDA_WebServer::WebSerialPrint(error_msg);
	}
}

HAQuDA_Logger::~HAQuDA_Logger() {
}