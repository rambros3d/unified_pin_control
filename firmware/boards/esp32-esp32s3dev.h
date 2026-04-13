#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// Board: ESP32-S3 Dev Board (ARDUINO_ESP32S3_DEV / ARDUINO_LOLIN_S3_MINI)
//
// ADC1  : GPIO1–GPIO10
// ADC2  : GPIO11–GPIO20 (unavailable when Wi-Fi is active)
// Touch : GPIO1–GPIO14
// DAC   : none on S3
// OD    : all output-capable pins support OUTPUT_OPEN_DRAIN
// UART0 : GPIO43 (TX), GPIO44 (RX)  — usable but labelled
// ─────────────────────────────────────────────────────────────────────────────

#define BOARD_NAME "ESP32-S3 Dev Board"
#define BOARD_ID   "esp32s3"

#include "../board_caps.h"

static const PinDef BOARD_PINS[] = {
  // name      caps
  {"GPIO0",  CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_ADC|                    CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO1",  CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_ADC|CAP_TOUCH|           CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO2",  CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_ADC|CAP_TOUCH|           CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO3",  CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_ADC|CAP_TOUCH|           CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO4",  CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_ADC|CAP_TOUCH|           CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO5",  CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_ADC|CAP_TOUCH|           CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO6",  CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_ADC|CAP_TOUCH|           CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO7",  CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_ADC|CAP_TOUCH|           CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO8",  CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_ADC|CAP_TOUCH|           CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO9",  CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_ADC|CAP_TOUCH|           CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO10", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_ADC|CAP_TOUCH|           CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO11", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_ADC|CAP_TOUCH|           CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO12", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_ADC|CAP_TOUCH|           CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO13", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_ADC|CAP_TOUCH|           CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO14", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_ADC|CAP_TOUCH|           CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO15", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_ADC|                    CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO16", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_ADC|                    CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO17", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_ADC|                    CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO18", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_ADC|                    CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO19", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_ADC|                    CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO20", CAP_IN|CAP_IN_UP|CAP_IN_DN|CAP_ADC|                    CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO21", CAP_IN|CAP_IN_UP|CAP_IN_DN|                            CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO26", CAP_IN|CAP_IN_UP|CAP_IN_DN|                            CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO33", CAP_IN|CAP_IN_UP|CAP_IN_DN|                            CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO34", CAP_IN|CAP_IN_UP|CAP_IN_DN|                            CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO35", CAP_IN|CAP_IN_UP|CAP_IN_DN|                            CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO36", CAP_IN|CAP_IN_UP|CAP_IN_DN|                            CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO37", CAP_IN|CAP_IN_UP|CAP_IN_DN|                            CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO38", CAP_IN|CAP_IN_UP|CAP_IN_DN|                            CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO39", CAP_IN|CAP_IN_UP|CAP_IN_DN|                            CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO40", CAP_IN|CAP_IN_UP|CAP_IN_DN|                            CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO41", CAP_IN|CAP_IN_UP|CAP_IN_DN|                            CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO42", CAP_IN|CAP_IN_UP|CAP_IN_DN|                            CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO43", CAP_IN|CAP_IN_UP|CAP_IN_DN|                            CAP_OUT|CAP_OUT_OD|CAP_PWM},  // UART0 TX
  {"GPIO44", CAP_IN|CAP_IN_UP|CAP_IN_DN|                            CAP_OUT|CAP_OUT_OD|CAP_PWM},  // UART0 RX
  {"GPIO45", CAP_IN|CAP_IN_UP|CAP_IN_DN|                            CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO46", CAP_IN|CAP_IN_UP|CAP_IN_DN|                            CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO47", CAP_IN|CAP_IN_UP|CAP_IN_DN|                            CAP_OUT|CAP_OUT_OD|CAP_PWM},
  {"GPIO48", CAP_IN|CAP_IN_UP|CAP_IN_DN|                            CAP_OUT|CAP_OUT_OD|CAP_PWM},
};

static const uint8_t PIN_COUNT = sizeof(BOARD_PINS) / sizeof(BOARD_PINS[0]);

inline int pinNameToNumber(const char* name) {
  if (strncmp(name, "GPIO", 4) == 0) return atoi(name + 4);
  return atoi(name);
}
