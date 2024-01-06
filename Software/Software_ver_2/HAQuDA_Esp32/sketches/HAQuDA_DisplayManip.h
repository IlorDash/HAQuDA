#pragma once

#include "HAQuDA_DisplayManip_typedefs.h"
#include "HAQuDA_TimeHelper.h"

class HAQuDA_DisplayManip {
	static curr_display Display;

	meas_type checkBadMeas(meas_params currUI_Params);
	uint8_t get_nightMode_hour(night_mode_time_border timeBorder);
	void standardMode();
	void multiMode(multi_mode_params multiModeParams);
	void nightMode();
	void getMeasRGB(uint8_t *_red, uint8_t *_green, uint8_t *_blue, const uint8_t brightness, const float data, const div_dots divideDots);
	void showMeas_multi(float *data, uint8_t dataSize, div_dots *divideDots, uint8_t divideDotsSize);
	void showMeas_total(float *data, uint8_t dataSize, div_dots *dataDivideDots, uint8_t divideDotsSize);
	void showMeas_standard(float data, div_dots divideDots);
	void showMeas_night(float data, div_dots divideDots, uint8_t time);

	static uint8_t getLedNum(uint8_t x, uint8_t y);

  public:
	HAQuDA_DisplayManip();

	static void SetBrightness(const uint8_t new_brightness);

	static bool SetMode(const display_mode new_mode);

	static void SetMeasMode(const meas_mode new_meas_mode);
	static void SetMeasType(const meas_type new_meas_type);

	static void SetEffectMode(const effect_mode new_effect_mode);
	static void SetEffectParam(const effects_params new_effect_param);

	static void SetError(const effect_mode new_error);

	uint8_t GetBrightness();
	static display_mode GetMode();
	meas_mode GetMeasMode();
	meas_type GetMeasType();
	static effect_mode GetEffectMode();

	void ShowMeas();
	static void ClearError();

	static void ShowStaticColor(uint32_t color);
	static void ShowStaticColor(int red, int green, int blue);
	static void ShowEffectGrow(const grow_effect_params params = Display.effectParams.growParams);
	static void ShowEffectSnake(const snake_effect_params params = Display.effectParams.snakeParams);
	static void ShowEffectRandom(const random_effect_params params = Display.effectParams.randomParams);
	static void ShowEffectFade(const fade_effect_params params = Display.effectParams.fadeParams);
	static void ShowEffectUpDown(const upDown_effects_params params = Display.effectParams.upDownParams);

	~HAQuDA_DisplayManip();
};