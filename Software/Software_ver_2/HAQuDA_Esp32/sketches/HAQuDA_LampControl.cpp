#include "HAQuDA_LampControl.h"

#include "HAQuDA_FileStorage.h"
#include "HAQuDA_LEDsParams.h"
#include "HAQuDA_Logger.h"

namespace {
	constexpr char FILE_NAME_LAMP[] = "/lamp";
	// A slider or a row of taps writes once, after the last change: flash wear and the LED frames a write skips
	constexpr uint32_t SAVE_DELAY_MS = 3000;

	// Holds the critical section for its lifetime
	class CriticalLock {
	  public:
		explicit CriticalLock(portMUX_TYPE &muxToLock) : mux(muxToLock) {
			portENTER_CRITICAL(&mux);
		}
		~CriticalLock() {
			portEXIT_CRITICAL(&mux);
		}
		CriticalLock(const CriticalLock &) = delete;
		CriticalLock &operator=(const CriticalLock &) = delete;

	  private:
		portMUX_TYPE &mux;
	};

	meas_type toMeasType(lamp_param p) {
		switch (p) {
			case lamp_param::temp:
				return temp;
			case lamp_param::humid:
				return humid;
			case lamp_param::eco2:
				return eCO2;
			case lamp_param::tvoc:
				return TVOC;
			case lamp_param::pm25:
				return PM2_5;
			case lamp_param::total:
			default:
				return total;
		}
	}
} // namespace

HAQuDA_LampControl::HAQuDA_LampControl(HAQuDA_DisplayManip &displayManip, HAQuDA_ErrorHandler &errorHandler)
	: display(displayManip), errHandler(errorHandler), settings(HAQuDA_LampDefaults()) {
}

lamp_settings HAQuDA_LampControl::copySettings() const {
	CriticalLock lock(mux);
	return settings;
}

void HAQuDA_LampControl::Load() {
	lamp_settings loaded = HAQuDA_LampDefaults();
	if (HAQuDA_FileStorage::Exists(FILE_NAME_LAMP)) {
		uint8_t record[LAMP_RECORD_LEN];
		if ((HAQuDA_FileStorage::FileSize(FILE_NAME_LAMP) == static_cast<int>(LAMP_RECORD_LEN))
			&& HAQuDA_FileStorage::ReadFile(FILE_NAME_LAMP, record, sizeof(record)) && HAQuDA_DecodeLampRecord(record, sizeof(record), loaded)) {
			HAQuDA_Logger::LogInfo("Lamp settings loaded");
		} else {
			// A failed decode leaves `loaded` at the defaults
			HAQuDA_Logger::LogError("Lamp settings bad, defaults");
		}
	}
	CriticalLock lock(mux);
	settings = loaded;
}

bool HAQuDA_LampControl::errorShown() {
	// The test HAQuDA_DisplayManip::SetMode() uses to refuse a change
	return (HAQuDA_DisplayManip::GetMode() == error) && (HAQuDA_ErrorHandler::GetCurrentError() != noneError);
}

bool HAQuDA_LampControl::needsSensors(const lamp_settings &s) {
	return s.on && (s.mode != lamp_mode::light);
}

void HAQuDA_LampControl::SensorsStarted() {
	sensorsRunning = true;
}

void HAQuDA_LampControl::showSensorsError() {
	if (errorShown()) {
		return;
	}
	// The same pair setup() uses; the error is the one an unstarted sensor set leaves on the matrix
	display.SetMode(error);
	errHandler.FailedToStartSensors();
}

void HAQuDA_LampControl::Apply() {
	// One copy for the decision and the picture: Submit() may change the settings in between
	const lamp_settings s = copySettings();
	if (!sensorsRunning && needsSensors(s)) {
		// No readings to draw. Nothing else is queued: SensorsStarted() is followed by an Apply()
		showSensorsError();
		return;
	}
	// In the loop task an error on the display is the sensors one (the other errors of setup() end in a stop or are cleared), and the
	// display refuses to leave it. The user chose a picture that needs no sensors, so the error ends; the page, the log and the next
	// measurement mode still report the failure
	if (errorShown()) {
		errHandler.ClearError();
	}
	show(s);
}

void HAQuDA_LampControl::show(const lamp_settings &s) {
	display.SetBrightness(s.bright);

	if (!s.on) {
		// Dark, not an error picture: the measurements, the web server and the log keep running
		display.SetEffectParam(static_cast<uint32_t>(COLOR_OFF));
		display.SetEffect(staticColor);
		return;
	}
	switch (s.mode) {
		case lamp_mode::standard:
			display.SetStandardMeasParam(toMeasType(s.standardParam));
			display.SetMeasMode(standard);
			break;
		case lamp_mode::multi: {
			multi_mode_params params = {};
			// The meas handler counts the bands from the bottom of the lamp, the page from the top
			for (size_t i = 0; i < MULTI_MODE_PARAM_NUM; i++) {
				params.paramsArr[i] = toMeasType(s.multi[MULTI_MODE_PARAM_NUM - 1 - i]);
			}
			display.SetMultiMeasParam(params);
			display.SetMeasMode(multi);
			break;
		}
		case lamp_mode::night: {
			const night_time window = {s.nightFrom, s.nightTo};
			display.SetStandardMeasParam(toMeasType(s.nightParam));
			display.SetNightTime(window);
			display.SetMeasMode(night);
			break;
		}
		case lamp_mode::light:
			display.SetEffectParam(HAQuDA_LampColor(s.color));
			display.SetEffect(staticColor);
			break;
	}
}

void HAQuDA_LampControl::Handle(bool timeKnown, uint16_t minuteOfDay) {
	minuteNow.store(timeKnown ? minuteOfDay : MINUTE_UNKNOWN);

	if (applyPending.exchange(false)) {
		Apply();
	}
	if (saveDirty.load() && ((uint32_t)(millis() - changedMs.load()) >= SAVE_DELAY_MS)) {
		saveDirty.store(false);
		save();
	}
}

void HAQuDA_LampControl::save() {
	uint8_t record[LAMP_RECORD_LEN];
	if (!HAQuDA_EncodeLampRecord(copySettings(), record, sizeof(record)) || !HAQuDA_FileStorage::WriteFile(FILE_NAME_LAMP, record, sizeof(record))) {
		HAQuDA_Logger::LogError("Lamp settings: save failed");
	}
}

lamp_update_result HAQuDA_LampControl::Submit(const lamp_request &request) {
	lamp_settings next = copySettings();
	const uint16_t minuteOfDay = minuteNow.load();
	const lamp_update_result result = HAQuDA_LampUpdate(next, request, minuteOfDay != MINUTE_UNKNOWN);
	if (result != lamp_update_result::ok) {
		return result;
	}
	{
		CriticalLock lock(mux);
		settings = next;
	}
	// The flags follow the store, so Handle() that sees them reads the new settings
	changedMs.store(millis());
	saveDirty.store(true);
	applyPending.store(true);
	return lamp_update_result::ok;
}

size_t HAQuDA_LampControl::BuildJson(char *buf, size_t size) const {
	const uint16_t minuteOfDay = minuteNow.load();
	return HAQuDA_BuildLampJson(copySettings(), minuteOfDay != MINUTE_UNKNOWN, minuteOfDay, buf, size);
}
