#include "serial_handler.h"
#include "pin_manager.h"
#include "board_def.h"

// ─── Buffer sizes ─────────────────────────────────────────────────
// No ArduinoJson — all parsing is manual token-based
#define RX_BUF_SIZE 128
#define TX_BUF_SIZE 512

static char g_rxBuf[RX_BUF_SIZE];
static uint16_t g_rxLen = 0;

// ─── Compact protocol helpers ─────────────────────────────────────
//
//  OUTGOING:
//    Board info   → BINFO:<BOARD_ID>|<BOARD_NAME>|<pin>:<caps>,<pin>:<caps>,...
//                   caps chars: I=IN, U=IN_UP, D=IN_DN, A=ADC, T=TOUCH,
//                               O=OUT, Z=OUT_OD, P=PWM, C=DAC
//    Config dump  → CONFIG:<pin>:<mode>,<pin>:<mode>,...   (active pins only)
//    Delta update → STAT:<pin>:<value>,<pin>:<value>,...   (changed pins only)
//    Ack          → ACK:<pin>:<mode>:<value>
//    Reset ack    → RESET:OK
//    Save ack     → SAVE:OK
//    Error        → ERR:<message>
//
//  INCOMING (newline-terminated, space-delimited tokens):
//    GET_DEF
//    GET_CONFIG
//    GET_STATUS
//    PIN_SET <pin> <mode> [value] [res] [freq]
//    RESET
//    SAVE_CONFIG
// ─────────────────────────────────────────────────────────────────

static void sendError(const char* msg) {
  Serial.print(F("ERR:"));
  Serial.println(msg);
}

// ─── GET_DEF ──────────────────────────────────────────────────────
static void handleGetDef() {
  Serial.print(F("BINFO:"));
  Serial.print(BOARD_ID);
  Serial.print('|');
  Serial.print(BOARD_NAME);
  Serial.print('|');

  for (uint8_t i = 0; i < PIN_COUNT; i++) {
    Serial.print(BOARD_PINS[i].name);
    Serial.print(':');

    uint16_t caps = BOARD_PINS[i].caps;
    if (caps & CAP_IN)    Serial.print('I');
    if (caps & CAP_IN_UP) Serial.print('U');
    if (caps & CAP_IN_DN) Serial.print('D');
    if (caps & CAP_ADC)   Serial.print('A');
    if (caps & CAP_TOUCH) Serial.print('T');
    if (caps & CAP_OUT)   Serial.print('O');
    if (caps & CAP_OUT_OD)Serial.print('Z');
    if (caps & CAP_PWM)   Serial.print('P');
    if (caps & CAP_DAC)   Serial.print('C');

    if (i < PIN_COUNT - 1) Serial.print(',');
  }
  Serial.print('\n');
}

// ─── GET_CONFIG ───────────────────────────────────────────────────
static void handleGetConfig() {
  bool first = true;
  Serial.print(F("CONFIG:"));

  for (uint8_t i = 0; i < PIN_COUNT; i++) {
    PinState s = PinManager::getState(i);
    if (s.mode == MODE_NONE) continue;
    if (!first) Serial.print(',');
    first = false;
    Serial.print(BOARD_PINS[i].name);
    Serial.print(':');
    Serial.print(PinManager::modeToString(s.mode));
  }
  Serial.print('\n');
}

// ─── GET_STATUS (full dump, forces all active pins) ───────────────
static void handleGetStatus() {
  bool first = true;
  Serial.print(F("STAT:"));

  for (uint8_t i = 0; i < PIN_COUNT; i++) {
    PinState s = PinManager::getState(i);
    if (s.mode == MODE_NONE) continue;
    if (!first) Serial.print(',');
    first = false;
    Serial.print(BOARD_PINS[i].name);
    Serial.print(':');
    Serial.print(PinManager::readPin(i));
  }
  Serial.print('\n');
}

// ─── PIN_SET <pin> <mode> [value] [res] [freq] ───────────────────
static void handlePinSet(char* args) {
  // args points to the remainder of the line after "PIN_SET "
  char* pinName = strtok(args,  " ");
  char* modeStr = strtok(NULL,  " ");
  char* valStr  = strtok(NULL,  " ");
  char* resStr  = strtok(NULL,  " ");
  char* freqStr = strtok(NULL,  " ");

  if (!pinName || !modeStr) { sendError("missing pin or mode"); return; }

  PinMode_t mode = PinManager::modeFromString(modeStr);
  if (mode == MODE_NONE) { sendError("unknown mode"); return; }

  int      value = valStr  ? atoi(valStr)  : 0;
  uint8_t  res   = resStr  ? (uint8_t)atoi(resStr)  : 8;
  uint32_t freq  = freqStr ? (uint32_t)atol(freqStr) : 1000;

  if (!PinManager::setPin(pinName, mode, value, res, freq)) {
    sendError("pin not found");
    return;
  }

  // Compact ack
  Serial.print(F("ACK:"));
  Serial.print(pinName); Serial.print(':');
  Serial.print(modeStr); Serial.print(':');
  Serial.println(value);
}

// ─── RESET ────────────────────────────────────────────────────────
static void handleReset() {
  PinManager::reset();
  Serial.println(F("RESET:OK"));
}

// ─── SAVE_CONFIG ─────────────────────────────────────────────────
static void handleSaveConfig() {
  PinManager::saveConfig();
  Serial.println(F("SAVE:OK"));
}

// ─── Dispatch (no JSON deserialisation) ──────────────────────────
static void dispatch(char* buf) {
  char* cmd  = strtok(buf, " ");
  char* rest = strtok(NULL, ""); // remainder of line (may be NULL)

  if (!cmd) return;

  if      (strcmp(cmd, "GET_DEF")     == 0) handleGetDef();
  else if (strcmp(cmd, "GET_CONFIG")  == 0) handleGetConfig();
  else if (strcmp(cmd, "GET_STATUS")  == 0) handleGetStatus();
  else if (strcmp(cmd, "PIN_SET")     == 0) handlePinSet(rest ? rest : (char*)"");
  else if (strcmp(cmd, "RESET")       == 0) handleReset();
  else if (strcmp(cmd, "SAVE_CONFIG") == 0) handleSaveConfig();
  else                                       sendError("unknown command");
}

// ─── Non-blocking serial reader ──────────────────────────────────
void SerialHandler::init() {
  g_rxLen = 0;
}

void SerialHandler::update() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (g_rxLen > 0) {
        g_rxBuf[g_rxLen] = '\0';
        dispatch(g_rxBuf);
        g_rxLen = 0;
      }
    } else {
      if (g_rxLen < RX_BUF_SIZE - 1) {
        g_rxBuf[g_rxLen++] = c;
      } else {
        g_rxLen = 0;
        sendError("rx overflow");
      }
    }
  }
}
