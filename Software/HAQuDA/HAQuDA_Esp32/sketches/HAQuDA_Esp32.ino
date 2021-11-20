
#include <WiFi.h>
#include <WebServer.h>
#include "WebPages.h"
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include "Arduino.h"
#include "TimeHelper.h"
#include <SoftwareSerial.h>

#include "winsen_ze25o3.h"
#include <Adafruit_NeoPixel.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_I2CRegister.h>
#include <Adafruit_SPIDevice.h>
#include <Adafruit_CCS811.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>
#include <Adafruit_SSD1306.h>
#include <splash.h>

#include <vector>

using namespace std;

#define LENG 31 // 0x42 + 31 bytes equal to 32 bytes
#define LED_BUILTIN 2

#define DHTPIN 32
#define DHTTYPE DHT11

#define CCS811_WAK 33

#define LED_PIN 18
#define LED_NUM_PIXELS 109
#define MAX_BRIGHTNESS 250
#define LED_COLUMN_NUM 9
#define LED_ROW_NUM 12
#define WHITE_BRIGHTNESS_COEFF 150

#define WIFI_CREDS_NUM 3

#define DISP_MEAS_PERIOD 300000 //=5 min in ms

typedef struct {
	uint32_t value;
	uint16_t measNum;
} measStruct;

measStruct eCO2_meas;
measStruct TVOC_meas;
measStruct PM01_meas;  // define PM1.0 value of the air detector module
measStruct PM2_5_meas; // define PM2.5 value of the air detector module
measStruct PM10_meas;  // define PM10 value of the air detector module

Adafruit_NeoPixel pixels(LED_NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_CCS811 CCS811;

char BlynkAuth[] = "4MdAV357utNNjm7vmCUEY2NPAdlHQMSM";
char ssid_AP[] = "HAQuDA_ESP32";
char pass_AP[] = "1234567!";

IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

char *ssidLocal = "ilor";
char *passLocal = "ilor66142222!";

char *ssidArr[WIFI_CREDS_NUM] = {"ilor", "ordash", "realme 7"};
char *passArr[WIFI_CREDS_NUM] = {"ilor66142222!", "or14591711", "1a387fa49c2b"};

unsigned char buf[LENG];
byte O3_buf[9];

enum dispParams { eCO2, TVOC, PM2_5, none };
dispParams whatToDisp = none;

bool WiFiCredsFound = true;

WidgetTerminal terminal(V1);
WebServer server(80);

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
void UpdateVirtualPins();

void connectToWiFi(char *ssidLocal, char *passLocal);
void createAP();

int transmitPM01(unsigned char *thebuf);
int transmitPM2_5(unsigned char *thebuf);
int transmitPM10(unsigned char *thebuf);

char checkValue(unsigned char *thebuf, char leng);

SoftwareSerial PMSerial;
SoftwareSerial O3Serial;

void NeoFade(int FadeSpeed, int startBrightness, uint32_t color) {
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

void NeoRandom(int speed, int brightness) {
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

void NeoSnake(int speed, int brightness, int tailLength, uint32_t color) {
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

int firstDot;
int secondDot; // parametrs of dispParams
int thirdDot;
float coefficient;

void getRGB(int *_red, int *_green, int *_blue, int data) {
	if ((data < secondDot) && (data >= firstDot)) {
		*_blue = round(-abs(data - firstDot) * coefficient) + MAX_BRIGHTNESS;
		*_green = round(-abs(data - secondDot) * coefficient) + MAX_BRIGHTNESS;
		*_red = 0;
	} else if ((data >= secondDot) && (data <= thirdDot)) { // calculating values of LED colors (using parametres of color function)
		*_blue = 0;
		*_green = round(-abs(data - secondDot) * coefficient) + MAX_BRIGHTNESS;
		*_red = round(-abs(data - thirdDot) * coefficient) + MAX_BRIGHTNESS;
	} else if (data < firstDot) {
		*_blue = MAX_BRIGHTNESS;
		*_green = 0;
		*_red = 0;
	} else if (data > thirdDot) {
		*_blue = 0;
		*_green = 0;
		*_red = MAX_BRIGHTNESS;
	}
}

void LED_showParams(int data) {
	/**************************************************/
	int time = 9; // get real time
	/**************************************************/

	int red, green, blue; // values of LED colors
	getRGB(&red, &green, &blue, data);

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

bool dispFirstParam = false;

void dispParam() {
	if (whatToDisp == eCO2) {
		firstDot = 400;
		secondDot = 1000;
		thirdDot = 5000;
		coefficient = pixels.getBrightness() * 2 / (thirdDot - firstDot);
		LED_showParams(eCO2_meas.value / eCO2_meas.measNum);
		eCO2_meas.value = 0;
		eCO2_meas.measNum = 0;
	} else if (whatToDisp == TVOC) {
		firstDot = 220;
		secondDot = 660;
		thirdDot = 1000;
		coefficient = pixels.getBrightness() * 2 / (thirdDot - firstDot);
		LED_showParams(TVOC_meas.value / TVOC_meas.measNum);
		TVOC_meas.value = 0;
		TVOC_meas.measNum = 0;
	} else if (whatToDisp == PM2_5) {
		firstDot = 15;
		secondDot = 20;
		thirdDot = 45;
		coefficient = pixels.getBrightness() * 2 / (thirdDot - firstDot);
		LED_showParams(PM2_5_meas.value / PM2_5_meas.measNum);
		PM2_5_meas.value = 0;
		PM2_5_meas.measNum = 0;
	}
}

void WiFi_handler() {
	wl_status_t status = WiFi.status();
	int i = 0;
	int j = 0;
	while (WiFiCredsFound && (status != WL_CONNECTED) && (i < WIFI_CREDS_NUM)) {
		pixels.clear();
		pixels.show();
		delay(100);
		connectToWiFi(ssidArr[i], passArr[i]);
		// connectToWiFi(ssidLocal, passLocal);
		status = WiFi.status();
		if (status != WL_CONNECTED) {
			pixels.setPixelColor(j, COLOR_YELLOW);
			pixels.show();
			log_i("Connecting to %s", ssidArr[i]);
			WiFi.reconnect();
			uint32_t timer = millis();

			while (status != WL_CONNECTED && !PeriodInRange(timer, 5000)) {
				status = WiFi.status();
				delay(200);
			}
			delay(500);
		}
		i++;
		j++;
		if (i == WIFI_CREDS_NUM) {
			i = 0;
		}
		if (j > LED_NUM_PIXELS) {
			pixels.clear();
			pixels.show();
			delay(100);
			j = 0;
		}
	}
}

void connectToWiFi_AP() {
	int i = 0;
	wl_status_t status = WiFi.status();
	while (WiFiCredsFound && (status != WL_CONNECTED) && (i < WIFI_CREDS_NUM)) {
		connectToWiFi(ssidArr[i], passArr[i]);
		// connectToWiFi(ssidLocal, passLocal);
		i++;
		status = WiFi.status();
		if (i == WIFI_CREDS_NUM) {
			i = 0;
		}
	}
	//	if (WiFiCredsFound && (status != WL_CONNECTED)) {
	//		connectToWiFi(ssidLocal, passLocal);
	//		i++;
	//		status = WiFi.status();
	//	}
}

void setup() {
	Serial.begin(115200);

	PMSerial.begin(9600, SWSERIAL_8N1, 17, 16, false, 256);
	PMSerial.setTimeout(1500);

	O3Serial.begin(9600, SWSERIAL_8N1, 2, 5, false, 256);
	O3Serial.setTimeout(1500);

	pixels.begin();
	pixels.setBrightness(200);

	pinMode(CCS811_WAK, OUTPUT);
	digitalWrite(CCS811_WAK, LOW);

	eCO2_meas.measNum = 0;
	eCO2_meas.value = 0;

	TVOC_meas.measNum = 0;
	TVOC_meas.value = 0;

	PM01_meas.measNum = 0;
	PM01_meas.value = 0;

	PM10_meas.measNum = 0;
	PM10_meas.value = 0;

	PM2_5_meas.measNum = 0;
	PM2_5_meas.value = 0;

	if (!CCS811.begin()) {
		Serial.println("Failed to start sensor! Please check your wiring.");
		while (1)
			;
	}

	// pinMode(LED_BUILTIN, OUTPUT);

	// createAP();

	// Start web server-

	// Serial.print("AP IP address: ");
	// Serial.println(ip_address);

	// server.on("/", setWiFiCreds);
	// server.onNotFound(handle_NotFound);
	// server.begin();
	// Serial.println("HTTP server started");
}

uint16_t measNum = 0;
uint32_t dht11_meas_time = 0;
uint32_t dispMeasTimer = 0;

void loop() {
	server.handleClient();
	WiFi_handler();
	if (!Blynk.connected()) {
		Blynk.connect();
	} else {
		Blynk.run();
	}

	if (CCS811.available()) {
		if (!CCS811.readData()) {
			eCO2_meas.value += CCS811.geteCO2(); // returns eCO2 reading
			eCO2_meas.measNum++;
			TVOC_meas.value += CCS811.getTVOC(); // return TVOC reading
			TVOC_meas.measNum++;
		}
	}
	delay(100);

	if (PMSerial.find(0x42)) {
		PMSerial.readBytes(buf, LENG);

		if (buf[0] == 0x4d) {
			if (checkValue(buf, LENG)) {
				PM01_meas.value += transmitPM01(buf); // count PM1.0 value of the air detector module
				PM01_meas.measNum++;

				PM2_5_meas.value += transmitPM2_5(buf); // count PM2.5 value of the air detector module
				PM2_5_meas.measNum++;

				PM10_meas.value += transmitPM10(buf); // count PM10 value of the air detector module
				PM10_meas.measNum++;
			}
		}
	}

	//	if (O3Serial.available() > 8) {
	//		memset(O3_buf, 0, 9);
	//		O3Serial.readBytes(O3_buf, 9);
	//	}
	bool isMeasExist = (eCO2_meas.measNum) && (TVOC_meas.measNum) && (PM01_meas.measNum) && (PM2_5_meas.measNum) && (PM10_meas.measNum);

	static unsigned long OledTimer = millis();
	if ((millis() - OledTimer >= 1000) && isMeasExist) {
		measNum++;
		OledTimer = millis();

		terminal.print("	Measurment:");
		terminal.println(measNum);

		terminal.print("PM1.0: ");
		terminal.print(PM01_meas.value / PM01_meas.measNum);
		terminal.println("  ug/m3");

		terminal.print("PM2.5: ");
		terminal.print(PM2_5_meas.value / PM2_5_meas.measNum);
		terminal.println("  ug/m3");

		terminal.print("PM1 0: ");
		terminal.print(PM10_meas.value / PM10_meas.measNum);
		terminal.println("  ug/m3");
		terminal.println();

		terminal.print("eCO2: ");
		terminal.print(eCO2_meas.value / eCO2_meas.measNum);
		terminal.println("  ppm");
		terminal.println();

		terminal.print("TVOC: ");
		terminal.print(TVOC_meas.value / TVOC_meas.measNum);
		terminal.println("  ppb");
		terminal.println();

		switch (whatToDisp) {
			case eCO2: // Item 1
				terminal.println("Display eCO2");
				break;
			case TVOC: // Item 2
				terminal.println("Display TVOC");
				break;
			case PM2_5: // Item 3
				terminal.println("Display PM2_5");
				break;
			default:
				terminal.println("Display none");
		}

		terminal.flush();
	}
	if (((millis() - dispMeasTimer > DISP_MEAS_PERIOD) || dispFirstParam) && isMeasExist) {
		dispMeasTimer = millis();
		dispParam();
		dispFirstParam = false;
	}
}

void createAP() {
	WiFi.mode(WIFI_AP);
	WiFi.softAP(ssid_AP, pass_AP);
	delay(2000);
	WiFi.softAPConfig(local_ip, gateway, subnet);
	// IPAddress ip_address = WiFi.softAPIP(); // IP Address of our accesspoint
	delay(100);
}

void connectToWiFi(char *ssidLocal, char *passLocal) {
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssidLocal, passLocal, 0, 0);
	WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
	WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);

	Blynk.config(BlynkAuth, BLYNK_DEFAULT_DOMAIN, BLYNK_DEFAULT_PORT);
}

void setWiFiCreds(String ssidLocal, String passLocal) {
	if (server.hasArg("ssid") && server.hasArg("password")) {
		ssidLocal = server.arg("ssid");
		passLocal = server.arg("password");
		WiFiCredsFound = true;
		server.send(200, "text/html", WiFiCredsPage);
		// server.stop();
	} else {
		// If one of the creds is missing, go back to form page
		server.send(200, "text/html", WiFiCredsPage);
	}
}

void handle_NotFound() {
	server.send(404, "text/plain", "Not found");
}

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	// digitalWrite(LED_BUILTIN, HIGH);
	pixels.fill(COLOR_GREEN, 0, LED_NUM_PIXELS);
	pixels.show();
	delay(1000);
	pixels.clear();
	pixels.show();
	delay(100);
	log_i("WiFiStationConnected");
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	// digitalWrite(LED_BUILTIN, LOW);
	//	pixels.fill(COLOR_RED, 0, LED_NUM_PIXELS);
	//	pixels.show();
	//	delay(1000);
	//	pixels.clear();
	//	pixels.show();
	//	delay(100);
	log_i("WiFiStationDisconnected");
}

BLYNK_WRITE(V0) {
	int red = param[0].asInt();
	int green = param[1].asInt();
	int blue = param[2].asInt();
	log_i("V0, r: %d, g: %d, b: %d,", red, green, blue);
	pixels.fill(pixels.Color(red, green, blue), 0, LED_NUM_PIXELS);
	pixels.show();
	delay(100);
	whatToDisp = none;
}

BLYNK_WRITE(V2) {
	switch (param.asInt()) {
		case 1: // Item 1
			whatToDisp = eCO2;
			pixels.clear();
			pixels.show();
			delay(100);
			break;
		case 2: // Item 2
			whatToDisp = TVOC;
			pixels.clear();
			pixels.show();
			delay(100);
			break;
		case 3: // Item 3
			whatToDisp = PM2_5;
			pixels.clear();
			pixels.show();
			delay(100);
			break;
		default:
			whatToDisp = none;
	}
	dispFirstParam = true;
}

BLYNK_WRITE(V3) {
	pixels.setBrightness(param.asInt());
	pixels.show();
}

char checkValue(unsigned char *thebuf, char leng) {
	char receiveflag = 0;
	int receiveSum = 0;

	for (int i = 0; i < (leng - 2); i++) {
		receiveSum = receiveSum + thebuf[i];
	}
	receiveSum = receiveSum + 0x42;

	if (receiveSum == ((thebuf[leng - 2] << 8) + thebuf[leng - 1])) // check the serial data
	{
		receiveSum = 0;
		receiveflag = 1;
	}
	return receiveflag;
}

int transmitPM01(unsigned char *thebuf) {
	int PM01Val;
	PM01Val = ((thebuf[3] << 8) + thebuf[4]); // count PM1.0 value of the air detector module
	return PM01Val;
}

// transmit PM Value to PC
int transmitPM2_5(unsigned char *thebuf) {
	int PM2_5Val;
	PM2_5Val = ((thebuf[5] << 8) + thebuf[6]); // count PM2.5 value of the air detector module
	return PM2_5Val;
}

// transmit PM Value to PC
int transmitPM10(unsigned char *thebuf) {
	int PM10Val;
	PM10Val = ((thebuf[7] << 8) + thebuf[8]); // count PM10 value of the air detector module
	return PM10Val;
}