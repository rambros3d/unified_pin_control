<template>
  <!-- Collapsed tab trigger -->
  <div class="fixed bottom-0 left-0 right-0 z-40">
    <button
      @click="open = !open"
      class="w-full bg-surface-800 border-t border-surface-600 text-xs text-tgray-400 hover:text-twhite py-1.5 flex items-center justify-center gap-2 transition-colors"
    >
      <span>{{ open ? '▼' : '▲' }} Serial Monitor</span>
      <span v-if="entries.length" class="bg-surface-600 text-tgray-300 rounded px-1.5">{{ entries.length }}</span>
      <button
        v-if="open && entries.length"
        @click.stop="clear"
        class="ml-auto mr-3 text-gray-500 hover:text-red-400 transition-colors"
      >✕ Clear</button>
    </button>

    <!-- Log panel -->
    <div
      v-show="open"
      class="bg-surface-900 border-t border-surface-600 h-52 overflow-y-auto font-mono text-xs px-3 py-2 flex flex-col gap-0.5"
    >
      <div v-if="!entries.length" class="text-tgray-400 italic">No messages yet.</div>
      <div
        v-for="(e, i) in entries"
        :key="i"
        :class="e.dir === 'rx' ? 'text-green-400' : 'text-blue-400'"
      >
        <span class="text-tgray-400 mr-2">{{ e.ts }}</span>
        <span class="mr-1">{{ e.dir === 'rx' ? '◀' : '▶' }}</span>
        <span>{{ e.text }}</span>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import { useSerial } from '@/composables/useSerial'

const open = ref(false)
const entries = ref([])
const { onRaw } = useSerial()

const removeRaw = onRaw((line, dir) => {
  const now = new Date()
  const ts = now.toTimeString().slice(0, 8)
  entries.value.push({ ts, dir, text: line })
  if (entries.value.length > 200) entries.value.shift()
})

import { onUnmounted } from 'vue'
onUnmounted(() => removeRaw())

function clear() { entries.value = [] }
</script>
