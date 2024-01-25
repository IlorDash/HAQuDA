#include "HAQuDA_ErrorHandler.h"
#include "HAQuDA_DisplayManip.h"
#include "HAQuDA_Logger.h"

effect_mode HAQuDA_ErrorHandler::errorEffectDisp = noneEffect;

grow_effect_params HAQuDA_ErrorHandler::failedToConnectToWiFiEffect;
snake_effect_params HAQuDA_ErrorHandler::failedToCreateAPEffect;

fade_effect_params HAQuDA_ErrorHandler::failedToStartFSEffect;

random_effect_params HAQuDA_ErrorHandler::failedToStartSensorsEffect;

errorTypes HAQuDA_ErrorHandler::currentError = noneError;

HAQuDA_ErrorHandler::HAQuDA_ErrorHandler() {
	failedToConnectToWiFiEffect.color = COLOR_RED; // grow
	failedToConnectToWiFiEffect.speed = 200;

	failedToCreateAPEffect.color = COLOR_RED; // snake
	failedToCreateAPEffect.speed = 200;
	failedToCreateAPEffect.tailLength = 1;

	failedToStartFSEffect.color = COLOR_RED; // fade
	failedToStartFSEffect.speed = 10;
	failedToStartFSEffect.startBrightness = 100;
	failedToStartFSEffect.stopBrightness = 0;
	failedToStartFSEffect.step = 5;

	failedToStartSensorsEffect.speed = 50; // random
	failedToStartSensorsEffect.pauseTime = 500;
}

void HAQuDA_ErrorHandler::setErrorToDisplayManip(const effect_mode new_error) {
	HAQuDA_DisplayManip::SetMode(error);
	HAQuDA_DisplayManip::SetEffectMode(new_error);
}

void HAQuDA_ErrorHandler::resetErrorOnDisplayManip() {
	HAQuDA_DisplayManip::SetMode(noneMode);
}

void HAQuDA_ErrorHandler::ShowError() {
	switch (currentError) {
		case failedToConnectToWiFi: {
			HAQuDA_DisplayManip::ShowEffectGrow(HAQuDA_ErrorHandler::failedToConnectToWiFiEffect);
			break;
		}
		case failedToCreateAP: {
			HAQuDA_DisplayManip::ShowEffectSnake(HAQuDA_ErrorHandler::failedToCreateAPEffect);
			break;
		}
		case failedToStartFS: {
			HAQuDA_DisplayManip::ShowEffectFade(HAQuDA_ErrorHandler::failedToStartFSEffect);
			break;
		}
		case failedToStartSensors: {
			HAQuDA_DisplayManip::ShowEffectRandom(HAQuDA_ErrorHandler::failedToStartSensorsEffect);
			break;
		}
		default:
			break;
	}
}

void HAQuDA_ErrorHandler::FailedToStartFS() {
	currentError = failedToStartFS;
	HAQuDA_Logger::LogError("SPIFFS not mounted!");
	HAQuDA_DisplayManip::SetMode(error);
	HAQuDA_DisplayManip::SetEffectMode(fade);
	vTaskDelay(DEFAULT_ERROR_DISPLAY_TIME / portTICK_PERIOD_MS);
}

void HAQuDA_ErrorHandler::FailedToConnectToWiFi() {
	currentError = failedToConnectToWiFi;
	HAQuDA_Logger::LogError("Failed connection to WiFi");
	setErrorToDisplayManip(grow);
	vTaskDelay(DEFAULT_ERROR_DISPLAY_TIME / portTICK_PERIOD_MS);
}

void HAQuDA_ErrorHandler::FailedToCreateAP() {
	currentError = failedToCreateAP;
	HAQuDA_Logger::LogError("Failed creating Acces Point");
	setErrorToDisplayManip(snake);
	vTaskDelay(DEFAULT_ERROR_DISPLAY_TIME / portTICK_PERIOD_MS);
}

void HAQuDA_ErrorHandler::FailedToStartSensors() {
	currentError = failedToStartSensors;
	HAQuDA_Logger::LogError("Failed to start sensors");
	setErrorToDisplayManip(randomPixel);
	vTaskDelay(DEFAULT_ERROR_DISPLAY_TIME / portTICK_PERIOD_MS);
}

void HAQuDA_ErrorHandler::ClearCurrentError() {
	currentError = noneError;
	resetErrorOnDisplayManip();
}

HAQuDA_ErrorHandler::~HAQuDA_ErrorHandler() {
}
