#include "HAQuDA_ErrorHandler.h"
#include "HAQuDA_DisplayService.h"
#include "HAQuDA_Logger.h"
#include "HAQuDA_TimeHelper.h"
#include "HAQuDA_LEDsParams.h"

namespace {
	void postError(HAQuDA_DisplayService *display, display_error kind) {
		if (display != nullptr) {
			display->Post(kind);
		}
	}
} // namespace

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
			show_fading(failedToStartFSEffect);
			break;
		}
		case failedToStartSensors: {
			show_random(failedToStartSensorsEffect);
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

void HAQuDA_ErrorHandler::SetDisplay(HAQuDA_DisplayService *displayService) {
	display = displayService;
}

void HAQuDA_ErrorHandler::FailedToStartFS() {
	currentError = failedToStartFS;
	postError(display, display_error::fsFailed);
}

void HAQuDA_ErrorHandler::FailedToConnectToWiFi() {
	currentError = failedToConnectToWiFi;
	postError(display, display_error::wifiConnectFailed);
	HAQuDA_Logger::LogError("Failed connection to WiFi");
}

void HAQuDA_ErrorHandler::FailedToCreateAP() {
	currentError = failedToCreateAP;
	postError(display, display_error::apFailed);
	HAQuDA_Logger::LogError("Failed creating Acces Point");
}

void HAQuDA_ErrorHandler::FailedToStartSensors() {
	currentError = failedToStartSensors;
	postError(display, display_error::sensorsFailed);
	HAQuDA_Logger::LogError("Failed to start sensors");
}

// Not posted and not logged: while NTP has never synced every log line lands here. A recorded error never replaces another one,
// so the web page keeps the picture the lamp shows.
void HAQuDA_ErrorHandler::FailedToUpdateNTP() {
	ntpSyncFailed.store(true);
	if (currentError == noneError) {
		currentError = failedToUpdateNTP;
	}
}

void HAQuDA_ErrorHandler::UpdateNtpError(bool timeKnown) {
	if (timeKnown) {
		// After the first sync a lost server keeps the last time, which is not an error: the flag is cleared and nothing sets it again
		ntpSyncFailed.store(false);
		// A dropped end (full queue) is retried on the next pass, or the yellow would stay until a reboot
		if (ntpErrorPosted && (display != nullptr) && display->End(display_end::ntpSynced)) {
			ntpErrorPosted = false;
		}
		return;
	}
	if (ntpErrorPosted) {
		DateTimeStruct retryTime = {};
		HAQuDA_TimeService::GetDateTime(&retryTime);
		return;
	}
	if (!ntpSyncFailed.load() || (display == nullptr)) {
		return;
	}
	// A dropped post (full queue) is retried on the next pass
	ntpErrorPosted = display->Post(display_error::ntpFailed);
	if (ntpErrorPosted && !ntpErrorLogged) {
		ntpErrorLogged = true;
		HAQuDA_Logger::LogError("NTP not synced");
	}
}

void HAQuDA_ErrorHandler::FailedToGetMeas(const char *msg) {
	HAQuDA_Logger::LogError(msg);
}

// Also called in the AP branch after the WiFi error: the end is harmless when no sensors error plays
void HAQuDA_ErrorHandler::ClearError() {
	currentError = noneError;
	SetEffect(noneEffect);
	if (display != nullptr) {
		display->End(display_end::sensorsRecovered);
	}
}

HAQuDA_ErrorHandler::~HAQuDA_ErrorHandler() {
}
