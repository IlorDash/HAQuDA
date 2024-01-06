#include "HAQuDA_ErrorHandler.h"
#include "HAQuDA_DisplayManip.h"
#include "HAQuDA_Logger.h"

effect_mode HAQuDA_ErrorHandler::errorEffectDisp = noneEffect;

grow_effect_params HAQuDA_ErrorHandler::failedToConnectToWiFi_displayParams;
snake_effect_params HAQuDA_ErrorHandler::failedToCreateAP_displayParams;

fade_effect_params HAQuDA_ErrorHandler::failedToStartFS_displayParams;

upDown_effects_params HAQuDA_ErrorHandler::failedToStartSensors_displayParams;

errorTypes_enum HAQuDA_ErrorHandler::CurrentError = noneError;

HAQuDA_ErrorHandler::HAQuDA_ErrorHandler() {
	failedToConnectToWiFi_displayParams.color = COLOR_RED;	//grow
	failedToConnectToWiFi_displayParams.speed = 200;

	failedToCreateAP_displayParams.color = COLOR_RED;	//snake
	failedToCreateAP_displayParams.speed = 200;
	failedToCreateAP_displayParams.tailLength = 1;

	failedToStartFS_displayParams.color = COLOR_RED;	//fade
	failedToStartFS_displayParams.speed = 10;
	failedToStartFS_displayParams.startBrightness = 100;
	failedToStartFS_displayParams.stopBrightness = 0;
	failedToStartFS_displayParams.step = 5;

	failedToStartSensors_displayParams.color = COLOR_RED;	//upDown
	failedToStartSensors_displayParams.speed = 200;
	failedToStartSensors_displayParams.length = 3;
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
	HAQuDA_DisplayManip::SetError(upDown);
	vTaskDelay(DEFAULT_ERROR_DISPLAY_TIME / portTICK_PERIOD_MS);
}

void HAQuDA_ErrorHandler::ClearCurrentError() {
	CurrentError = noneError;
	HAQuDA_DisplayManip::ClearError();
}

HAQuDA_ErrorHandler::~HAQuDA_ErrorHandler() {
}
