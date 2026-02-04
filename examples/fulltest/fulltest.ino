#include <Adafruit_AS7331.h>

Adafruit_AS7331 as7331;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  if (!as7331.begin()) {
    Serial.println("AS7331 not found");
    while (1) {
      delay(10);
    }
  }

  as7331.powerDown(true);
  as7331.setGain(AS7331_GAIN_2X);
  as7331.setIntegrationTime(AS7331_TIME_64MS);
  as7331.setMeasurementMode(AS7331_MODE_CONT);
  as7331.powerDown(false);
}

void loop() {
  uint16_t uva = 0;
  uint16_t uvb = 0;
  uint16_t uvc = 0;

  if (as7331.readAllUV(&uva, &uvb, &uvc)) {
    Serial.print("UVA: ");
    Serial.print(uva);
    Serial.print("  UVB: ");
    Serial.print(uvb);
    Serial.print("  UVC: ");
    Serial.print(uvc);
    Serial.print("  Temp: ");
    Serial.println(as7331.readTemperature(), 2);
  }

  delay(500);
}
