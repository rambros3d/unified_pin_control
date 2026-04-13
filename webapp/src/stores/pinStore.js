import { defineStore } from 'pinia'
import { ref, reactive, computed } from 'vue'
import { useSerial } from '@/composables/useSerial'

// ─── Firmware caps string decoder
// BINFO caps chars: I=IN, U=IN_UP, D=IN_DN, A=ADC, T=TOUCH, O=OUT, Z=OUT_OD, P=PWM, C=DAC
function parseCaps(capsStr) {
  const inCaps  = []
  const outCaps = []
  let cap = 'DIO' // legacy single cap field default

  for (const ch of capsStr) {
    switch (ch) {
      case 'I': inCaps.push('DIN');   break
      case 'U': inCaps.push('DIN');   break  // INPUT_PULLUP — still a digital input
      case 'D': inCaps.push('DIN');   break  // INPUT_PULLDOWN
      case 'A': inCaps.push('AIN');   break
      case 'T': inCaps.push('TOUCH'); break
      case 'O': outCaps.push('DOUT'); break
      case 'Z': outCaps.push('DOUT'); break  // open-drain — same DOUT mode
      case 'P': outCaps.push('PWM');  break
      case 'C': outCaps.push('DAC');  break
    }
  }

  // Deduplicate
  const uniqIn  = [...new Set(inCaps)]
  const uniqOut = [...new Set(outCaps)]

  // Derive legacy single cap label
  if (uniqOut.includes('PWM'))       cap = 'PWM'
  else if (uniqOut.includes('DAC'))  cap = 'DAC'
  else if (uniqIn.includes('AIN'))   cap = 'AI'
  else if (uniqOut.includes('DOUT') && uniqIn.includes('DIN')) cap = 'DIO'
  else if (uniqOut.includes('DOUT')) cap = 'DO'
  else if (uniqIn.includes('DIN'))   cap = 'DI'

  return { cap, inCaps: uniqIn, outCaps: uniqOut }
}

export const usePinStore = defineStore('pins', () => {
  const { send } = useSerial()

  const boardName = ref('')
  const boardId   = ref('')

  const pins     = reactive(new Map())
  const pinOrder = ref([])

  const pinList = computed(() =>
    pinOrder.value.map(name => pins.get(name)).filter(Boolean)
  )

  // ─── Load board definition from BINFO payload
  // payload: { id, name, pins: [ { pin: 'GPIO0', caps: 'IUDATOZP' }, ... ] }
  function loadDef(payload) {
    boardName.value = payload.name ?? ''
    boardId.value   = payload.id   ?? ''
    pins.clear()
    pinOrder.value  = []

    for (const entry of (payload.pins ?? [])) {
      const { cap, inCaps, outCaps } = parseCaps(entry.caps ?? '')
      pins.set(entry.pin, {
        name:    entry.pin,
        cap,
        inCaps,
        outCaps,
        mode:    null,
        value:   0,
        res:     8,
        freq:    1000,
      })
      pinOrder.value.push(entry.pin)
    }
  }

  // ─── Apply CONFIG payload
  // payload: { config: [ { pin, mode }, ... ] }
  function loadConfig(payload) {
    for (const entry of (payload.config ?? [])) {
      const pin = pins.get(entry.pin)
      if (pin) pin.mode = entry.mode
    }
  }

  // ─── Apply STAT payload
  // payload: { updates: [ { pin, value }, ... ] }
  function applyUpdate(payload) {
    for (const entry of (payload.updates ?? [])) {
      const pin = pins.get(entry.pin)
      if (pin) pin.value = entry.value
    }
  }

  // ─── Apply ACK payload
  // payload: { pin, mode, value }
  function applyAck(payload) {
    const pin = pins.get(payload.pin)
    if (!pin) return
    pin.mode  = payload.mode
    pin.value = payload.value ?? 0
  }

  // ─── Commands (plain-text protocol)
  async function getDef()       { await send('GET_DEF') }
  async function getConfig()    { await send('GET_CONFIG') }
  async function requestUpdate(){ await send('GET_STATUS') }
  async function reset()        {
    await send('RESET')
    for (const p of pins.values()) { p.mode = null; p.value = 0 }
  }
  async function saveConfig()   { await send('SAVE_CONFIG') }

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

  // Convenience: update value only on an already-configured pin
  async function setValue(pinName, value) {
    const p = pins.get(pinName)
    if (!p || !p.mode) return
    await send(`PIN_SET ${pinName} ${p.mode} ${value}`)
    p.value = value
  }

  // Convenience: change mode, reset value
  async function setMode(pinName, mode) {
    await setPin({ pin: pinName, mode, value: 0 })
  }

  return {
    boardName, boardId,
    pins, pinOrder, pinList,
    loadDef, loadConfig, applyUpdate, applyAck,
    getDef, getConfig,
    setPin, setValue, setMode,
    requestUpdate, reset, saveConfig,
  }
})
