# 🛠️ Unified Pin Control API

## Overview

- **Transport:** WebSerial (USB Serial)
- **Datatype:** JSON with ArduinoJson, newline (`\n`) terminated

---

## Glossary

| Short    | Description               |
| -------- | ------------------------- |
| `IN`     | Digital INPUT             |
| `IN_UP`  | Digital INPUT_PULLUP      |
| `IN_DN`  | Digital INPUT_PULLDOWN    |
| `ADC`    | ANALOG_INPUT              |
| `TOUCH`  | Capacitive Touch Input    |
| `OUT`    | Digital OUTPUT*           |
| `OUT_OD` | Digital OUTPUT_OPEN_DRAIN |
| `PWM`    | ANALOG_WRITE (PWM)        |
| `DAC`    | True Analog Output        |

---

## Commands

### 1 `GET_DEF`

Fetches the board name, PlatformIO board ID, and the capabilities of each pin.

#### Command
```json
{ "cmd": "GET_INFO" }
```

#### Response
```json
{
  "board": "ESP32-S3 dev board",
  "id": "esp32s3",
  "pins": [
    {
      "pin": "GPIO0",
      "in": ["IN", "IN_UP", "IN_DN", "ADC"],
      "out": ["OUT", "PWM", "DAC"]
    }
  ]
}
```

---

---

### 2 `GET_CONFIG`

Returns the currently configured pins and their types (mode).

#### Command
```json
{ "cmd": "GET_INFO" }
```

#### Response
```json
{
  "configured": [
    { "pin": "GPIO0", "mode": "OUT" },
    { "pin": "GPIO1", "mode": "PWM" },
    { "pin": "A0",    "mode": "ADC" }
  ]
}
```
---

### 3 `PIN_SET`

Sets the mode and value of a specific pin. Optional `res` (resolution) and `freq` (frequency) for PWM/DAC. If `"ack": 1` is provided, the board responds with confirmation.

#### Command
```json
{
  "cmd": "PIN_SET",
  "pin": "GPIO0",
  "mode": "OUT",
  "value": 1,
  "res": 8,
  "freq": 1000,
  "ack": 1
}
```

#### Response (if `"ack": 1`)
```json
{
  "pin": "GPIO0",
  "mode": "OUT",
  "value": 1
}
```

---

### 4 `UPDATE`

Fetches the current state, mode, and value (with optional `res`/`freq`) of **all pins** for efficient monitoring.

#### Command
```json
{ "cmd": "UPDATE" }
```

#### Response
```json
{
  "pins": [
    {
      "pin": "GPIO0",
      "mode": "IN",
      "value": 1
    },
    {
      "pin": "GPIO1",
      "mode": "PWM",
      "value": 128,
      "res": 8,
      "freq": 1000
    },
    {
      "pin": "A0",
      "mode": "ADC",
      "res": 10,
      "value": 523
    }
  ]
}
```

---

### 5 `RESET`

Resets all pins to their default safe state.

#### Command
```json
{ "cmd": "RESET" }
```

#### Response
```json
{ "reset": "ok" }
```

---