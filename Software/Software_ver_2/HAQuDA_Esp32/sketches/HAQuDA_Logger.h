#pragma once

#include "HAQuDA_TimeHelper.h"

class HAQuDA_Logger {
  public:
	explicit  HAQuDA_Logger();
	static void LogInfo(const char *msg);
	static void LogError(const char *msg);
	~HAQuDA_Logger();
};
