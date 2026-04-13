import { ref, readonly } from 'vue'

// Singleton port & state shared across all callers
let port = null
let writer = null
let readLoopActive = false

const isConnected = ref(false)
const lastError = ref(null)
const _messageHandlers = []
const _rawHandlers = []

export function useSerial() {
  // Register a handler called for every parsed JSON message
  function onMessage(handler) {
    if (!_messageHandlers.includes(handler)) {
      _messageHandlers.push(handler)
    }
    return () => {
      const idx = _messageHandlers.indexOf(handler)
      if (idx !== -1) _messageHandlers.splice(idx, 1)
    }
  }

  // Register a handler called for every raw line (RX and TX)
  // handler(line: string, dir: 'rx' | 'tx')
  function onRaw(handler) {
    if (!_rawHandlers.includes(handler)) {
      _rawHandlers.push(handler)
    }
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
      if (port) { await port.close(); port = null }
    } catch (_) { /* ignore close errors */ }
    isConnected.value = false
  }

  // Send a JSON object as a newline-terminated JSON string
  async function send(cmdObject) {
    if (!writer) throw new Error('Not connected')
    const line = JSON.stringify(cmdObject) + '\n'
    _emitRaw(line.trim(), 'tx')
    await writer.write(line)
  }

  // Send a raw string line (no JSON wrapping)
  async function sendRaw(text) {
    if (!writer) throw new Error('Not connected')
    const line = text.endsWith('\n') ? text : text + '\n'
    _emitRaw(line.trim(), 'tx')
    await writer.write(line)
  }

  return {
    isConnected: readonly(isConnected),
    lastError: readonly(lastError),
    connect,
    disconnect,
    send,
    sendRaw,
    onMessage,
    onRaw
  }
}

// ─── Internal helpers ────────────────────────────────────────────

function _emitRaw(line, dir) {
  for (const h of _rawHandlers) h(line, dir)
}

let _lineBuffer = ''

async function _startReadLoop(reader) {
  try {
    while (readLoopActive) {
      const { value, done } = await reader.read()
      if (done) break
      _lineBuffer += value
      const lines = _lineBuffer.split(/\r?\n/)
      _lineBuffer = lines.pop() // keep incomplete last fragment
      for (const line of lines) {
        if (!line.trim()) continue
        _emitRaw(line.trim(), 'rx')
        try {
          const parsed = JSON.parse(line)
          for (const handler of _messageHandlers) handler(parsed, line)
        } catch {
          // Non-JSON noise — passed to raw handlers only
        }
      }
    }
  } catch (err) {
    lastError.value = err.message
    isConnected.value = false
  } finally {
    reader.releaseLock()
  }
}
