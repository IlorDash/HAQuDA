#include "HAQuDA_ButtonsControl.h"

#include "HAQuDA_CriticalLock.h"

namespace {
	constexpr char FILE_NAME_BUTTONS[] = "/buttons";
} // namespace

HAQuDA_ButtonsControl::HAQuDA_ButtonsControl(HAQuDA_IFileStorage &fileStorage, HAQuDA_ILogger &buttonsLogger)
	: files(fileStorage), logger(buttonsLogger), settings(HAQuDA_ButtonsDefaults()), saved(settings) {
}

buttons_settings HAQuDA_ButtonsControl::CopySettings() const {
	HAQuDA_CriticalLock lock(mux);
	return settings;
}

void HAQuDA_ButtonsControl::Load() {
	buttons_settings loaded = HAQuDA_ButtonsDefaults();
	if (files.Exists(FILE_NAME_BUTTONS)) {
		uint8_t record[BUTTONS_RECORD_LEN];
		if ((files.FileSize(FILE_NAME_BUTTONS) == static_cast<int>(BUTTONS_RECORD_LEN)) && files.ReadFile(FILE_NAME_BUTTONS, record, sizeof(record))
			&& HAQuDA_DecodeButtonsRecord(record, sizeof(record), loaded)) {
			logger.LogInfo("Buttons settings loaded");
		} else {
			// A failed decode leaves `loaded` at the defaults
			logger.LogError("Buttons settings bad, defaults");
		}
	}
	// After an unreadable existing file `saved` is the defaults, so re-selecting the defaults writes nothing and the old file content
	// comes back after a reboot: accepted
	HAQuDA_CriticalLock lock(mux);
	settings = loaded;
	saved = loaded;
}

void HAQuDA_ButtonsControl::Handle() {
	// The flag is cleared before the copy: a change after the copy sets it again and is saved on the next pass
	if (!savePending.exchange(false)) {
		return;
	}
	buttons_settings s = HAQuDA_ButtonsDefaults();
	buttons_settings stored = s;
	{
		HAQuDA_CriticalLock lock(mux);
		s = settings;
		stored = saved;
	}
	// The flag a reverted change left behind ends here
	if (HAQuDA_ButtonsSame(s, stored)) {
		return;
	}
	uint8_t record[BUTTONS_RECORD_LEN];
	if (HAQuDA_EncodeButtonsRecord(s, record, sizeof(record)) && files.WriteFile(FILE_NAME_BUTTONS, record, sizeof(record))) {
		HAQuDA_CriticalLock lock(mux);
		saved = s;
		return;
	}
	// A failed write is never retried: the settings go back to the stored ones, also the change that arrived during the write
	{
		HAQuDA_CriticalLock lock(mux);
		settings = saved;
	}
	saveErrors.fetch_add(1);
	logger.LogError("Buttons settings: save failed");
}

buttons_update_result HAQuDA_ButtonsControl::Submit(const buttons_request &request) {
	buttons_update_result result = buttons_update_result::nothing;
	{
		HAQuDA_CriticalLock lock(mux);
		buttons_settings next = settings;
		result = HAQuDA_ButtonsUpdate(next, request);
		if (result == buttons_update_result::ok) {
			settings = next;
		}
	}
	if (result == buttons_update_result::ok) {
		// The flag follows the store, so Handle() that sees it reads the new settings
		savePending.store(true);
	}
	return result;
}

bool HAQuDA_ButtonsControl::SetLocked(bool locked) {
	{
		HAQuDA_CriticalLock lock(mux);
		if (settings.locked == locked) {
			return locked;
		}
		settings.locked = locked;
	}
	savePending.store(true);
	return locked;
}

bool HAQuDA_ButtonsControl::ToggleLocked() {
	bool locked = false;
	{
		HAQuDA_CriticalLock lock(mux);
		locked = !settings.locked;
		settings.locked = locked;
	}
	savePending.store(true);
	return locked;
}

size_t HAQuDA_ButtonsControl::BuildJson(char *buf, size_t size) const {
	// The counter is read first: a grown counter then always comes with the reverted settings
	const uint32_t errors = saveErrors.load();
	return HAQuDA_BuildButtonsJson(CopySettings(), errors, buf, size);
}
