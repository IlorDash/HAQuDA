#include "HAQuDA_LampControl.h"

#include "HAQuDA_LEDsParams.h"

namespace {
	constexpr char FILE_NAME_LAMP[] = "/lamp";

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
} // namespace

HAQuDA_LampControl::HAQuDA_LampControl(HAQuDA_IFileStorage &fileStorage, HAQuDA_ILampDisplay &lampDisplay, HAQuDA_ILogger &lampLogger)
	: files(fileStorage), display(lampDisplay), logger(lampLogger), settings(HAQuDA_LampDefaults()) {
}

lamp_settings HAQuDA_LampControl::copySettings() const {
	CriticalLock lock(mux);
	return settings;
}

void HAQuDA_LampControl::Load() {
	lamp_settings loaded = HAQuDA_LampDefaults();
	if (files.Exists(FILE_NAME_LAMP)) {
		uint8_t record[LAMP_RECORD_LEN];
		if ((files.FileSize(FILE_NAME_LAMP) == static_cast<int>(LAMP_RECORD_LEN)) && files.ReadFile(FILE_NAME_LAMP, record, sizeof(record))
			&& HAQuDA_DecodeLampRecord(record, sizeof(record), loaded)) {
			logger.LogInfo("Lamp settings loaded");
		} else {
			// A failed decode leaves `loaded` at the defaults
			logger.LogError("Lamp settings bad, defaults");
		}
	}
	CriticalLock lock(mux);
	settings = loaded;
}

bool HAQuDA_LampControl::needsSensors(const lamp_settings &s) {
	return s.on && (s.mode != lamp_mode::light);
}

void HAQuDA_LampControl::SensorsStarted() {
	sensorsRunning = true;
}

void HAQuDA_LampControl::showSensorsError() {
	if (display.ErrorShown()) {
		return;
	}
	display.ShowSensorsError();
}

void HAQuDA_LampControl::Apply() {
	apply(copySettings());
}

void HAQuDA_LampControl::apply(const lamp_settings &s) {
	if (!sensorsRunning && needsSensors(s)) {
		// No readings to draw. Nothing else is queued: SensorsStarted() is followed by an Apply()
		showSensorsError();
		return;
	}
	// In the loop task an error on the display is the sensors one (the other errors of setup() end in a stop or are cleared), and the
	// display refuses to leave it. The user chose a picture that needs no sensors, so the error ends; the page, the log and the next
	// measurement mode still report the failure
	if (display.ErrorShown()) {
		display.ClearError();
	}
	show(s);
}

void HAQuDA_LampControl::show(const lamp_settings &s) {
	display.SetBrightnessPercent(s.brightPercent);

	if (!s.on) {
		// Dark, not an error picture: the measurements, the web server and the log keep running
		display.ShowColor(static_cast<uint32_t>(COLOR_OFF));
		return;
	}
	switch (s.mode) {
		case lamp_mode::standard:
			display.ShowStandard(s.standardParam);
			break;
		case lamp_mode::multi: {
			lamp_param bottomToTop[LAMP_MULTI_NUM] = {};
			// The meas handler counts the bands from the bottom of the lamp, the page from the top
			for (size_t i = 0; i < LAMP_MULTI_NUM; i++) {
				bottomToTop[i] = s.multi[LAMP_MULTI_NUM - 1 - i];
			}
			display.ShowMulti(bottomToTop);
			break;
		}
		case lamp_mode::night:
			display.ShowNight(s.nightParam, s.nightFromMin, s.nightToMin);
			break;
		case lamp_mode::light:
			display.ShowColor(HAQuDA_LampColor(s.color));
			break;
	}
}

void HAQuDA_LampControl::Handle(bool timeKnown, uint16_t minuteOfDay) {
	minuteNow.store(timeKnown ? minuteOfDay : MINUTE_UNKNOWN);

	if (applyPending.exchange(false)) {
		// The flag is cleared before the copy: a Submit() after the copy sets it again and is saved and applied on the next pass
		const lamp_settings s = copySettings();
		save(s);
		apply(s);
	}
}

void HAQuDA_LampControl::save(const lamp_settings &s) {
	uint8_t record[LAMP_RECORD_LEN];
	if (!HAQuDA_EncodeLampRecord(s, record, sizeof(record)) || !files.WriteFile(FILE_NAME_LAMP, record, sizeof(record))) {
		logger.LogError("Lamp settings: save failed");
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
	// The flag follows the store, so Handle() that sees it reads the new settings
	applyPending.store(true);
	return lamp_update_result::ok;
}

size_t HAQuDA_LampControl::BuildJson(char *buf, size_t size) const {
	const uint16_t minuteOfDay = minuteNow.load();
	return HAQuDA_BuildLampJson(copySettings(), minuteOfDay != MINUTE_UNKNOWN, minuteOfDay, buf, size);
}
