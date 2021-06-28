
#include <WiFi.h>

#include <WebServer.h>
#include "WebPages.h"

#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#include "Board.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <SoftwareSerial.h>

//#include <SoftwareSerial.h>

#define LENG 31 // 0x42 + 31 bytes equal to 32 bytes

#define DHTPIN 27
#define DHTTYPE DHT11

char BlynkAuth[] = "4MdAV357utNNjm7vmCUEY2NPAdlHQMSM";
char ssid_AP[] = "HAQuDA_ESP32";
char pass_AP[] = "1234567!";

IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

String ssidLocal = "";
String passLocal = "";

unsigned char buf[LENG];

int PM01Value = 0;  // define PM1.0 value of the air detector module
int PM2_5Value = 0; // define PM2.5 value of the air detector module
int PM10Value = 0;  // define PM10 value of the air detector module

bool WiFiCredsFound = false;

WidgetTerminal terminal(V1);
WebServer server(80);

void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
void UpdateVirtualPins();
int transmitPM01(unsigned char *thebuf);
int transmitPM2_5(unsigned char *thebuf);
int transmitPM10(unsigned char *thebuf);

char checkValue(unsigned char *thebuf, char leng);

SoftwareSerial PMSerial;
DHT dht(DHTPIN, DHTTYPE);

void setup() {
	Serial.begin(115200);
	PMSerial.begin(9600, SWSERIAL_8N1, 17, 16, false, 256);
	PMSerial.setTimeout(1500);

	dht.begin();

	pinMode(LED_BUILTIN, OUTPUT);

	WiFi.mode(WIFI_AP);
	WiFi.softAP(ssid_AP, pass_AP);
	WiFi.softAPConfig(local_ip, gateway, subnet);
	// IPAddress ip_address = WiFi.softAPIP(); // IP Address of our accesspoint
	delay(100);
	// Start web server

	// Serial.print("AP IP address: ");
	// Serial.println(ip_address);

	server.on("/", setWiFiCreds);
	server.onNotFound(handle_NotFound);
	server.begin();
	Serial.println("HTTP server started");
}

uint16_t measNum = 0;
void loop() {
	server.handleClient();
	wl_status_t status = WiFi.status();
	if (WiFiCredsFound) {
		WiFiCredsFound = false;
		connectToWiFi();
	} else if (!WiFiCredsFound && (status != WL_CONNECTED)) {
		return;
	}

	if (status != WL_CONNECTED) {
		log_i("Connecting to %s", ssidLocal.c_str());
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

	// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
	float humid = dht.readHumidity();
	// Read temperature as Celsius (the default)
	float temp = dht.readTemperature();

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

		if (isnan(humid) || isnan(temp)) {
			Serial.println(F("Failed to read from DHT sensor!"));
		} else {
			Serial.println("Read DHT data");
			terminal.print("Temp: ");
			terminal.print(temp);
			terminal.println("  C");
			terminal.println();
			terminal.print("Humid: ");
			terminal.print(humid);
			terminal.println("  %");
			terminal.println();
		}

		terminal.flush();
	}
}

void connectToWiFi() {
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssidLocal.c_str(), passLocal.c_str(), 0, 0);
	WiFi.onEvent(WiFiStationConnected, SYSTEM_EVENT_STA_CONNECTED);
	WiFi.onEvent(WiFiStationDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);

	Blynk.config(BlynkAuth, BLYNK_DEFAULT_DOMAIN, BLYNK_DEFAULT_PORT);
}

void setWiFiCreds() {
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