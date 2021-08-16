/**
 * Implementation based on datasheet from
 * https://www.winsen-sensor.com/d/files/ZE25-O3.pdf
 **/

#include <Arduino.h>
#include "winsen_ze25o3.h"
#include <stdint.h>

/**
 * Initialize to listen on digital port nr rxPin
 **/
WinsenZE25O3::WinsenZE25O3(uint8_t rxPin) {
  SoftwareSerial* serial = new SoftwareSerial{rxPin, 255};
  serial->begin(9600);
  sensorInputStream_ = serial;
}

/**
 * Returns ozone concentration in PPB or -1 if either
 * there's a checksum mismatch or no data was available.
 **/
int WinsenZE25O3::readPPB() {
  int ozonePPB = -1;

  if (sensorInputStream_->available() > 8) {
    byte sensorMessage[9] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    sensorInputStream_->readBytes(sensorMessage, 9);

    if (isChecksumCorrect(sensorMessage)) {
      ozonePPB = sensorMessage[4] * 256 + sensorMessage[5];
    }
  }

  return ozonePPB;
}

/**
 * Check checksum as per datasheet.
 **/
boolean WinsenZE25O3::isChecksumCorrect(byte *sensorMessage) {
  byte checksum = 0;

  for (int i = 1; i < 8; i++) {
    checksum += sensorMessage[i];
  }

  checksum = (~checksum) + 1;

  return checksum == sensorMessage[8];
}
