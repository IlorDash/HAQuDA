/**
 * Implementation based on datasheet from
 * https://www.winsen-sensor.com/d/files/ZE25-O3.pdf
 **/

#include <Arduino.h>
#include "winsen_ze25o3.h"
#include <stdint.h>

#define MSG_LENGTH 9

const uint8_t comMsgQuestion[MSG_LENGTH]
	= {0xFF, 0x01, 0x78, 0x41, 0x00, 0x00, 0x00, 0x00, 0x46}; // payload that needs to be sent to sensor to switch to QUESTION/ANSWER mode
const uint8_t comMsgActive[MSG_LENGTH]
	= {0xFF, 0x01, 0x78, 0x40, 0x00, 0x00, 0x00, 0x00, 0x47}; // payload that needs to be sent to sensor to switch to ACTIVE UPLOAD mode
const uint8_t comMsgRequest[MSG_LENGTH]
	= {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79}; // payload that needs to be sent to sensor to REQUEST actual reading

/**
 * Initialize to listen on digital port rxPin and writt on txPin
 **/

SoftwareSerial O3Serial;

void ZE25O3_init(uint8_t rxPin, uint8_t txPin) {
	O3Serial.begin(9600, SWSERIAL_8N1, rxPin, txPin, false, 255);
}

/**
 * Returns ozone concentration in PPM or -1 if either
 * there's a checksum mismatch or no data was available.
 **/
float ZE25O3_readPPB(uint16_t timeout) {
	float ozonePPM = -1;
	uint32_t O3_time = millis();
	while ((O3Serial.available() < MSG_LENGTH) && ((millis() - O3_time) < timeout)) {
	}

	int temp = O3Serial.available();

	if (O3Serial.available() < MSG_LENGTH) {
		return ozonePPM;
	}

	byte sensorMessage[MSG_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	O3Serial.readBytes(sensorMessage, 9);

	if (sensorMessage[0] != 0xff) {
		return ozonePPM;
	}

	if (isChecksumCorrect(sensorMessage)) {
		ozonePPM = (sensorMessage[4] * 256 + sensorMessage[5]) / 1000;
	}

	return ozonePPM;
}

void ZE25O3_setCommQuestionMode() {
	for (uint8_t i = 0; i < MSG_LENGTH; i++) {
		O3Serial.write(comMsgQuestion[i]);
	}
}

// Requests the sensor to send reading to serial
void ZE25O3_requestSensorReading() {
	for (uint8_t i = 0; i < MSG_LENGTH; i++) {
		O3Serial.write(comMsgRequest[i]);
	}
}

/**
 * Check checksum as per datasheet.
 **/
boolean isChecksumCorrect(byte *sensorMessage) {
	byte checksum = 0;

	for (int i = 1; i < 8; i++) {
		checksum += sensorMessage[i];
	}

	checksum = (~checksum) + 1;

	return checksum == sensorMessage[8];
}

void ZE25O3_clearRxBuff() {
	while (O3Serial.available() > 0) {
		char t = O3Serial.read();
	}
}
