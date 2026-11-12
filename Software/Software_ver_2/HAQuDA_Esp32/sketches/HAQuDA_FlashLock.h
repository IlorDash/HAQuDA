#pragma once

#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// One mutex that keeps a flash operation and an LED frame transmission from overlapping.
// Every SPIFFS access (write, erase, garbage collection and also plain reads) stops the flash cache on BOTH cores for
// its duration, so the RMT interrupt that refills the NeoPixel pulse buffer cannot run. The RMT then replays stale items and
// part of the LED chain shows random colours until the next frame. HAQuDA_FileStorage and HAQuDA_LEDs::Show() hold this
// mutex around the flash access and around pixels->show() respectively, so they never run at the same time.
//
// The handle is a process-wide static because the LED object is a singleton and HAQuDA_FileStorage has static methods:
// neither has an instance the handle could be passed to. It is written once by Create() before any other task exists.
class HAQuDA_FlashLock {
  public:
	// Creates the mutex. Call once from setup() before HAQuDA_LEDs::Begin() and HAQuDA_FileStorage::Start(). loop task.
	// If creation fails the lock stays absent and all users run without it (the LEDs still work).
	static void Create();

	// Takes the mutex, waits at most timeoutMs. Returns true when the mutex is held or does not exist.
	// Any task that uses the LEDs or the file storage (ShowTask, loop, async_tcp).
	static bool Take(uint32_t timeoutMs);

	// Gives back a mutex taken by Take(); call only after Take() returned true with the mutex present.
	// Same task that called Take() (ShowTask, loop, async_tcp).
	static void Give();

	// True when Create() succeeded. Any task.
	static bool Exists();
};

// Scope guard: Take() in the constructor, Give() in the destructor, so every return path releases the mutex.
// Not recursive: never create a second guard while one is alive in the same task.
class HAQuDA_FlashLockGuard {
  public:
	explicit HAQuDA_FlashLockGuard(uint32_t timeoutMs) : acquired(HAQuDA_FlashLock::Take(timeoutMs)), held(acquired && HAQuDA_FlashLock::Exists()) {
	}
	~HAQuDA_FlashLockGuard() {
		if (held) {
			HAQuDA_FlashLock::Give();
		}
	}
	HAQuDA_FlashLockGuard(const HAQuDA_FlashLockGuard &) = delete;
	HAQuDA_FlashLockGuard &operator=(const HAQuDA_FlashLockGuard &) = delete;

	// False only when the mutex exists and could not be taken within the timeout.
	bool Acquired() const {
		return acquired;
	}

  private:
	const bool acquired;
	const bool held;
};
