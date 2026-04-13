<template>
  <div class="flex flex-col items-center justify-start h-full overflow-y-auto p-6">
    <div class="w-full max-w-lg flex flex-col gap-6">

      <!-- Header -->
      <div>
        <h2 class="text-lg font-bold text-twhite">Firmware Flasher</h2>
        <p class="text-sm text-tgray-400 mt-0.5">
          Select a <span class="font-mono">.bin</span> or <span class="font-mono">.hex</span>
          file and your target device to flash new firmware.
        </p>
      </div>

      <!-- Form card -->
      <div class="bg-surface-800 rounded-xl border border-surface-600 p-5 flex flex-col gap-5">

        <!-- File picker -->
        <div class="flex flex-col gap-1.5">
          <label class="text-sm font-medium text-tgray-200">
            📁 Firmware File
          </label>
          <div
            class="relative border-2 border-dashed rounded-lg p-5 text-center transition-colors cursor-pointer"
            :class="file
              ? 'border-blue-500/60 bg-blue-500/5'
              : 'border-surface-600 hover:border-surface-500'"
            @click="fileInputEl?.click()"
            @dragover.prevent
            @drop.prevent="onDrop"
          >
            <input
              ref="fileInputEl"
              type="file"
              accept=".bin,.hex"
              class="hidden"
              :disabled="isFlashing"
              @change="onFileChange"
            />
            <div v-if="!file" class="flex flex-col items-center gap-2 text-tgray-500">
              <span class="text-3xl">&#128190;</span>
              <span class="text-sm">Click to browse or drag &amp; drop</span>
              <span class="text-xs">.bin or .hex only</span>
            </div>
            <div v-else class="flex items-center justify-center gap-3">
              <span class="text-2xl">&#9989;</span>
              <div class="text-left">
                <p class="text-sm font-medium text-twhite">{{ file.name }}</p>
                <p class="text-xs text-tgray-400">{{ formatBytes(file.size) }}</p>
              </div>
              <button
                class="ml-auto text-tgray-500 hover:text-red-400 transition-colors text-lg leading-none"
                :disabled="isFlashing"
                @click.stop="file = null"
              >&times;</button>
            </div>
          </div>
        </div>

        <!-- Target device -->
        <div class="flex flex-col gap-1.5">
          <label class="text-sm font-medium text-tgray-200">
            🖥️ Target Device
          </label>
          <select
            v-model="target"
            :disabled="isFlashing"
            class="bg-surface-700 text-tgray-100 rounded-lg px-3 py-2 border border-surface-600 text-sm outline-none focus:border-blue-500 transition-colors disabled:opacity-40"
          >
            <option value="">Select a target…</option>
            <optgroup label="Espressif">
              <option value="esp32">ESP32</option>
              <option value="esp32s2">ESP32-S2</option>
              <option value="esp32s3">ESP32-S3</option>
              <option value="esp32c3">ESP32-C3</option>
              <option value="esp8266">ESP8266</option>
            </optgroup>
            <optgroup label="Raspberry Pi">
              <option value="rp2040">RP2040 (Pico)</option>
              <option value="rp2350">RP2350 (Pico 2)</option>
            </optgroup>
            <optgroup label="AVR / Arduino">
              <option value="avr_uno">Arduino Uno (ATmega328P)</option>
              <option value="avr_mega">Arduino Mega (ATmega2560)</option>
              <option value="avr_nano">Arduino Nano</option>
            </optgroup>
            <optgroup label="STM32">
              <option value="stm32">STM32 (generic)</option>
            </optgroup>
          </select>
        </div>

        <!-- Flash button -->
        <button
          @click="handleFlash"
          :disabled="isFlashing || !file || !target"
          class="w-full py-2.5 rounded-lg font-semibold text-sm transition-colors
                 bg-blue-600 hover:bg-blue-500 text-white
                 disabled:opacity-40 disabled:cursor-not-allowed"
        >
          <span v-if="!isFlashing">⚡ Flash Device</span>
          <span v-else class="flex items-center justify-center gap-2">
            <svg class="animate-spin w-4 h-4" viewBox="0 0 24 24" fill="none">
              <circle class="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" stroke-width="4"/>
              <path class="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8v4a4 4 0 00-4 4H4z"/>
            </svg>
            Flashing…
          </span>
        </button>

        <!-- Progress -->
        <div v-if="isFlashing || progressLog.length" class="flex flex-col gap-2">
          <div v-if="isFlashing" class="flex flex-col gap-1">
            <div class="flex justify-between text-xs text-tgray-400">
              <span>{{ statusMsg }}</span>
              <span>{{ progress }}%</span>
            </div>
            <div class="w-full bg-surface-700 rounded-full h-2">
              <div
                class="bg-blue-500 h-2 rounded-full transition-all duration-300"
                :style="{ width: progress + '%' }"
              />
            </div>
          </div>

          <!-- Log output -->
          <div
            ref="logEl"
            class="bg-surface-900 rounded-lg border border-surface-700 p-3 font-mono text-xs text-tgray-300 max-h-48 overflow-y-auto flex flex-col gap-0.5"
          >
            <div
              v-for="(entry, i) in progressLog"
              :key="i"
              :class="entry.type === 'success' ? 'text-green-400'
                    : entry.type === 'error'   ? 'text-red-400'
                    : 'text-tgray-400'"
            >
              <span class="text-tgray-600 mr-2">{{ entry.ts }}</span>
              {{ entry.msg }}
            </div>
          </div>
        </div>
      </div>

      <!-- Notice -->
      <p class="text-xs text-tgray-600 text-center">
        ⚠️ Actual flashing requires esptool / avrdude integration. This UI demonstrates the flash workflow.
      </p>
    </div>
  </div>
</template>

<script setup>
import { ref, nextTick } from 'vue'

const file = ref(null)
const target = ref('')
const isFlashing = ref(false)
const progress = ref(0)
const statusMsg = ref('')
const progressLog = ref([])
const fileInputEl = ref(null)
const logEl = ref(null)

function onFileChange(e) {
  if (e.target.files?.[0]) file.value = e.target.files[0]
}

function onDrop(e) {
  const dropped = e.dataTransfer.files?.[0]
  if (dropped && (dropped.name.endsWith('.bin') || dropped.name.endsWith('.hex'))) {
    file.value = dropped
  }
}

function formatBytes(bytes) {
  if (bytes < 1024) return bytes + ' B'
  if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB'
  return (bytes / 1024 / 1024).toFixed(2) + ' MB'
}

function logEntry(msg, type = 'info') {
  const now = new Date()
  const ts = now.toTimeString().slice(0, 8)
  progressLog.value.push({ ts, msg, type })
  nextTick(() => {
    if (logEl.value) logEl.value.scrollTop = logEl.value.scrollHeight
  })
}

async function delay(ms) {
  return new Promise(r => setTimeout(r, ms))
}

async function handleFlash() {
  if (!file.value || !target.value || isFlashing.value) return
  isFlashing.value = true
  progress.value = 0
  progressLog.value = []

  const steps = [
    { msg: `Connecting to ${target.value}...`,         progress: 10, delay: 900 },
    { msg: 'Detecting chip and verifying connection...',progress: 20, delay: 700 },
    { msg: 'Erasing flash memory...',                  progress: 35, delay: 1200 },
    { msg: `Writing ${file.value.name} (${formatBytes(file.value.size)})...`, progress: 60, delay: 1500 },
    { msg: 'Verifying written data...',                progress: 85, delay: 800 },
    { msg: 'Resetting device...',                      progress: 95, delay: 500 },
    { msg: 'Flash complete!',                          progress: 100, delay: 300, type: 'success' },
  ]

  for (const step of steps) {
    statusMsg.value = step.msg
    progress.value = step.progress
    logEntry(step.msg, step.type ?? 'info')
    await delay(step.delay)
  }

  isFlashing.value = false
}
</script>
