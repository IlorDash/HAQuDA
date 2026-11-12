#pragma once

// Host stand-in for the FreeRTOS base types used by the sketches. Ticks are 1 ms on the host.
#include <stdint.h>

typedef int32_t BaseType_t;
typedef uint32_t TickType_t;

#define pdFALSE ((BaseType_t)0)
#define pdTRUE ((BaseType_t)1)
#define pdMS_TO_TICKS(ms) ((TickType_t)(ms))
