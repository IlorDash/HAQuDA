#pragma once

#include "winsen_ze25o3.h"

#include <Adafruit_CCS811.h>

#include <Adafruit_SSD1306.h>

#include <DHT.h>

#define DHT_MEAS_PERIOD 2000

typedef struct {
	uint32_t value;
	uint16_t measNum;
} measStruct;

extern measStruct eCO2_meas;
extern measStruct TVOC_meas;
extern measStruct PM01_meas;  // define PM1.0 value of the air detector module
extern measStruct PM2_5_meas; // define PM2.5 value of the air detector module
extern measStruct PM10_meas;  // define PM10 value of the air detector module
extern measStruct temp_meas;  // define PM10 value of the air detector module
extern measStruct humid_meas; // define PM10 value of the air detector module

bool sensorsBegin();
bool getCCS811_meas();
bool getPM_meas();
bool getDHT11_meas();