#pragma once

#include <atomic>

#include "freertos/FreeRTOS.h"

#include "HAQuDA_ButtonsSettings.h"
#include "HAQuDA_IFileStorage.h"
#include "HAQuDA_ILogger.h"

class HAQuDA_ButtonsControl {
  public:
	HAQuDA_ButtonsControl(HAQuDA_IFileStorage &fileStorage, HAQuDA_ILogger &buttonsLogger);

	// A missing, damaged or foreign file gives the defaults. Needs the file storage started; must run before the web server starts.
	// setup() only.
	void Load();

	// Writes the settings a Submit() or SetLocked() changed. A failed write is not retried: the settings return to the last stored ones and
	// the error counter in the JSON grows. Loop task only.
	void Handle();

	// All or nothing. Two writers (this and SetLocked() in the loop task), so the read-merge-store runs under the lock; it touches no
	// flash and no logger. async_tcp task only.
	buttons_update_result Submit(const buttons_request &request);

	// Return the new state; a change is saved by Handle(). Loop task only.
	bool SetLocked(bool locked);
	bool ToggleLocked();

	// Any task: the bindings, the lock and (through HAQuDA_ButtonRepeatable) the repeating buttons are read from the copy.
	buttons_settings CopySettings() const;

	// Returns 0 when it does not fit; BUTTONS_JSON_BUF_LEN bytes are enough. Any task.
	size_t BuildJson(char *buf, size_t size) const;

  private:
	HAQuDA_IFileStorage &files;
	HAQuDA_ILogger &logger;
	// Written by Submit() (async_tcp task), SetLocked() (loop task) and Load() before the web server runs
	mutable portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
	buttons_settings settings;
	// The last settings written to flash (or loaded); written by Handle() and Load(), under the lock
	buttons_settings saved;
	std::atomic<bool> savePending{false};
	// Single writer: Handle() in the loop task
	std::atomic<uint32_t> saveErrors{0};
};
