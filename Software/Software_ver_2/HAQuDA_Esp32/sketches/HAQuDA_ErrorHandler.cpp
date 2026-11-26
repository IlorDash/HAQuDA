#include "HAQuDA_ErrorHandler.h"
#include "HAQuDA_DisplayService.h"
#include "HAQuDA_Logger.h"
#include "HAQuDA_TimeHelper.h"

namespace {
	void postError(HAQuDA_DisplayService *display, display_error kind) {
		if (display != nullptr) {
			display->Post(kind);
		}
	}
} // namespace

void HAQuDA_ErrorHandler::SetDisplay(HAQuDA_DisplayService *displayService) {
	display = displayService;
}

void HAQuDA_ErrorHandler::FailedToStartFS() {
	postError(display, display_error::fsFailed);
}

void HAQuDA_ErrorHandler::FailedToConnectToWiFi() {
	postError(display, display_error::wifiConnectFailed);
	HAQuDA_Logger::LogError("Failed connection to WiFi");
}

void HAQuDA_ErrorHandler::FailedToCreateAP() {
	postError(display, display_error::apFailed);
	HAQuDA_Logger::LogError("Failed creating Acces Point");
}

void HAQuDA_ErrorHandler::FailedToStartSensors() {
	postError(display, display_error::sensorsFailed);
	HAQuDA_Logger::LogError("Failed to start sensors");
}

// Not posted and not logged: while NTP has never synced every log line lands here
void HAQuDA_ErrorHandler::FailedToUpdateNTP() {
	ntpSyncFailed.store(true);
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
	if (display != nullptr) {
		display->End(display_end::sensorsRecovered);
	}
}

HAQuDA_ErrorHandler::~HAQuDA_ErrorHandler() {
}
