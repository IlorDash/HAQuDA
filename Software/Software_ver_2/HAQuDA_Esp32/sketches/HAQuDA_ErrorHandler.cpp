#include "HAQuDA_ErrorHandler.h"
#include "HAQuDA_Logger.h"
#include "HAQuDA_LEDsParams.h"

snake_params HAQuDA_ErrorHandler::failedToCreateAPEffect;
growing_params HAQuDA_ErrorHandler::failedToConnectToWiFiEffect;
random_led_params HAQuDA_ErrorHandler::failedToStartSensorsEffect;
fading_params HAQuDA_ErrorHandler::failedToStartFSEffect;
fading_params HAQuDA_ErrorHandler::failedToUpdateNTPEffect;

error_type HAQuDA_ErrorHandler::currentError = noneError;

HAQuDA_ErrorHandler::HAQuDA_ErrorHandler() {
	failedToCreateAPEffect.color = COLOR_RED;
	failedToCreateAPEffect.move_pause_ms = 100;
	failedToCreateAPEffect.tail_len = 1;

	failedToConnectToWiFiEffect.color = COLOR_RED;
	failedToConnectToWiFiEffect.grow_pause_ms = 100;

	failedToStartSensorsEffect.pause_ms = 50;
	failedToStartSensorsEffect.end_pause_ms = 500;

	failedToStartFSEffect.color = COLOR_RED;
	failedToStartFSEffect.fade_pause_ms = 10;
	failedToStartFSEffect.start_bright = 100;
	failedToStartFSEffect.stop_bright = 0;
	failedToStartFSEffect.step = 5;

	failedToUpdateNTPEffect.color = COLOR_YELLOW;
	failedToUpdateNTPEffect.fade_pause_ms = 10;
	failedToUpdateNTPEffect.start_bright = 0;
	failedToUpdateNTPEffect.stop_bright = 100;
	failedToUpdateNTPEffect.step = 10;
}

void HAQuDA_ErrorHandler::Show() {
	switch (currentError) {
		case failedToCreateAP: {
			show_snake(failedToCreateAPEffect);
			break;
		}
		case failedToConnectToWiFi: {
			show_growing(failedToConnectToWiFiEffect);
			break;
		}
		case failedToStartFS: {
			show_random(failedToStartSensorsEffect);
			break;
		}
		case failedToStartSensors: {
			show_fading(failedToStartFSEffect);
			break;
		}
		case failedToUpdateNTP: {
			show_fading(failedToUpdateNTPEffect);
			break;
		}
		default:
			break;
	}
}

void HAQuDA_ErrorHandler::FailedToStartFS() {
	currentError = failedToStartFS;
	vTaskDelay(DEFAULT_ERROR_DISPLAY_TIME / portTICK_PERIOD_MS);
}

void HAQuDA_ErrorHandler::FailedToConnectToWiFi() {
	currentError = failedToConnectToWiFi;
	HAQuDA_Logger::LogError("Failed connection to WiFi");
	vTaskDelay(DEFAULT_ERROR_DISPLAY_TIME / portTICK_PERIOD_MS);
}

void HAQuDA_ErrorHandler::FailedToCreateAP() {
	currentError = failedToCreateAP;
	HAQuDA_Logger::LogError("Failed creating Acces Point");
	vTaskDelay(DEFAULT_ERROR_DISPLAY_TIME / portTICK_PERIOD_MS);
}

void HAQuDA_ErrorHandler::FailedToStartSensors() {
	currentError = failedToStartSensors;
	HAQuDA_Logger::LogError("Failed to start sensors");
	vTaskDelay(DEFAULT_ERROR_DISPLAY_TIME / portTICK_PERIOD_MS);
}

void HAQuDA_ErrorHandler::FailedToUpdateNTP() {
	currentError = failedToUpdateNTP;
	vTaskDelay(DEFAULT_ERROR_DISPLAY_TIME / portTICK_PERIOD_MS);
}

void HAQuDA_ErrorHandler::FailedToGetMeas(const char *msg) {
	HAQuDA_Logger::LogError(msg);
}

void HAQuDA_ErrorHandler::ClearError() {
	currentError = noneError;
	SetEffect(noneEffect);
}

HAQuDA_ErrorHandler::~HAQuDA_ErrorHandler() {
}
