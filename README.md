# Unified Pin Control

## Overview

The **Unified Pin Control** code provides an easy way for controlling GPIO pins with flexible commands. This code allows you to control pins for various modes such as digital output, analog output (pwm), digital input, and analog input. It includes commands to set pin modes, values, and retrieve status information.

## Features

- Interactive serial command interface with help instructions
- Write digital and PWM values
- Read digital and analog values
- Deactivate pins when no longer required
- Periodic updates of pin states when a pin is in INPUT mode

## Commands

### 1. `GPIO(pin number) INPUT`

- **Function**: Configures the pin for digital input and reads its value.

- **Mode**: INPUT

- **Type**: DIGITAL

- **Expected Output**:
  
  `PIO(pin number) INPUT  DIGITAL  (0 or 1)`
  
  **Example:**
  
  `GPIO7 INPUT`
  
  This sets pin 7 to input mode and displays its current digital value.

### 2. `GPIO(pin number) ANALOG`

- **Function**: Configures the pin for analog input and reads its value.

- **Mode**: INPUT

- **Type**: ANALOG

- **Expected Output**:
  
  `GPIO(pin number) INPUT  ANALOG   (0 to 1023)`
  
  **Example:**
  
  `GPIO1 ANALOG`
  
  This sets pin 1 to analog input mode and displays its current analog value.

### 3. `GPIO(pin number) HIGH` or `GPIO(pin number) LOW`

- **Function**: Sets the specified pin to `HIGH` or `LOW`.

- **Mode**: OUTPUT

- **Type**: DIGITAL

- **Expected Output**:
  
  `GPIO(pin number) OUTPUT  DIGITAL  HIGH or LOW`
  
  **Example:**
  
  `GPIO13 HIGH`
  
  This sets pin 13 to `HIGH`.

### 4. `GPIO(pin number) (integer)`

- **Function**: Sets the specified pin to an analog value (PWM).

- **Mode**: OUTPUT

- **Type**: PWM

- **Expected Output**:
  
  `GPIO(pin number) OUTPUT  PWM     (integer value)`
  
  **Example:**
  
  `GPIO9 128`
  
  This sets pin 9 to a PWM value of 128.

### 5. `GPIO(pin number) REMOVE`

- **Function**: Deactivates the pin and removes it from control list.

### 6. `REMOVE ALL`

- **Function**: Deactivates all active pins and removes them from control list.

## Installation

1. Clone the repository:
   
   `git clone [GitHub - rambrosteam/unified_pin_control](https://github.com/rambrosteam/unified_pin_control).git`
   
   or download and unzip from github directly

2. Upload the code to your Arduino board.

## Usage

1. Open the Serial Monitor (115200 baud rate).
2. Enter commands based on the formats described above.
3. Observe pin status and updates as specified.

## LIMITATIONS

- Only numerical pin names are accepted. Pin names like A0, A1, etc., are not supported.
- Limited error messages. Set CORE DEBUG LEVEL: DEBUG in Arduino for additional messages

## Contributions

Feel free to fork this repository, submit issues, and create pull requests. Your contributions are welcome!

#### Other info:

Code was partially generated and fully commented using GPT
