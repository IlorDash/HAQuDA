#pragma once

// Loop task only, like HAQuDA_Logger behind it
class HAQuDA_ILogger {
  public:
	virtual ~HAQuDA_ILogger() = default;

	virtual void LogInfo(const char *msg) = 0;
	virtual void LogError(const char *msg) = 0;
};
