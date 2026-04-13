import { defineStore } from 'pinia'
import { ref, reactive, computed } from 'vue'
import { useSerial } from '@/composables/useSerial'

// Cap char → firmware mode string (must match pin_manager.cpp MODE_STRINGS exactly)
const CAP_MODE = {
  I: 'IN',
  U: 'IN_UP',
  D: 'IN_DN',
  A: 'ADC',
  T: 'TOUCH',
  O: 'OUT',
  Z: 'OUT_OD',
  P: 'PWM',
  C: 'DAC'
}

// Input caps (read-only pin modes)
const INPUT_CAPS = new Set(['IN', 'IN_UP', 'IN_DN', 'ADC', 'TOUCH'])

function parseCaps(capsStr) {
  const inCaps  = []
  const outCaps = []
  for (const ch of capsStr) {
    const mode = CAP_MODE[ch]
    if (!mode) continue
    if (INPUT_CAPS.has(mode)) inCaps.push(mode)
    else                       outCaps.push(mode)
  }
  return { inCaps, outCaps }
}

export const usePinStore = defineStore('pins', () => {
  const { send } = useSerial()

  // ─── Board meta
  const boardName = ref('')
  const boardId   = ref('')

  // ─── Pin map: key = pin label string
  // Each entry:
  // { name, caps, inCaps, outCaps, mode, value, res, freq }
  const pins     = reactive(new Map())
  const pinOrder = ref([])

  const pinList = computed(() =>
    pinOrder.value.map(name => pins.get(name)).filter(Boolean)
  )

  // ─── BINFO handler
  function loadDef({ id, name, pins: pinDefs }) {
    boardId.value   = id
    boardName.value = name
    pins.clear()
    pinOrder.value  = []
    for (const { pin, caps } of pinDefs) {
      const { inCaps, outCaps } = parseCaps(caps)
      pins.set(pin, {
        name: pin,
        caps,
        inCaps,
        outCaps,
        mode:  null,
        value: 0,
        res:   8,
        freq:  1000
      })
      pinOrder.value.push(pin)
    }
  }

  // ─── CONFIG handler
  function loadConfig({ config }) {
    for (const { pin, mode } of config) {
      const p = pins.get(pin)
      if (p) p.mode = mode
    }
  }

  // ─── STAT handler (delta updates)
  function applyUpdates({ updates }) {
    for (const { pin, value } of updates) {
      const p = pins.get(pin)
      if (p) p.value = value
    }
  }

  // ─── ACK handler
  function applyAck({ pin, mode, value }) {
    const p = pins.get(pin)
    if (!p) return
    p.mode  = mode
    p.value = value
  }

  // ─── Commands (plain-text, mode strings match firmware exactly)

  async function getDef() {
    await send('GET_DEF')
  }

  async function getConfig() {
    await send('GET_CONFIG')
  }

  async function getStatus() {
    await send('GET_STATUS')
  }

  // PIN_SET <pin> <mode> [value] [res] [freq]
  async function setPin({ pin, mode, value = 0, res, freq }) {
    const parts = ['PIN_SET', pin, mode, value]
    if (res  !== undefined) parts.push(res)
    if (freq !== undefined) parts.push(freq)
    await send(parts.join(' '))
    // Optimistic update
    const p = pins.get(pin)
    if (p) {
      p.mode  = mode
      p.value = value
      if (res  !== undefined) p.res  = res
      if (freq !== undefined) p.freq = freq
    }
  }

  async function setValue(pinName, value) {
    const p = pins.get(pinName)
    if (!p || !p.mode) return
    await send(`PIN_SET ${pinName} ${p.mode} ${value}`)
    p.value = value
  }

  async function setMode(pinName, mode) {
    await setPin({ pin: pinName, mode, value: 0 })
  }

  async function reset() {
    await send('RESET')
    for (const p of pins.values()) {
      p.mode  = null
      p.value = 0
    }
  }

  async function saveConfig() {
    await send('SAVE_CONFIG')
  }

  return {
    boardName, boardId,
    pins, pinOrder, pinList,
    loadDef, loadConfig, applyUpdates, applyAck,
    getDef, getConfig, getStatus,
    setPin, setValue, setMode,
    reset, saveConfig
  }
})
