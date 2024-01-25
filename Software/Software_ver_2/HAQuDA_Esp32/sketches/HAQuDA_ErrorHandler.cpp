#include "HAQuDA_ErrorHandler.h"
#include "HAQuDA_DisplayManip.h"
#include "HAQuDA_Logger.h"

effect_mode HAQuDA_ErrorHandler::errorEffectDisp = noneEffect;

grow_effect_params HAQuDA_ErrorHandler::failedToConnectToWiFiEffect;
snake_effect_params HAQuDA_ErrorHandler::failedToCreateAPEffect;

fade_effect_params HAQuDA_ErrorHandler::failedToStartFSEffect;

random_effect_params HAQuDA_ErrorHandler::failedToStartSensorsEffect;

errorTypes HAQuDA_ErrorHandler::CurrentError = noneError;

HAQuDA_ErrorHandler::HAQuDA_ErrorHandler() {
	failedToConnectToWiFiEffect.color = COLOR_RED;	//grow
	failedToConnectToWiFiEffect.speed = 200;

	failedToCreateAPEffect.color = COLOR_RED;	//snake
	failedToCreateAPEffect.speed = 200;
	failedToCreateAPEffect.tailLength = 1;

	failedToStartFSEffect.color = COLOR_RED;	//fade
	failedToStartFSEffect.speed = 10;
	failedToStartFSEffect.startBrightness = 100;
	failedToStartFSEffect.stopBrightness = 0;
	failedToStartFSEffect.step = 5;

    failedToStartSensorsEffect.speed = 50;      //random
	failedToStartSensorsEffect.pauseTime = 500;
}

errorTypes_enum HAQuDA_ErrorHandler::GetCurrentError() {
	return CurrentError;
}

void HAQuDA_ErrorHandler::FailedToStartFS() {
	CurrentError = failedToStartFS;
	HAQuDA_Logger::LogError("SPIFFS not mounted!");
	HAQuDA_DisplayManip::SetError(fade);
	vTaskDelay(DEFAULT_ERROR_DISPLAY_TIME / portTICK_PERIOD_MS);
}

void HAQuDA_ErrorHandler::FailedToConnectToWiFi() {
	CurrentError = failedToConnectToWiFi;
	HAQuDA_Logger::LogError("Failed connection to WiFi");
	HAQuDA_DisplayManip::SetError(grow);
	vTaskDelay(DEFAULT_ERROR_DISPLAY_TIME / portTICK_PERIOD_MS);
}

void HAQuDA_ErrorHandler::FailedToCreateAP() {
	CurrentError = failedToCreateAP;
	HAQuDA_Logger::LogError("Failed creating Acces Point");
	HAQuDA_DisplayManip::SetError(snake);
	vTaskDelay(DEFAULT_ERROR_DISPLAY_TIME / portTICK_PERIOD_MS);
}

void HAQuDA_ErrorHandler::FailedToStartSensors() {
	CurrentError = failedToStartSensors;
	HAQuDA_Logger::LogError("Failed to start sensors");
	HAQuDA_DisplayManip::SetError(randomPixel);
	vTaskDelay(DEFAULT_ERROR_DISPLAY_TIME / portTICK_PERIOD_MS);
}

void HAQuDA_ErrorHandler::ClearCurrentError() {
	CurrentError = noneError;
	HAQuDA_DisplayManip::ClearError();
}

HAQuDA_ErrorHandler::~HAQuDA_ErrorHandler() {
}
