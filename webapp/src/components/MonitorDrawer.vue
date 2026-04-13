<template>
  <!-- Collapsed tab trigger -->
  <div class="fixed bottom-0 left-0 right-0 z-40">
    <button
      @click="open = !open"
      class="w-full bg-surface-800 border-t border-surface-600 text-xs text-gray-400 hover:text-white py-1.5 flex items-center justify-center gap-2 transition-colors"
    >
      <span>{{ open ? '▼' : '▲' }}</span>
      <span>Serial Monitor</span>
      <span v-if="entries.length" class="bg-surface-600 text-gray-300 rounded px-1.5">{{ entries.length }}</span>
      <button
        v-if="entries.length"
        @click.stop="entries = []"
        class="ml-auto mr-3 text-gray-500 hover:text-red-400 transition-colors"
      >clear</button>
    </button>

    <!-- Drawer panel -->
    <transition name="slide-up">
      <div
        v-if="open"
        class="bg-surface-900 border-t border-surface-600 h-52 overflow-y-auto font-mono text-xs px-3 py-2 flex flex-col gap-0.5"
        ref="logEl"
      >
        <div v-if="!entries.length" class="text-gray-600 italic">No messages yet.</div>
        <div
          v-for="(entry, i) in entries"
          :key="i"
          class="flex gap-2 leading-5"
          :class="entryColor(entry.type)"
        >
          <span class="text-gray-600 shrink-0 select-none">{{ entry.time }}</span>
          <span class="shrink-0 select-none font-bold">{{ entry.type === 'tx' ? '→' : entry.type === 'rx' ? '←' : '!' }}</span>
          <span class="break-all">{{ entry.text }}</span>
        </div>
      </div>
    </transition>
  </div>
</template>

<script setup>
import { ref, nextTick, onUnmounted } from 'vue'
import { useSerial } from '@/composables/useSerial'

const { onMessage } = useSerial()

const open = ref(false)
const entries = ref([])
const logEl = ref(null)

const MAX_ENTRIES = 200

function timestamp() {
  return new Date().toLocaleTimeString('en-GB', { hour12: false })
}

function addEntry(type, text) {
  entries.value.push({ type, text, time: timestamp() })
  if (entries.value.length > MAX_ENTRIES) entries.value.shift()
  if (open.value) {
    nextTick(() => {
      if (logEl.value) logEl.value.scrollTop = logEl.value.scrollHeight
    })
  }
}

function entryColor(type) {
  if (type === 'tx') return 'text-blue-400'
  if (type === 'rx') return 'text-green-400'
  return 'text-red-400'
}

// Listen to all incoming RX messages
const removeRx = onMessage((msg, rawLine) => {
  if (msg.error) {
    addEntry('err', rawLine)
  } else {
    addEntry('rx', rawLine)
  }
})

// Listen to outgoing TX events dispatched by useSerial send()
function onTx(e) {
  addEntry('tx', e.detail)
}
window.addEventListener('upc:tx', onTx)

onUnmounted(() => {
  removeRx()
  window.removeEventListener('upc:tx', onTx)
})
</script>

<style scoped>
.slide-up-enter-active, .slide-up-leave-active {
  transition: max-height 0.2s ease, opacity 0.2s ease;
  overflow: hidden;
}
.slide-up-enter-from, .slide-up-leave-to {
  max-height: 0;
  opacity: 0;
}
.slide-up-enter-to, .slide-up-leave-from {
  max-height: 208px;
  opacity: 1;
}
</style>
