#pragma once

#include <cstdint>

#define LED_PIN 26
#define LED_NUM_PIXELS 109
// Raw NeoPixel brightness (0..255) at 100 %: the LED current limit. Everything else counts brightness in percent.
#define MAX_BRIGHTNESS 150
// Brightness in percent (5..100 in steps of 5, the lamp page's scale) the lamp starts with
#define DEFAULT_BRIGHTNESS 35
#define LED_COLUMN_NUM 9
#define LED_ROW_NUM 12
// Night bar grey as a percentage of the largest colour channel (COLOR_SCALE_MAX_CHANNEL)
#define WHITE_BRIGHTNESS_COEFF 70

#define COLORS_NUM 3

#define COLOR_OFF 0x000000
#define COLOR_AQUA 0x00FFFF
#define COLOR_LIME 0x99FF33
#define COLOR_GREEN 0x01FF00
#define COLOR_RED 0xFF0000
#define COLOR_YELLOW 0xFFFF00
#define COLOR_WHITE 0xFFFFFF

// Raw NeoPixel brightness (0..MAX_BRIGHTNESS) of a brightness in percent; more than 100 counts as 100. The only
// percent to raw conversion of the firmware, used by HAQuDA_LEDs::SetBright(). Any task.
constexpr uint8_t HAQuDA_BrightPercentToRaw(uint8_t brightPerCent) {
	return static_cast<uint8_t>((((brightPerCent > 100) ? 100 : brightPerCent) * MAX_BRIGHTNESS) / 100);
}
