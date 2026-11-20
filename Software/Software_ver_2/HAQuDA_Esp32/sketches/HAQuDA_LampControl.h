#pragma once

#include <atomic>

#include "freertos/FreeRTOS.h"

#include "HAQuDA_IFileStorage.h"
#include "HAQuDA_ILampDisplay.h"
#include "HAQuDA_ILogger.h"
#include "HAQuDA_LampSettings.h"

class HAQuDA_LampControl {
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
	// effect that restores its own brightness (the access point fading) has returned. Loop task only.
	void Apply();

	// Saves a change accepted by Submit() and only then shows it, so the lamp never shows settings that a reset would lose.
	// minuteOfDay is valid only when timeKnown. Loop task only.
	void Handle(bool timeKnown, uint16_t minuteOfDay);

	// All or nothing. async_tcp task only: its single writer, so the read-merge-store needs no lock; it touches no flash, display or logger.
	lamp_update_result Submit(const lamp_request &request);

	// Returns 0 when it does not fit; LAMP_JSON_BUF_LEN bytes are enough.
	size_t BuildJson(char *buf, size_t size) const;

  private:
	lamp_settings copySettings() const;
	static bool needsSensors(const lamp_settings &s);
	void showSensorsError();
	void show(const lamp_settings &s);
	void save(const lamp_settings &s);
	// Handle() shows the copy it saved, not a newer one
	void apply(const lamp_settings &s);

	HAQuDA_IFileStorage &files;
	HAQuDA_ILampDisplay &display;
	HAQuDA_ILogger &logger;
	// Mirrors the sketch's sensorsRunning, so every path that starts the sensors must call SensorsStarted(). Loop task only
	bool sensorsRunning = false;
	// Written by Submit() (async_tcp task) and by Load() before the web server runs, copied under the lock by every reader
	mutable portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
	lamp_settings settings;
	std::atomic<bool> applyPending{false};
	// Stored by Handle() (loop task), read by the async_tcp task
	static constexpr uint16_t MINUTE_UNKNOWN = 0xFFFF;
	std::atomic<uint16_t> minuteNow{MINUTE_UNKNOWN};
};
