#pragma once

#include <Adafruit_CCS811.h>
#include <DHT.h>
#include <SoftwareSerial.h>

#include "HAQuDA_PMResult.h"

typedef struct {
	float value;
	uint16_t measNum;
	bool newMeasDone;
} meas_data;

extern meas_data eCO2_meas;
extern meas_data TVOC_meas;
extern meas_data PM_1_0_meas; // define PM1.0 value of the air detector module
extern meas_data PM_2_5_meas; // define PM2.5 value of the air detector module
extern meas_data PM_10_meas;  // define PM10 value of the air detector module
extern meas_data temp_meas;	  // define PM10 value of the air detector module
extern meas_data humid_meas;  // define PM10 value of the air detector module

// Starts the ZH03B UART once, before the first sensorsBegin(). loop task (setup) only.
void pmSerialBegin();
bool sensorsBegin();
bool getCCS811_meas();
// Adds one PM sample to the sums; takes none during the warm-up after the latest sensor power-on. loop task only.
pm_meas_result getPM_meas();
bool getDHT22_meas();
void ClearMeas();
void sensors_reboot();
