# Arduino Repeater Controller v9.0

A simple yet powerful Arduino-based repeater controller for FM radios (like Motorola GP/GM300), featuring dual activation modes, hysteresis, non-blocking timing, and various beacon and courtesy tone functions.
Controll a UHF emergency repeater (or any other repeater) with Arduino Board

Hardware details, here:
https://yo3hjv.blogspot.com/2016/02/low-profile-duplex-uhf-repeater-for-ham.html?m=0




## Features

### Core Functionality
- **Dual Activation Modes**: Choose between RSSI-based or Carrier Detect-based repeater activation
  - **Runtime Mode Selection**: Select mode at power-on via pin 8 (LOW = RSSI mode, HIGH = Carrier Detect mode)
  - **No Recompilation Needed**: Change modes by simply connecting/disconnecting pin 8 to ground

### Signal Processing
- **Improved Hysteresis**: Dual thresholds prevent erratic toggling
  - RSSI High threshold (default: 12) to activate repeater
  - RSSI Low threshold (default: 9) to deactivate repeater
- **Non-blocking Timing**: Uses millis() instead of delays for responsive operation
  - timeOpen (default: 120ms): Time signal must be present to activate
  - timeSustain (default: 250ms): Time signal must be absent to deactivate

### Beacons and Tones
- **Beacon 1**: CW identifier transmitted at fixed intervals
  - Configurable interval in minutes
  - Includes callsign, QTH locator, and other customizable messages
  - Can be manually triggered via button
- **Beacon 2**: Four-tone sequence played during active QSOs when beacon timer expires
- **Courtesy Tones**: Indicate battery voltage status with different tones
  - Very low battery: 740Hz for 300ms (< 10.5V)
  - Low battery: 1680Hz + 1040Hz sequence (10.5V - 11.5V)
  - Normal operation: 1700Hz for 90ms (11.5V - 13.6V)
  - Over voltage: 1880Hz + 2480Hz sequence (> 13.6V)

### Monitoring
- **Serial Output**: Displays RSSI values when in RSSI mode
- **Battery Monitoring**: Continuously monitors battery voltage for courtesy tones

## Hardware Connections

### Arduino Pin Assignments

| Pin | Function | Description |
|-----|----------|-------------|
| A0  | VccBatteryPin | !!!!!!!!!!!!!!!!!!!Battery voltage monitoring through voltage divider !!!!!!!!!!!!!!!!!!! |
| A2  | RssiPin | RSSI input from receiver (max.5V)!!!! |
| D2  | TxLedPin | TX LED indicator |
| D4  | BeaconPin | Manual beacon trigger (active LOW) |
| D5  | CW_pin | Audio output for beacons and courtesy tones |
| D6  | CarDetPin | Carrier detect input (active LOW) |
| D8  | ModeSelectPin | Mode selection at startup (LOW = RSSI, HIGH = Carrier Detect) |
| D12 | PttPin | PTT output to transmitter |

### Wiring Diagram

```
                                  +------------+
                                  |            |
Battery Voltage Divider --------- | A0         |
                                  |            |
RSSI from Receiver -------------- | A2         |
                                  |            |
TX LED Indicator ---------------- | D2         |
                                  |            |
Manual Beacon Switch ------------ | D4         |
                                  |            |
Audio Output -------------------- | D5    ARD  |
                                  |      UINO  |
Carrier Detect Input ------------ | D6         |
                                  |            |
Mode Selection Switch ----------- | D8         |
                                  |            |
PTT to Transmitter -------------- | D12        |
                                  |            |
                                  +------------+
```

## Configuration Variables

### Key Variables to Adjust

#### Repeater Activation Parameters
```cpp
// RSSI thresholds with hysteresis
int RssiThrsHigh = 12;                // High threshold to activate repeater
int RssiThrsLow = 9;                  // Low threshold to deactivate repeater

// Timing constants for hysteresis
const unsigned long timeOpen = 120;     // Time in ms signal must be present to activate
const unsigned long timeSustain = 250;  // Time in ms signal must be absent to deactivate
```

#### Beacon Settings
```cpp
// Beacon timing
const long Beacon1_timer = 7;             // Beacon 1 interval in minutes

// CW settings
#define  DOTLEN         (100)      // Length of a morse code 'dot' in milliseconds
#define  DASHLEN        (DOTLEN*3) // Length of a morse code 'dash'
#define  CW_PITCH       (700)      // CW pitch in Hz

// Beacon messages
#define  RPT_call       "YOURCALL"  // Your callsign here
#define  RPT_loc        "KN00XX"    // Your QTH locator here
#define  RPT_ctcss      "CTCSS 88.5 HZ"  // CTCSS info if used
#define  END_msg        "QRV"       // End message
```

#### Debug Options
```cpp
#define RssiDebug          // Uncomment to see RSSI values on serial when in RSSI mode
```

## Installation and Setup

1. Connect your Arduino according to the pin assignments above
2. Adjust the configuration variables in the code to match your requirements:
   - Set your callsign and QTH locator
   - Adjust RSSI thresholds based on your receiver's characteristics
   - Set beacon timing as needed
3. Upload the code to your Arduino
4. Connect pin 8 to ground for RSSI mode or leave it floating for Carrier Detect mode
5. Power on the system

## Tuning Tips

### RSSI Mode
1. Monitor the RSSI values via serial output
2. Adjust RssiThrsHigh and RssiThrsLow based on observed values:
   - RssiThrsHigh should be set to activate on valid signals
   - RssiThrsLow should be set to avoid premature deactivation
   - The difference between thresholds creates hysteresis

### Timing Constants
1. Increase timeOpen if the repeater activates too easily on noise
2. Increase timeSustain if the repeater drops out too quickly during pauses

## Credits
Original code by Adrian Florescu YO3HJV with CW generation routines by Mark VandeWettering K6HX.
Enhanced with hysteresis and runtime mode selection.

## License
This code is released under "Beerware License" (buy the original author a beer when you see him).
