#include "Adafruit_AS7331.h"

Adafruit_AS7331::Adafruit_AS7331() {}

bool Adafruit_AS7331::begin(TwoWire *wire, uint8_t addr) {
  if (_i2c_dev) {
    delete _i2c_dev;
    _i2c_dev = nullptr;
  }

  _i2c_dev = new Adafruit_I2CDevice(addr, wire);
  if (!_i2c_dev->begin()) {
    return false;
  }

  Adafruit_BusIO_Register agen =
      Adafruit_BusIO_Register(_i2c_dev, AS7331_REG_AGEN);
  uint8_t part_id = 0;
  if (!agen.read(&part_id)) {
    return false;
  }

  return (part_id == AS7331_PART_ID);
}
