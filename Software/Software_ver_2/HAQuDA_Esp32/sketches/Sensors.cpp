#include "Sensors.h"
#include "Board.h"
#include "HAQuDA_ZH03BFrame.h"

#define SENS_POW 23

#define DHTPIN 18
// The assembled board carries a DHT22 (AM2302); the DHT library decodes it differently from the DHT11
#define DHTTYPE DHT22

#define ZH03B_RX_PIN 17
#define ZH03B_TX_PIN 16
constexpr size_t PM_RX_BUF_SIZE = 256;
// For 10 s after power-on the ZH03B PM2.5 field holds its firmware version number, not a reading (datasheet p.5 note 1).
// 12 s = 10 s + one 1 s frame period (a frame sent just before the end of the window can still be waiting in the buffer
// at the first read after it) + 1 s for the module's boot, since its own 10 s start after SENS_POW goes HIGH.
constexpr uint32_t PM_WARMUP_MS = 12 * 1000;

#define SENS_REBOOT_DELAY 1000

Adafruit_CCS811 CCS811;

meas_data eCO2_meas;
meas_data TVOC_meas;
meas_data PM_1_0_meas;
meas_data PM_2_5_meas;
meas_data PM_10_meas;
meas_data temp_meas;
meas_data humid_meas;

uint8_t pm_buf[ZH03B_PAYLOAD_LEN];
const uint8_t pm_frame_marker[] = {ZH03B_MARKER_FIRST, ZH03B_MARKER_SECOND};

// Written and read in the loop task only
static uint32_t pmPowerOnMs = 0;
static bool pmPowered = false;

DHT dht(DHTPIN, DHTTYPE);

SoftwareSerial PMSerial;

void pmSerialBegin() {
	// 256 is the RX buffer size; begin() also allocates a ~2560-entry (~10 KB) ISR buffer before freeing the old one,
	// so it runs once: retries only power-cycle the sensors, the pins and the buffers stay configured.
	PMSerial.begin(9600, SWSERIAL_8N1, ZH03B_RX_PIN, ZH03B_TX_PIN, false, PM_RX_BUF_SIZE);
	PMSerial.setTimeout(1000);
}

bool sensorsBegin() {
	pinMode(SENS_POW, OUTPUT);
	digitalWrite(SENS_POW, HIGH);
	// Every power cycle (first start, retries, sensors_reboot()) passes through here, so this is always the latest power-on
	pmPowerOnMs = millis();
	pmPowered = true;

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

// Empties the RX buffer and copies the payload (the 22 bytes after 42 4D) of the newest checksum-valid frame into pm_buf.
// The ZH03B sends one frame per second while polling is slower, so older frames are stale and are dropped.
// loop task only.
static bool readNewestPMFrame() {
	uint8_t raw[PM_RX_BUF_SIZE] = {};
	size_t rawLen = 0;
	while ((rawLen < sizeof(raw)) && (PMSerial.available() > 0)) {
		raw[rawLen++] = (uint8_t)PMSerial.read();
	}

	if (HAQuDA_FindNewestZH03BFrame(raw, rawLen, pm_buf)) {
		return true;
	}

	// Nothing usable was buffered (sensor just powered up, or only a partial frame): wait for the next frame, up to the 1 s stream timeout per step
	if (PMSerial.find(pm_frame_marker, sizeof(pm_frame_marker))) {
		return (PMSerial.readBytes(pm_buf, ZH03B_PAYLOAD_LEN) == ZH03B_PAYLOAD_LEN) && HAQuDA_IsZH03BPayloadValid(pm_buf);
	}
	return false;
}

// Empties the RX buffer without waiting, so frames sent during the warm-up are not left to pile up. loop task only.
static void drainPMSerial() {
	while (PMSerial.available() > 0) {
		PMSerial.read();
	}
}

// True inside the warm-up window after the latest power-on. Once the window has passed the flag is cleared for good: the
// unsigned difference becomes small again every 2^32 ms (49.7 days) and would reopen the window. loop task only.
static bool isPMWarmingUp() {
	if (!pmPowered) {
		return false;
	}
	if ((uint32_t)(millis() - pmPowerOnMs) < PM_WARMUP_MS) {
		return true;
	}
	pmPowered = false;
	return false;
}

pm_meas_result getPM_meas() {
	if (isPMWarmingUp()) {
		drainPMSerial();
		PM_1_0_meas.newMeasDone = false;
		PM_2_5_meas.newMeasDone = false;
		PM_10_meas.newMeasDone = false;
		return pm_meas_result::warming_up;
	}

	if (readNewestPMFrame()) {
		PM_1_0_meas.value += HAQuDA_GetZH03BPM1_0(pm_buf);
		PM_1_0_meas.measNum++;
		PM_1_0_meas.newMeasDone = true;

		PM_2_5_meas.value += HAQuDA_GetZH03BPM2_5(pm_buf);
		PM_2_5_meas.measNum++;
		PM_2_5_meas.newMeasDone = true;

		PM_10_meas.value += HAQuDA_GetZH03BPM10(pm_buf);
		PM_10_meas.measNum++;
		PM_10_meas.newMeasDone = true;

		return pm_meas_result::ok;
	}
	PM_1_0_meas.newMeasDone = false;
	PM_2_5_meas.newMeasDone = false;
	PM_10_meas.newMeasDone = false;

	return pm_meas_result::failed;
}

bool getDHT22_meas() {
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