#include "HAQuDA_LampControl.h"

#include "HAQuDA_CriticalLock.h"
#include "HAQuDA_LEDsParams.h"

namespace {
	constexpr char FILE_NAME_LAMP[] = "/lamp";
} // namespace

HAQuDA_LampControl::HAQuDA_LampControl(HAQuDA_IFileStorage &fileStorage, HAQuDA_ILampDisplay &lampDisplay, HAQuDA_ILogger &lampLogger)
	: files(fileStorage), display(lampDisplay), logger(lampLogger), settings(HAQuDA_LampDefaults()), saved(settings) {
}

lamp_settings HAQuDA_LampControl::copySettings() const {
	HAQuDA_CriticalLock lock(mux);
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
	// After an unreadable existing file `saved` is the defaults, so re-selecting the defaults writes nothing and the old file content
	// comes back after a reboot: accepted
	HAQuDA_CriticalLock lock(mux);
	settings = loaded;
	saved = loaded;
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
	// Every picture of the settings ends a reading or a flash, including the one this call is about to draw
	tempPicture = temp_picture::none;
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

void HAQuDA_LampControl::Handle(uint32_t nowMs, bool timeKnown, uint16_t minuteOfDay) {
	minuteNow.store(timeKnown ? minuteOfDay : MINUTE_UNKNOWN);

	// The flags are cleared before the copy: a change after the copy sets its flag again and is saved and applied on the next pass
	const bool applyNow = applyPending.exchange(false);
	const bool saveNow = savePending.exchange(false);
	if (applyNow || saveNow) {
		lamp_settings shown{};
		const bool stored = save(shown);
		// A pass that only saves leaves the picture alone, unless the settings went back to the stored ones
		if (applyNow || !stored) {
			apply(shown);
		}
	}

	if (tempPicture == temp_picture::none) {
		return;
	}
	// The time of the action is not known here: the picture lasts from the first pass after it was drawn
	if (!tempTimed) {
		tempTimed = true;
		tempStartMs = nowMs;
		return;
	}
	const uint32_t durationMs = (tempPicture == temp_picture::reading) ? LAMP_SHOW_READING_MS : LAMP_FLASH_MS;
	if (static_cast<uint32_t>(nowMs - tempStartMs) >= durationMs) {
		apply(copySettings());
	}
}

bool HAQuDA_LampControl::save(lamp_settings &shown) {
	lamp_settings current{};
	lamp_settings stored{};
	{
		HAQuDA_CriticalLock lock(mux);
		current = settings;
		stored = saved;
	}
	shown = current;
	// The flag a reverted change left behind ends here
	if (HAQuDA_LampSame(current, stored)) {
		return true;
	}
	uint8_t record[LAMP_RECORD_LEN];
	if (HAQuDA_EncodeLampRecord(current, record, sizeof(record)) && files.WriteFile(FILE_NAME_LAMP, record, sizeof(record))) {
		HAQuDA_CriticalLock lock(mux);
		saved = current;
		return true;
	}
	// A failed write is never retried: the settings go back to the stored ones, also the change that arrived during the write
	{
		HAQuDA_CriticalLock lock(mux);
		settings = saved;
		shown = saved;
	}
	// After the revert: a reader that sees the grown counter then reads the reverted settings
	saveErrors.fetch_add(1);
	logger.LogError("Lamp settings: save failed");
	return false;
}

lamp_update_result HAQuDA_LampControl::Submit(const lamp_request &request) {
	const uint16_t minuteOfDay = minuteNow.load();
	lamp_update_result result = lamp_update_result::nothing;
	{
		HAQuDA_CriticalLock lock(mux);
		lamp_settings next = settings;
		result = HAQuDA_LampUpdate(next, request, minuteOfDay != MINUTE_UNKNOWN);
		if (result == lamp_update_result::ok) {
			settings = next;
		}
	}
	if (result == lamp_update_result::ok) {
		// The flag follows the store, so Handle() that sees it reads the new settings
		applyPending.store(true);
	}
	return result;
}

size_t HAQuDA_LampControl::BuildJson(char *buf, size_t size) const {
	// The counter is read first: a grown counter then always comes with the reverted settings
	const uint32_t errors = saveErrors.load();
	const uint16_t minuteOfDay = minuteNow.load();
	return HAQuDA_BuildLampJson(copySettings(), minuteOfDay != MINUTE_UNKNOWN, minuteOfDay, errors, buf, size);
}

void HAQuDA_LampControl::TogglePower() {
	{
		HAQuDA_CriticalLock lock(mux);
		settings.on = !settings.on;
	}
	applyPending.store(true);
}

bool HAQuDA_LampControl::StepBrightness(bool up) {
	bool wasOn = false;
	bool changed = false;
	lamp_settings s{};
	{
		HAQuDA_CriticalLock lock(mux);
		wasOn = settings.on;
		changed = HAQuDA_StepLampBrightness(settings.brightPercent, up);
		if (!wasOn) {
			settings.on = true;
			changed = true;
		}
		s = settings;
	}
	if (!changed) {
		return false;
	}
	if (wasOn && (tempPicture == temp_picture::none)) {
		// Only the brightness: redrawing the picture on every step of a held button would restart it
		display.SetBrightnessPercent(s.brightPercent);
	} else {
		apply(s);
	}
	return true;
}

void HAQuDA_LampControl::CommitBrightness() {
	savePending.store(true);
}

void HAQuDA_LampControl::NextMode() {
	const bool timeKnown = minuteNow.load() != MINUTE_UNKNOWN;
	{
		HAQuDA_CriticalLock lock(mux);
		settings.on = true;
		settings.mode = HAQuDA_NextLampMode(settings.mode, timeKnown);
	}
	applyPending.store(true);
}

bool HAQuDA_LampControl::SetMode(lamp_mode mode) {
	if ((static_cast<size_t>(mode) >= LAMP_MODE_NUM) || ((mode == lamp_mode::night) && (minuteNow.load() == MINUTE_UNKNOWN))) {
		return false;
	}
	{
		HAQuDA_CriticalLock lock(mux);
		settings.on = true;
		settings.mode = mode;
	}
	applyPending.store(true);
	return true;
}

bool HAQuDA_LampControl::ShowReading(lamp_param param) {
	// The display refuses to leave an error, and the readings of a lamp without sensors do not exist
	if ((static_cast<size_t>(param) >= LAMP_PARAM_NUM) || !sensorsRunning || display.ErrorShown()) {
		return false;
	}
	display.SetBrightnessPercent(copySettings().brightPercent);
	display.ShowStandard(param);
	tempPicture = temp_picture::reading;
	tempTimed = false;
	return true;
}

void HAQuDA_LampControl::Flash(lamp_flash kind) {
	if ((static_cast<size_t>(kind) > static_cast<size_t>(lamp_flash::refused)) || display.ErrorShown()) {
		return;
	}
	display.SetBrightnessPercent(copySettings().brightPercent);
	display.ShowFlash(kind);
	tempPicture = temp_picture::flash;
	tempTimed = false;
}
