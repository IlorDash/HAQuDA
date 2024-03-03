#include "Sensors.h"

#define SENS_POW 23

#define DHTPIN 18
#define DHTTYPE DHT11

#define ZH03B_RX_PIN 17
#define ZH03B_TX_PIN 16
#define PM_BUF_LEN 22

#define CCS811_WAK 5

#define SENS_REBOOT_DELAY 1000

Adafruit_CCS811 CCS811;

meas_data eCO2_meas;
meas_data TVOC_meas;
meas_data PM_1_0_meas;
meas_data PM_2_5_meas;
meas_data PM_10_meas;
meas_data temp_meas;
meas_data humid_meas;

unsigned char pm_buf[PM_BUF_LEN];
const uint16_t pm_data_marker = 0x4D42;

DHT dht(DHTPIN, DHTTYPE);

SoftwareSerial PMSerial;

char checkValue(unsigned char *thebuf) {
	char receiveflag = 0;
	int receiveSum = 0;

	for (int i = 0; i < (PM_BUF_LEN - 2); i++) {
		receiveSum += thebuf[i];
	}
	receiveSum += (int)(pm_data_marker & 0xff) + (int)((pm_data_marker >> 8) & 0xff);

	if (receiveSum == ((thebuf[PM_BUF_LEN - 2] << 8) + thebuf[PM_BUF_LEN - 1])) {
		receiveSum = 0;
		receiveflag = 1;
	}
	return receiveflag;
}

int transmitPM01(unsigned char *thebuf) {
	int PM01Val;
	PM01Val = ((thebuf[3] << 8) + thebuf[4]);
	return PM01Val;
}

int transmitPM2_5(unsigned char *thebuf) {
	int PM2_5Val;
	PM2_5Val = ((thebuf[5] << 8) + thebuf[6]);
	return PM2_5Val;
}

int transmitPM10(unsigned char *thebuf) {
	int PM10Val;
	PM10Val = ((thebuf[7] << 8) + thebuf[8]);
	return PM10Val;
}

bool sensorsBegin() {
	pinMode(SENS_POW, OUTPUT);
	digitalWrite(SENS_POW, HIGH);

	PMSerial.begin(9600, SWSERIAL_8N1, ZH03B_RX_PIN, ZH03B_TX_PIN, false, 256);
	PMSerial.setTimeout(1000);

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

	vTaskDelay(100 / portTICK_PERIOD_MS);

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
			eCO2_meas.value += CCS811.geteCO2();
			eCO2_meas.measNum++;
			eCO2_meas.newMeasDone = true;
			TVOC_meas.value += CCS811.getTVOC();
			TVOC_meas.measNum++;
			TVOC_meas.newMeasDone = true;
			return true;
		}
	}
	eCO2_meas.value = 0;
	eCO2_meas.measNum = 0;
	eCO2_meas.newMeasDone = false;
	TVOC_meas.value = 0;
	TVOC_meas.measNum = 0;
	TVOC_meas.newMeasDone = false;
	return false;
}

bool getPM_meas() {
	if (PMSerial.find((uint8_t *)&pm_data_marker, sizeof(pm_data_marker))) {
		PMSerial.readBytes(pm_buf, PM_BUF_LEN);
		if (checkValue(pm_buf)) {
			PM_1_0_meas.value += transmitPM01(pm_buf);
			PM_1_0_meas.measNum++;
			PM_1_0_meas.newMeasDone = true;

			PM_2_5_meas.value += transmitPM2_5(pm_buf);
			PM_2_5_meas.measNum++;
			PM_2_5_meas.newMeasDone = true;

			PM_10_meas.value += transmitPM10(pm_buf);
			PM_10_meas.measNum++;
			PM_10_meas.newMeasDone = true;

			return true;
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
	float temp_celc = dht.readTemperature();
	if (isnan(humid) || isnan(temp_celc)) {
		temp_meas.newMeasDone = false;
		humid_meas.newMeasDone = false;
		return false;
	} else {
		temp_meas.measNum++;
		temp_meas.value += temp_celc;
		temp_meas.newMeasDone = true;

		humid_meas.measNum++;
		humid_meas.value += humid;
		humid_meas.newMeasDone = true;
	}
	return true;
}

void ClearMeas() {
	temp_meas.value = 0;
	temp_meas.measNum = 0;

	humid_meas.value = 0;
	humid_meas.measNum = 0;

	eCO2_meas.value = 0;
	eCO2_meas.measNum = 0;

	TVOC_meas.value = 0;
	TVOC_meas.measNum = 0;

	PM_2_5_meas.value = 0;
	PM_2_5_meas.measNum = 0;
}

void sensors_reboot() {
	digitalWrite(SENS_POW, LOW);
	vTaskDelay(SENS_REBOOT_DELAY / portTICK_PERIOD_MS);
	digitalWrite(SENS_POW, LOW);
	vTaskDelay(SENS_REBOOT_DELAY / portTICK_PERIOD_MS);
}