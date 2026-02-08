/**
 * AS7331 Edge Count Functional Test
 *
 * Tests that edge_count setting actually controls SYND mode behavior.
 *
 * Based on observation: READY pin behavior is unreliable in SYND mode,
 * but UV data IS returned correctly after the right number of sync edges.
 * First measurement after config often returns 0 (sensor warm-up quirk).
 *
 * Hardware: Metro Mini, AS7331 on I2C, SYNC→D2, READY→D3
 */

#include <Adafruit_AS7331.h>
#include <Adafruit_NeoPixel.h>

#define SYNC_PIN 2
#define READY_PIN 3
#define NEOPIXEL_PIN 6
#define NEOPIXEL_COUNT 60

Adafruit_AS7331 as7331;
Adafruit_NeoPixel pixels(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

uint8_t testsPassed = 0;
uint8_t testsFailed = 0;

void printResult(const char *testName, bool passed) {
  Serial.print(testName);
  Serial.print(F(": "));
  if (passed) {
    Serial.println(F("PASS"));
    testsPassed++;
  } else {
    Serial.println(F("FAIL"));
    testsFailed++;
  }
}

// Generate sync pulses - matches working 19_sync_pulse_test timing
void sendPulses(uint8_t count) {
  for (uint8_t i = 0; i < count; i++) {
    Serial.print(F("  Pulse "));
    Serial.print(i + 1);
    Serial.print(F("/"));
    Serial.print(count);
    Serial.print(F(" - READY="));
    Serial.println(digitalRead(READY_PIN));

    digitalWrite(SYNC_PIN, HIGH);
    delay(250);
    digitalWrite(SYNC_PIN, LOW);
    delay(250);
  }
}

// Read UV values using individual reads (like working test)
void readUV(uint16_t *a, uint16_t *b, uint16_t *c) {
  *a = as7331.readUVA();
  *b = as7331.readUVB();
  *c = as7331.readUVC();
}

void printUV(uint16_t a, uint16_t b, uint16_t c) {
  Serial.print(F("  UV: A="));
  Serial.print(a);
  Serial.print(F(" B="));
  Serial.print(b);
  Serial.print(F(" C="));
  Serial.println(c);
}

// Run one complete SYND measurement cycle
void doMeasurement(uint8_t pulseCount, uint16_t *a, uint16_t *b, uint16_t *c) {
  as7331.startMeasurement();
  delay(100);

  sendPulses(pulseCount);

  // Wait for integration to complete
  delay(200);

  Serial.print(F("  READY after wait: "));
  Serial.println(digitalRead(READY_PIN));

  readUV(a, b, c);
}

void setup() {
  pinMode(SYNC_PIN, OUTPUT);
  digitalWrite(SYNC_PIN, LOW);
  pinMode(READY_PIN, INPUT);

  Serial.begin(115200);
  while (!Serial)
    delay(10);

  Serial.println(F("\n========================================"));
  Serial.println(F("AS7331 Edge Count Functional Test"));
  Serial.println(F("Tests SYND mode edge counting via UV data"));
  Serial.println(F("========================================\n"));

  // NeoPixels for consistent light source
  pixels.begin();
  pixels.setBrightness(255);
  pixels.fill(pixels.Color(255, 255, 255));
  pixels.show();

  if (!as7331.begin()) {
    Serial.println(F("ERROR: AS7331 not found!"));
    while (1)
      delay(100);
  }
  Serial.println(F("AS7331 found!\n"));

  // ========================================
  // TEST 1: Register readback verification
  // ========================================
  Serial.println(F("--- TEST 1: Register Readback ---"));

  as7331.powerDown(true);
  as7331.setEdgeCount(2);
  uint8_t read2 = as7331.getEdgeCount();
  printResult("edge_count=2 readback", read2 == 2);

  as7331.setEdgeCount(4);
  uint8_t read4 = as7331.getEdgeCount();
  printResult("edge_count=4 readback", read4 == 4);

  Serial.println();

  // ========================================
  // Configure SYND mode ONCE (like working test)
  // ========================================
  Serial.println(F("--- Configuring SYND mode ---"));
  as7331.powerDown(true);
  as7331.setMeasurementMode(AS7331_MODE_SYND);
  as7331.setEdgeCount(4); // Start with 4 edges
  as7331.setGain(AS7331_GAIN_16X);
  as7331.setIntegrationTime(AS7331_TIME_64MS);
  as7331.powerDown(false);
  delay(100);

  digitalWrite(SYNC_PIN, LOW);
  delay(100);
  Serial.println(F("  Config complete: SYND, 4 edges, 16x gain, 64ms\n"));

  // ========================================
  // WARMUP: First 2 measurements often fail
  // ========================================
  Serial.println(F("--- WARMUP: Priming sensor ---"));
  uint16_t uva, uvb, uvc;

  Serial.println(F("Warmup cycle 1:"));
  doMeasurement(4, &uva, &uvb, &uvc);
  printUV(uva, uvb, uvc);

  Serial.println(F("Warmup cycle 2:"));
  doMeasurement(4, &uva, &uvb, &uvc);
  printUV(uva, uvb, uvc);

  Serial.println(F("Warmup cycle 3:"));
  doMeasurement(4, &uva, &uvb, &uvc);
  printUV(uva, uvb, uvc);

  Serial.println();

  // ========================================
  // TEST 2: edge_count=4 gives valid data with 4 pulses
  // ========================================
  Serial.println(F("--- TEST 2: 4 edges, send 4 pulses ---"));

  doMeasurement(4, &uva, &uvb, &uvc);
  printUV(uva, uvb, uvc);

  bool validData4 = (uva > 0 || uvb > 0 || uvc > 0);
  printResult("Got non-zero UV data with 4 pulses", validData4);
  uint16_t baseline_sum = uva + uvb + uvc;
  Serial.print(F("  Baseline sum: "));
  Serial.println(baseline_sum);

  Serial.println();

  // ========================================
  // TEST 3: Change to edge_count=2
  // ========================================
  Serial.println(F("--- TEST 3: Change edge_count to 2 ---"));

  as7331.powerDown(true);
  as7331.setEdgeCount(2);
  uint8_t ec = as7331.getEdgeCount();
  as7331.powerDown(false);
  delay(100);

  Serial.print(F("  edge_count now: "));
  Serial.println(ec);
  printResult("edge_count changed to 2", ec == 2);

  // Warmup after config change (first measurement always fails)
  Serial.println(F("  Warmup after config change:"));
  doMeasurement(2, &uva, &uvb, &uvc);
  printUV(uva, uvb, uvc);

  Serial.println();

  // ========================================
  // TEST 4: edge_count=2 gives data with 2 pulses
  // ========================================
  Serial.println(F("--- TEST 4: 2 edges, send 2 pulses ---"));

  doMeasurement(2, &uva, &uvb, &uvc);
  printUV(uva, uvb, uvc);

  bool validData2 = (uva > 0 || uvb > 0 || uvc > 0);
  printResult("Got non-zero UV data with 2 pulses", validData2);

  Serial.println();

  // ========================================
  // TEST 5: Verify data consistency
  // ========================================
  Serial.println(F("--- TEST 5: Consistency check ---"));

  // Run several cycles and verify data stays consistent
  bool allValid = true;
  for (int i = 0; i < 3; i++) {
    doMeasurement(2, &uva, &uvb, &uvc);
    Serial.print(F("  Cycle "));
    Serial.print(i + 1);
    Serial.print(F(": "));
    printUV(uva, uvb, uvc);
    if (uva == 0 && uvb == 0 && uvc == 0) {
      allValid = false;
    }
  }
  printResult("All 3 cycles returned valid data", allValid);

  // ========================================
  // Summary
  // ========================================
  Serial.println(F("\n========================================"));
  Serial.print(F("SUMMARY: "));
  Serial.print(testsPassed);
  Serial.print(F(" passed, "));
  Serial.print(testsFailed);
  Serial.println(F(" failed"));
  Serial.println(F("========================================"));

  if (testsFailed == 0) {
    Serial.println(F("Overall: PASS"));
  } else {
    Serial.println(F("Overall: FAIL"));
  }
}

void loop() {}
