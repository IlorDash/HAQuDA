#pragma once

#include <atomic>

#include "freertos/FreeRTOS.h"

#include "HAQuDA_IFileStorage.h"
#include "HAQuDA_ILampActions.h"
#include "HAQuDA_ILampDisplay.h"
#include "HAQuDA_ILogger.h"
#include "HAQuDA_LampSettings.h"

class HAQuDA_LampControl : public HAQuDA_ILampActions {
  public:
	HAQuDA_LampControl(HAQuDA_IFileStorage &fileStorage, HAQuDA_ILampDisplay &lampDisplay, HAQuDA_ILogger &lampLogger);

	// A missing, damaged or foreign file gives the defaults. Needs the file storage started; must run before the web server starts.
	// setup() only.
	void Load();

	// Call once, before the Apply() that follows the start. Loop task only.
	void SensorsStarted();

	// Boot animations, events and errors draw over the user's picture; whoever ends one calls Apply() to bring it back. While the
	// sensors are not running only power off and the lamp light are shown (they need no readings, and the sensors error ends); a
	// measurement mode shows the sensors error instead. The brightness reaches the LEDs at the top of the next ShowTask pass, after an
	// effect that restores its own brightness (the access point fading) has returned. Ends a shown reading or flash. Loop task only.
	void Apply();

	// Saves what Submit() and the actions changed and only then shows it, so the lamp never shows settings that a reset would lose
	// (except the brightness steps, which are drawn first). A failed write is not retried: the settings return to the last stored ones,
	// are shown again and the error counter in the JSON grows. nowMs is millis(): a reading or flash lasts from the first call after it
	// was drawn. minuteOfDay is valid only when timeKnown. Loop task only.
	void Handle(uint32_t nowMs, bool timeKnown, uint16_t minuteOfDay);

	// All or nothing. Two writers (this and the actions in the loop task), so the read-merge-store runs under the lock; it touches no
	// flash, display or logger. async_tcp task only.
	lamp_update_result Submit(const lamp_request &request);

	// Returns 0 when it does not fit; LAMP_JSON_BUF_LEN bytes are enough.
	size_t BuildJson(char *buf, size_t size) const;

	// A lamp that is off is turned on first by every action except TogglePower(), ShowReading() and Flash(): those two are temporary
	// pictures, after which the lamp is dark again. A change is saved and shown by Handle() on its pass, as after Submit().
	void TogglePower() override;
	bool StepBrightness(bool up) override;
	void CommitBrightness() override;
	void NextMode() override;
	bool SetMode(lamp_mode mode) override;
	bool ShowReading(lamp_param param) override;
	void Flash(lamp_flash kind) override;

  private:
	enum class temp_picture : uint8_t { none, reading, flash };

	lamp_settings copySettings() const;
	static bool needsSensors(const lamp_settings &s);
	void showSensorsError();
	void show(const lamp_settings &s);
	// False after a failed write; `shown` is then the stored state the settings went back to
	bool save(lamp_settings &shown);
	// Handle() shows the copy it saved, not a newer one
	void apply(const lamp_settings &s);

	HAQuDA_IFileStorage &files;
	HAQuDA_ILampDisplay &display;
	HAQuDA_ILogger &logger;
	// Mirrors the sketch's sensorsRunning, so every path that starts the sensors must call SensorsStarted(). Loop task only
	bool sensorsRunning = false;
	// Written by Submit() (async_tcp task), by the actions and Handle() (loop task) and by Load() before the web server runs, copied
	// under the lock by every reader
	mutable portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
	lamp_settings settings;
	// The last settings written to flash (or loaded); written by Handle() and Load(), under the lock
	lamp_settings saved;
	// Submit() and the actions that draw nothing themselves; Handle() saves, then draws
	std::atomic<bool> applyPending{false};
	// CommitBrightness(): the brightness was drawn already, only the save is left
	std::atomic<bool> savePending{false};
	// Single writer: Handle() in the loop task
	std::atomic<uint32_t> saveErrors{0};
	// Stored by Handle() (loop task), read by the async_tcp task
	static constexpr uint16_t MINUTE_UNKNOWN = 0xFFFF;
	std::atomic<uint16_t> minuteNow{MINUTE_UNKNOWN};
	// The reading or flash on the display and when it ends. Loop task only
	temp_picture tempPicture = temp_picture::none;
	bool tempTimed = false;
	uint32_t tempStartMs = 0;
};
