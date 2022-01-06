#pragma once

#include "winsen_ze25o3.h"
#include <Adafruit_CCS811.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

typedef struct {
	float value;
	uint16_t measNum;
	bool newMeasDone;
} measStruct;

extern measStruct eCO2_meas;
extern measStruct TVOC_meas;
extern measStruct PM_1_0_meas;  // define PM1.0 value of the air detector module
extern measStruct PM_2_5_meas; // define PM2.5 value of the air detector module
extern measStruct PM_10_meas;  // define PM10 value of the air detector module
extern measStruct temp_meas;  // define PM10 value of the air detector module
extern measStruct humid_meas; // define PM10 value of the air detector module
extern measStruct O3_meas; // define PM10 value of the air detector module

bool sensorsBegin();
bool getCCS811_meas();
bool getPM_meas();
bool getDHT11_meas();
bool getO3_meas();