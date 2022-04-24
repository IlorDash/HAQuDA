#include "WS2812.h"

Adafruit_NeoPixel pixels(LED_NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

void WS2812_begin() {
	pixels.begin();
	pixels.setBrightness(MAX_BRIGHTNESS);
}

void WS2812_setBrightnessPerCent(uint8_t brightnessPerCent) {
	brightnessPerCent = (brightnessPerCent > 100) ? 100 : brightnessPerCent;
	uint16_t brightness = brightnessPerCent * MAX_BRIGHTNESS / 100;
	pixels.setBrightness(brightness);
	pixels.show();
}

void WS2812_clear() {
	pixels.clear();
	pixels.show();
}

void WS2812_show() {
	pixels.show();
}

void WS2812_setPixelColor(uint16_t pixelNum, uint32_t color) {
	pixels.setPixelColor(pixelNum, color);
	pixels.show();
}

void WS2812_fillColor(uint32_t color) {
	pixels.fill(color, 0, LED_NUM_PIXELS);
	pixels.show();
}

uint8_t christmasTreeMaxWidth = 7;
uint8_t *christmasTreePixelsArr;

uint8_t GetPosition(int x, int y) {
	int n;

	x = (x >= LED_COLUMN_NUM) ? (x % LED_COLUMN_NUM) : x;
	y = (y >= LED_ROW_NUM) ? (y % LED_ROW_NUM) : y;

	if (x % 2 == 0) {
		n = LED_ROW_NUM - y;
	} else {
		n = y + 1;
	}
	return n + LED_ROW_NUM * x;
}

void christmasLightsSnake() {
	uint8_t startColumn = LED_COLUMN_NUM - 1;

	//	for (int i = 0; i < length; i++) {
	//	}
}

void drawStarOnTree(int middleColumn) {
	pixels.setPixelColor(GetPosition(middleColumn, LED_ROW_NUM - 1), CHRISTMAS_TREE_STAR_COLOR);
	pixels.setPixelColor(GetPosition(middleColumn, LED_ROW_NUM - 2), CHRISTMAS_TREE_STAR_COLOR);
	pixels.setPixelColor(GetPosition(middleColumn, LED_ROW_NUM - 3), CHRISTMAS_TREE_STAR_COLOR);
	pixels.setPixelColor(GetPosition(middleColumn - 1, LED_ROW_NUM - 2), CHRISTMAS_TREE_STAR_COLOR);
	pixels.setPixelColor(GetPosition(middleColumn + 1, LED_ROW_NUM - 2), CHRISTMAS_TREE_STAR_COLOR);
}

void drawChristmasTree(uint8_t middleColumn) {

	bool isLedStripFromUp = (middleColumn % 2 == 0);

	uint32_t trunkColor = pixels.Color(190, 85, 30);
	pixels.setPixelColor(GetPosition(middleColumn, 0), trunkColor);
	pixels.show();
	delay(100);
	vTaskDelay(100 / portTICK_PERIOD_MS);

	int globalMiddleColumn = LED_COLUMN_NUM / 2;
	int middleColumnOffset = globalMiddleColumn - middleColumn;
	for (int i = 0; i < (CHRISTMAS_TREE_HEIGHT / 2); i++) {
		for (int j = (globalMiddleColumn - CHRISTMAS_TREE_HALF_WIDTH + i); j <= (globalMiddleColumn + CHRISTMAS_TREE_HALF_WIDTH - i); j++) {
			pixels.setPixelColor(GetPosition(j + middleColumnOffset, i), CHRISTMAS_TREE_COLOR);
			pixels.setPixelColor(GetPosition(j + middleColumnOffset, i + 1), CHRISTMAS_TREE_COLOR);
		}
	}
	pixels.show();
	vTaskDelay(100 / portTICK_PERIOD_MS);
	drawStarOnTree(middleColumn);
}

void WS2812_ChristmasTree(int speed) {
	uint8_t treeMiddleColumn = 0;

	uint8_t redBackgroundIntens = 190 * 1000 / MAX_BRIGHTNESS * WHITE_BRIGHTNESS_COEFF / 1000;
	uint8_t greenBackgroundIntens = 190 * 1000 / MAX_BRIGHTNESS * WHITE_BRIGHTNESS_COEFF / 1000;
	uint8_t blueBackgroundIntens = 230 * 1000 / MAX_BRIGHTNESS * WHITE_BRIGHTNESS_COEFF / 1000;
	uint32_t backgroundColor = pixels.Color(redBackgroundIntens, greenBackgroundIntens, blueBackgroundIntens);
	pixels.fill(backgroundColor, 0, LED_NUM_PIXELS); // drawing christmas tree with white-blue background
	pixels.show();
	delay(100);
	drawChristmasTree(treeMiddleColumn);

	delay(100);

	uint8_t lightsEffect = rand() % 3;
}

void getRGB(int *_red, int *_green, int *_blue, float data, paramsDivideDots divideDots) {
	volatile float temp = pixels.getBrightness();
	volatile float coefficient = (temp * 2) / (divideDots.thirdDot - divideDots.firstDot);
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

void WS2812_showParams_standard(float data, paramsDivideDots divideDots) {
	int red, green, blue; // values of LED colors
	getRGB(&red, &green, &blue, data, divideDots);
	pixels.fill(pixels.Color(red, green, blue), 0, LED_NUM_PIXELS);
	pixels.show();
	delay(100);
}

void WS2812_showParams_night(float data, paramsDivideDots divideDots, uint8_t time) {

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

void WS2812_showParams_multi(float *data, paramsDivideDots *divideDots) {
	int red[MULTI_MODE_PARAM_NUM], green[MULTI_MODE_PARAM_NUM], blue[MULTI_MODE_PARAM_NUM]; // values of LED colors
	for (int i = 0; i < MULTI_MODE_PARAM_NUM; i++) {
		getRGB(&(red[i]), &(green[i]), &(blue[i]), data[i], divideDots[i]);
	}
	for (int i = 0; i < LED_ROW_NUM; i++) {
		if (!(i % 2)) {
			pixels.fill(pixels.Color(red[0], green[0], blue[0]), i * LED_ROW_NUM + 1, 3);
			pixels.fill(pixels.Color(red[1], green[1], blue[1]), i * LED_ROW_NUM + 5, 3);
			pixels.fill(pixels.Color(red[2], green[2], blue[2]), i * LED_ROW_NUM + 9, 3);
		} else {
			pixels.fill(pixels.Color(red[2], green[2], blue[2]), i * LED_ROW_NUM, 3);
			pixels.fill(pixels.Color(red[1], green[1], blue[1]), i * LED_ROW_NUM + 4, 3);
			pixels.fill(pixels.Color(red[0], green[0], blue[0]), i * LED_ROW_NUM + 8, 3);
		}
	}
	pixels.show();
	delay(100);
}

void WS2812_showParams_standardTotal(float *data) {
	int colorsArr[DISP_PARAMS_NUM][COLORS_NUM];
	for (int i = 0; i < DISP_PARAMS_NUM; i++) {
		getRGB(&(colorsArr[i][0]), &(colorsArr[i][1]), &(colorsArr[i][2]), data[i], temp_divideDots); // calculating all 3 colors for all displayable parameters
	}

	int red, green, blue; // values of LED colors

	for (int i = 0; i < DISP_PARAMS_NUM; i++) {
		red += colorsArr[i][0];
		green += colorsArr[i][1]; // calculating mean of colors of all displayable parameters
		blue += colorsArr[i][2];
	}
	red /= DISP_PARAMS_NUM;
	green /= DISP_PARAMS_NUM;
	blue /= DISP_PARAMS_NUM;
	pixels.fill(pixels.Color(red, green, blue), 0, LED_NUM_PIXELS);
	pixels.show();
	delay(100);
}