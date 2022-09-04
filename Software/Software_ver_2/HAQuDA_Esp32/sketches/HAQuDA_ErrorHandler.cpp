#include "HAQuDA_ErrorHandler.h"


HAQuDA_ErrorHandler::HAQuDA_ErrorHandler() {
	failedToConnectToWiFi_displayParams.color = COLOR_RED;
	failedToConnectToWiFi_displayParams.speed = 200;

	failedToCreateAP_displayParams.color = COLOR_RED;
	failedToCreateAP_displayParams.speed = 200;
	failedToCreateAP_displayParams.tailLength = 1;
}

HAQuDA_ErrorHandler::~HAQuDA_ErrorHandler() {
}
