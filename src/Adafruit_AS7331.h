#pragma once

#include <Adafruit_BusIO_Register.h>
#include <Arduino.h>
#include <Wire.h>

#define AS7331_DEFAULT_ADDRESS 0x74

#define AS7331_REG_OSR 0x00
#define AS7331_REG_AGEN 0x02

#define AS7331_PART_ID 0x21

class Adafruit_AS7331 {
public:
  Adafruit_AS7331();

  bool begin(TwoWire *wire = &Wire, uint8_t addr = AS7331_DEFAULT_ADDRESS);

private:
  Adafruit_I2CDevice *_i2c_dev = nullptr;
};
