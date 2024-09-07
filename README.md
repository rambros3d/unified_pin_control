# # Unified Pin Control

## Overview

The **Unified Pin Control** code provides an Arduino library for managing GPIO pins with flexible commands. This library allows you to control pins for various modes such as digital output, analog output, digital input, and analog input. It includes commands to set pin modes, values, and retrieve status information.

## Features

- Control digital and analog pins with simple commands
- Dynamically update pin values and modes
- Non-blocking periodic updates for input and analog modes
- Detailed command handling with helpful feedback

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
  
  `GPIOA1 ANALOG`
  
  This sets pin A1 to analog input mode and displays its current analog value.

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

- **Function**: Deactivates the specified pin.
- **Mode**: NONE
- **Type**: N/A
- **Output**: No output on the serial monitor.

### 6. `REMOVE ALL`

- **Function**: Sets all active pins to NONE mode.
- **Mode**: NONE
- **Type**: N/A
- **Output**: No output on the serial monitor.

## Installation

1. Clone the repository:
   
   bash
   
   Copy code
   
   `git clone https://github.com/yourusername/unified-pin-control.git`

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


