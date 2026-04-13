import { ref, readonly } from 'vue'

// Singleton port & state shared across all callers
let port = null
let writer = null
let readLoopActive = false

const isConnected = ref(false)
const lastError = ref(null)

// Listeners for parsed protocol messages
const _messageHandlers = []
// Listeners for every raw line (rx/tx)
const _rawHandlers = []

export function useSerial() {
  // Register a handler called for every parsed protocol message
  // handler({ type, payload, raw })
  //   type: 'BINFO' | 'CONFIG' | 'STAT' | 'ACK' | 'RESET' | 'SAVE' | 'ERR'
  //   payload: parsed object specific to the type
  //   raw: the original line string
  function onMessage(handler) {
    if (!_messageHandlers.includes(handler)) _messageHandlers.push(handler)
    return () => {
      const idx = _messageHandlers.indexOf(handler)
      if (idx !== -1) _messageHandlers.splice(idx, 1)
    }
  }

  // Register a handler called for every raw line
  // handler(line: string, dir: 'rx' | 'tx')
  function onRaw(handler) {
    if (!_rawHandlers.includes(handler)) _rawHandlers.push(handler)
    return () => {
      const idx = _rawHandlers.indexOf(handler)
      if (idx !== -1) _rawHandlers.splice(idx, 1)
    }
  }

  async function connect(baudRate = 115200) {
    try {
      port = await navigator.serial.requestPort()
      await port.open({ baudRate })

      const textDecoder = new TextDecoderStream()
      port.readable.pipeTo(textDecoder.writable)
      const reader = textDecoder.readable.getReader()

      const textEncoder = new TextEncoderStream()
      textEncoder.readable.pipeTo(port.writable)
      writer = textEncoder.writable.getWriter()

      isConnected.value = true
      lastError.value = null
      readLoopActive = true

      _startReadLoop(reader)
    } catch (err) {
      lastError.value = err.message
      isConnected.value = false
    }
  }

  async function disconnect() {
    readLoopActive = false
    try {
      if (writer) { await writer.close(); writer = null }
      if (port)   { await port.close();   port = null }
    } catch (_) { /* ignore */ }
    isConnected.value = false
  }

  // Send a plain-text command line (no JSON wrapping)
  async function send(text) {
    if (!writer) throw new Error('Not connected')
    const line = text.endsWith('\n') ? text : text + '\n'
    _emitRaw(line.trim(), 'tx')
    await writer.write(line)
  }

  return {
    isConnected: readonly(isConnected),
    lastError:   readonly(lastError),
    connect,
    disconnect,
    send,
    onMessage,
    onRaw
  }
}

// ─── Internal helpers ─────────────────────────────────────────────

function _emitRaw(line, dir) {
  for (const h of _rawHandlers) h(line, dir)
}

function _emitMessage(type, payload, raw) {
  for (const h of _messageHandlers) h({ type, payload, raw })
}

// ─── Protocol parser ──────────────────────────────────────────────
// BINFO:<id>|<name>|<pin>:<caps>,...
// CONFIG:<pin>:<mode>,...
// STAT:<pin>:<value>,...
// ACK:<pin>:<mode>:<value>
// RESET:OK
// SAVE:OK
// ERR:<message>

function _parseLine(line) {
  const raw = line.trim()
  if (!raw) return

  if (raw.startsWith('BINFO:')) {
    const body = raw.slice(6)
    const parts = body.split('|')
    const id   = parts[0] ?? ''
    const name = parts[1] ?? ''
    const pinsRaw = parts[2] ?? ''
    const pins = pinsRaw ? pinsRaw.split(',').map(entry => {
      const colonIdx = entry.indexOf(':')
      const pinName = entry.slice(0, colonIdx)
      const caps    = entry.slice(colonIdx + 1)
      return { pin: pinName, caps }
    }) : []
    _emitMessage('BINFO', { id, name, pins }, raw)
    return
  }

  if (raw.startsWith('CONFIG:')) {
    const body = raw.slice(7)
    const config = body ? body.split(',').map(entry => {
      const [pin, mode] = entry.split(':')
      return { pin, mode }
    }) : []
    _emitMessage('CONFIG', { config }, raw)
    return
  }

  if (raw.startsWith('STAT:')) {
    const body = raw.slice(5)
    const updates = body ? body.split(',').map(entry => {
      const [pin, value] = entry.split(':')
      return { pin, value: Number(value) }
    }) : []
    _emitMessage('STAT', { updates }, raw)
    return
  }

  if (raw.startsWith('ACK:')) {
    const parts = raw.slice(4).split(':')
    _emitMessage('ACK', { pin: parts[0], mode: parts[1], value: Number(parts[2] ?? 0) }, raw)
    return
  }

  if (raw === 'RESET:OK') {
    _emitMessage('RESET', {}, raw)
    return
  }

  if (raw === 'SAVE:OK') {
    _emitMessage('SAVE', {}, raw)
    return
  }

  if (raw.startsWith('ERR:')) {
    _emitMessage('ERR', { message: raw.slice(4) }, raw)
    return
  }

  // Unknown / noise — emit as raw only (already done in read loop)
}

let _lineBuffer = ''

async function _startReadLoop(reader) {
  try {
    while (readLoopActive) {
      const { value, done } = await reader.read()
      if (done) break
      _lineBuffer += value
      const lines = _lineBuffer.split(/\r?\n/)
      _lineBuffer = lines.pop() // keep incomplete fragment
      for (const line of lines) {
        if (!line.trim()) continue
        _emitRaw(line.trim(), 'rx')
        _parseLine(line)
      }
    }
  } catch (err) {
    lastError.value = err.message
    isConnected.value = false
  } finally {
    reader.releaseLock()
  }
}
