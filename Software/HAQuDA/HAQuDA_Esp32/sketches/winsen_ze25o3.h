#include <Arduino.h>
#include <SoftwareSerial.h>
#include <stdint.h>

class WinsenZE25O3
{
public:
  WinsenZE25O3(uint8_t rxPin);
  int readPPB();
private:
  Stream *sensorInputStream_;
  boolean isChecksumCorrect(byte *sensorMessage);
};
