#include <Arduino.h>
#include <SoftwareSerial.h>
#include <stdint.h>

void ZE25O3_init(uint8_t rxPin, uint8_t txPin);
float ZE25O3_readPPB(uint16_t timeout);
void ZE25O3_setCommQuestionMode();
void ZE25O3_requestSensorReading();
void ZE25O3_clearRxBuff();
boolean isChecksumCorrect(byte *sensorMessage);