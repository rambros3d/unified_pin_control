<template>
  <header class="bg-surface-800 border-b border-surface-700 px-4 py-2 flex items-center gap-3">
    <!-- Status dot -->
    <span
      class="w-2.5 h-2.5 rounded-full flex-shrink-0"
      :class="isConnected ? 'bg-green-400 animate-pulse' : 'bg-red-500'"
    />

    <!-- Board name -->
    <div v-if="isConnected && pinStore.boardName" class="flex items-center gap-2">
      <span class="font-semibold text-sm">{{ pinStore.boardName }}</span>
      <span class="text-xs bg-surface-700 text-tgray-400 px-2 py-0.5 rounded font-mono">{{ pinStore.boardId }}</span>
    </div>
    <span v-else class="text-tgray-400 text-sm">
      {{ isConnected ? 'Fetching board info...' : 'No device connected' }}
    </span>

    <div class="flex-1" />

    <!-- Baud rate (visible when disconnected) -->
    <div v-if="!isConnected" class="flex items-center gap-2 text-sm text-tgray-400">
      <label>Baud:</label>
      <select
        v-model="baudRate"
        class="bg-surface-700 text-tgray-200 rounded px-2 py-1 border border-surface-600 text-xs"
      >
        <option :value="9600">9600</option>
        <option :value="57600">57600</option>
        <option :value="115200">115200</option>
        <option :value="230400">230400</option>
      </select>
    </div>

    <!-- Theme Toggle -->
    <button
      @click="toggleTheme"
      class="p-1.5 rounded-lg bg-surface-700 hover:bg-surface-600 border border-surface-600 transition-colors text-twhite"
      title="Toggle Dark Mode"
    >
      {{ isDark ? '\uD83C\uDF19' : '\u2600\uFE0F' }}
    </button>

    <!-- Connect / Disconnect -->
    <button
      @click="isConnected ? handleDisconnect() : handleConnect()"
      class="text-sm px-4 py-1.5 rounded-lg font-medium transition-colors"
      :class="isConnected
        ? 'bg-red-700 hover:bg-red-600 text-twhite'
        : 'bg-blue-600 hover:bg-blue-500 text-twhite'"
    >
      {{ isConnected ? 'Disconnect' : 'Connect Device' }}
    </button>
  </header>
</template>

<script setup>
import { ref, watchEffect } from 'vue'
import { useSerial } from '@/composables/useSerial'
import { usePinStore } from '@/stores/pinStore'

const { isConnected, connect, disconnect } = useSerial()
const pinStore = usePinStore()

const baudRate = ref(115200)

// Theme toggle
const isDark = ref(localStorage.getItem('theme') === 'dark' || (!localStorage.getItem('theme') && window.matchMedia('(prefers-color-scheme: dark)').matches))
watchEffect(() => {
  if (isDark.value) {
    document.documentElement.classList.add('dark')
    localStorage.setItem('theme', 'dark')
  } else {
    document.documentElement.classList.remove('dark')
    localStorage.setItem('theme', 'light')
  }
})

function toggleTheme() {
  isDark.value = !isDark.value
}

async function handleConnect() {
  await connect(baudRate.value)
  if (isConnected.value) {
    await pinStore.getDef()
  }
}

async function handleDisconnect() {
  await disconnect()
}
</script>
