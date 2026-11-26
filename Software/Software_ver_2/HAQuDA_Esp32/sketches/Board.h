#pragma once

#include "Arduino.h"
#include "TimeHelper.h"

#define APPLICATION_NAME "HAQuDA"

// CCS811 WAKE input, active low: the sensor sleeps and ignores I2C until it is pulled low.
// Board Ver 2.1 routes it to IO27 (Ver 1.1 used IO5).
#define CCS811_WAK 27

// TTP223 touch ICs on HAQuDA_upper_PCB: push-pull output, active high (AHLB and TOG jumpers open).
// They run from 3.3 V: the Ver 2 board fed them 5 V, which exceeds the ESP32 input limit.
// The pins use pull-downs, because an unplugged LED board leaves the ESP32 end of the line open.
// The numbers are the pad names on the board.
#define TOUCH_2_PIN 25
#define TOUCH_6_PIN 4
#define TOUCH_8_PIN 32
#define TOUCH_9_PIN 33

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