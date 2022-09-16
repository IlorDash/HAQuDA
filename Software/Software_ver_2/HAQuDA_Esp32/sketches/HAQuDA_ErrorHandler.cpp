#include "HAQuDA_ErrorHandler.h"
#include "HAQuDA_DisplayManip.h"
#include "HAQuDA_Logger.h"

displayEffectMode_enum HAQuDA_ErrorHandler::errorEffectDisp = noneEffect;

growEffectsParams_struct HAQuDA_ErrorHandler::failedToConnectToWiFi_displayParams;
snakeEffectsParams_struct HAQuDA_ErrorHandler::failedToCreateAP_displayParams;

fadeEffectsParams_struct HAQuDA_ErrorHandler::failedToStartFS_displayParams;

upDownEffectsParams_struct HAQuDA_ErrorHandler::failedToStartSensors_displayParams;

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
	HAQuDA_DisplayManip::SetDisplayError(fade);
	vTaskDelay(DEFAULT_ERROR_DISPLAY_TIME / portTICK_PERIOD_MS);
}

void HAQuDA_ErrorHandler::FailedToConnectToWiFi() {
	CurrentError = failedToConnectToWiFi;
	HAQuDA_Logger::LogError("Failed connection to WiFi");
	HAQuDA_DisplayManip::SetDisplayError(grow);
	vTaskDelay(DEFAULT_ERROR_DISPLAY_TIME / portTICK_PERIOD_MS);
}

void HAQuDA_ErrorHandler::FailedToCreateAP() {
	CurrentError = failedToCreateAP;
	HAQuDA_Logger::LogError("Failed creating Acces Point");
	HAQuDA_DisplayManip::SetDisplayError(snake);
	vTaskDelay(DEFAULT_ERROR_DISPLAY_TIME / portTICK_PERIOD_MS);
}

void HAQuDA_ErrorHandler::FailedToStartSensors() {
	CurrentError = failedToStartSensors;
	HAQuDA_Logger::LogError("Failed to start sensors");
	HAQuDA_DisplayManip::SetDisplayError(upDown);
	vTaskDelay(DEFAULT_ERROR_DISPLAY_TIME / portTICK_PERIOD_MS);
}

void HAQuDA_ErrorHandler::ClearCurrentError() {
	CurrentError = noneError;
	HAQuDA_DisplayManip::ClearErrorDisplayMode();
}

HAQuDA_ErrorHandler::~HAQuDA_ErrorHandler() {
}
