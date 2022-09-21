#pragma once

#include "HAQuDA_TimeHelper.h"

class HAQuDA_Logger {
	static HAQuDA_TimeHelper *timeHelper;

  public:
	explicit  HAQuDA_Logger(HAQuDA_TimeHelper *currTimeHelper);
	static void LogInfo(const char *msg);
	static void LogError(const char *msg);
	~HAQuDA_Logger();
};
