#include "HAQuDA_ErrorHandler.h"


HAQuDA_ErrorHandler::HAQuDA_ErrorHandler() {
	failedToConnectToWiFi_displayParams.color = COLOR_RED;
	failedToConnectToWiFi_displayParams.speed = 200;

	failedToCreateAP_displayParams.color = COLOR_RED;
	failedToCreateAP_displayParams.speed = 200;
	failedToCreateAP_displayParams.tailLength = 1;

	errorEffectDisp = noneEffect;
	
	CurrentError = noneError;
}

void HAQuDA_ErrorHandler::FailedToConnectToWiFi() {
	CurrentError = failedToConnectToWiFi;
}

void HAQuDA_ErrorHandler::FailedToCreateAP() {
	CurrentError = failedToCreateAP;
}

HAQuDA_ErrorHandler::~HAQuDA_ErrorHandler() {
}
