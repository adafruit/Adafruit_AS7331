# AS7331 UV Spectral Sensor - Design Document

## Chip Overview

| Property | Value |
|----------|-------|
| Manufacturer | ams-OSRAM |
| Part Number | AS7331 |
| Package | OLGA16 (3.1×2.0mm) |
| Datasheet | DS001047 v4-00 (2023-Mar-24) |
| Supply Voltage | 2.7V - 3.6V (typ 3.3V) |
| Operating Temp | -40°C to +85°C |
| Interface | I²C Fast Mode (400 kHz) |

### I²C Address
The slave address is configurable via A1 and A0 pins:

| A1 | A0 | 7-bit Address | 8-bit Write | 8-bit Read |
|----|----|--------------:|------------:|-----------:|
| 0  | 0  | 0x74          | 0xE8        | 0xE9       |
| 0  | 1  | 0x75          | 0xEA        | 0xEB       |
| 1  | 0  | 0x76          | 0xEC        | 0xED       |
| 1  | 1  | 0x77          | 0xEE        | 0xEF       |

Address format: `1110_1_A1_A0`

### Part ID Register
| Register | Address | Expected Value | Description |
|----------|---------|----------------|-------------|
| AGEN | 0x02 | **0x21** | DEVID[7:4]=0x2, MUT[3:0]=0x1 |

### Key Features
- Three UV channels with interference filters:
  - **UVA**: 315-410nm (peak 360nm)
  - **UVB**: 280-315nm (peak 300nm)
  - **UVC**: 240-280nm (peak 260nm)
- 16-24 bit ADC resolution
- 12 gain steps (1x to 2048x)
- 15 integration time settings (1ms to 16.384s)
- 4 measurement modes (CMD, CONT, SYNS, SYND)
- Integrated temperature sensor (12-bit, ±10K accuracy)
- Programmable READY pin (push-pull or open-drain)
- External sync via SYN pin
- Power-down and standby modes

---

## Register Map

### Configuration State Registers

| Address | Name | R/W | Width | Default | Description |
|---------|------|-----|-------|---------|-------------|
| 0x00 | OSR | RW | 8 | 0x42 | Operational State Register |
| 0x02 | AGEN | RO | 8 | 0x21 | API Generation / Device ID |
| 0x06 | CREG1 | RW | 8 | 0xA6 | Configuration 1 (Gain/Time) |
| 0x07 | CREG2 | RW | 8 | 0x40 | Configuration 2 (Divider) |
| 0x08 | CREG3 | RW | 8 | 0x50 | Configuration 3 (Mode/Clock) |
| 0x09 | BREAK | RW | 8 | 0x19 | Break Time (inter-measurement) |
| 0x0A | EDGES | RW | 8 | 0x01 | Edge Count (SYND mode) |
| 0x0B | OPTREG | RW | 8 | 0x73 | Option Register |

### Measurement State Registers

| Address | Name | R/W | Width | Description |
|---------|------|-----|-------|-------------|
| 0x00 | OSR+STATUS | RW/RO | 16 | OSR (byte 0) + STATUS (byte 1) |
| 0x01 | TEMP | RO | 16 | Temperature (12-bit value) |
| 0x02 | MRES1 | RO | 16 | UVA Channel Result |
| 0x03 | MRES2 | RO | 16 | UVB Channel Result |
| 0x04 | MRES3 | RO | 16 | UVC Channel Result |
| 0x05 | OUTCONV_L | RO | 16 | Conv Time Low (SYND only) |
| 0x06 | OUTCONV_H | RO | 16 | Conv Time High (SYND only) |

**Note**: Multi-byte registers are LSB first.

---

## Bit Field Details

### OSR (0x00) - Operational State Register

```cpp
// RegisterBits notation: RegisterBits<reg_addr, bit_pos, width>
RegisterBits<0x00, 7, 1> OSR_SS;       // Start/Stop measurement
RegisterBits<0x00, 6, 1> OSR_PD;       // Power Down enable
RegisterBits<0x00, 3, 1> OSR_SW_RES;   // Software Reset (write 1)
RegisterBits<0x00, 0, 3> OSR_DOS;      // Device Operational State
```

| Bits | Name | Access | Default | Description |
|------|------|--------|---------|-------------|
| 7 | SS | RW | 0 | Start/Stop: 0=stop, 1=start measurement |
| 6 | PD | RW | 1 | Power Down: 0=off, 1=on |
| 5:4 | - | - | 0 | Reserved |
| 3 | SW_RES | RW | 0 | Software Reset (write 1 to reset, reads as 0) |
| 2:0 | DOS | RW | 010 | Device Operational State |

**DOS Values:**
| Value | State |
|-------|-------|
| 0b00x | NOP (no change) |
| 0b010 | Configuration State |
| 0b011 | Measurement State |
| 0b1xx | NOP (no change) |

**Common OSR Write Values:**
| Value | Action |
|-------|--------|
| 0x42 | Config state, power down ON |
| 0x02 | Config state, power down OFF |
| 0x03 | Measurement state, power down OFF |
| 0x83 | Measurement state, START measurement |
| 0x43 | Measurement state, power down ON |
| 0xC3 | Measurement + START + power down (auto startup) |
| 0x0A | Software Reset |

---

### AGEN (0x02) - API Generation Register (Read-Only)

```cpp
RegisterBits<0x02, 4, 4> AGEN_DEVID;   // Device ID = 0x2
RegisterBits<0x02, 0, 4> AGEN_MUT;     // Mutation = 0x1
```

| Bits | Name | Value | Description |
|------|------|-------|-------------|
| 7:4 | DEVID | 0x2 | Device ID (AS7331) |
| 3:0 | MUT | 0x1 | Control register bank mutation |

**Expected value: 0x21** - Use this to verify chip presence.

---

### CREG1 (0x06) - Configuration Register 1

```cpp
RegisterBits<0x06, 4, 4> CREG1_GAIN;   // Gain setting
RegisterBits<0x06, 0, 4> CREG1_TIME;   // Integration time
```

| Bits | Name | Access | Default | Description |
|------|------|--------|---------|-------------|
| 7:4 | GAIN | RW | 0xA (2x) | Gain setting (0-11) |
| 3:0 | TIME | RW | 0x6 (64ms) | Integration time (0-15) |

**Default: 0xA6** (GAIN=2x, TIME=64ms)

---

### CREG2 (0x07) - Configuration Register 2

```cpp
RegisterBits<0x07, 6, 1> CREG2_EN_TM;   // Enable time measurement
RegisterBits<0x07, 3, 1> CREG2_EN_DIV;  // Enable divider
RegisterBits<0x07, 0, 3> CREG2_DIV;     // Divider value
```

| Bits | Name | Access | Default | Description |
|------|------|--------|---------|-------------|
| 7 | - | - | 0 | Reserved |
| 6 | EN_TM | RW | 1 | Enable conv time measurement (SYND) |
| 5:4 | - | - | 0 | Reserved |
| 3 | EN_DIV | RW | 0 | Enable digital divider |
| 2:0 | DIV | RW | 0 | Divider: factor = 2^(1+DIV) |

**Default: 0x40**

---

### CREG3 (0x08) - Configuration Register 3

```cpp
RegisterBits<0x08, 6, 2> CREG3_MMODE;   // Measurement mode
RegisterBits<0x08, 4, 1> CREG3_SB;      // Standby enable
RegisterBits<0x08, 3, 1> CREG3_RDYOD;   // READY pin open-drain
RegisterBits<0x08, 0, 2> CREG3_CCLK;    // Internal clock frequency
```

| Bits | Name | Access | Default | Description |
|------|------|--------|---------|-------------|
| 7:6 | MMODE | RW | 01 | Measurement mode |
| 5 | - | - | 0 | Reserved |
| 4 | SB | RW | 1 | Standby: 0=off, 1=on |
| 3 | RDYOD | RW | 0 | READY output: 0=push-pull, 1=open-drain |
| 2 | - | - | 0 | Reserved |
| 1:0 | CCLK | RW | 00 | Clock frequency |

**Default: 0x50** (CMD mode, Standby ON, 1.024MHz)

---

### BREAK (0x09) - Break Time Register

```cpp
RegisterBits<0x09, 0, 8> BREAK_TIME;    // Break time value
```

| Bits | Name | Access | Default | Description |
|------|------|--------|---------|-------------|
| 7:0 | BREAK | RW | 0x19 | Break time = value × 8µs (0-2040µs) |

**Default: 0x19 = 25 × 8µs = 200µs**

---

### EDGES (0x0A) - Edge Count Register (SYND mode)

```cpp
RegisterBits<0x0A, 0, 8> EDGES_COUNT;   // Edge count
```

| Bits | Name | Access | Default | Description |
|------|------|--------|---------|-------------|
| 7:0 | EDGES | RW | 0x01 | Number of SYN edges (1-255, 0→1) |

---

### OPTREG (0x0B) - Option Register

```cpp
RegisterBits<0x0B, 0, 1> OPTREG_INIT_IDX;  // Index initialization
```

| Bits | Name | Access | Default | Description |
|------|------|--------|---------|-------------|
| 7:1 | - | - | 0111001 | Reserved (write 0000000b) |
| 0 | INIT_IDX | RW | 1 | Auto-init read address |

**INIT_IDX:**
- 0 = Manual addressing only (for simple I²C masters)
- 1 = Auto-init to 0x02 (Measurement) or 0x00 (Config) on read

---

### STATUS (0x00 MSB in Measurement State)

```cpp
RegisterBits<0x00, 15, 1> STATUS_OUTCONVOF;    // OUTCONV overflow
RegisterBits<0x00, 14, 1> STATUS_MRESOF;       // Result overflow
RegisterBits<0x00, 13, 1> STATUS_ADCOF;        // ADC overflow
RegisterBits<0x00, 12, 1> STATUS_LDATA;        // Lost data
RegisterBits<0x00, 11, 1> STATUS_NDATA;        // New data available
RegisterBits<0x00, 10, 1> STATUS_NOTREADY;     // Not ready (measuring)
RegisterBits<0x00,  9, 1> STATUS_STANDBYSTATE; // Current standby state
RegisterBits<0x00,  8, 1> STATUS_POWERSTATE;   // Current power state
```

| Bit | Name | Description |
|-----|------|-------------|
| 7 (15) | OUTCONVOF | OUTCONV 24-bit counter overflow |
| 6 (14) | MRESOF | Any channel result register overflow |
| 5 (13) | ADCOF | ADC input overdrive (reduce gain!) |
| 4 (12) | LDATA | Lost data (results overwritten) - cleared on STATUS read |
| 3 (11) | NDATA | New data available - cleared on any result read |
| 2 (10) | NOTREADY | 1=measurement in progress, 0=ready |
| 1 (9) | STANDBYSTATE | Actual standby state |
| 0 (8) | POWERSTATE | Actual power down state |

---

## Typed Enumerations

### Gain Settings (CREG1:GAIN)

```cpp
typedef enum {
  AS7331_GAIN_2048X = 0,   // Highest sensitivity
  AS7331_GAIN_1024X = 1,
  AS7331_GAIN_512X  = 2,
  AS7331_GAIN_256X  = 3,
  AS7331_GAIN_128X  = 4,
  AS7331_GAIN_64X   = 5,
  AS7331_GAIN_32X   = 6,
  AS7331_GAIN_16X   = 7,
  AS7331_GAIN_8X    = 8,
  AS7331_GAIN_4X    = 9,
  AS7331_GAIN_2X    = 10,  // Default
  AS7331_GAIN_1X    = 11,  // Lowest sensitivity
} as7331_gain_t;
```

**Note**: Higher clock frequencies limit available gains (see Gain vs Clock table below).

### Integration Time (CREG1:TIME)

```cpp
typedef enum {
  AS7331_TIME_1MS     = 0,   // 1024 clocks, 10-bit
  AS7331_TIME_2MS     = 1,   // 2048 clocks, 11-bit
  AS7331_TIME_4MS     = 2,   // 4096 clocks, 12-bit
  AS7331_TIME_8MS     = 3,   // 8192 clocks, 13-bit
  AS7331_TIME_16MS    = 4,   // 16384 clocks, 14-bit
  AS7331_TIME_32MS    = 5,   // 32768 clocks, 15-bit
  AS7331_TIME_64MS    = 6,   // 65536 clocks, 16-bit (default)
  AS7331_TIME_128MS   = 7,   // 131072 clocks, 17-bit
  AS7331_TIME_256MS   = 8,   // 262144 clocks, 18-bit
  AS7331_TIME_512MS   = 9,   // 524288 clocks, 19-bit
  AS7331_TIME_1024MS  = 10,  // 1048576 clocks, 20-bit
  AS7331_TIME_2048MS  = 11,  // 2097152 clocks, 21-bit
  AS7331_TIME_4096MS  = 12,  // 4194304 clocks, 22-bit
  AS7331_TIME_8192MS  = 13,  // 8388608 clocks, 23-bit
  AS7331_TIME_16384MS = 14,  // 16777216 clocks, 24-bit
  // 15 = same as 0 (1ms)
} as7331_time_t;
```

### Measurement Mode (CREG3:MMODE)

```cpp
typedef enum {
  AS7331_MODE_CONT = 0,  // Continuous measurement
  AS7331_MODE_CMD  = 1,  // Single command (default)
  AS7331_MODE_SYNS = 2,  // Sync start via SYN pin
  AS7331_MODE_SYND = 3,  // Sync start+stop via SYN pin
} as7331_mode_t;
```

### Clock Frequency (CREG3:CCLK)

```cpp
typedef enum {
  AS7331_CLOCK_1MHZ = 0,  // 1.024 MHz (default)
  AS7331_CLOCK_2MHZ = 1,  // 2.048 MHz
  AS7331_CLOCK_4MHZ = 2,  // 4.096 MHz
  AS7331_CLOCK_8MHZ = 3,  // 8.192 MHz
} as7331_clock_t;
```

### Device Operational State (OSR:DOS)

```cpp
typedef enum {
  AS7331_STATE_CONFIG = 0b010,  // Configuration state
  AS7331_STATE_MEAS   = 0b011,  // Measurement state
} as7331_state_t;
```

### Divider (CREG2:DIV)

```cpp
typedef enum {
  AS7331_DIV_2   = 0,  // Divide by 2
  AS7331_DIV_4   = 1,  // Divide by 4
  AS7331_DIV_8   = 2,  // Divide by 8
  AS7331_DIV_16  = 3,  // Divide by 16
  AS7331_DIV_32  = 4,  // Divide by 32
  AS7331_DIV_64  = 5,  // Divide by 64
  AS7331_DIV_128 = 6,  // Divide by 128
  AS7331_DIV_256 = 7,  // Divide by 256
} as7331_divider_t;
```

---

## Gain vs Clock Frequency Compatibility

Not all gain settings work at higher clock frequencies:

| CCLK | 1.024 MHz | 2.048 MHz | 4.096 MHz | 8.192 MHz |
|------|-----------|-----------|-----------|-----------|
| GAIN=0 | 2048x | — | — | — |
| GAIN=1 | 1024x | 1024x | — | — |
| GAIN=2 | 512x | 512x | 512x | — |
| GAIN=3 | 256x | 256x | 256x | 256x |
| GAIN=4 | 128x | 128x | 128x | 64x |
| GAIN=5 | 64x | 64x | 64x | 64x |
| GAIN=6 | 32x | 32x | 32x | 16x |
| GAIN=7 | 16x | 16x | 16x | 16x |
| GAIN=8 | 8x | 8x | 8x | 4x |
| GAIN=9 | 4x | 4x | 4x | 4x |
| GAIN=10 | 2x | 2x | 2x | 1x |
| GAIN=11 | 1x | 1x | 1x | 1x |

---

## Multi-Byte Read Order

**All multi-byte values are LSB first.**

Reading measurement results:
1. Address 0x02: MRES1_L, MRES1_H (UVA)
2. Address 0x03: MRES2_L, MRES2_H (UVB)
3. Address 0x04: MRES3_L, MRES3_H (UVC)

OUTCONV (24-bit, SYND mode only):
1. Address 0x05: OUTCONV[7:0], OUTCONV[15:8]
2. Address 0x06: OUTCONV[23:16], 0x00

Temperature (12-bit in 16-bit register):
- Bits 11:0 contain the temperature value
- Bits 15:12 are zero

---

## Timing Requirements

### Startup Times
| Parameter | Typical | Maximum | Condition |
|-----------|---------|---------|-----------|
| Power-down startup (TSTARTPD) | 1.2 ms | 2 ms | OSR:PD 1→0 |
| Standby startup (TSTARTSB) | 4 µs | 5 µs | After standby exit |

### SYN Pin Timing
| Parameter | Min | Unit | Notes |
|-----------|-----|------|-------|
| SYN pulse width | 3 | 1/fCLK | Minimum recognizable pulse |
| SYN trigger delay | — | 3 1/fCLK max | From falling edge to measurement start |

### Conversion Times (at 1.024 MHz clock)
| TIME Setting | Conversion Time | ADC Resolution |
|--------------|-----------------|----------------|
| 0 | 1 ms | 10-bit |
| 6 (default) | 64 ms | 16-bit |
| 10 | 1024 ms | 20-bit |
| 14 | 16384 ms | 24-bit |

### Break Time (TBREAK)
- Range: 0 to 2040 µs (in 8 µs steps)
- Default: 200 µs (0x19)
- Value 0 = minimum 3 clock cycles

### I²C Timing
| Parameter | Min | Max | Unit |
|-----------|-----|-----|------|
| Clock frequency | — | 400 | kHz |
| SCL high pulse | 0.6 | — | µs |
| SCL low pulse | 1.3 | — | µs |
| Start hold time | 0.6 | — | µs |
| Stop setup time | 0.6 | — | µs |
| Data setup time | 0.1 | — | µs |
| Data hold (master) | 0.02 | — | µs |
| Data hold (slave) | 0.3 | 0.9 | µs |
| Bus free time | 1.3 | — | µs |

---

## Temperature Calculation

The TEMP register contains a 12-bit value. To convert to degrees Celsius:

```cpp
float temperature_c = (temp_raw * 0.05f) - 66.9f;
```

Reference point: TEMP = 0x922 (2338 dec) = 50°C

Temperature measurement is available in CONT, CMD, and SYNS modes.
In SYND mode, temperature is only available if EN_TM=1 and OUTCONV > 4096.

---

## Quirks and Gotchas

### 1. State Machine Behavior
- **Configuration registers (CREG1-3, BREAK, EDGES, OPTREG) can only be written in Configuration state**
- **Measurement results can only be read in Measurement state**
- Switching from Measurement → Configuration state clears all result registers
- OSR register is always accessible

### 2. Power-Up Sequence
- After power-on or software reset, device is in Configuration state with Power Down ON
- Must write OSR:PD=0 to exit power down before measuring
- Wait for TSTARTPD (1.2ms typ, 2ms max) after exiting power down

### 3. Gain Limitations
- Higher clock frequencies disable higher gain settings
- At 8.192 MHz, maximum gain is 256x (GAIN=3)
- Using invalid gain for clock frequency will use the effective gain from the table

### 4. Integration Time > 64ms
- For TIME > 6, ADC resolution exceeds 16 bits
- Use the divider (EN_DIV=1) to access upper bits
- Without divider, only lower 16 bits are available

### 5. SYND Mode Special Cases
- Set EN_TM=1 to get temperature readings in SYND mode
- OUTCONV must be > 4096 clocks for valid temperature
- EDGES register value of 0 is treated as 1

### 6. I²C Communication During Measurement
- **Do NOT communicate via I²C during conversion** (causes measurement distortion)
- Use BREAK time for data transfer
- READY pin indicates safe time to read (high = ready)

### 7. Result Register Locking
- Reading any result register locks all results until I²C STOP
- This prevents partial updates during multi-byte reads
- NDATA flag clears on read of STATUS or any result register
- LDATA flag only clears on STATUS read

### 8. READY Pin Wiring
- For multiple AS7331 on same READY line: use open-drain mode (RDYOD=1) with pull-up
- READY stays low while ANY device is measuring

### 9. Address Pointer Wrap
- In Measurement state, address auto-wraps from 0x04 (or 0x06 if EN_TM=1) back to 0x02
- Does NOT wrap if address was set above valid range

### 10. External Resistor (REXT)
- Must use 3.3 MΩ ±1% with TCR ≤ 50ppm/K
- Critical for accurate measurements

---

## Function Map (Proposed Public API)

```cpp
class Adafruit_AS7331 {
public:
  // Initialization
  bool begin(uint8_t addr = AS7331_DEFAULT_ADDRESS, TwoWire *wire = &Wire);
  bool reset();                         // Software reset
  uint8_t getDeviceID();               // Read AGEN register (expect 0x21)

  // State Management
  bool setConfigurationState();         // Enter config mode (DOS=010)
  bool setMeasurementState();           // Enter measurement mode (DOS=011)
  bool setPowerDown(bool enable);       // Control power down
  bool setStandby(bool enable);         // Control standby

  // Configuration (only in Config state)
  bool setGain(as7331_gain_t gain);
  bool setIntegrationTime(as7331_time_t time);
  bool setMeasurementMode(as7331_mode_t mode);
  bool setClockFrequency(as7331_clock_t clock);
  bool setBreakTime(uint8_t time_8us);  // 0-255 (units of 8µs)
  bool setDivider(bool enable, as7331_divider_t div = AS7331_DIV_2);
  bool setReadyPinMode(bool openDrain);

  // Measurement (only in Measurement state)
  bool startMeasurement();              // Set SS=1
  bool stopMeasurement();               // Set SS=0
  bool isReady();                       // Check READY pin or STATUS:NOTREADY
  
  // Read Results
  bool readAllChannels(uint16_t *uva, uint16_t *uvb, uint16_t *uvc);
  uint16_t readUVA();                   // MRES1
  uint16_t readUVB();                   // MRES2  
  uint16_t readUVC();                   // MRES3
  float readTemperature();              // TEMP → °C
  
  // Status
  uint8_t readStatus();
  bool hasNewData();                    // STATUS:NDATA
  bool hasLostData();                   // STATUS:LDATA
  bool hasOverflow();                   // STATUS:MRESOF or ADCOF

  // One-shot convenience (handles state transitions)
  bool oneShot(uint16_t *uva, uint16_t *uvb, uint16_t *uvc);

protected:
  bool writeRegister(uint8_t reg, uint8_t value);
  uint8_t readRegister(uint8_t reg);
  bool readRegisters(uint8_t reg, uint8_t *buffer, uint8_t len);
};
```

---

## Initialization Sequence

### Basic Startup
```cpp
// 1. After power-on, device is in Config state with PD=1
// 2. Read AGEN to verify device (expect 0x21)
// 3. Configure measurement parameters
// 4. Exit power down and enter measurement state
// 5. Start measurement

writeRegister(0x06, 0xA6);   // CREG1: GAIN=2x, TIME=64ms
writeRegister(0x08, 0x50);   // CREG3: CMD mode, SB=1, 1MHz
writeRegister(0x00, 0x83);   // OSR: SS=1, PD=0, DOS=011 (start measurement)
delay(2);                     // Wait for TSTARTPD
// Wait for READY pin high or poll STATUS:NOTREADY
```

### Reading Results
```cpp
// After READY goes high:
writeRegister(0x00, 0x02);   // Stay in config to read AGEN, or...

// In measurement state, burst read from 0x02:
uint8_t buf[6];
readRegisters(0x02, buf, 6);
uint16_t uva = buf[0] | (buf[1] << 8);
uint16_t uvb = buf[2] | (buf[3] << 8);
uint16_t uvc = buf[4] | (buf[5] << 8);
```

---

## References

- [AS7331 Datasheet (DS001047 v4-00)](https://look.ams-osram.com/m/1856fd2c69c35605/original/AS7331-Spectral-UVA-B-C-Sensor.pdf)
- ams-OSRAM Application Notes
