#include "HAQuDA_ErrorHandler.h"

displayEffectMode_enum HAQuDA_ErrorHandler::errorEffectDisp = noneEffect;

growEffectsParams_struct HAQuDA_ErrorHandler::failedToConnectToWiFi_displayParams;
snakeEffectsParams_struct HAQuDA_ErrorHandler::failedToCreateAP_displayParams;

fadeEffectsParams_struct HAQuDA_ErrorHandler::failedToStartFS_displayParams;

upDownEffectsParams_struct HAQuDA_ErrorHandler::failedToStartSensors_displayParams;

errorTypes_enum HAQuDA_ErrorHandler::CurrentError = noneError;

HAQuDA_ErrorHandler::HAQuDA_ErrorHandler() {
	failedToConnectToWiFi_displayParams.color = COLOR_RED;
	failedToConnectToWiFi_displayParams.speed = 50;

	failedToCreateAP_displayParams.color = COLOR_RED;
	failedToCreateAP_displayParams.speed = 50;
	failedToCreateAP_displayParams.tailLength = 1;

	failedToStartFS_displayParams.color = COLOR_RED;
	failedToStartFS_displayParams.speed = 50;
	failedToStartFS_displayParams.startBrightness = 100;
	failedToStartFS_displayParams.stopBrightness = 0;
	failedToStartFS_displayParams.step = 5;

	failedToStartSensors_displayParams.color = COLOR_RED;
	failedToStartSensors_displayParams.speed = 50;
	failedToStartSensors_displayParams.length = 3;
}

errorTypes_enum HAQuDA_ErrorHandler::GetCurrentError() {
	return CurrentError;
}

void HAQuDA_ErrorHandler::FailedToStartFS() {
	log_e("SPIFFS not mounted!");
	
	CurrentError = failedToStartFS;
}

void HAQuDA_ErrorHandler::FailedToConnectToWiFi() {
	log_e("Failed connection to WiFi");
	CurrentError = failedToConnectToWiFi;
}

void HAQuDA_ErrorHandler::FailedToCreateAP() {
	log_e("Failed creating acces point");
	CurrentError = failedToCreateAP;
}

void HAQuDA_ErrorHandler::FailedToStartSensors() {
	log_e("Failed to start sensors");
	CurrentError = failedToStartSensors;
}

HAQuDA_ErrorHandler::~HAQuDA_ErrorHandler() {
}
