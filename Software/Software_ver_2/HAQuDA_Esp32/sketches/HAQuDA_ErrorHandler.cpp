#include "HAQuDA_ErrorHandler.h"

displayEffectMode_enum HAQuDA_ErrorHandler::errorEffectDisp = noneEffect;
growEffectsParams_struct HAQuDA_ErrorHandler::failedToConnectToWiFi_displayParams = {.color = COLOR_RED, .speed = 200};

snakeEffectsParams_struct HAQuDA_ErrorHandler::failedToCreateAP_displayParams = {.color = COLOR_RED, .speed = 200, .tailLength = 1};

errorTypes_enum HAQuDA_ErrorHandler::CurrentError = noneError;

HAQuDA_ErrorHandler::HAQuDA_ErrorHandler() {
}

void HAQuDA_ErrorHandler::FailedToConnectToWiFi() {
	CurrentError = failedToConnectToWiFi;
}

void HAQuDA_ErrorHandler::FailedToCreateAP() {
	CurrentError = failedToCreateAP;
}

HAQuDA_ErrorHandler::~HAQuDA_ErrorHandler() {
}
