#include "Sensors.h"
#include "Board.h"
#include "HAQuDA_Logger.h"
#include "HAQuDA_PMReader.h"
#include "HAQuDA_SensorsPresence.h"
#include "HAQuDA_ZH03BFrame.h"

#include <atomic>

#define SENS_POW 23

#define DHTPIN 18
// The assembled board carries a DHT22 (AM2302); the DHT library decodes it differently from the DHT11
#define DHTTYPE DHT22

#define ZH03B_RX_PIN 17
#define ZH03B_TX_PIN 16

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

DHT dht(DHTPIN, DHTTYPE);

SoftwareSerial PMSerial;

namespace {
	// Thin adapter: the PM read flow sees only the calls it needs
	class PMSerialAdapter : public HAQuDA_IPMSerial {
	  public:
		int Available() override {
			return PMSerial.available();
		}
		int Read() override {
			return PMSerial.read();
		}
		bool Find(const uint8_t *marker, size_t len) override {
			return PMSerial.find(marker, len);
		}
		size_t ReadBytes(uint8_t *buf, size_t len) override {
			return PMSerial.readBytes(buf, len);
		}
	};
} // namespace

// Sensors is a free-function module with no owning object, so the PM objects live here until it becomes a class.
// Used in the loop task only.
static PMSerialAdapter pmSerialAdapter;
static HAQuDA_PMWarmup pmWarmup;
static HAQuDA_PMReader pmReader(pmSerialAdapter, pmWarmup);

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
	const uint32_t powerOnMs = millis();
	pmWarmup.PowerOn(powerOnMs);

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
		HAQuDA_Logger::LogError("Sensors start: CCS811 not found");
		return false;
	}

	dht.begin();
	// The buffer may hold bytes received before this attempt (the ZH03B streams into it while nobody reads, or line noise from a
	// power cycle), so only bytes received from here on may prove the sensor is alive now. A healthy ZH03B sends the next frame
	// within 1 s, inside the 5 s budget.
	pmReader.DiscardBuffered();
	// Force bypasses the library's 2 s cache, which would otherwise hand back the failed result of the previous probe
	const sensors_presence presence
		= HAQuDA_WaitForSensors([]() { return !isnan(dht.readTemperature(false, true)); }, []() { return pmReader.HasValidFrame(); }, []() { return millis(); },
								[](uint32_t ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }, powerOnMs);
	if (!presence.dht22) {
		HAQuDA_Logger::LogError("Sensors start: DHT22 not responding");
	}
	if (!presence.zh03b) {
		HAQuDA_Logger::LogError("Sensors start: ZH03B sends no frames");
	}
	return presence.dht22 && presence.zh03b;
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

pm_meas_result getPM_meas() {
	pm_meas_result result = pmReader.Read(millis(), pm_buf);

	if (result == pm_meas_result::warming_up) {
		PM_1_0_meas.newMeasDone = false;
		PM_2_5_meas.newMeasDone = false;
		PM_10_meas.newMeasDone = false;
		return pm_meas_result::warming_up;
	}

	if (result == pm_meas_result::ok) {
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

void ClearMeasData(meas_data &data) {
	// The count goes first: loop() publishes sum / count for the web page and treats an unchanged count
	// before and after reading the sum as proof that no clear ran in between.
	data.measNum = 0;
	std::atomic_thread_fence(std::memory_order_seq_cst);
	data.value = 0;
}

void ClearMeas() {
	ClearMeasData(temp_meas);
	ClearMeasData(humid_meas);
	ClearMeasData(eCO2_meas);
	ClearMeasData(TVOC_meas);
	ClearMeasData(PM_2_5_meas);
}

void sensors_reboot() {
	digitalWrite(SENS_POW, LOW);
	vTaskDelay(SENS_REBOOT_DELAY / portTICK_PERIOD_MS);
	digitalWrite(SENS_POW, LOW);
	vTaskDelay(SENS_REBOOT_DELAY / portTICK_PERIOD_MS);
}