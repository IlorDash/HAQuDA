#include "WS2812.h"

using namespace std;

Adafruit_NeoPixel pixels(LED_NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

void WS2812_begin() {
	pixels.begin();
	pixels.setBrightness(MAX_BRIGHTNESS);
}

void WS2812_setBrightness(uint8_t brightnessPerCent) {
	brightnessPerCent = (brightnessPerCent > 100) ? 100 : brightnessPerCent;
	uint16_t brightness = brightnessPerCent * MAX_BRIGHTNESS / 100;
	pixels.setBrightness(brightness);
	pixels.show();
}

void WS2812_clear() {
	pixels.clear();
	pixels.show();
}

void WS2812_fillPixelColor(uint16_t pixelNum, uint32_t color) {
	pixels.setPixelColor(pixelNum, color);
	pixels.show();
}

void WS2812_fillColor(uint32_t color) {
	pixels.fill(color, 0, LED_NUM_PIXELS);
	pixels.show();
}

void WS2812_Fade(int FadeSpeed, int startBrightness, uint32_t color) {
	pixels.clear();
	pixels.show();
	delay(100);
	for (int i = 0; i < LED_NUM_PIXELS; i++) {
		pixels.setPixelColor(i, color);
	}
	for (int j = 255; j > 0; j = j - 2) {
		pixels.setBrightness(j);
		pixels.show();
		delay(FadeSpeed);
	}
}

void WS2812_Random(int speed, int brightness) {
	pixels.clear();
	pixels.show();
	delay(100);
	pixels.setBrightness(brightness);
	vector<int> pixelEnArr;
	for (int i = 0; i < LED_NUM_PIXELS; i++) {
		int pixelNum = random(0, LED_NUM_PIXELS);
		while (find(pixelEnArr.begin(), pixelEnArr.end(), pixelNum) != pixelEnArr.end()) {
			pixelNum = random(0, LED_NUM_PIXELS);
		}
		pixelEnArr.push_back(pixelNum);
		pixels.setPixelColor(pixelNum, random(0, 255), random(0, 255), random(0, 255));
		pixels.show();
		delay(speed);
	}
	delay(1000);
}

void WS2812_Snake(int speed, int brightness, int tailLength, uint32_t color) {
	if (tailLength > 10) {
		return;
	}
	pixels.clear();
	pixels.show();
	delay(100);
	pixels.setBrightness(brightness);
	for (int i = 0; i < LED_NUM_PIXELS; i++) {
		int pixelNum = 0;
		for (int j = 0; j < tailLength; j++) {
			pixelNum = i - j;
			pixelNum = (pixelNum < 0) ? 0 : pixelNum;
			pixelNum = (pixelNum > LED_NUM_PIXELS) ? LED_NUM_PIXELS : pixelNum;
			pixels.setPixelColor(pixelNum, color);
			pixels.show();
			delay(speed);
		}
		pixels.clear();
		pixels.show();
	}
	delay(1000);
}

void getRGB(int *_red, int *_green, int *_blue, int data, showParamsDivideDots divideDots) {
	float coefficient = pixels.getBrightness() * 2 / (divideDots.thirdDot - divideDots.firstDot);
	if ((data < divideDots.secondDot) && (data >= divideDots.firstDot)) {
		*_blue = round(-abs(data - divideDots.firstDot) * coefficient) + MAX_BRIGHTNESS;
		*_green = round(-abs(data - divideDots.secondDot) * coefficient) + MAX_BRIGHTNESS;
		*_red = 0;
	} else if ((data >= divideDots.secondDot) && (data <= divideDots.thirdDot)) { // calculating values of LED colors (using parametres of color function)
		*_blue = 0;
		*_green = round(-abs(data - divideDots.secondDot) * coefficient) + MAX_BRIGHTNESS;
		*_red = round(-abs(data - divideDots.thirdDot) * coefficient) + MAX_BRIGHTNESS;
	} else if (data < divideDots.firstDot) {
		*_blue = MAX_BRIGHTNESS;
		*_green = 0;
		*_red = 0;
	} else if (data > divideDots.thirdDot) {
		*_blue = 0;
		*_green = 0;
		*_red = MAX_BRIGHTNESS;
	}
}

void WS2812_showParams(int data, showParamsDivideDots divideDots) {
	/**************************************************/
	int time = 9; // get real time
	/**************************************************/

	int red, green, blue; // values of LED colors
	getRGB(&red, &green, &blue, data, divideDots);

	for (int i = 0; i < LED_COLUMN_NUM; i++) {
		uint8_t whiteBrightness = pixels.getBrightness() * 1000 / MAX_BRIGHTNESS * WHITE_BRIGHTNESS_COEFF / 1000;
		uint32_t color = (!(i % 3)) ? pixels.Color(red, green, blue) : pixels.Color(whiteBrightness, whiteBrightness, whiteBrightness);
		uint8_t pixelNum = (!(i % 3)) ? LED_ROW_NUM : time;
		uint8_t startPixel = (!(i % 2)) ? (i * LED_ROW_NUM) : (i * LED_ROW_NUM + LED_ROW_NUM - pixelNum);
		pixels.fill(color, startPixel, pixelNum);
	}
	pixels.show();
	delay(100);
}