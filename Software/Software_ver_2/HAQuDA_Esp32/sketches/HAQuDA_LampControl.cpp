#include "HAQuDA_LampControl.h"

#include "HAQuDA_FileStorage.h"
#include "HAQuDA_LEDsParams.h"
#include "HAQuDA_Logger.h"
#include "HAQuDA_TimeHelper.h"

// The lamp starts at the brightness LEDs::Begin() sets (raw DEFAULT_BRIGHTNESS). The percent scale has steps of 5, so the nearest
// value gives raw 52 instead of 50
static_assert((LAMP_DEFAULT_BRIGHT * MAX_BRIGHTNESS / 100) - DEFAULT_BRIGHTNESS <= 2, "the lamp's default brightness must match the boot brightness");
static_assert(DEFAULT_BRIGHTNESS - (LAMP_DEFAULT_BRIGHT * MAX_BRIGHTNESS / 100) <= 2, "the lamp's default brightness must match the boot brightness");

namespace {
	constexpr char FILE_NAME_LAMP[] = "/lamp";
	// A slider or a row of taps writes once, after the last change: flash wear and the LED frames a write skips
	constexpr uint32_t SAVE_DELAY_MS = 3000;
	// A fading effect stops within one step (≈ 0.2 s of LED show plus its pause), so a second pass after 1 s sees the final brightness
	constexpr uint32_t REAPPLY_DELAY_MS = 1000;

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

HAQuDA_LampControl::HAQuDA_LampControl(HAQuDA_DisplayManip &displayManip) : display(displayManip), settings(HAQuDA_LampDefaults()) {
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

void HAQuDA_LampControl::Apply() {
	if (errorShown()) {
		applyPending.store(true);
		return;
	}
	show();
	reapplyDue = true;
	reapplyMs = millis();
}

void HAQuDA_LampControl::show() {
	const lamp_settings s = copySettings();
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

void HAQuDA_LampControl::Handle() {
	uint16_t minuteOfDay = 0;
	minuteNow.store(HAQuDA_TimeService::GetCachedMinuteOfDay(&minuteOfDay) ? minuteOfDay : MINUTE_UNKNOWN);

	// While an error is shown the display refuses to change; the settings stay pending until the error ends
	if (!errorShown() && applyPending.exchange(false)) {
		Apply();
	}
	// The second pass: the effect Apply() stopped (the access point fading) has restored its own brightness by now, and a
	// meas picture drawn at that brightness is drawn again
	if (reapplyDue && ((uint32_t)(millis() - reapplyMs) >= REAPPLY_DELAY_MS) && !errorShown()) {
		reapplyDue = false;
		show();
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
