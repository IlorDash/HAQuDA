#pragma once

#include "Arduino.h"

typedef enum { snake, growing, randomLED, fading, staticColor, noneEffect } effect_type;

typedef struct {
	uint32_t color;
	uint16_t move_pause_ms;
	uint8_t tail_len;
} snake_params;

typedef struct {
	uint32_t color;
	uint16_t grow_pause_ms;
} growing_params;

typedef struct {
	uint16_t pause_ms;
	uint16_t end_pause_ms;
} random_led_params;

typedef struct {
	uint32_t color;
	uint16_t fade_pause_ms;
	uint8_t start_bright;
	uint8_t stop_bright;
	uint8_t step;
} fading_params;

class HAQuDA_EffectsHandler {

	static effect_type effect;
	static bool stop_effect;

	static snake_params snake_p;
	static growing_params growing_p;
	static random_led_params random_led_p;
	static fading_params fading_p;
	static uint32_t static_color;

	static uint8_t get_led_num(uint8_t x, uint8_t y);

  protected:
	static void show_static_color(uint32_t c);
	static void show_snake(const snake_params p);
	static void show_growing(const growing_params p);
	static void show_random(const random_led_params p);
	static void show_fading(const fading_params p);

  public:
	HAQuDA_EffectsHandler();

	static void Show();

	static void SetEffect(effect_type e);
	effect_type GetEffect();

	static void SetSnakeParam(const snake_params _snake_p);
	static void SetGrowParam(const growing_params _growing_p);
	static void SetFadeParam(const fading_params _fading_p);
	static void SetRandomLedParam(const random_led_params _random_led_p);
	static void SetStaticColorParam(const uint32_t _static_color_p);

	~HAQuDA_EffectsHandler();
};