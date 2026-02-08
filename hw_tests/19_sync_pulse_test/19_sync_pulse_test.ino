/**
 * AS7331 SYNC Pin Pulse Test
 * 
 * Tests SYND (synchronous digital) mode by generating pulses on the
 * SYN pin and verifying the sensor integrates on each edge.
 * 
 * Hardware setup:
 *   - Arduino pin 2 → AS7331 SYN pin
 *   - Arduino pin 3 → AS7331 READY pin (optional, for monitoring)
 * 
 * Pulse rate is slow (500ms) so you can observe on a scope.
 */

#include <Adafruit_AS7331.h>
#include <Wire.h>

#define SYNC_PIN 2
#define READY_PIN 3

Adafruit_AS7331 as7331;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  pinMode(SYNC_PIN, OUTPUT);
  digitalWrite(SYNC_PIN, LOW);
  pinMode(READY_PIN, INPUT);

  Serial.println("AS7331 SYNC pulse test");
  Serial.println("Scope the SYN pin (Arduino pin 2)");
  Serial.println();

  if (!as7331.begin()) {
    Serial.println("FAIL: begin()");
    while (1) delay(100);
  }

  // Configure for SYND mode with 4 edges
  as7331.powerDown(true);
  as7331.setMeasurementMode(AS7331_MODE_SYND);
  as7331.setEdgeCount(4);
  as7331.setGain(AS7331_GAIN_16X);
  as7331.setIntegrationTime(AS7331_TIME_64MS);
  as7331.powerDown(false);

  Serial.println("Config: SYND mode, 4 edges, 16x gain, 64ms time");
  Serial.println("Each measurement needs 4 rising edges on SYN pin");
  Serial.println();
}

void loop() {
  Serial.println("--- Starting measurement ---");
  Serial.println("Generating 4 pulses at 500ms intervals...");
  
  // Start measurement
  as7331.startMeasurement();
  delay(100);

  // Generate 4 pulses, slowly
  for (int i = 1; i <= 4; i++) {
    Serial.print("Pulse ");
    Serial.print(i);
    Serial.print("/4 - READY=");
    Serial.println(digitalRead(READY_PIN));
    
    // Rising edge triggers integration
    digitalWrite(SYNC_PIN, HIGH);
    delay(250);
    digitalWrite(SYNC_PIN, LOW);
    delay(250);
  }

  // Wait for measurement to complete
  delay(200);
  
  Serial.print("READY pin after pulses: ");
  Serial.println(digitalRead(READY_PIN));

  // Read results
  uint16_t uva = as7331.readUVA();
  uint16_t uvb = as7331.readUVB();
  uint16_t uvc = as7331.readUVC();

  Serial.print("UVA=");
  Serial.print(uva);
  Serial.print(" UVB=");
  Serial.print(uvb);
  Serial.print(" UVC=");
  Serial.println(uvc);

  Serial.println();
  delay(3000);
}
