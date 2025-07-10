# Unified Pin Control API

## 📌 Overview

- **Transport:** WebSerial (USB Serial)
- **Encoding:** UTF-8 JSON, newline (`\n`) terminated
- **Baud Rate:** (115200 recommended)


| Command     | Description                         | Example                                              |
| ----------- | ----------------------------------- | ---------------------------------------------------- |
| `GET_INFO`  | Get board name and pin capabilities | `{ "cmd": "GET_INFO" }`                              |
| `PIN_READ`  | Read digital/analog value of a pin  | `{ "cmd": "PIN_READ", "pin": 5, "mode": "digital" }` |
| `PIN_WRITE` | Write digital value to a pin        | `{ "cmd": "PIN_WRITE", "pin": 5, "value": 1 }`       |
| `PWM_WRITE` | Write PWM value (0-255) to a pin    | `{ "cmd": "PWM_WRITE", "pin": 5, "value": 128 }`     |
| `RESET`     | Reset pin states                    | `{ "cmd": "RESET" }`                                 |

## GET_INFO

### Command

```
{ "cmd": "GET_INFO" }
```

### Response

```
{
  "name": "ESP32-S3 dev board",
  "board": esp32s3
  "pins": [
    {
      "pin": GPIO0,
      "in": ["IN", "IN_UP", "IN_DOWN"],
      "out": ["OUT", "PWM", "ANA"]
    },
    .
    .
    .
    .
  ]
}

```



