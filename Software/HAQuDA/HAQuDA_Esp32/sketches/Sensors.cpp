#include "Sensors.h"

#define DHTPIN 18
#define DHTTYPE DHT11

#define ZH03B_RX_PIN 17
#define ZH03B_TX_PIN 16

#define CCS811_WAK 5

Adafruit_CCS811 CCS811;

measStruct eCO2_meas;
measStruct TVOC_meas;
measStruct PM_1_0_meas; // define PM1.0 value of the air detector module
measStruct PM_2_5_meas; // define PM2.5 value of the air detector module
measStruct PM_10_meas;  // define PM10 value of the air detector module
measStruct temp_meas;   // define PM10 value of the air detector module
measStruct humid_meas;  // define PM10 value of the air detector module

byte O3_buf[9];

DHT dht(DHTPIN, DHTTYPE);

SoftwareSerial PMSerial;

bool sensorsBegin() {
	PMSerial.begin(9600, SWSERIAL_8N1, ZH03B_RX_PIN, ZH03B_TX_PIN, false, 256);
	PMSerial.setTimeout(100);

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

const uint16_t PmSerialDataMarker = 0x4D42;
typedef struct {
	uint16_t length;
	uint16_t reserv0;
	uint16_t reserv1;
	uint16_t reserv2;
	uint16_t pm1_0;
	uint16_t pm2_5;
	uint16_t pm10;
	uint16_t reserv3;
	uint16_t reserv4;
	uint16_t reserv5;
} __attribute__((packed)) TPmSerialData, *PTPmSerialData;

uint16_t calcPM_checkSum(uint8_t *buffer, int size) {
	uint16_t sum = 0;

	for (int i = 0; i < size; i++) {
		sum += *buffer++;
	}
	return sum;
}

bool readPM_data(PTPmSerialData pPmSerialData) {
	uint16_t checkSum;
	if (!PMSerial.find((uint8_t *)&PmSerialDataMarker, sizeof(PmSerialDataMarker))) {
		return false;
	}

	if (PMSerial.readBytes((uint8_t *)pPmSerialData, sizeof(*pPmSerialData)) != sizeof(*pPmSerialData)) {
		return false;
	}
	if (pPmSerialData->length != __builtin_bswap16(sizeof(*pPmSerialData))) {
		return false;
	}
	if (PMSerial.readBytes((uint8_t *)&checkSum, sizeof(checkSum)) != sizeof(checkSum)) {
		return false;
	}
	uint16_t calcSum
		= calcPM_checkSum((uint8_t *)&PmSerialDataMarker, sizeof(PmSerialDataMarker)) + calcPM_checkSum((uint8_t *)pPmSerialData, sizeof(*pPmSerialData));
	if (calcSum != __builtin_bswap16(checkSum)) {
		return false;
	}
	return true;
}

bool getPM_meas() {
	TPmSerialData serialData;
	uint16_t sum;
	if (!readPM_data(&serialData)) {
		while (PMSerial.available() > 0) {
			char t = PMSerial.read();
		}
		PM_1_0_meas.newMeasDone = false;
		PM_2_5_meas.newMeasDone = false;
		PM_10_meas.newMeasDone = false;
		return false;
	}

	PM_1_0_meas.value += __builtin_bswap16(serialData.pm1_0); // count PM1.0 value of the air detector module
	PM_1_0_meas.measNum++;
	PM_1_0_meas.newMeasDone = true;

	PM_2_5_meas.value += __builtin_bswap16(serialData.pm2_5); // count PM2.5 value of the air detector module
	PM_2_5_meas.measNum++;
	PM_2_5_meas.newMeasDone = true;

	PM_10_meas.value += __builtin_bswap16(serialData.pm10); // count PM10 value of the air detector module
	PM_10_meas.measNum++;
	PM_10_meas.newMeasDone = true;
	return true;
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
