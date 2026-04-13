# Firmware — Implementation & Usage Guide

This document describes how the Unified Pin Control firmware is structured, how to add board support, and how to build and flash it.

---

## Overview

The firmware turns any supported microcontroller into a **serial-controlled GPIO bridge**. 
A host (web app via WebSerial) sends plain-text commands; the firmware configures pins, reads/writes them, and streams state changes back — all without any hardcoded board knowledge on the host side.

Key design goals:
- **Zero dynamic allocation** — no `String` objects, no heap fragmentation
- **Non-blocking** — `loop()` never calls `delay()`
- **Board-agnostic host** — the web app learns everything from `GET_DEF` at connect time
- **Delta reporting** — only changed pin values are transmitted, keeping serial traffic minimal

---

## File Structure

```
firmware/
├── main.cpp                  Entry point — setup() and loop()
├── board_caps.h              Shared CAP_* bitmask flags and PinDef struct
├── board_def.h               Auto-selects the correct board header
├── boards/
│   ├── esp32-esp32s3dev.h    Pin table for ESP32-S3 Dev Board
│   └── rp2040-rpipico.h      Pin table for Raspberry Pi Pico (RP2040)
├── pin_manager.h / .cpp      Pin state tracking, mode application, NVS persistence
└── serial_handler.h / .cpp   Serial receive loop, command dispatch, response formatting
```

---

## How It Works

### 1. Boot
`PinManager::init()` loads any previously saved pin configuration from NVS (ESP32) and re-applies it to hardware. `SerialHandler::init()` clears the receive buffer.

### 2. Main Loop
`loop()` does two things on every iteration:

1. **Serial receive** — `SerialHandler::update()` reads one character at a time into a 128-byte static buffer. When a `\n` is received the complete line is dispatched to the command handler.
2. **Delta reporting** — every `REPORT_INTERVAL_MS` (default 1000 ms) the firmware reads all active pins and emits a `STAT:` line **only if at least one value changed** since the last report.

### 3. Command Dispatch
Incoming lines are tokenised with `strtok()` — no JSON parsing, no dynamic allocation. The first token is the command name; remaining tokens are arguments. See `serial_handler.cpp → dispatch()`.

### 4. Pin State Cache
`pin_manager.cpp` maintains a `PinState` array (one entry per board pin) that tracks:
- `mode` — current `PinMode_t` (NONE, IN, IN\_UP, IN\_DN, ADC, TOUCH, OUT, OUT\_OD, PWM, DAC)
- `value` — last written value (output pins) or last read value (input pins)
- `res` — PWM/ADC bit resolution
- `freq` — PWM frequency

Before applying any hardware change, the manager checks whether the mode or value actually differs from the cached state. Redundant `pinMode()` / `digitalWrite()` / `ledcWrite()` calls are skipped.

---

## Board Support

### How Board Selection Works

`board_def.h` is a thin selector that reads the build target macro set by PlatformIO/Arduino IDE and includes the matching header from `firmware/boards/`:

```cpp
#if defined(ARDUINO_ESP32S3_DEV) || defined(ARDUINO_LOLIN_S3_MINI)
  #include "boards/esp32-esp32s3dev.h"
#elif defined(ARDUINO_RASPBERRY_PI_PICO)
  #include "boards/rp2040-rpipico.h"
#else
  #error "Unsupported board"
#endif
```

If the wrong board is selected in the IDE, the firmware will **fail at compile time** with a clear `#error` rather than silently misbehaving at runtime.

### Adding a New Board

1. Create `firmware/boards/<vendor>-<boardname>.h`
2. Inside, define:
   - `BOARD_NAME` — human-readable string shown in `BINFO` responses
   - `BOARD_ID` — short machine-readable slug (e.g. `"mega2560"`)
   - Include `"../board_caps.h"` for shared types
   - `static const PinDef BOARD_PINS[]` — one entry per usable pin with a `name` string and `caps` bitmask
   - `static const uint8_t PIN_COUNT` — `sizeof(BOARD_PINS) / sizeof(BOARD_PINS[0])`
   - `inline int pinNameToNumber(const char* name)` — converts a pin label string to an Arduino integer pin number
3. Add a `#elif` block in `board_def.h`

**Template:**
```cpp
#pragma once
#define BOARD_NAME "My Board"
#define BOARD_ID   "myboard"
#include "../board_caps.h"

static const PinDef BOARD_PINS[] = {
  {"D2", CAP_IN|CAP_IN_UP|CAP_OUT|CAP_PWM},
  {"A0", CAP_IN|CAP_IN_UP|CAP_ADC|CAP_OUT},
  // ...
};

static const uint8_t PIN_COUNT = sizeof(BOARD_PINS) / sizeof(BOARD_PINS[0]);

inline int pinNameToNumber(const char* name) {
  if (name[0] == 'D') return atoi(name + 1);
  if (name[0] == 'A') return A0 + atoi(name + 1);
  return atoi(name);
}
```

### Capability Flags (`board_caps.h`)

| Flag | Bit | Meaning |
|------|-----|---------|
| `CAP_IN` | 0 | Digital INPUT |
| `CAP_IN_UP` | 1 | INPUT\_PULLUP |
| `CAP_IN_DN` | 2 | INPUT\_PULLDOWN |
| `CAP_ADC` | 3 | Analog input |
| `CAP_TOUCH` | 4 | Capacitive touch |
| `CAP_OUT` | 5 | Digital OUTPUT |
| `CAP_OUT_OD` | 6 | OUTPUT\_OPEN\_DRAIN |
| `CAP_PWM` | 7 | PWM |
| `CAP_DAC` | 8 | DAC (true analog out) |

---

## Building & Flashing

### PlatformIO (recommended)

```bash
# ESP32-S3 Dev Board
pio run -e esp32s3 -t upload

# Raspberry Pi Pico
pio run -e rpipico -t upload
```

Environments are defined in `platformio.ini` at the repo root.

### Arduino IDE

1. Open `firmware/main.cpp` (or rename to `.ino`)
2. Select the correct board from **Tools → Board**
3. Click **Upload**

> Ensure the `ArduinoJson` library is **not** required — it has been removed. The firmware has no external library dependencies beyond the board's own Arduino core.

---

## Configuration

Constants at the top of `serial_handler.cpp` / `main.cpp`:

| Constant | Default | Description |
|----------|---------|-------------|
| `BAUD_RATE` | `115200` | Serial baud rate |
| `REPORT_INTERVAL_MS` | `1000` | Delta status report interval (ms) |
| `RX_BUF_SIZE` | `128` | Max incoming command length (bytes) |

---

## NVS Persistence (ESP32 only)

Calling `SAVE_CONFIG` from the web app persists the full pin configuration to ESP32 NVS via the `Preferences` library under the namespace `"upc"`. On next boot, `PinManager::init()` reads this config and re-applies all modes and values automatically.

RP2040 and AVR persistence is not yet implemented. `SAVE_CONFIG` is accepted but has no effect on those platforms.

---

## See Also

- [`docs/API.md`](./API.md) — Full serial command reference
- [`firmware/boards/`](../firmware/boards/) — Board definition headers
- [`firmware/board_caps.h`](../firmware/board_caps.h) — Capability flag definitions
