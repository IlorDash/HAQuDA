#pragma once

#include "Arduino.h"
#include "TimeHelper.h"

#define TACHO_PULSES_PER_ROTATE (18 * 2)
#define STEPS_PER_ROTATE (180)

#define LED_BUILTIN 2

#define PIN_PWM_CH1 4
#define PIN_PWM_CH2 5

#define PIN_OPTO1 17
#define PIN_OPTO2 16

#define PIN_MAGNET1 18
#define PIN_MAGNET2 19
#define PIN_MAGNET3 23
#define PIN_MAGNET4 26

#define PIN_FG1 21
#define PIN_FG2 22

#define PIN_CTRL1 25
#define PIN_CTRL2 27

#define PIN_TEST1 33
#define PIN_TEST2 32

#define PWM_FREQ 25000
#define PWM_CHANNEL0 0
#define PWM_CHANNEL1 1
#define PWM_RESOLUTION 10

#define TIMER_LED 0
#define TIMER_MOTOR_CONTROL 1
#define TIMER_MAGNET1 2
#define TIMER_MAGNET2 3

#define FREQ_CALC_BASE 3000000

#define TEST1_TOGGLE()                                                                                                                                         \
	{ digitalWrite(PIN_TEST1, digitalRead(PIN_TEST1) == 0); }
#define TEST1_ON()                                                                                                                                             \
	{ digitalWrite(PIN_TEST1, true); }
#define TEST1_OFF()                                                                                                                                             \
	{ digitalWrite(PIN_TEST1, false); }

#define TEST2_TOGGLE()                                                                                                                                         \
	{ digitalWrite(PIN_TEST2, digitalRead(PIN_TEST2) == 0); }
#define TEST2_ON()                                                                                                                                             \
	{ digitalWrite(PIN_TEST2, true); }
#define TEST2_OFF()                                                                                                                                            \
	{ digitalWrite(PIN_TEST2, false); }

#define MOTOR1_START()                                                                                                                                         \
	{ digitalWrite(PIN_CTRL1, true); }
#define MOTOR1_STOP()                                                                                                                                          \
	{ digitalWrite(PIN_CTRL1, false); }

#define MOTOR2_START()                                                                                                                                         \
	{ digitalWrite(PIN_CTRL2, true); }
#define MOTOR2_STOP()                                                                                                                                          \
	{ digitalWrite(PIN_CTRL2, false); }
