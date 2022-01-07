#include "Sensors.h"

#define DHTPIN 32
#define DHTTYPE DHT11

#define ZE2503_RX_PIN 2
#define ZE2503_TX_PIN 5

#define ZH03B_RX_PIN 17
#define ZH03B_TX_PIN 16

#define CCS811_WAK 33

#define PM_BUF_LEN 31 // 0x42 + 31 bytes equal to 32 bytes

Adafruit_CCS811 CCS811;

measStruct eCO2_meas;
measStruct TVOC_meas;
measStruct PM_1_0_meas; // define PM1.0 value of the air detector module
measStruct PM_2_5_meas; // define PM2.5 value of the air detector module
measStruct PM_10_meas;  // define PM10 value of the air detector module
measStruct temp_meas;   // define PM10 value of the air detector module
measStruct humid_meas;  // define PM10 value of the air detector module
measStruct O3_meas;		// define O3 value of the ZE25-O3

unsigned char buf[PM_BUF_LEN];
byte O3_buf[9];

DHT dht(DHTPIN, DHTTYPE);

SoftwareSerial PMSerial;

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

bool sensorsBegin() {
	PMSerial.begin(9600, SWSERIAL_8N1, ZH03B_RX_PIN, ZH03B_TX_PIN, false, 256);
	PMSerial.setTimeout(1000);

	ZE25O3_init(ZE2503_RX_PIN, ZE2503_TX_PIN);
	ZE25O3_setCommQuestionMode(); // set ZE25-O3 in Q/A mode

	pinMode(CCS811_WAK, OUTPUT);
	digitalWrite(CCS811_WAK, LOW);

	eCO2_meas.measNum = 0;
	eCO2_meas.value = 0;
	eCO2_meas.newMeasDone = false;

	TVOC_meas.measNum = 0;
	TVOC_meas.value = 0;
	TVOC_meas.newMeasDone = false;

	PM_1_0_meas.measNum = 0;
	PM_1_0_meas.value = 0;
	PM_1_0_meas.newMeasDone = false;

	PM_10_meas.measNum = 0;
	PM_10_meas.value = 0;
	PM_10_meas.newMeasDone = false;

	PM_2_5_meas.measNum = 0;
	PM_2_5_meas.value = 0;
	PM_2_5_meas.newMeasDone = false;

	temp_meas.measNum = 0;
	temp_meas.value = 0;
	temp_meas.newMeasDone = false;

	humid_meas.measNum = 0;
	humid_meas.value = 0;
	humid_meas.newMeasDone = false;

	O3_meas.measNum = 0;
	O3_meas.value = 0;
	O3_meas.newMeasDone = false;

	if (!CCS811.begin()) {
		// Serial.println("Failed to start sensor! Please check your wiring.");
		return false;
	}

	dht.begin();
	return true;
}

bool getCCS811_meas() {
	if (CCS811.available()) {
		if (!CCS811.readData()) {
			eCO2_meas.value += CCS811.geteCO2(); // returns eCO2 reading
			eCO2_meas.measNum++;
			eCO2_meas.newMeasDone = true;
			TVOC_meas.value += CCS811.getTVOC(); // return TVOC reading
			TVOC_meas.measNum++;
			TVOC_meas.newMeasDone = true;
			return true;
		}
	}
	eCO2_meas.value = 0; // returns eCO2 reading
	eCO2_meas.measNum = 0;
	eCO2_meas.newMeasDone = false;
	TVOC_meas.value = 0; // return TVOC reading
	TVOC_meas.measNum = 0;
	TVOC_meas.newMeasDone = false;
	return false;
}

 bool getPM_meas() {
	if (PMSerial.find(0x42)) {
		PMSerial.readBytes(buf, PM_BUF_LEN);

		if (buf[0] == 0x4d) {
			if (checkValue(buf, PM_BUF_LEN)) {
				PM_1_0_meas.value += transmitPM01(buf); // count PM1.0 value of the air detector module
				PM_1_0_meas.measNum++;
				PM_1_0_meas.newMeasDone = true;

				PM_2_5_meas.value += transmitPM2_5(buf); // count PM2.5 value of the air detector module
				PM_2_5_meas.measNum++;
				PM_2_5_meas.newMeasDone = true;

				PM_10_meas.value += transmitPM10(buf); // count PM10 value of the air detector module
				PM_10_meas.measNum++;
				PM_10_meas.newMeasDone = true;

				return true;
			}
		}
	}
	while (PMSerial.available() > 0) {
		char t = PMSerial.read();
	}
	PM_1_0_meas.newMeasDone = false;
	PM_2_5_meas.newMeasDone = false;
	PM_10_meas.newMeasDone = false;

	return false;
}

bool getDHT11_meas() {
	// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
	float humid = dht.readHumidity();
	// Read temperature as Celsius (the default)
	float temp = dht.readTemperature();
	if (isnan(humid) || isnan(temp)) {
		temp_meas.newMeasDone = false;
		humid_meas.newMeasDone = false;
		return false;
	} else {
		temp_meas.measNum++;
		temp_meas.value += temp;
		temp_meas.newMeasDone = true;

		humid_meas.measNum++;
		humid_meas.value += humid;
		humid_meas.newMeasDone = true;
	}
	return true;
}

bool getO3_meas() {
	ZE25O3_requestSensorReading();
	delay(10);
	int val = ZE25O3_readPPB(500);
	if (val == -1) {
		ZE25O3_clearRxBuff();
		O3_meas.newMeasDone = false;
		return false;
	}
	O3_meas.measNum++;
	O3_meas.value += val;
	O3_meas.newMeasDone = true;
	return true;
}

