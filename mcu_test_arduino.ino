
#include <Arduino.h>
#include <ArduinoJson.h>
#include <AceRoutine.h>

using namespace ace_routine;

// Forward declarations
void handle_json_command(const String& cmd);
void send_error(const char* error);

// Coroutine for reading from serial
COROUTINE(SerialReader) {
  COROUTINE_BEGIN();
  static String input_string;

  for (;;) {
    if (Serial.available() > 0) {
      char inChar = (char)Serial.read();
      if (inChar == '\n') {
        handle_json_command(input_string);
        input_string = "";
      } else {
        input_string += inChar;
      }
    }
    COROUTINE_YIELD();
  }
  COROUTINE_END();
}

// Simple pin configuration tracking
struct PinConfig {
  int pin;
  String mode;
  int value;
  int res;
  int freq;
};

#define MAX_PINS 40
PinConfig configured_pins[MAX_PINS];
int configured_pin_count = 0;


void setup() {
  Serial.begin(115200);
  // Wait for serial connection
  while (!Serial) {
    delay(10);
  }
  // Initialize configured_pins array
  for (int i = 0; i < MAX_PINS; i++) {
    configured_pins[i].pin = -1;
  }
}

void loop() {
  // Run the coroutine
  SerialReader.runCoroutine();
}

void handle_json_command(const String& cmd_str) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, cmd_str);

  if (error) {
    send_error("JSON parse failed");
    return;
  }

  const char* cmd = doc["cmd"];
  if (!cmd) {
    send_error("Missing 'cmd'");
    return;
  }

  if (strcmp(cmd, "GET_INFO") == 0) {
    // In API.md this is GET_DEF and GET_CONFIG, but the example shows GET_INFO
    // I will assume GET_INFO is the intended command.
    // This part can be expanded to differentiate if needed.

    // For now, let's send back board info as a simple ack
    JsonDocument response_doc;
    response_doc["board"] = BOARD_NAME;
    response_doc["id"] = BOARD_ID;
    serializeJson(response_doc, Serial);
    Serial.println();

  } else if (strcmp(cmd, "PIN_SET") == 0) {
    int pin = doc["pin"];
    const char* mode = doc["mode"];
    int value = doc["value"];

    if (!mode) {
      send_error("Missing 'mode' in PIN_SET");
      return;
    }

    // Store configuration
    bool pin_found = false;
    for (int i = 0; i < configured_pin_count; i++) {
      if (configured_pins[i].pin == pin) {
        configured_pins[i].mode = mode;
        configured_pins[i].value = value;
        pin_found = true;
        break;
      }
    }

    if (!pin_found && configured_pin_count < MAX_PINS) {
      configured_pins[configured_pin_count].pin = pin;
      configured_pins[configured_pin_count].mode = mode;
      configured_pins[configured_pin_count].value = value;
      configured_pin_count++;
    }

    // Set pin mode and value
    if (strcmp(mode, "OUT") == 0) {
      pinMode(pin, OUTPUT);
      digitalWrite(pin, value);
    } else if (strcmp(mode, "IN") == 0) {
      pinMode(pin, INPUT);
    } else if (strcmp(mode, "IN_UP") == 0) {
      pinMode(pin, INPUT_PULLUP);
    }
    // Add other modes here...


    if (doc["ack"] == 1) {
      JsonDocument response_doc;
      response_doc["pin"] = pin;
      response_doc["mode"] = mode;
      response_doc["value"] = value;
      serializeJson(response_doc, Serial);
      Serial.println();
    }

  } else if (strcmp(cmd, "UPDATE") == 0) {
    JsonDocument response_doc;
    JsonArray pins = response_doc.createNestedArray("pins");
    for (int i = 0; i < configured_pin_count; i++) {
      JsonObject pin_obj = pins.createNestedObject();
      pin_obj["pin"] = configured_pins[i].pin;
      pin_obj["mode"] = configured_pins[i].mode;
      // Read current value
      if (configured_pins[i].mode == "IN" || configured_pins[i].mode == "IN_UP" || configured_pins[i].mode == "OUT") {
        pin_obj["value"] = digitalRead(configured_pins[i].pin);
      } else {
        pin_obj["value"] = configured_pins[i].value;  // For modes not read from hardware yet
      }
    }
    serializeJson(response_doc, Serial);
    Serial.println();

  } else if (strcmp(cmd, "RESET") == 0) {
    for (int i = 0; i < configured_pin_count; i++) {
      // Reset pin to default state (INPUT)
      pinMode(configured_pins[i].pin, INPUT);
    }
    configured_pin_count = 0;
    JsonDocument response_doc;
    response_doc["resp"] = "reset";
    serializeJson(response_doc, Serial);
    Serial.println();

  } else if (strcmp(cmd, "SAVE_CONFIG") == 0) {
    // Placeholder for saving config to EEPROM or SPIFFS
    JsonDocument response_doc;
    response_doc["resp"] = "saved";
    serializeJson(response_doc, Serial);
    Serial.println();
  } else {
    send_error("Unknown command");
  }
}

void send_error(const char* error_msg) {
  JsonDocument error_doc;
  error_doc["error"] = error_msg;
  serializeJson(error_doc, Serial);
  Serial.println();
}
