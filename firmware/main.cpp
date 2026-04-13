#include <Arduino.h>
#include "pin_manager.h"
#include "serial_handler.h"

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  PinManager::init();
  SerialHandler::init();
}

void loop() {
  SerialHandler::update();
}
