# 🛠️ Unified Pin Control Firmware

## 📌 Description

This firmware provides a **universal, cross-platform solution to manually test and validate all GPIO pins** on Arduino-compatible MCUs, including ESP32, STM32, ATmega, RP2040, and CH32 families.

Using a simple JSON API over WebSerial, it enables **manual pin testing, validation, and monitoring** directly from your browser.

---

## 🚀 Features

✅ Universal firmware for all Arduino-compatible boards  
✅ Manual control and validation of pins  
✅ Live monitoring of pin states and readings

---

## 📌 Available Pin Modes

### 🔹 INPUT MODES

- `DIGITAL_INPUT`
- `DIGITAL_INPUT_PULLUP`
- `DIGITAL_INPUT_PULLDOWN`
- `ANALOG_INPUT`
- `CAPACITATIVE_TOUCH` *(if supported)*

### 🔹 OUTPUT MODES

- `DIGITAL_OUTPUT`
- `DIGITAL_OUTPUT_OPEN_DRAIN`
- `PWM_OUTPUT`
- `ANALOG_OUTPUT` *(for DAC-capable pins)*

---

## 🔧 Requirements

- Arduino IDE or PlatformIO for flashing.
- Webserial compatible browser.
- Compatible with: (to be added)
  - ESP32 / ESP8266
  - STM32
  - RP2040
  - ATmega328P / Mega2560
  - CH32 series

---

## 📄 Documentation

JSON command reference available in [`API.md`](API.md).

---

## 💡 Contributing

Contributions for board definitions, test rig integration, or frontend test UI are welcome.

---
