
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include "Board.h"

#include <HardwareSerial.h>

#define LENG 31 // 0x42 + 31 bytes equal to 32 bytes

char auth[] = "4MdAV357utNNjm7vmCUEY2NPAdlHQMSM";
char ssid[] = "ilor";
char pass[] = "ilor66142222!";

unsigned char buf[LENG];

int PM01Value = 0;  // define PM1.0 value of the air detector module
int PM2_5Value = 0; // define PM2.5 value of the air detector module
int PM10Value = 0;  // define PM10 value of the air detector module

WidgetTerminal terminal(V1);

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
void UpdateVirtualPins();
int transmitPM01(unsigned char *thebuf);
int transmitPM2_5(unsigned char *thebuf);
int transmitPM10(unsigned char *thebuf);

char checkValue(unsigned char *thebuf, char leng);

HardwareSerial PMSerial(1);

void setup() {
	Serial.begin(115200);
	PMSerial.begin(9600, SERIAL_8N1, 10, 9);
	PMSerial.setTimeout(1500);

	pinMode(LED_BUILTIN, OUTPUT);

	WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
	WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, pass, 0, 0);

	Blynk.config(auth, BLYNK_DEFAULT_DOMAIN, BLYNK_DEFAULT_PORT);
}

uint16_t measNum = 0;
void loop() {
	wl_status_t status = WiFi.status();
	if (status != WL_CONNECTED) {
		log_i("Connecting to %s", ssid);
		WiFi.reconnect();
		uint32_t timer = millis();

		while (WiFi.status() != WL_CONNECTED && !PeriodInRange(timer, 5000)) {
			delay(200);
		}
	} else if (!Blynk.connected()) {
		Blynk.connect();
	} else {
		Blynk.run();
	}

	//	while (MySerial.available() > 0) {
	//		uint8_t byteFromSerial = MySerial.read();
	//		Serial.println(byteFromSerial);
	//		terminal.println(byteFromSerial);
	//		terminal.flush();
	//	}

	if (PMSerial.find(0x42)) {
		PMSerial.readBytes(buf, LENG);

		if (buf[0] == 0x4d) {
			if (checkValue(buf, LENG)) {
				PM01Value = transmitPM01(buf);   // count PM1.0 value of the air detector module
				PM2_5Value = transmitPM2_5(buf); // count PM2.5 value of the air detector module
				PM10Value = transmitPM10(buf);   // count PM10 value of the air detector module
			}
		}
	}

	static unsigned long OledTimer = millis();
	if (millis() - OledTimer >= 1000) {
		measNum++;
		OledTimer = millis();
		
		terminal.print("	Measurment:");
		terminal.println(measNum);

		terminal.print("PM1.0: ");
		terminal.print(PM01Value);
		terminal.println("  ug/m3");

		terminal.print("PM2.5: ");
		terminal.print(PM2_5Value);
		terminal.println("  ug/m3");

		terminal.print("PM1 0: ");
		terminal.print(PM10Value);
		terminal.println("  ug/m3");
		terminal.println();

		terminal.flush();
	}
}

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	digitalWrite(LED_BUILTIN, HIGH);
	log_i("WiFiStationConnected");
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
	digitalWrite(LED_BUILTIN, LOW);
	log_i("WiFiStationDisconnected");
}

BLYNK_WRITE(V0) {
	int red = param[0].asInt();
	int green = param[1].asInt();
	int blue = param[2].asInt();
	log_i("V0, r: %d, g: %d, b: %d,", red, green, blue);
	if (red > 80) {
		digitalWrite(LED_BUILTIN, HIGH);
	} else {
		digitalWrite(LED_BUILTIN, LOW);
	}
}

// BLYNK_WRITE(V1) {
//
//	// if you type "Marco" into Terminal Widget - it will respond: "Polo:"
//	if (String("Marco") == param.asStr()) {
//		terminal.println("You said: 'Marco'");
//		terminal.println("I said: 'Polo'");
//	} else {
//
//		// Send it back
//		terminal.print("You said:");
//		terminal.write(param.getBuffer(), param.getLength());
//		terminal.println();
//	}
//
//	// Ensure everything is sent
//	terminal.flush();
//}
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