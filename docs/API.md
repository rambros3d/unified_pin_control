# Serial API Reference

**Transport:** USB Serial (WebSerial) 
**Baud rate:** 115200 
**Framing:** All messages are plain text, newline (`\n`) terminated 
**No JSON** — the protocol uses compact, delimiter-based strings to minimise serial overhead

---

## Mode Glossary

| Mode | Meaning |
|------|----------------------------------------------------------|
| `IN` | Digital INPUT |
| `IN_UP` | Digital INPUT\_PULLUP |
| `IN_DN` | Digital INPUT\_PULLDOWN |
| `ADC` | Analog input (ADC read) |
| `TOUCH` | Capacitive touch input (ESP32 only) |
| `OUT` | Digital OUTPUT |
| `OUT_OD` | Digital OUTPUT\_OPEN\_DRAIN |
| `PWM` | PWM output (analogWrite / LEDC) |
| `DAC` | True analog output (ESP32 classic only) |

## Capability Chars (used in `BINFO` response)

| Char | Capability |
|------|------------|
| `I` | IN |
| `U` | IN\_UP |
| `D` | IN\_DN |
| `A` | ADC |
| `T` | TOUCH |
| `O` | OUT |
| `Z` | OUT\_OD |
| `P` | PWM |
| `C` | DAC |

---

## Commands (Web App → Firmware)

All commands are sent as a plain-text line terminated with `\n`.

---

### `GET_DEF`
Request the board identity and full pin capability map. 
Send this once on connect so the web app can build its UI without any hardcoded board knowledge.

```
GET_DEF
```

**Response format:**
```
BINFO:<board_id>|<board_name>|<pin>:<caps>,<pin>:<caps>,...
```

**Example (ESP32-S3):**
```
BINFO:esp32s3|ESP32-S3 Dev Board|GPIO0:IUDOZP,GPIO1:IUDATOZP,GPIO2:IUDATOZP,...
```

**Example (Pi Pico):**
```
BINFO:rp2040|Raspberry Pi Pico|GP0:IUDOZP,GP1:IUDOZP,...,GP26:IUDAOZP,GP27:IUDAOZP,GP28:IUDAOZP
```

---

### `GET_CONFIG`
Request the current mode of every active (configured) pin.

```
GET_CONFIG
```

**Response format:**
```
CONFIG:<pin>:<mode>,<pin>:<mode>,...
```

**Example:**
```
CONFIG:GPIO4:OUT,GPIO5:IN_UP,GPIO1:ADC
```

If no pins are configured, the firmware still responds with `CONFIG:` (empty payload) followed by `\n`.

---

### `GET_STATUS`
Force an immediate full status dump of all active pins and their current values. 
Use this on reconnect or page load to sync the web app state.

```
GET_STATUS
```

**Response format:**
```
STAT:<pin>:<value>,<pin>:<value>,...
```

**Example:**
```
STAT:GPIO4:1,GPIO5:0,GPIO1:874
```

- Digital pins report `0` or `1`
- ADC pins report the raw ADC reading (0–4095 on 12-bit, 0–1023 on 10-bit)
- PWM/DAC output pins echo their last written value

---

### `PIN_SET`
Configure a pin's mode and optionally write a value. 
For PWM, `res` (bit resolution, default `8`) and `freq` (Hz, default `1000`) are optional.

```
PIN_SET <pin> <mode> [value] [res] [freq]
```

| Argument | Required | Default | Description |
|----------|----------|---------|-------------|
| `pin` | ✅ | — | Pin label (e.g. `GPIO4`, `GP0`) |
| `mode` | ✅ | — | One of the modes in the glossary |
| `value` | ❌ | `0` | Initial value to write |
| `res` | ❌ | `8` | PWM/DAC bit resolution |
| `freq` | ❌ | `1000` | PWM frequency in Hz |

**Examples:**
```
PIN_SET GPIO4 OUT 1
PIN_SET GPIO5 IN_UP
PIN_SET GPIO1 ADC
PIN_SET GPIO9 PWM 128 8 5000
```

**Response (always):**
```
ACK:<pin>:<mode>:<value>
```

**Example:**
```
ACK:GPIO4:OUT:1
```

---

### `RESET`
Release all configured pins and reset them to `INPUT` (safe default). 
Clears the in-memory pin state. Does **not** erase saved NVS config.

```
RESET
```

**Response:**
```
RESET:OK
```

---

### `SAVE_CONFIG`
Persist the current pin configuration to non-volatile storage (ESP32 NVS). 
The saved config is automatically re-applied on next boot.

```
SAVE_CONFIG
```

**Response:**
```
SAVE:OK
```

> RP2040/AVR: NVS/EEPROM persistence is not yet implemented. The command is accepted and `SAVE:OK` is returned, but config is not stored.

---

## Unsolicited Messages (Firmware → Web App)

The firmware may emit these at any time, unprompted.

### `STAT` — Delta Status Update
Emitted periodically (default every 1000 ms). **Only pins whose value changed** since the last report are included.

```
STAT:<pin>:<value>,<pin>:<value>,...
```

**Example:**
```
STAT:GPIO5:1,GPIO1:512
```

### `ERR` — Error
Emitted when a command cannot be executed.

```
ERR:<message>
```

**Common messages:**

| Message | Cause |
|---------|-------|
| `ERR:unknown command` | Command string not recognised |
| `ERR:missing pin or mode` | `PIN_SET` called without required arguments |
| `ERR:unknown mode` | Mode string not in the glossary |
| `ERR:pin not found` | Pin label does not exist on this board |
| `ERR:rx overflow` | Incoming line exceeded 128-byte buffer |

---

## Typical Web App Handshake

```
[Web App connects via WebSerial]
  → GET_DEF          # discover board and pin capabilities
  ← BINFO:...        # web app builds pin UI
  → GET_CONFIG       # restore any previously active pins
  ← CONFIG:...       # web app marks those pins as active
  → GET_STATUS       # sync current values
  ← STAT:...         # web app renders live values

[Running]
  → PIN_SET GPIO4 OUT 1   # user interacts
  ← ACK:GPIO4:OUT:1
  ← STAT:GPIO4:1          # next periodic update
```
