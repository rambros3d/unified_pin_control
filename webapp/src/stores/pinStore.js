import { defineStore } from 'pinia'
import { ref, reactive, computed } from 'vue'
import { useSerial } from '@/composables/useSerial'

export const usePinStore = defineStore('pins', () => {
  const { send } = useSerial()

  // ─── Board meta
  const boardName = ref('')
  const boardId = ref('')

  // ─── Pin map: key = pin name string, value = PinDef + PinState
  // Structure:
  // {
  //   name: string
  //   cap: string            legacy cap field (DIO, AI, PWM, SERVO, DAC)
  //   inCaps: string[]       e.g. ['DIN', 'AIN']
  //   outCaps: string[]      e.g. ['DOUT', 'PWM', 'DAC']
  //   mode: string | null    currently active mode
  //   value: number
  //   res: number            PWM/ADC resolution bits
  //   freq: number           PWM frequency Hz
  // }
  const pins = reactive(new Map())

  // ─── Ordered list of pin names (preserves GET_DEF order)
  const pinOrder = ref([])

  const pinList = computed(() =>
    pinOrder.value.map(name => pins.get(name)).filter(Boolean)
  )

  // ─── Load board definition from GET_DEF response
  function loadDef(response) {
    boardName.value = response.board ?? ''
    boardId.value = response.id ?? ''
    pins.clear()
    pinOrder.value = []
    for (const p of (response.pins ?? [])) {
      pins.set(p.pin, {
        name: p.pin,
        cap: p.cap ?? '',
        inCaps: p.in ?? [],
        outCaps: p.out ?? [],
        mode: null,
        value: 0,
        res: 8,
        freq: 1000
      })
      pinOrder.value.push(p.pin)
    }
  }

  // ─── Apply GET_CONFIG response
  function loadConfig(response) {
    for (const entry of (response.config ?? [])) {
      const pin = pins.get(entry.pin)
      if (pin) pin.mode = entry.mode
    }
  }

  // ─── Apply UPDATE response
  function applyUpdate(response) {
    for (const p of (response.pins ?? [])) {
      const pin = pins.get(p.pin)
      if (!pin) continue
      pin.mode = p.mode
      pin.value = p.value ?? 0
      if (p.res  !== undefined) pin.res  = p.res
      if (p.freq !== undefined) pin.freq = p.freq
    }
  }

  // ─── Apply PIN_SET ack response
  function applyAck(response) {
    const pin = pins.get(response.pin)
    if (!pin) return
    pin.mode  = response.mode
    pin.value = response.value ?? 0
  }

  // ─── Send commands

  async function getDef() {
    await send({ cmd: 'GET_DEF' })
  }

  async function getConfig() {
    await send({ cmd: 'GET_CONFIG' })
  }

  // Full pin set (mode + value + optional res/freq)
  async function setPin({ pin, mode, value = 0, res, freq, ack = 1 }) {
    const p = pins.get(pin)
    const payload = { cmd: 'PIN_SET', pin, mode, value, ack }
    if (res  !== undefined) payload.res  = res
    if (freq !== undefined) payload.freq = freq
    await send(payload)
    // Optimistic update
    if (p) {
      p.mode  = mode
      p.value = value
      if (res  !== undefined) p.res  = res
      if (freq !== undefined) p.freq = freq
    }
  }

  // Convenience: just update the value of an already-configured pin
  async function setValue(pinName, value) {
    const p = pins.get(pinName)
    if (!p || !p.mode) return
    await send({ cmd: 'PIN_SET', pin: pinName, mode: p.mode, value, ack: 0 })
    p.value = value
  }

  // Convenience: just change the mode
  async function setMode(pinName, mode) {
    await setPin({ pin: pinName, mode, value: 0 })
  }

  async function requestUpdate() {
    await send({ cmd: 'UPDATE' })
  }

  async function reset() {
    await send({ cmd: 'RESET' })
    for (const p of pins.values()) {
      p.mode  = null
      p.value = 0
    }
  }

  async function saveConfig() {
    await send({ cmd: 'SAVE_CONFIG' })
  }

  return {
    boardName, boardId,
    pins, pinOrder, pinList,
    loadDef, loadConfig, applyUpdate, applyAck,
    getDef, getConfig,
    setPin, setValue, setMode,
    requestUpdate, reset, saveConfig
  }
})
