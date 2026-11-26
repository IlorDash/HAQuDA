#include "HAQuDA_LampAdapters.h"

#include "HAQuDA_DisplayManip.h"
#include "HAQuDA_FileStorage.h"
#include "HAQuDA_LEDsParams.h"
#include "HAQuDA_LampDisplayRules.h"
#include "HAQuDA_Logger.h"

static_assert(MULTI_MODE_PARAM_NUM == LAMP_MULTI_NUM, "the lamp page and the meas handler must have the same number of bands");

namespace {
	// COLOR_YELLOW is greenish on the WS2812B, this orange reads as amber
	constexpr uint32_t COLOR_FLASH_AMBER = 0xFF8C00;
} // namespace

bool HAQuDA_FileStorageAdapter::Exists(const char *path) {
	return HAQuDA_FileStorage::Exists(path);
}

int HAQuDA_FileStorageAdapter::FileSize(const char *path) {
	return HAQuDA_FileStorage::FileSize(path);
}

bool HAQuDA_FileStorageAdapter::ReadFile(const char *path, uint8_t *data, size_t len) {
	return HAQuDA_FileStorage::ReadFile(path, data, len);
}

bool HAQuDA_FileStorageAdapter::WriteFile(const char *path, const uint8_t *data, size_t len) {
	return HAQuDA_FileStorage::WriteFile(path, data, len);
}

void HAQuDA_LoggerAdapter::LogInfo(const char *msg) {
	HAQuDA_Logger::LogInfo(msg);
}

void HAQuDA_LoggerAdapter::LogError(const char *msg) {
	HAQuDA_Logger::LogError(msg);
}

HAQuDA_LampDisplayAdapter::HAQuDA_LampDisplayAdapter(HAQuDA_ErrorHandler &errorHandler) : errHandler(errorHandler) {
}

bool HAQuDA_LampDisplayAdapter::ErrorShown() {
	return HAQuDA_ErrorShown(HAQuDA_DisplayManip::GetMode(), HAQuDA_ErrorHandler::GetCurrentError());
}

void HAQuDA_LampDisplayAdapter::ClearError() {
	errHandler.ClearError();
}

void HAQuDA_LampDisplayAdapter::ShowSensorsError() {
	// The same pair setup() uses; the error is the one an unstarted sensor set leaves on the matrix
	HAQuDA_DisplayManip::SetMode(error);
	errHandler.FailedToStartSensors();
}

void HAQuDA_LampDisplayAdapter::SetBrightnessPercent(uint8_t brightPercent) {
	HAQuDA_DisplayManip::SetBrightness(brightPercent);
}

void HAQuDA_LampDisplayAdapter::ShowColor(uint32_t rgb) {
	HAQuDA_DisplayManip::SetEffectParam(rgb);
	HAQuDA_DisplayManip::SetEffect(staticColor);
}

void HAQuDA_LampDisplayAdapter::ShowStandard(lamp_param param) {
	HAQuDA_DisplayManip::SetStandardMeasParam(HAQuDA_ToMeasType(param));
	HAQuDA_DisplayManip::SetMeasMode(standard);
}

void HAQuDA_LampDisplayAdapter::ShowMulti(const lamp_param (&bottomToTop)[LAMP_MULTI_NUM]) {
	multi_mode_params params = {};
	for (size_t i = 0; i < MULTI_MODE_PARAM_NUM; i++) {
		params.paramsArr[i] = HAQuDA_ToMeasType(bottomToTop[i]);
	}
	HAQuDA_DisplayManip::SetMultiMeasParam(params);
	HAQuDA_DisplayManip::SetMeasMode(multi);
}

void HAQuDA_LampDisplayAdapter::ShowNight(lamp_param param, uint16_t fromMin, uint16_t toMin) {
	const night_time window = {fromMin, toMin};
	HAQuDA_DisplayManip::SetStandardMeasParam(HAQuDA_ToMeasType(param));
	HAQuDA_DisplayManip::SetNightTime(window);
	HAQuDA_DisplayManip::SetMeasMode(night);
}

void HAQuDA_LampDisplayAdapter::ShowFlash(lamp_flash kind) {
	uint32_t rgb = COLOR_RED;
	if (kind == lamp_flash::locked) {
		rgb = COLOR_FLASH_AMBER;
	} else if (kind == lamp_flash::unlocked) {
		rgb = COLOR_GREEN;
	}
	ShowColor(rgb);
}
