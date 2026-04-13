# Web App — UI Guide

The `webapp/` directory contains the **Unified Pin Control** browser-side interface.
It is built with **Vue 3 + Vite + Tailwind CSS + PrimeVue** and communicates with
the firmware over the browser's [Web Serial API](https://developer.mozilla.org/en-US/docs/Web/API/Web_Serial_API).

---

## Running the Web App

```bash
cd webapp
npm install
npm run dev      # development server (Vite, HMR)
npm run build    # production build → dist/
npm run preview  # preview production build locally
```

Open the URL printed by Vite (usually `http://localhost:5173`).
Chrome / Edge 89+ required (Web Serial API).

---

## Architecture

```
webapp/src/
├── main.js                    Vue app bootstrap + PrimeVue setup
├── App.vue                    Root shell: connection guard + 4-tab layout
├── composables/
│   └── useSerial.js           Singleton Web Serial abstraction (connect, send, onMessage, onRaw)
├── stores/
│   └── pinStore.js            Pinia store — board def, pin state, all firmware commands
└── components/
    ├── ConnectionBar.vue      Sticky top bar: connect/disconnect, baud rate, board name, theme toggle
    ├── PinGrid.vue            Responsive grid of PinCard components
    ├── PinCard.vue            Individual pin card: mode selector, value control, freq/res display
    ├── PlotterTab.vue         Live serial plotter (Chart.js, CSV channel parsing)
    ├── TerminalTab.vue        Full serial terminal: RX/TX log, command input, autoscroll
    └── FlasherTab.vue         Firmware flasher UI: file picker, target selector, progress log
```

---

## Tab Layout

The app is organised into four tabs, all accessible once a device is connected:

| Tab | Component | Description |
|-----|-----------|-------------|
| 🔲 **Pin Control** | `PinGrid` + `PinCard` | View and control every GPIO pin on the connected board |
| 📈 **Serial Plotter** | `PlotterTab` | Live chart of comma-separated numeric data from serial RX |
| 🖥️ **Serial Terminal** | `TerminalTab` | Full RX/TX log with command input |
| ⚡ **Firmware Flasher** | `FlasherTab` | Firmware file upload UI (simulated flash flow) |

---

## Components

### `ConnectionBar.vue`

Always-visible sticky header.

- **Connect / Disconnect** button — triggers `useSerial.connect(baudRate)` / `disconnect()`
- **Baud rate** selector (visible when disconnected)
- **Board name + ID** badge (visible when connected, from `pinStore`)
- **Theme toggle** (dark/light) — persisted to `localStorage`
- Animated green pulse dot when connected, red when disconnected

---

### `PinCard.vue`

One card per pin, populated from `pinStore.pinList`.

**Header row:**
- Pin name (`font-mono`)
- Capability badge (`DIO`, `AI`, `PWM`, `DAC`, `SERVO`) with colour coding
- ℹ️ Info tooltip listing all available IN and OUT modes from the firmware

**Mode selector:** Grouped `<optgroup>` dropdown — Output modes above, Input modes below.
Changing mode sends `PIN_SET` via `pinStore.setPin()`.

**Value controls by mode:**

| Mode | Control |
|------|---------|
| `DOUT` | Toggle button (HIGH / LOW) |
| `DIN` | Coloured status badge |
| `PWM` | Range slider + number input; shows resolution (N-bit) and frequency below |
| `DAC` | Range slider + number input; shows resolution |
| `AIN` | Progress bar + `value / max` readout; max derived from `pin.res` |
| `SERVO` | Range slider 0–180° + number input |
| Other input | Mono numeric readout box |

**PWM / DAC resolution and frequency** are displayed under the slider, derived from
the firmware-reported `res` and `freq` fields in `UPDATE` responses.

---

### `PlotterTab.vue`

Live line chart plotting numeric serial data.

- Parses every raw RX line as comma-separated floats (e.g. `"1.23,45.6,7"`)
- Auto-discovers channels (`ch1`, `ch2`, …) as data arrives
- Per-channel colour-coded **toggle checkboxes** in the controls bar
- Configurable **rolling window** (50 / 100 / 200 / 500 points)
- **Clear Data** button resets chart and channel list
- Status bar shows current point count and latest values
- Chart.js loaded from CDN (no build-time dependency)
- Non-JSON serial lines (plain numeric data) are passed through `onRaw` — they do not
  interfere with the JSON command channel

---

### `TerminalTab.vue`

Full-featured serial monitor.

- Colour-coded log: ◀ RX lines in green, ▶ TX lines in blue
- Timestamp prefix on every entry (`HH:MM:SS`)
- **Command input** — press Enter or click Send
  - If input is valid JSON, it is sent as a JSON command via `useSerial.send()`
  - Otherwise it is sent as a raw string via `useSerial.sendRaw()`
- **Autoscroll** checkbox — auto-scrolls to newest line
- **Clear** button
- Rolling 500-line cap to prevent memory growth

---

### `FlasherTab.vue`

Firmware flasher UI.

- Drag-and-drop or click-to-browse **file picker** (`.bin` / `.hex`)
- Shows selected filename and size; click × to deselect
- **Target device** dropdown:
  - Espressif: ESP32, ESP32-S2, ESP32-S3, ESP32-C3, ESP8266
  - Raspberry Pi: RP2040, RP2350
  - AVR/Arduino: Uno, Mega, Nano
  - STM32 (generic)
- Animated **progress bar** with step-by-step status messages
- Timestamped **log panel** showing each flash step
- ⚠️ Note: actual flashing requires backend integration (esptool, avrdude, etc.).
  The current UI demonstrates the complete flash workflow with a simulated sequence.

---

## `useSerial.js` Composable

Singleton — all components share one serial port connection.

### API

| Export | Type | Description |
|--------|------|-------------|
| `isConnected` | `Readonly<Ref<boolean>>` | Reactive connection state |
| `lastError` | `Readonly<Ref<string\|null>>` | Last connection error message |
| `connect(baudRate)` | `async fn` | Open Web Serial port at given baud rate |
| `disconnect()` | `async fn` | Close port and reset state |
| `send(cmdObject)` | `async fn` | JSON-stringify `cmdObject` and write as a newline-terminated line |
| `sendRaw(text)` | `async fn` | Write a raw string line (appends `\n` if missing) |
| `onMessage(handler)` | `fn → cleanup` | Register handler called for every parsed JSON line received. Returns cleanup fn. |
| `onRaw(handler)` | `fn → cleanup` | Register handler called for every raw line (RX and TX). Handler signature: `(line: string, dir: 'rx'\|'tx') => void`. Returns cleanup fn. |

### Notes

- Both `send()` and `sendRaw()` emit the sent line through `onRaw` handlers with `dir = 'tx'`,
  so the terminal always shows outgoing commands.
- Non-JSON RX lines (e.g. CSV numeric data for the plotter) are passed to `onRaw` handlers
  but silently ignored by `onMessage` handlers.
- Call the cleanup function returned by `onMessage` / `onRaw` in `onUnmounted()` to prevent
  handler leaks.

---

## `pinStore.js` (Pinia)

### State

| Property | Type | Description |
|----------|------|-------------|
| `boardName` | `Ref<string>` | Board name from `GET_DEF` response |
| `boardId` | `Ref<string>` | Board ID slug |
| `pins` | `Map<string, PinObj>` | Reactive map of all pins keyed by name |
| `pinOrder` | `Ref<string[]>` | Pin names in original `GET_DEF` order |
| `pinList` | `ComputedRef<PinObj[]>` | Ordered array of pin objects |

### PinObj Shape

```js
{
  name: string,      // e.g. "D2", "GPIO4"
  cap: string,       // legacy cap string (e.g. "DIO", "AI", "PWM")
  inCaps: string[],  // available input modes (e.g. ["DIN", "AIN"])
  outCaps: string[], // available output modes (e.g. ["DOUT", "PWM"])
  mode: string|null, // active mode, null = disabled
  value: number,     // current value
  res: number,       // PWM/ADC bit resolution
  freq: number,      // PWM frequency in Hz
}
```

### Actions

| Method | Description |
|--------|-------------|
| `getDef()` | Send `GET_DEF` |
| `getConfig()` | Send `GET_CONFIG` |
| `setPin({pin, mode, value, res?, freq?, ack?})` | Send `PIN_SET` with full parameters |
| `setValue(pinName, value)` | Set value on an already-configured pin (sends `PIN_SET` with current mode) |
| `setMode(pinName, mode)` | Change pin mode (value resets to 0) |
| `requestUpdate()` | Send `UPDATE` |
| `reset()` | Send `RESET` and clear local state |
| `saveConfig()` | Send `SAVE_CONFIG` |
| `loadDef(response)` | Process `GET_DEF` response |
| `loadConfig(response)` | Process `GET_CONFIG` response |
| `applyUpdate(response)` | Process `UPDATE` response |
| `applyAck(response)` | Process `PIN_SET` ack |

---

## See Also

- [`docs/API.md`](./API.md) — Firmware serial command reference
- [`docs/GENERAL.md`](./GENERAL.md) — Firmware implementation guide
