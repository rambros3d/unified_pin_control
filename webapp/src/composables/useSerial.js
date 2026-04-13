import { ref, readonly } from 'vue'

// Singleton port & state shared across all callers
let port = null
let writer = null
let readLoopActive = false

const isConnected = ref(false)
const lastError = ref(null)
const _messageHandlers = []

export function useSerial() {
  // ─── Register a handler called for every parsed JSON line received
  function onMessage(handler) {
    if (!_messageHandlers.includes(handler)) {
      _messageHandlers.push(handler)
    }
    // Return cleanup function
    return () => {
      const idx = _messageHandlers.indexOf(handler)
      if (idx !== -1) _messageHandlers.splice(idx, 1)
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

  async function send(cmdObject) {
    if (!writer) throw new Error('Not connected')
    const line = JSON.stringify(cmdObject) + '\n'
    // Emit for MonitorDrawer to capture outgoing commands
    window.dispatchEvent(new CustomEvent('upc:tx', { detail: line.trim() }))
    await writer.write(line)
  }

  return {
    isConnected: readonly(isConnected),
    lastError: readonly(lastError),
    connect,
    disconnect,
    send,
    onMessage
  }
}

// ─── Internal read loop ──────────────────────────────────────────
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
        try {
          const parsed = JSON.parse(line)
          for (const handler of _messageHandlers) handler(parsed, line)
        } catch {
          // Non-JSON noise — ignore silently
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
