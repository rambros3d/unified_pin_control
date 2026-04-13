import { defineStore } from 'pinia'
import { ref, reactive, computed } from 'vue'
import { useSerial } from '@/composables/useSerial'

// ─── Firmware caps string → webapp mode arrays
// BINFO caps chars: I=IN, U=IN_UP, D=IN_DN, A=ADC, T=TOUCH, O=OUT, Z=OUT_OD, P=PWM, C=DAC
function parseCaps(capsStr) {
  const inCaps  = []
  const outCaps = []
  for (const ch of capsStr) {
    switch (ch) {
      case 'I': inCaps.push('IN');     break
      case 'U': inCaps.push('IN_UP'); break
      case 'D': inCaps.push('IN_DN'); break
      case 'A': inCaps.push('ADC');   break
      case 'T': inCaps.push('TOUCH'); break
      case 'O': outCaps.push('OUT');  break
      case 'Z': outCaps.push('OUT_OD'); break
      case 'P': outCaps.push('PWM');  break
      case 'C': outCaps.push('DAC');  break
    }
  }
  return {
    inCaps:  [...new Set(inCaps)],
    outCaps: [...new Set(outCaps)],
  }
}

export const usePinStore = defineStore('pins', () => {
  const { send } = useSerial()

  const boardName = ref('')
  const boardId   = ref('')
  const pins      = reactive(new Map())
  const pinOrder  = ref([])

  const pinList = computed(() =>
    pinOrder.value.map(name => pins.get(name)).filter(Boolean)
  )

  // Load board definition from BINFO payload
  // payload: { id, name, pins: [ { pin: 'GPIO0', caps: 'IUDATOZP' }, ... ] }
  function loadDef(payload) {
    boardName.value = payload.name ?? ''
    boardId.value   = payload.id   ?? ''
    pins.clear()
    pinOrder.value  = []
    for (const entry of (payload.pins ?? [])) {
      const { inCaps, outCaps } = parseCaps(entry.caps ?? '')
      pins.set(entry.pin, {
        name: entry.pin,
        inCaps,
        outCaps,
        mode:  null,
        value: 0,
        res:   8,
        freq:  1000,
      })
      pinOrder.value.push(entry.pin)
    }
  }

  // Apply CONFIG payload: { config: [ { pin, mode }, ... ] }
  function loadConfig(payload) {
    for (const entry of (payload.config ?? [])) {
      const pin = pins.get(entry.pin)
      if (pin) pin.mode = entry.mode
    }
  }

  // Apply STAT payload: { updates: [ { pin, value }, ... ] }
  function applyUpdate(payload) {
    for (const entry of (payload.updates ?? [])) {
      const pin = pins.get(entry.pin)
      if (pin) pin.value = entry.value
    }
  }

  // Apply ACK payload: { pin, mode, value }
  function applyAck(payload) {
    const pin = pins.get(payload.pin)
    if (!pin) return
    pin.mode  = payload.mode
    pin.value = payload.value ?? 0
  }

  // Disable a single pin: reset it to INPUT on the board, clear mode locally
  async function disablePin(pinName) {
    await send(`PIN_SET ${pinName} IN`)
    const pin = pins.get(pinName)
    if (pin) { pin.mode = null; pin.value = 0 }
  }

  // Full pin configure: PIN_SET <pin> <mode> [value] [res] [freq]
  async function setPin({ pin, mode, value = 0, res, freq }) {
    const p = pins.get(pin)
    let cmd = `PIN_SET ${pin} ${mode} ${value}`
    if (res  !== undefined) cmd += ` ${res}`
    if (freq !== undefined) cmd += ` ${freq}`
    await send(cmd)
    // Optimistic update
    if (p) {
      p.mode  = mode
      p.value = value
      if (res  !== undefined) p.res  = res
      if (freq !== undefined) p.freq = freq
    }
  }

  // Update value only on an already-configured pin
  async function setValue(pinName, value) {
    const p = pins.get(pinName)
    if (!p || !p.mode) return
    await send(`PIN_SET ${pinName} ${p.mode} ${value}`)
    p.value = value
  }

  // Commands
  async function getDef()        { await send('GET_DEF') }
  async function getConfig()     { await send('GET_CONFIG') }
  async function requestUpdate() { await send('GET_STATUS') }
  async function saveConfig()    { await send('SAVE_CONFIG') }
  async function reset() {
    await send('RESET')
    for (const p of pins.values()) { p.mode = null; p.value = 0 }
  }

  return {
    boardName, boardId, pins, pinOrder, pinList,
    loadDef, loadConfig, applyUpdate, applyAck,
    getDef, getConfig, setPin, setValue, disablePin,
    requestUpdate, reset, saveConfig,
  }
})
