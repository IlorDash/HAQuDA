#pragma once

#define WIFI_CREDS_NUM 3

#define DISP_MEAS_PERIOD 300000 //=5 min in ms
#define SENSORS_MEAS_PERIOD 2000

#define DISP_PARAMS_NUM 5

typedef enum { snake, randomPixel, fade, christmasTree, noneEffect } dispEffects;

typedef struct {
	uint16_t firstDot;
	uint16_t secondDot;
	uint16_t thirdDot;
} paramsDivideDots;

extern paramsDivideDots temp_divideDots;
extern paramsDivideDots humid_divideDots;
extern paramsDivideDots eCO2_divideDots;
extern paramsDivideDots TVOC_divideDots;
extern paramsDivideDots PM2_5_divideDots;