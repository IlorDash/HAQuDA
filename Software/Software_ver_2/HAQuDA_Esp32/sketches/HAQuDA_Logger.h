#pragma once

#include "HAQuDA_TimeHelper.h"
#include "HAQuDA_ErrorHandler.h"

#define MSG_MAX_LEN 64
#define MSG_PREFIX_LEN 8
#define MSG_DATA_MAX_LEN 32

#define ERROR_MSG_PREFIX "ERROR"
#define INFO_MSG_PREFIX "Info"

#define CANT_GET_TIME_PREFIX "Fail get time!"

typedef struct {
	char timestamp[DATE_TIME_STR_LEN + 1] = {0};
	char prefix[MSG_PREFIX_LEN] = {0};
	char data[MSG_DATA_MAX_LEN] = {0};
} LogMsgStruct;

class HAQuDA_Logger {
	static void prepareLogInfoText(LogMsgStruct *log_i_struct, char *log_i_text, const char *msg);
	static void prepareLogErrorText(LogMsgStruct *log_e_struct, char *log_e_text, const char *msg);
	static LogMsgStruct GetStored_Log(int num);

  public:
	explicit HAQuDA_Logger();

	static void SetErrHandler(HAQuDA_ErrorHandler *_err_handler);
	static void PreareLogsFile();

	static void LogInfo(const char *msg);
	static void LogError(const char *msg);

	static void GetStored_Log(int num, char *text);
	static int GetStored_LogsNum();
	~HAQuDA_Logger();
};
