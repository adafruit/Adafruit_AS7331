#pragma once

#include <Adafruit_BusIO_Register.h>
#include <Arduino.h>
#include <Wire.h>

#define AS7331_DEFAULT_ADDRESS 0x74

#define AS7331_REG_OSR 0x00
#define AS7331_REG_TEMP 0x01
#define AS7331_REG_AGEN 0x02
#define AS7331_REG_MRES1 0x02
#define AS7331_REG_MRES2 0x03
#define AS7331_REG_MRES3 0x04
#define AS7331_REG_CREG1 0x06
#define AS7331_REG_CREG3 0x08

#define AS7331_PART_ID 0x21

typedef enum {
  AS7331_GAIN_2048X = 0,
  AS7331_GAIN_1024X = 1,
  AS7331_GAIN_512X = 2,
  AS7331_GAIN_256X = 3,
  AS7331_GAIN_128X = 4,
  AS7331_GAIN_64X = 5,
  AS7331_GAIN_32X = 6,
  AS7331_GAIN_16X = 7,
  AS7331_GAIN_8X = 8,
  AS7331_GAIN_4X = 9,
  AS7331_GAIN_2X = 10,
  AS7331_GAIN_1X = 11,
} as7331_gain_t;

typedef enum {
  AS7331_TIME_1MS = 0,
  AS7331_TIME_2MS = 1,
  AS7331_TIME_4MS = 2,
  AS7331_TIME_8MS = 3,
  AS7331_TIME_16MS = 4,
  AS7331_TIME_32MS = 5,
  AS7331_TIME_64MS = 6,
  AS7331_TIME_128MS = 7,
  AS7331_TIME_256MS = 8,
  AS7331_TIME_512MS = 9,
  AS7331_TIME_1024MS = 10,
  AS7331_TIME_2048MS = 11,
  AS7331_TIME_4096MS = 12,
  AS7331_TIME_8192MS = 13,
  AS7331_TIME_16384MS = 14,
} as7331_time_t;

typedef enum {
  AS7331_MODE_CONT = 0,
  AS7331_MODE_CMD = 1,
  AS7331_MODE_SYNS = 2,
  AS7331_MODE_SYND = 3,
} as7331_mode_t;

class Adafruit_AS7331 {
public:
  Adafruit_AS7331();

  bool begin(TwoWire *wire = &Wire, uint8_t addr = AS7331_DEFAULT_ADDRESS);

  bool powerDown(bool pd);
  bool setMeasurementMode(as7331_mode_t mode);

  bool setGain(as7331_gain_t gain);
  as7331_gain_t getGain(void);

  bool setIntegrationTime(as7331_time_t time);
  as7331_time_t getIntegrationTime(void);

  uint16_t readUVA(void);
  uint16_t readUVB(void);
  uint16_t readUVC(void);
  bool readAllUV(uint16_t *uva, uint16_t *uvb, uint16_t *uvc);

  float readTemperature(void);
  bool isDataReady(void);

private:
  bool readRegister(uint8_t reg, uint8_t *value);
  bool readRegister(uint8_t reg, uint16_t *value);
  bool readRegisters(uint8_t reg, uint8_t *buffer, uint8_t len);

  Adafruit_I2CDevice *_i2c_dev = nullptr;
};
