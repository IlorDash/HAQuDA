#pragma once

#include "HAQuDA_DisplayService.h"
#include "HAQuDA_ErrorHandler.h"
#include "HAQuDA_IFileStorage.h"
#include "HAQuDA_ILampDisplay.h"
#include "HAQuDA_ILogger.h"

// HAQuDA_FileStorage and HAQuDA_Logger have static methods only, which cannot override the interface methods
class HAQuDA_FileStorageAdapter : public HAQuDA_IFileStorage {
  public:
	bool Exists(const char *path) override;
	int FileSize(const char *path) override;
	bool ReadFile(const char *path, uint8_t *data, size_t len) override;
	bool WriteFile(const char *path, const uint8_t *data, size_t len) override;
};

class HAQuDA_LoggerAdapter : public HAQuDA_ILogger {
  public:
	void LogInfo(const char *msg) override;
	void LogError(const char *msg) override;
};

class HAQuDA_LampDisplayAdapter : public HAQuDA_ILampDisplay {
  public:
	HAQuDA_LampDisplayAdapter(HAQuDA_ErrorHandler &errorHandler, HAQuDA_DisplayService &displayService);

	bool ErrorShown() override;
	void ClearError() override;
	void ShowSensorsError() override;

	void SetBrightnessPercent(uint8_t brightPercent) override;
	void ShowColor(uint32_t rgb) override;
	void ShowStandard(lamp_param param) override;
	void ShowMulti(const lamp_param (&bottomToTop)[LAMP_MULTI_NUM]) override;
	void ShowNight(lamp_param param, uint16_t fromMin, uint16_t toMin) override;

  private:
	HAQuDA_ErrorHandler &errHandler;
	HAQuDA_DisplayService &display;
};
