// Structure to hold information about each pin
struct PinInfoList {
  int8_t pinNumber;    // Pin number
  int8_t pin_mode;     // Pin mode (e.g., INPUT, OUTPUT)
  int16_t pinValue;    // Current value read from the pin
  String modeType;     // Type of mode (e.g., INPUT, OUTPUT)
  String valueType;    // Type of value (e.g., DIGITAL, ANALOG, PWM)
};

// Array to store information for up to 50 pins
PinInfoList pinInfo[50];

void setup() {
  Serial.begin(115200);  // Initialize serial communication at 115200 baud rate
  delay(500);            // Short delay to stabilize serial communication
  initializePins();     // Initialize pinInfo array
  delay(500);            // Short delay to ensure pins are initialized

  printHelpMessage();   // Print the help message with command instructions
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n'); // Read command from serial input
    input.trim();                             // Remove any leading or trailing whitespace
    handleCommand(input);                     // Process the received command
  }
  nonBlockingDelay();  // Check if it's time to update and print pin information
}

void initializePins() {
  // Initialize all pins to default values
  for (int i = 0; i < 50; i++) {
    pinInfo[i].pinNumber = -1;
    pinInfo[i].pin_mode = 0;
    pinInfo[i].pinValue = 0;
    pinInfo[i].modeType = "NONE";
    pinInfo[i].valueType = "N/A";
  }
}

void handleCommand(const String& input) {
  // Handle different types of commands
  if (input == "h" || input == "H" || input == "HELP") {
    printHelpMessage();  // Print the help message
  } else if (input.startsWith("GPIO") && input.indexOf(' ') != -1) {
    // Extract pin number and command from input
    String pinNumberStr = input.substring(4);
    int spaceIndex = pinNumberStr.indexOf(' ');

    if (spaceIndex != -1) {
      int pinNumber = pinNumberStr.substring(0, spaceIndex).toInt();
      String commandStr = pinNumberStr.substring(spaceIndex + 1);
      int equalsIndex = commandStr.indexOf('=');

      // Handle commands with '=' (e.g., GPIO(pin) mode=value)
      if (equalsIndex != -1) {
        String modeStr = commandStr.substring(0, equalsIndex);
        int pinValue = commandStr.substring(equalsIndex + 1).toInt();
        processCommand(pinNumber, modeStr, pinValue);
      } else {
        // Handle commands without '=' (e.g., GPIO(pin) INPUT)
        processCommand(pinNumber, commandStr, 0);
      }
    } else {
      int pinNumber = pinNumberStr.toInt();
      processCommand(pinNumber, "", 0);
    }
  } else if (input == "REMOVE ALL") {
    removeAllPins();  // Handle REMOVE ALL command to deactivate all pins
  } else {
    Serial.println("Invalid command format. Use GPIO(pin number) HIGH or LOW, GPIO(pin number) (integer), GPIO(pin number) REMOVE, GPIO(pin number) INPUT, GPIO(pin number) ANALOG, REMOVE ALL, or h/H/HELP for command help.");
  }
}

void processCommand(int pinNumber, const String& commandStr, int pinValue) {
  int8_t pin_mode = 0;       // Default pin mode (NONE)
  String modeType = "NONE";  // Default mode type
  String valueType = "N/A";  // Default value type

  // Process different commands
  if (commandStr == "REMOVE") {
    pin_mode = 0;
    modeType = "NONE";
    valueType = "N/A";
    pinMode(pinNumber, INPUT);  // Deactivate pin by setting it to INPUT mode
  } else if (commandStr == "INPUT") {
    pin_mode = 1;
    modeType = "INPUT";
    valueType = "DIGITAL";
    pinMode(pinNumber, INPUT);
    pinValue = digitalRead(pinNumber);  // Read digital value from pin
  } else if (commandStr == "ANALOG") {
    pin_mode = 3;
    modeType = "INPUT";
    valueType = "ANALOG";
    pinMode(pinNumber, INPUT);
    pinValue = analogRead(pinNumber);  // Read analog value from pin
  } else if (commandStr == "HIGH" || commandStr == "LOW") {
    pin_mode = 2;
    modeType = "OUTPUT";
    valueType = "DIGITAL";
    pinMode(pinNumber, OUTPUT);
    pinValue = (commandStr == "LOW" ? LOW : HIGH);
    digitalWrite(pinNumber, pinValue);  // Write digital value to pin
  } else if (commandStr.toInt() >= 0) {
    pin_mode = 4;
    modeType = "OUTPUT";
    valueType = "PWM";
    pinValue = commandStr.toInt();
    pinMode(pinNumber, OUTPUT);
    analogWrite(pinNumber, pinValue);  // Write PWM value to pin
  } else {
    Serial.println("Invalid command format.");
    return;
  }

  // Update pinInfo array with the current pin status
  pinInfo[pinNumber].pinNumber = pinNumber;
  pinInfo[pinNumber].pin_mode = pin_mode;
  pinInfo[pinNumber].pinValue = pinValue;
  pinInfo[pinNumber].modeType = modeType;
  pinInfo[pinNumber].valueType = valueType;

  updatePinValues();  // Update pin values for INPUT and ANALOG modes
  printPinInfo();     // Print the updated pin information
}

void removeAllPins() {
  // Set all active pins to NONE mode
  for (int i = 0; i < 50; i++) {
    if (pinInfo[i].pin_mode != 0) {
      pinMode(pinInfo[i].pinNumber, INPUT);  // Deactivate pin by setting it to INPUT mode
      pinInfo[i].pin_mode = 0;
      pinInfo[i].modeType = "NONE";
      pinInfo[i].valueType = "N/A";
    }
  }
  printPinInfo();  // Print the updated pin information after removing all pins
}

void updatePinValues() {
  // Update the values of pins set to INPUT or ANALOG mode
  for (int i = 0; i < 50; i++) {
    if (pinInfo[i].pin_mode == 1) {  // INPUT mode
      pinInfo[i].pinValue = digitalRead(pinInfo[i].pinNumber);
    } else if (pinInfo[i].pin_mode == 3) {  // ANALOG mode
      pinInfo[i].pinValue = analogRead(pinInfo[i].pinNumber);
    }
  }
}

void printPinInfo() {
  // Print the current state of all active pins
  Serial.println("PIN\tMODE\tTYPE\tVALUE");
  for (int i = 0; i < 50; i++) {
    if (pinInfo[i].pin_mode != 0) {
      Serial.print("GPIO");
      Serial.print(pinInfo[i].pinNumber);
      Serial.print("\t");
      Serial.print(pinInfo[i].modeType);
      Serial.print("\t");
      Serial.print(pinInfo[i].valueType);
      Serial.print("\t");
      Serial.println(pinInfo[i].pinValue);
    }
  }
}

void printHelpMessage() {
  // Print instructions for available commands
  Serial.println("Unified Pin Control");
  Serial.println("Available commands:");
  Serial.println("1. GPIO(pin) INPUT for digitalRead");
  Serial.println("2. GPIO(pin) ANALOG for analogRead");
  Serial.println("3. GPIO(pin) HIGH or LOW for digitalWrite");
  Serial.println("4. GPIO(pin) (integer) for analogWrite");
  Serial.println("5. GPIO(pin) REMOVE to deactivate pin");
  Serial.println("6. REMOVE ALL to set all active pins to NONE mode");
  Serial.println("7. h, H, or HELP for command help");
}

void nonBlockingDelay() {
  static unsigned long lastMillis = 0;  // Store the last time the delay was executed
  unsigned long currentMillis = millis(); // Get the current time

  // Check if 500ms have passed since the last update
  if (currentMillis - lastMillis >= 500) {
    lastMillis = currentMillis;  // Update the lastMillis to the current time

    // Check if any pin is set to INPUT or ANALOG mode and update pin values and print information
    for (int i = 0; i < 50; i++) {
      if (pinInfo[i].pin_mode == 1 || pinInfo[i].pin_mode == 3) {
        updatePinValues();
        printPinInfo();
        break;  // Print only once every 500ms
      }
    }
  }
}
