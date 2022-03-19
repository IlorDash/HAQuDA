#pragma once

#define APPLICATION_NAME "HAQuDA"

#ifdef USE_WDT
#include <esp_task_wdt.h>
#include <WiFi.h>
#endif

inline void ResetWdt(bool immediate = false) {
#ifdef USE_WDT
	static unsigned long wdt_counter = 0;
	unsigned long ms = millis();
	if (wdt_counter < ms || wdt_counter > ms + 500 || immediate) {
		esp_task_wdt_reset(); // esp_task_wdt_feed();
		wdt_counter = ms + 500;
		// log_i("wdt_counter+++++");
	}
#endif
}

inline void DisableWdt() {
#ifdef USE_WDT
	esp_task_wdt_delete(NULL);
#endif
}
inline void EnableWdt() {
#ifdef USE_WDT
	esp_task_wdt_add(NULL);
	esp_task_wdt_reset();
#endif
}