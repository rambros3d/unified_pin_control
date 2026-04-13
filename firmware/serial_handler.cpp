#include "serial_handler.h"
#include <ArduinoJson.h>
#include "pin_manager.h"
#include "board_def.h"

// ─── JSON buffer size ────────────────────────────────────────────
// 2 KB for incoming, 4 KB for outgoing (GET_DEF with many pins)
#define RX_BUF_SIZE 2048
#define TX_BUF_SIZE 4096

static char g_rxBuf[RX_BUF_SIZE];
static uint16_t g_rxLen = 0;

void SerialHandler::init() {
  g_rxLen = 0;
}

// ─── Helper: send a JSON doc over Serial ─────────────────────────
static void sendJson(JsonDocument& doc) {
  serializeJson(doc, Serial);
  Serial.print('\n');
}

static void sendError(const char* msg) {
  StaticJsonDocument<128> err;
  err["error"] = msg;
  sendJson(err);
}

// ─── Command handlers ─────────────────────────────────────────────

static void handleGetDef() {
  DynamicJsonDocument doc(TX_BUF_SIZE);
  doc["board"] = BOARD_NAME;
  doc["id"]    = BOARD_ID;
  JsonArray pins = doc.createNestedArray("pins");

  for (uint8_t i = 0; i < PIN_COUNT; i++) {
    JsonObject p = pins.createNestedObject();
    p["pin"] = BOARD_PINS[i].name;

    JsonArray inArr  = p.createNestedArray("in");
    JsonArray outArr = p.createNestedArray("out");

    uint16_t caps = BOARD_PINS[i].caps;
    if (caps & CAP_IN)    inArr.add("IN");
    if (caps & CAP_IN_UP) inArr.add("IN_UP");
    if (caps & CAP_IN_DN) inArr.add("IN_DN");
    if (caps & CAP_ADC)   inArr.add("ADC");
    if (caps & CAP_TOUCH) inArr.add("TOUCH");

    if (caps & CAP_OUT)    outArr.add("OUT");
    if (caps & CAP_OUT_OD) outArr.add("OUT_OD");
    if (caps & CAP_PWM)    outArr.add("PWM");
    if (caps & CAP_DAC)    outArr.add("DAC");
  }
  sendJson(doc);
}

static void handleGetConfig() {
  DynamicJsonDocument doc(TX_BUF_SIZE);
  JsonArray config = doc.createNestedArray("config");

  for (uint8_t i = 0; i < PIN_COUNT; i++) {
    PinState s = PinManager::getState(i);
    if (s.mode == MODE_NONE) continue;
    JsonObject p = config.createNestedObject();
    p["pin"]  = BOARD_PINS[i].name;
    p["mode"] = PinManager::modeToString(s.mode);
  }
  sendJson(doc);
}

static void handlePinSet(JsonDocument& req) {
  const char* pinName = req["pin"] | "";
  const char* modeStr = req["mode"] | "";
  int   value = req["value"] | 0;
  uint8_t  res  = req["res"]  | 8;
  uint32_t freq = req["freq"] | 1000;
  bool ack = req["ack"] | 0;

  if (pinName[0] == '\0' || modeStr[0] == '\0') {
    sendError("missing pin or mode");
    return;
  }

  PinMode_t mode = PinManager::modeFromString(modeStr);
  if (mode == MODE_NONE) {
    sendError("unknown mode");
    return;
  }

  bool ok = PinManager::setPin(pinName, mode, value, res, freq);
  if (!ok) {
    sendError("pin not found");
    return;
  }

  if (ack) {
    StaticJsonDocument<128> resp;
    resp["pin"]   = pinName;
    resp["mode"]  = modeStr;
    resp["value"] = value;
    sendJson(resp);
  }
}

static void handleUpdate() {
  DynamicJsonDocument doc(TX_BUF_SIZE);
  JsonArray pins = doc.createNestedArray("pins");

  for (uint8_t i = 0; i < PIN_COUNT; i++) {
    PinState s = PinManager::getState(i);
    if (s.mode == MODE_NONE) continue;

    JsonObject p = pins.createNestedObject();
    p["pin"]   = BOARD_PINS[i].name;
    p["mode"]  = PinManager::modeToString(s.mode);
    p["value"] = PinManager::readPin(i);

    if (s.mode == MODE_PWM || s.mode == MODE_DAC) {
      p["res"]  = s.res;
      p["freq"] = s.freq;
    }
    if (s.mode == MODE_ADC) {
      p["res"] = s.res;
    }
  }
  sendJson(doc);
}

static void handleReset() {
  PinManager::reset();
  StaticJsonDocument<32> resp;
  resp["resp"] = "reset";
  sendJson(resp);
}

static void handleSaveConfig() {
  PinManager::saveConfig();
  StaticJsonDocument<32> resp;
  resp["resp"] = "saved";
  sendJson(resp);
}

// ─── Command dispatch ─────────────────────────────────────────────
static void dispatch(const char* json, uint16_t len) {
  DynamicJsonDocument doc(RX_BUF_SIZE);
  DeserializationError err = deserializeJson(doc, json, len);
  if (err) {
    sendError("json parse error");
    return;
  }

  const char* cmd = doc["cmd"] | "";

  if      (strcmp(cmd, "GET_DEF")     == 0) handleGetDef();
  else if (strcmp(cmd, "GET_CONFIG")  == 0) handleGetConfig();
  else if (strcmp(cmd, "PIN_SET")     == 0) handlePinSet(doc);
  else if (strcmp(cmd, "UPDATE")      == 0) handleUpdate();
  else if (strcmp(cmd, "RESET")       == 0) handleReset();
  else if (strcmp(cmd, "SAVE_CONFIG") == 0) handleSaveConfig();
  else                                       sendError("unknown command");
}

// ─── Non-blocking serial reader (newline-terminated) ─────────────
void SerialHandler::update() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (g_rxLen > 0) {
        g_rxBuf[g_rxLen] = '\0';
        dispatch(g_rxBuf, g_rxLen);
        g_rxLen = 0;
      }
    } else {
      if (g_rxLen < RX_BUF_SIZE - 1)
        g_rxBuf[g_rxLen++] = c;
      else {
        // Buffer overflow – discard and reset
        g_rxLen = 0;
        sendError("rx buffer overflow");
      }
    }
  }
}
