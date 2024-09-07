/*
 * Unified Pin Control System
 *
 * This Arduino sketch provides an easy way for pin control interface via serial communication. 
 * It allows you to configure and manage GPIO pins using various commands:
 * 
 * 1. Set pin mode to INPUT for digital or analog readings.
 * 2. Set pin mode to OUTPUT for digital writes or PWM outputs.
 * 3. Remove pin configurations or deactivate pins.
 * 4. Get help and see the available commands.
 * 
 * The system supports real-time updates and printing of pin states every 500ms if the pin is in INPUT or ANALOG mode.
 * It handles commands in the format: GPIO(pin number) <command> and prints a status report of active pins.
 *
 * Commands:
 * - GPIO(pin) INPUT for digitalRead
 * - GPIO(pin) ANALOG for analogRead
 * - GPIO(pin) HIGH or LOW for digitalWrite
 * - GPIO(pin) (integer) for analogWrite
 * - GPIO(pin) REMOVE to deactivate pin
 * - REMOVE ALL to set all active pins to NONE mode
 * - h, H, or HELP for command help
 */

// Structure to hold information about each pin
struct PinInfoList {
  int8_t pinNumber;  // Pin number
  int8_t pin_mode;   // Pin mode (e.g., INPUT, OUTPUT)
  int16_t pinValue;  // Current value read from the pin
  String modeType;   // Type of mode (e.g., INPUT, OUTPUT)
  String valueType;  // Type of value (e.g., DIGITAL, ANALOG, PWM)
};

// Array to store information for up to 50 pins
PinInfoList pinInfo[50];

uint8_t adcResolution = 10;      // resolution of ADC in bits
uint8_t pwmResolution = 8;       // resolution of pwm in bits
uint16_t pwmFrequency = 1000;    // frequency of pwm in Hz
uint16_t updateInterval = 1000;  // frequency of pwm in Hz

void setup() {
  initializePins();      // Initialize pinInfo array
  delay(500);            // Short delay to stabilize serial communication
  Serial.begin(115200);  // Initialize serial communication at 115200 baud rate
  printHelpMessage();    // Print the help message with command instructions
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');  // Read command from serial input
    input.trim();                                 // Remove any leading or trailing whitespace
    handleCommand(input);                         // Process the received command
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
    Serial.println("Invalid command format. Use GPIO(pin number) HIGH or LOW, GPIO(pin number) (integer), GPIO(pin number) REMOVE, GPIO(pin number) INPUT, GPIO(pin number) ANALOG, GPIO(pin number) INPUT_PULLDOWN, REMOVE ALL, or h/H/HELP for command help.");
  }
}

void processCommand(int pinNumber, const String& commandStr, int pinValue) {
  int8_t pin_mode = 0;       // Default pin mode (NONE)
  String modeType = "NONE";  // Default mode type
  String valueType = "N/A";  // Default value type

  // Compute the maximum PWM value based on pwmResolution
  int maxPWMValue = (1 << pwmResolution) - 1;  // 2^pwmResolution - 1

  // Process different commands
  if (commandStr == "REMOVE") {
    pin_mode = 0;
    modeType = "NONE";
    valueType = "N/A";
    pinMode(pinNumber, INPUT);  // Deactivate pin by setting it to INPUT mode
  } else if (commandStr == "INPUT") {
    pin_mode = 1;
    modeType = "INPUT";
    valueType = "LOGIC^";
    pinMode(pinNumber, INPUT_PULLUP);
    pinValue = digitalRead(pinNumber);           // Read digital value from pin
    pinValue = (pinValue == HIGH) ? HIGH : LOW;  // Convert to HIGH/LOW
  } else if (commandStr == "INPUT_PULLDOWN") {
    pin_mode = 1;
    modeType = "INPUT";
    valueType = "LOGIC";
    pinMode(pinNumber, INPUT_PULLDOWN);          // Set pin mode to INPUT_PULLDOWN
    pinValue = digitalRead(pinNumber);           // Read digital value from pin
    pinValue = (pinValue == HIGH) ? HIGH : LOW;  // Convert to HIGH/LOW
  } else if (commandStr == "HIGH" || commandStr == "LOW") {
    pin_mode = 2;
    modeType = "OUTPUT";
    valueType = "DIGITAL";
    pinMode(pinNumber, OUTPUT);
    pinValue = (commandStr == "LOW" ? LOW : HIGH);
    digitalWrite(pinNumber, pinValue);  // Write digital value to pin
  } else if (commandStr == "ANALOG") {
    pin_mode = 3;
    modeType = "INPUT";
    valueType = "ANALOG";
    pinMode(pinNumber, INPUT);
    pinValue = analogRead(pinNumber);  // Read analog value from pin
  } else if (commandStr.toInt() >= 0) {
    pin_mode = 4;
    modeType = "OUTPUT";
    valueType = "PWM";
    pinValue = constrain(commandStr.toInt(), 0, maxPWMValue);  // Constrain PWM value
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
    if (pinInfo[i].pin_mode == 1 || pinInfo[i].pin_mode == 3 || pinInfo[i].pin_mode == 5) {  // INPUT, ANALOG, or INPUT_PULLDOWN mode
      pinInfo[i].pinValue = digitalRead(pinInfo[i].pinNumber);
      pinInfo[i].pinValue = (pinInfo[i].pinValue == HIGH) ? HIGH : LOW;  // Convert to HIGH/LOW
    } else if (pinInfo[i].pin_mode == 3) {                               // ANALOG mode
      pinInfo[i].pinValue = analogRead(pinInfo[i].pinNumber);
    }
  }
}

void printPinInfo() {
  // Print the current state of all active pins
  Serial.println();
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
      if (pinInfo[i].pin_mode == 1) {
        if (pinInfo[i].pinValue == 1) {
          Serial.println("HIGH");
        } else {
          Serial.println("LOW");
        }
      } else {
        Serial.println(pinInfo[i].pinValue);
      }
    }
  }
}

void printHelpMessage() {
  // Print instructions for available commands
  Serial.println("\n");
  Serial.println("Unified Pin Control\nAvailable commands:");
  Serial.println("1. GPIO(pin) INPUT for digitalRead");
  Serial.println("2. GPIO(pin) ANALOG for analogRead");
  Serial.println("3. GPIO(pin) INPUT_PULLDOWN for digitalRead with pull-down resistor");
  Serial.println("4. GPIO(pin) HIGH or LOW for digitalWrite");
  Serial.println("5. GPIO(pin) (integer) for analogWrite");
  Serial.println("6. GPIO(pin) REMOVE to deactivate pin");
  Serial.println("7. REMOVE ALL to deactivate all pins");
  Serial.println("8. h, H, or HELP for command help");
}

void nonBlockingDelay() {
  static unsigned long lastMillis = 0;     // Store the last time the delay was executed
  unsigned long currentMillis = millis();  // Get the current time

  // Check if updateInterval has passed since the last update
  if (currentMillis - lastMillis >= updateInterval) {
    lastMillis = currentMillis;  // Update the lastMillis to the current time

    // Check if any pin is set to INPUT, ANALOG, or INPUT_PULLDOWN mode and update pin values and print information
    for (int i = 0; i < 50; i++) {
      if (pinInfo[i].pin_mode == 1 || pinInfo[i].pin_mode == 3 || pinInfo[i].pin_mode == 5) {
        updatePinValues();
        printPinInfo();
        break;
      }
    }
  }
}
