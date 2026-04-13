#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// Board: Raspberry Pi Pico (RP2040)  —  ARDUINO_RASPBERRY_PI_PICO
//
// All GP0–GP22, GP26–GP28 are digital I/O with PWM.
// GP26, GP27, GP28 are also ADC-capable (ADC0, ADC1, ADC2).
// GP23 = SMPS power-save pin (internal, not exposed on header).
// GP24 = VBUS sense (internal).
// GP25 = on-board LED.
// No INPUT_PULLDOWN on Pico via standard Arduino API — omitted here.
// No DAC, no capacitive touch.
// ─────────────────────────────────────────────────────────────────────────────

#define BOARD_NAME "Raspberry Pi Pico"
#define BOARD_ID   "rp2040"

#include "../board_caps.h"

static const PinDef BOARD_PINS[] = {
  // name   caps
  {"GP0",  CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GP1",  CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GP2",  CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GP3",  CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GP4",  CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GP5",  CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GP6",  CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GP7",  CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GP8",  CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GP9",  CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GP10", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GP11", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GP12", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GP13", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GP14", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GP15", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GP16", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GP17", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GP18", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GP19", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GP20", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GP21", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GP22", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GP25", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_OUT|CAP_OUT_OD|CAP_PWM},  // on-board LED
  {"GP26", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_ADC|CAP_OUT|CAP_OUT_OD|CAP_PWM},  // ADC0
  {"GP27", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_ADC|CAP_OUT|CAP_OUT_OD|CAP_PWM},  // ADC1
  {"GP28", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_ADC|CAP_OUT|CAP_OUT_OD|CAP_PWM},  // ADC2
};

static const uint8_t PIN_COUNT = sizeof(BOARD_PINS) / sizeof(BOARD_PINS[0]);

inline int pinNameToNumber(const char* name) {
  if (strncmp(name, "GP", 2) == 0) return atoi(name + 2);
  return atoi(name);
}
