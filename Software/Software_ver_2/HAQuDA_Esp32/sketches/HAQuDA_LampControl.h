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
// one calls Apply(), which brings back the user's picture. While the display shows an error nothing is applied: the settings wait
// and Apply() (called when the sensors start again) puts them on.
class HAQuDA_LampControl {
  public:
	explicit HAQuDA_LampControl(HAQuDA_DisplayManip &displayManip);

	// Reads /lamp; a missing, damaged or foreign file gives the defaults (a damaged one is logged). Needs the file storage started
	// and must run before the web server starts. setup() (loop task) only; logs.
	void Load();

	// Puts the user's picture on the display: brightness, then power off (dark), a measurement mode or the lamp light colour. While
	// an error is shown it does nothing now and Handle() applies once the error has ended. Puts the picture on a second time about
	// 1 s later (from Handle()): an effect that was running (the access point fading) restores its own brightness when it is
	// stopped, after this call. May block the loop task for ≈ 0.2 s (brightness change). loop task only; never logs.
	void Apply();

	// Applies a change accepted since the last call (not while the display shows an error), writes the settings to flash once they
	// have been quiet for a few seconds, and reads the time for the night mode check of Submit(). loop task only; logs a failed write.
	void Handle();

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
	// The work of Apply(), without the error check and the second pass
	void show();
	void save();

	HAQuDA_DisplayManip &display;
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
	// Minute of the day from the time service, MINUTE_UNKNOWN until NTP has synced; written by Handle(), read by the async_tcp task
	static constexpr uint16_t MINUTE_UNKNOWN = 0xFFFF;
	std::atomic<uint16_t> minuteNow{MINUTE_UNKNOWN};
};
