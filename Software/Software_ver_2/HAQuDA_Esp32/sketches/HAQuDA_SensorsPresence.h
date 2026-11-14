#pragma once

#include <cstdint>

// Budget for DHT22 and ZH03B to show up after the sensor power-on. The DHT22 needs ~1 s to settle and at least 2 s between
// reads, so its first read is at 2 s and the retry at 4 s; the ZH03B boots in ~1 s and then sends a frame every second.
// 5 s therefore costs ~2 s on a healthy start and at most 5 s per failed start attempt.
constexpr uint32_t SENSORS_PRESENCE_TIMEOUT_MS = 5000;
constexpr uint32_t SENSORS_DHT_FIRST_READ_MS = 2000;
constexpr uint32_t SENSORS_DHT_RETRY_MS = 2000;
constexpr uint32_t SENSORS_PRESENCE_STEP_MS = 100;

struct sensors_presence {
	bool dht22;
	bool zh03b;
};

// Polls the DHT22 and ZH03B probes until both answer or SENSORS_PRESENCE_TIMEOUT_MS passed since powerOnMs; never waits longer.
// dhtPresent() / pmPresent() return true once the sensor answered, nowMs() is millis(), delayMs(ms) yields for ms.
// The ZH03B probe runs every step, the DHT22 probe at 2 s, 4 s, ... after power-on; a probe is not run again once it succeeded.
// loop task only (the probes touch the sensors).
template <class DhtProbe, class PmProbe, class Clock, class Delay>
sensors_presence HAQuDA_WaitForSensors(DhtProbe dhtPresent, PmProbe pmPresent, Clock nowMs, Delay delayMs, uint32_t powerOnMs) {
	sensors_presence presence = {false, false};
	uint32_t nextDhtReadMs = SENSORS_DHT_FIRST_READ_MS;

	while (true) {
		// Unsigned difference survives the millis() wrap
		const uint32_t elapsedMs = (uint32_t)(nowMs() - powerOnMs);

		if (!presence.zh03b) {
			presence.zh03b = pmPresent();
		}
		if (!presence.dht22 && (elapsedMs >= nextDhtReadMs)) {
			presence.dht22 = dhtPresent();
			nextDhtReadMs += SENSORS_DHT_RETRY_MS;
		}

		if ((presence.dht22 && presence.zh03b) || (elapsedMs >= SENSORS_PRESENCE_TIMEOUT_MS)) {
			return presence;
		}
		delayMs(SENSORS_PRESENCE_STEP_MS);
	}
}
