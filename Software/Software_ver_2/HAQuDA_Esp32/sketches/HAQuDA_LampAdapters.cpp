#include "HAQuDA_LampAdapters.h"

#include "HAQuDA_DisplayService.h"
#include "HAQuDA_FileStorage.h"
#include "HAQuDA_LampDisplayRules.h"
#include "HAQuDA_LEDsParams.h"
#include "HAQuDA_Logger.h"

static_assert(MULTI_MODE_PARAM_NUM == LAMP_MULTI_NUM, "the lamp page and the meas handler must have the same number of bands");

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

HAQuDA_LampDisplayAdapter::HAQuDA_LampDisplayAdapter(HAQuDA_ErrorHandler &errorHandler, HAQuDA_DisplayService &displayService,
													 HAQuDA_MeasHandler &measHandlerRef)
	: errHandler(errorHandler), display(displayService), measHandler(measHandlerRef) {
}

void HAQuDA_LampDisplayAdapter::ShowAccessPointRunning() {
	waitingItem = HAQuDA_ItemOf(display_event::apRunning);
	display.Post(display_event::apRunning);
}

void HAQuDA_LampDisplayAdapter::showMeasMode(meas_mode m) {
	measHandler.SetMeasDispMode(m);
	const anim_id measAnim = measHandler.CurrentAnim();
	display.SetBackground(measAnim, waitingItem);
	display.SetMeasEnabled(measAnim != anim_id::none);
}

bool HAQuDA_LampDisplayAdapter::ErrorShown() {
	return display.SensorsErrorActive();
}

void HAQuDA_LampDisplayAdapter::ClearError() {
	errHandler.ClearError();
}

void HAQuDA_LampDisplayAdapter::ShowSensorsError() {
	errHandler.FailedToStartSensors();
}

void HAQuDA_LampDisplayAdapter::SetBrightnessPercent(uint8_t brightPercent) {
	display.SetBrightness(brightPercent);
}

void HAQuDA_LampDisplayAdapter::ShowColor(uint32_t rgb) {
	display.SetMeasEnabled(false);
	if (rgb == COLOR_OFF) {
		display.SetLampOff();
	} else {
		display.SetLampLight(rgb);
	}
}

void HAQuDA_LampDisplayAdapter::ShowStandard(lamp_param param) {
	measHandler.SetStandardMeasParam(HAQuDA_ToMeasType(param));
	showMeasMode(standard);
}

void HAQuDA_LampDisplayAdapter::ShowMulti(const lamp_param (&bottomToTop)[LAMP_MULTI_NUM]) {
	multi_mode_params params = {};
	for (size_t i = 0; i < MULTI_MODE_PARAM_NUM; i++) {
		params.paramsArr[i] = HAQuDA_ToMeasType(bottomToTop[i]);
	}
	measHandler.SetMultiMeasParam(params);
	showMeasMode(multi);
}

void HAQuDA_LampDisplayAdapter::ShowNight(lamp_param param, uint16_t fromMin, uint16_t toMin) {
	const night_time window = {fromMin, toMin};
	measHandler.SetStandardMeasParam(HAQuDA_ToMeasType(param));
	measHandler.SetNightTime(window);
	showMeasMode(night);
}
