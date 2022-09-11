#include "HAQuDA_ErrorHandler.h"

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
	failedToStartFS_displayParams.speed = 200;
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
	log_e("SPIFFS not mounted!");
	CurrentError = failedToStartFS;
	vTaskDelay(DEFAULT_ERROR_DISPLAY_TIME / portTICK_PERIOD_MS);
}

void HAQuDA_ErrorHandler::FailedToConnectToWiFi() {
	log_e("Failed connection to WiFi");
	CurrentError = failedToConnectToWiFi;
	vTaskDelay(DEFAULT_ERROR_DISPLAY_TIME / portTICK_PERIOD_MS);
}

void HAQuDA_ErrorHandler::FailedToCreateAP() {
	log_e("Failed creating acces point");
	CurrentError = failedToCreateAP;
	vTaskDelay(DEFAULT_ERROR_DISPLAY_TIME / portTICK_PERIOD_MS);
}

void HAQuDA_ErrorHandler::FailedToStartSensors() {
	log_e("Failed to start sensors");
	CurrentError = failedToStartSensors;
	vTaskDelay(DEFAULT_ERROR_DISPLAY_TIME / portTICK_PERIOD_MS);
}

HAQuDA_ErrorHandler::~HAQuDA_ErrorHandler() {
}
