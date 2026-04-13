#pragma once
#include <Arduino.h>

// ─────────────────────────────────────────────────────────────────────────────
// Shared pin capability flags and PinDef struct.
// Included by every board definition header in firmware/boards/
// ─────────────────────────────────────────────────────────────────────────────

#define CAP_IN       (1 << 0)   // Digital INPUT
#define CAP_IN_UP    (1 << 1)   // INPUT_PULLUP
#define CAP_IN_DN    (1 << 2)   // INPUT_PULLDOWN
#define CAP_ADC      (1 << 3)   // Analog Input (ADC)
#define CAP_TOUCH    (1 << 4)   // Capacitive Touch
#define CAP_OUT      (1 << 5)   // Digital OUTPUT
#define CAP_OUT_OD   (1 << 6)   // OUTPUT_OPEN_DRAIN
#define CAP_PWM      (1 << 7)   // PWM
#define CAP_DAC      (1 << 8)   // True Analog Output (DAC)

struct PinDef {
  const char* name;   // Human-readable pin label (e.g. "GPIO4", "GP0")
  uint16_t    caps;   // Bitmask of CAP_* flags
};
