#pragma once
#include <Arduino.h>
#include "board_def.h"

// ─── Mode enum ────────────────────────────────────────────────────
enum PinMode_t : uint8_t {
  MODE_NONE = 0,
  MODE_IN,
  MODE_IN_UP,
  MODE_IN_DN,
  MODE_ADC,
  MODE_TOUCH,
  MODE_OUT,
  MODE_OUT_OD,
  MODE_PWM,
  MODE_DAC,
};

struct PinState {
  PinMode_t mode   = MODE_NONE;
  int       value  = 0;
  uint8_t   res    = 8;      // PWM/DAC resolution bits
  uint32_t  freq   = 1000;   // PWM frequency Hz
};

namespace PinManager {
  void       init();
  bool       setPin(const char* name, PinMode_t mode, int value, uint8_t res, uint32_t freq);
  PinState   getState(uint8_t idx);
  void       reset();
  void       saveConfig();
  void       loadConfig();
  PinMode_t  modeFromString(const char* s);
  const char* modeToString(PinMode_t m);
  int        readPin(uint8_t idx);
}
