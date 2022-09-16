#pragma once

class HAQuDA_Logger {
  public:
	HAQuDA_Logger();
	static void LogInfo(const char *msg);
	static void LogError(const char *msg);
	~HAQuDA_Logger();
};
