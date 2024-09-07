# Unified Pin Control

## Description

The **Unified Pin Control** system is an Arduino sketch designed to simplify GPIO pin control through serial communication. This system supports configuring and controlling GPIO pins with various commands. You can set pin modes, write values, read inputs, and deactivate pins, all while receiving real-time updates.

## Features

- Configure pin modes: `INPUT`, `OUTPUT`, `INPUT_PULLDOWN`
- Write digital values: `HIGH`, `LOW`
- Write PWM values with automatic range constraints
- Read digital and analog values
- Deactivate pins and reset all pin configurations
- Real-time status updates at predefined intervals

## Commands

### INPUT - digitalRead

**Description**: Configures the selected pin as an input pin for digital reads.

**Usage**:

```c++
GPIO(pin number) INPUT
```

**Example**:

```c++
GPIO4 INPUT
```

**Expected Output**:

```c++
PIN     MODE    TYPE    VALUE
GPIO4  INPUT   DIGITAL LOW
```

### GPIO(pin number) INPUT_PULLDOWN

**Description**: Configures the selected pin as an input pin with an internal pull-down resistor.

**Usage**:

```c++
GPIO(pin number) INPUT_PULLDOWN
```

**Example**:

```c++
GPIO2 INPUT_PULLDOWN
```

**Expected Output**:

```c++
PIN     MODE    TYPE    VALUE
GPIO2  INPUT   LOGIC   LOW
```

### ANALOG - analogRead

**Description**: Configures the selected pin as an input pin for analog reads.

**Usage**:

```c++
GPIO(pin number) ANALOG
```

**Example**:

```c++
GPIO12 ANALOG
```

**Expected Output**:

```c++
PIN     MODE    TYPE    VALUE
GPIO12  INPUT   ANALOG  512
```

### HIGH - digitalWrite

**Description**: Configures the selected pin as an output and sets it to HIGH.

**Usage**:

```c++
GPIO(pin number) HIGH
```

**Example**:

```c++
GPIO14 HIGH
```

**Expected Output**:

```c++
PIN     MODE    TYPE    VALUE
GPIO14  OUTPUT  DIGITAL HIGH
```

### LOW - digitalWrite

**Description**: Configures the selected pin as an output and sets it to LOW.

**Usage**:

```c++
GPIO(pin number) LOW
```

**Example**:

```c++
GPIO15 LOW
```

**Expected Output**:

```c++
PIN     MODE    TYPE    VALUE
GPIO15  OUTPUT  DIGITAL LOW
```

### PWM - analogWrite

**Description**: Configures the selected pin as an output and sets its PWM value. The value must be within the range of 0 to 255 (default 8-bit resolution).

**Usage**:

```c++
GPIO(pin number) (integer)
```

**Example**:

```c++
GPIO12 128
```

**Expected Output**:

```c++
PIN     MODE    TYPE    VALUE
GPIO12  OUTPUT  PWM     128
```

### REMOVE

**Description**: Deactivates the selected pin.

**Usage**:

```c++
GPIO(pin number) REMOVE
```

### **Example**:

```c++
GPIO22 REMOVE
```

### REMOVE ALL

**Description**: Deactivates all configured pins.

**Usage**:

```C++
REMOVE ALL
```

### HELP - print available commands

**Description**: Displays the help message with a summary of available commands.

**Usage**:

```C++
h
H
HELP
```

**Example**:

```C++
HELP
```

**Expected Output**:

```C++
Unified Pin Control
Available commands:
1. GPIO(pin) INPUT for digitalRead
2. GPIO(pin) ANALOG for analogRead
3. GPIO(pin) INPUT_PULLDOWN for digitalRead with pull-down resistor
4. GPIO(pin) HIGH or LOW for digitalWrite
5. GPIO(pin) (integer) for analogWrite
6. GPIO(pin) REMOVE to deactivate pin
7. REMOVE ALL to deactivate all pins
8. h, H, or HELP for command help
```

## Pin Status

The system displays the status of each pin, including:

- **PIN**: The GPIO pin number.
- **MODE**: The pin's mode (e.g., INPUT, OUTPUT).
- **TYPE**: The type of value read or written (e.g., DIGITAL, ANALOG, PWM).
- **VALUE**: The current value of the pin.

---

## Installation

1. Upload the sketch to your Arduino board using the Arduino IDE.
2. Open the Serial Monitor at a baud rate of 115200.
3. Enter commands as described above to configure and manage the GPIO pins.

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---
