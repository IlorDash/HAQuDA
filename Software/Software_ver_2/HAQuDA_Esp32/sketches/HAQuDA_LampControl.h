#pragma once

#include "Arduino.h"

#include <atomic>

#include "HAQuDA_DisplayManip.h"
#include "HAQuDA_LampSettings.h"

// The lamp as the user set it on the web page: power, brightness, mode and the parameters of the modes (HAQuDA_LampSettings.h).
//
// The async_tcp task validates a change and stores it (Submit()); loop() puts it on the display (Handle() -> Apply()) and writes it
// to /lamp a few seconds after the last change. The web task never touches the display, the flash or the logger. The accepted
// settings are what /api/status reports at once, the display follows within one loop() pass.
//
// Boot animations, events (WiFi joined or lost) and errors draw over the user's picture through HAQuDA_DisplayManip; whoever ends
// one calls Apply(), which brings back the user's picture.
//
// While the sensors are not running (they failed to start; the sketch retries every 5 minutes) the lamp light and power off are
// shown as chosen, because they need no readings, and the sensors error effect is dropped for them. A measurement mode cannot be
// drawn then, so it shows the sensors error (again) and the settings wait until SensorsStarted() and Apply().
class HAQuDA_LampControl {
  public:
	HAQuDA_LampControl(HAQuDA_DisplayManip &displayManip, HAQuDA_ErrorHandler &errorHandler);

	// Reads /lamp; a missing, damaged or foreign file gives the defaults (a damaged one is logged). Needs the file storage started
	// and must run before the web server starts. setup() (loop task) only; logs.
	void Load();

	// Tells the lamp that the sensors run, so measurement modes can be drawn. Call once, before the Apply() that follows the start.
	// loop task only.
	void SensorsStarted();

	// Puts the user's picture on the display: brightness, then power off (dark), a measurement mode or the lamp light colour. While
	// the sensors are not running only power off and the lamp light are put on (the sensors error effect ends); a measurement mode
	// shows the sensors error instead. Puts the picture on a second time about 1 s later (from Handle()): an effect that was
	// running (the access point fading) restores its own brightness when it is stopped, after this call. May block the loop task for
	// ≈ 0.2 s (brightness change). loop task only; logs only when it raises the sensors error.
	void Apply();

	// Applies a change accepted since the last call, writes the settings to flash once they have been quiet for a few seconds, and
	// stores the minute of the day for the night mode check of Submit(). minuteOfDay is valid only when timeKnown is true.
	// loop task only; logs a failed write.
	void Handle(bool timeKnown, uint16_t minuteOfDay);

	// Validates the fields of a web request and, if all are good, accepts them as the new settings (all or nothing). Returns the
	// result; ok means BuildJson() now reports the new settings. async_tcp task only: its single writer, so the read-merge-store needs
	// no lock; never blocks, no flash, no logging.
	lamp_update_result Submit(const lamp_request &request);

	// Writes the lamp block of /api/status (see HAQuDA_BuildLampJson()) and returns its length, 0 when it does not fit
	// (LAMP_JSON_BUF_LEN bytes are enough). Any task.
	size_t BuildJson(char *buf, size_t size) const;

  private:
	lamp_settings copySettings() const;
	// True while the display is in the error mode with an error set: it refuses every change then
	static bool errorShown();
	// False for the pictures that need no readings: power off and the lamp light
	static bool needsSensors(const lamp_settings &s);
	// Switches the display to the sensors error unless an error is already shown
	void showSensorsError();
	// The work of Apply(), without the sensors check and the second pass
	void show(const lamp_settings &s);
	void save();

	HAQuDA_DisplayManip &display;
	HAQuDA_ErrorHandler &errHandler;
	// Set by SensorsStarted(), never cleared; mirrors the sketch's sensorsRunning, so every path that starts the sensors must call
	// SensorsStarted(). loop task only
	bool sensorsRunning = false;
	// The settings the user chose: written by Submit() (async_tcp task) and by Load() before the web server runs, copied under
	// the lock by every reader
	mutable portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
	lamp_settings settings;
	// Set by Submit() after the store, taken by Handle()
	std::atomic<bool> applyPending{false};
	std::atomic<bool> saveDirty{false};
	// The second pass of Apply(): due since reapplyMs. loop task only
	bool reapplyDue = false;
	uint32_t reapplyMs = 0;
	std::atomic<uint32_t> changedMs{0};
	// Minute of the day from the time service, MINUTE_UNKNOWN until NTP has synced; stored by Handle() (loop task), read by the async_tcp task
	static constexpr uint16_t MINUTE_UNKNOWN = 0xFFFF;
	std::atomic<uint16_t> minuteNow{MINUTE_UNKNOWN};
};
