#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// board_def.h  —  Auto-selects the correct board header from firmware/boards/
// Add a new board by:
//   1. Creating firmware/boards/<vendor>-<board>.h
//   2. Adding a #elif block below
// ─────────────────────────────────────────────────────────────────────────────

#if defined(ARDUINO_ESP32S3_DEV) || defined(ARDUINO_LOLIN_S3_MINI)
  #include "boards/esp32-esp32s3dev.h"

#elif defined(ARDUINO_RASPBERRY_PI_PICO)
  #include "boards/rp2040-rpipico.h"

#else
  #error "Unsupported board. Add a definition under firmware/boards/ and register it in board_def.h"
#endif
