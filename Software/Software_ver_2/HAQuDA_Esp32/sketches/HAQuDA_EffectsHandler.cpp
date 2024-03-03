#include "HAQuDA_EffectsHandler.h"
#include "HAQuDA_LEDsParams.h"
#include "HAQuDA_LEDs.h"
#include <vector>

effect_type HAQuDA_EffectsHandler::effect;
bool HAQuDA_EffectsHandler::stop_effect;

snake_params HAQuDA_EffectsHandler::snake_p;
growing_params HAQuDA_EffectsHandler::growing_p;
random_led_params HAQuDA_EffectsHandler::random_led_p;
fading_params HAQuDA_EffectsHandler::fading_p;
uint32_t HAQuDA_EffectsHandler::static_color;

HAQuDA_EffectsHandler::HAQuDA_EffectsHandler() {

	effect = noneEffect;
	stop_effect = false;

	snake_p.color = COLOR_LIME;
	snake_p.move_pause_ms = 200;
	snake_p.tail_len = 5;

	growing_p.color = COLOR_AQUA;
	growing_p.grow_pause_ms = 200;

	fading_p.color = COLOR_AQUA;
	fading_p.fade_pause_ms = 10;
	fading_p.start_bright = 100;
	fading_p.stop_bright = 0;
	fading_p.step = 5;

	random_led_p.pause_ms = 200;
	random_led_p.end_pause_ms = 1000;

	static_color = COLOR_GREEN;
}

void HAQuDA_EffectsHandler::Show() {

	switch (effect) {
		case snake: {
			show_snake(snake_p);
			break;
		}
		case growing: {
			show_growing(growing_p);
			break;
		}
		case randomLED: {
			show_random(random_led_p);
			break;
		}
		case fading: {
			show_fading(fading_p);
			break;
		}
		case staticColor: {
			show_static_color(static_color);
		}
		default:
			break;
	}
}

void HAQuDA_EffectsHandler::SetEffect(effect_type e) {

	if ((e == noneEffect) || (e != effect))
		stop_effect = true;

	effect = e;
}

effect_type HAQuDA_EffectsHandler::GetEffect() {
	return effect;
};

 void HAQuDA_EffectsHandler::SetSnakeParam(const snake_params _snake_p) {
	snake_p = _snake_p;
}

void HAQuDA_EffectsHandler::SetGrowParam(const growing_params _growing_p) {
	growing_p = _growing_p;
}

void HAQuDA_EffectsHandler::SetFadeParam(const fading_params _fading_p) {
	fading_p = _fading_p;
}

void HAQuDA_EffectsHandler::SetRandomLedParam(const random_led_params _random_led_p) {
	random_led_p = _random_led_p;
}

void HAQuDA_EffectsHandler::SetStaticColorParam(const uint32_t _static_color_p) {
	static_color = _static_color_p;
}

void HAQuDA_EffectsHandler::show_static_color(uint32_t c) {
	HAQuDA_LEDs::getInstance()->FillColor(c);
	HAQuDA_LEDs::getInstance()->Show();
}

 void HAQuDA_EffectsHandler::show_snake(const snake_params p) {
	stop_effect = false;
	HAQuDA_LEDs::getInstance()->Clear();
	for (int i = 0; i < LED_NUM_PIXELS; i++) {
		int pixelNum = 0;
		for (int j = 0; j < p.tail_len; j++) {
			pixelNum = j + i;
			pixelNum = (pixelNum > LED_NUM_PIXELS) ? LED_NUM_PIXELS : pixelNum;
			HAQuDA_LEDs::getInstance()->SetPixelColor(pixelNum, p.color);

			if (stop_effect) {
				return;
			}
		}
		HAQuDA_LEDs::getInstance()->Show(p.move_pause_ms);
		HAQuDA_LEDs::getInstance()->SetPixelColor(i, 0);
	}
}

void HAQuDA_EffectsHandler::show_growing(const growing_params p) {
	stop_effect = false;
	HAQuDA_LEDs::getInstance()->Clear();
	for (int i = (LED_ROW_NUM - 1); i >= 0; i--) {
		for (int j = 0; j < LED_COLUMN_NUM; j++) {
			if (stop_effect) {
				return;
			}
			uint8_t pixelNum = get_led_num(j, i);
			HAQuDA_LEDs::getInstance()->SetPixelColor(pixelNum, p.color);
		}
		HAQuDA_LEDs::getInstance()->Show(p.grow_pause_ms);
	}
}

void HAQuDA_EffectsHandler::show_random(const random_led_params p) {
	stop_effect = false;
	HAQuDA_LEDs::getInstance()->Clear();
	std::vector<int> pixels_on;
	for (int i = 0; i < LED_NUM_PIXELS; i++) {
		int pixelNum = random(0, LED_NUM_PIXELS);
		while (find(pixels_on.begin(), pixels_on.end(), pixelNum) != pixels_on.end()) {
			pixelNum = random(0, LED_NUM_PIXELS);

			if (stop_effect) {
				return;
			}
		}
		pixels_on.push_back(pixelNum);
		HAQuDA_LEDs::getInstance()->SetPixelColor(pixelNum, ((uint32_t)random(0, 255) << 16) | ((uint32_t)random(0, 255) << 8) | random(0, 255));
		HAQuDA_LEDs::getInstance()->Show(p.pause_ms);
	}
	vTaskDelay(p.end_pause_ms / portTICK_PERIOD_MS);
}

void HAQuDA_EffectsHandler::show_fading(const fading_params p) {
	stop_effect = false;
	auto bright = HAQuDA_LEDs::getInstance()->GetBright();
	HAQuDA_LEDs::getInstance()->Clear();
	HAQuDA_LEDs::getInstance()->SetBright(p.start_bright);
	HAQuDA_LEDs::getInstance()->FillColor(p.color);
	HAQuDA_LEDs::getInstance()->Show();
	for (int i = p.start_bright; i > p.stop_bright; i = i - p.step) {
		HAQuDA_LEDs::getInstance()->SetBright(i);
		vTaskDelay(p.fade_pause_ms / portTICK_PERIOD_MS);
		if (stop_effect) {
			HAQuDA_LEDs::getInstance()->SetBright(bright);
			return;
		}
	}
}

uint8_t HAQuDA_EffectsHandler::get_led_num(uint8_t x, uint8_t y) {

	if ((x >= LED_COLUMN_NUM) || (y >= LED_ROW_NUM)) {
		return 0;
	}

	y = (x % 2 != 0) ? (LED_ROW_NUM - y - 1) : y;

	return y + LED_ROW_NUM * x;
}

HAQuDA_EffectsHandler::~HAQuDA_EffectsHandler() {
}