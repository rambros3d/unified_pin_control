#include "pin_manager.h"
#include <Preferences.h>   // ESP32 NVS; gracefully absent on AVR

static PinState g_states[PIN_COUNT];

// ─── Mode string maps ─────────────────────────────────────────────
static const char* const MODE_STRINGS[] = {
  "NONE", "IN", "IN_UP", "IN_DN", "ADC", "TOUCH",
  "OUT", "OUT_OD", "PWM", "DAC"
};

PinMode_t PinManager::modeFromString(const char* s) {
  for (uint8_t i = 0; i < 10; i++)
    if (strcmp(s, MODE_STRINGS[i]) == 0) return (PinMode_t)i;
  return MODE_NONE;
}

const char* PinManager::modeToString(PinMode_t m) {
  if (m < 10) return MODE_STRINGS[m];
  return "NONE";
}

// ─── Apply mode to hardware ───────────────────────────────────────
static void applyMode(uint8_t idx, PinMode_t mode, int value, uint8_t res, uint32_t freq) {
  int pin = pinNameToNumber(BOARD_PINS[idx].name);
  if (pin < 0) return;

  switch (mode) {
    case MODE_IN:     pinMode(pin, INPUT);           break;
    case MODE_IN_UP:  pinMode(pin, INPUT_PULLUP);     break;
#if defined(INPUT_PULLDOWN)
    case MODE_IN_DN:  pinMode(pin, INPUT_PULLDOWN);   break;
#else
    case MODE_IN_DN:  pinMode(pin, INPUT);            break; // fallback
#endif
    case MODE_ADC:    pinMode(pin, INPUT);            break;
    case MODE_TOUCH:  /* nothing needed */            break;

    case MODE_OUT:
      pinMode(pin, OUTPUT);
      digitalWrite(pin, value ? HIGH : LOW);
      break;

    case MODE_OUT_OD:
#if defined(OUTPUT_OPEN_DRAIN)
      pinMode(pin, OUTPUT_OPEN_DRAIN);
#else
      pinMode(pin, OUTPUT);
#endif
      digitalWrite(pin, value ? HIGH : LOW);
      break;

    case MODE_PWM:
#if defined(ESP32)
      ledcAttach(pin, freq, res);
      ledcWrite(pin, value);
#else
      pinMode(pin, OUTPUT);
      analogWrite(pin, value);
#endif
      break;

    case MODE_DAC:
#if defined(ESP32) && !defined(CONFIG_IDF_TARGET_ESP32S3)
      dacWrite(pin, value);
#else
      analogWrite(pin, value);
#endif
      break;

    default: break;
  }
}

// ─── Public API ───────────────────────────────────────────────────
void PinManager::init() {
  loadConfig();
  // Re-apply loaded states
  for (uint8_t i = 0; i < PIN_COUNT; i++)
    if (g_states[i].mode != MODE_NONE)
      applyMode(i, g_states[i].mode, g_states[i].value,
                g_states[i].res, g_states[i].freq);
}

bool PinManager::setPin(const char* name, PinMode_t mode,
                        int value, uint8_t res, uint32_t freq) {
  for (uint8_t i = 0; i < PIN_COUNT; i++) {
    if (strcmp(BOARD_PINS[i].name, name) == 0) {
      g_states[i] = {mode, value, res, freq};
      applyMode(i, mode, value, res, freq);
      return true;
    }
  }
  return false; // pin not found
}

PinState PinManager::getState(uint8_t idx) {
  return g_states[idx];
}

int PinManager::readPin(uint8_t idx) {
  int pin = pinNameToNumber(BOARD_PINS[idx].name);
  PinMode_t mode = g_states[idx].mode;
  switch (mode) {
    case MODE_IN:
    case MODE_IN_UP:
    case MODE_IN_DN:
    case MODE_OUT:
    case MODE_OUT_OD:
      return digitalRead(pin);
    case MODE_ADC:
      return analogRead(pin);
    case MODE_TOUCH:
#if defined(ESP32)
      return touchRead(pin);
#else
      return 0;
#endif
    case MODE_PWM:
    case MODE_DAC:
      return g_states[idx].value; // echo last set value
    default:
      return 0;
  }
}

void PinManager::reset() {
  for (uint8_t i = 0; i < PIN_COUNT; i++) {
    int pin = pinNameToNumber(BOARD_PINS[i].name);
    if (pin >= 0) pinMode(pin, INPUT); // safe default
    g_states[i] = {};
  }
}

void PinManager::saveConfig() {
#if defined(ESP32)
  Preferences prefs;
  prefs.begin("upc", false);
  for (uint8_t i = 0; i < PIN_COUNT; i++) {
    char key[8];
    snprintf(key, sizeof(key), "p%d", i);
    // Pack mode+res+freq+value into a blob
    uint8_t buf[8];
    buf[0] = (uint8_t)g_states[i].mode;
    buf[1] = g_states[i].res;
    memcpy(buf + 2, &g_states[i].freq,  4);
    buf[6] = (uint8_t)(g_states[i].value & 0xFF);
    buf[7] = (uint8_t)((g_states[i].value >> 8) & 0xFF);
    prefs.putBytes(key, buf, 8);
  }
  prefs.end();
#endif
  // AVR / RP2040: EEPROM support can be added here
}

void PinManager::loadConfig() {
#if defined(ESP32)
  Preferences prefs;
  prefs.begin("upc", true);
  for (uint8_t i = 0; i < PIN_COUNT; i++) {
    char key[8];
    snprintf(key, sizeof(key), "p%d", i);
    uint8_t buf[8] = {};
    if (prefs.getBytes(key, buf, 8) == 8) {
      g_states[i].mode  = (PinMode_t)buf[0];
      g_states[i].res   = buf[1];
      memcpy(&g_states[i].freq, buf + 2, 4);
      g_states[i].value = (int)(buf[6] | (buf[7] << 8));
    }
  }
  prefs.end();
#endif
}
