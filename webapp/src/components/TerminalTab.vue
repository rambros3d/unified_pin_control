<template>
  <!-- Outer wrapper fills the tab panel and sets up the sticky layout -->
  <div class="relative flex flex-col h-full overflow-hidden">

    <!-- Scrollable log — leaves room for the sticky bar at bottom -->
    <div
      ref="logEl"
      class="flex-1 overflow-y-auto font-mono text-xs px-4 py-3 bg-surface-900 flex flex-col gap-0.5"
    >
      <div v-if="!entries.length" class="text-tgray-400 italic">
        No messages yet. Connect a device and start receiving data.
      </div>
      <div
        v-for="(e, i) in entries"
        :key="i"
        class="flex gap-2 leading-relaxed"
      >
        <span class="text-tgray-500 flex-shrink-0 select-none">{{ e.ts }}</span>
        <span
          class="flex-shrink-0 select-none"
          :class="e.dir === 'rx' ? 'text-green-400' : 'text-blue-400'"
        >{{ e.dir === 'rx' ? '◀' : '▶' }}</span>
        <span
          class="break-all"
          :class="e.dir === 'rx' ? 'text-tgray-200' : 'text-blue-300'"
        >{{ e.text }}</span>
      </div>
    </div>

    <!-- Sticky input bar — always anchored to the bottom -->
    <div class="sticky bottom-0 flex items-center gap-2 px-3 py-2 bg-surface-800 border-t border-surface-700 z-10">
      <input
        ref="inputEl"
        v-model="command"
        @keydown.enter="handleSend"
        :disabled="!isConnected"
        placeholder="Type command and press Enter…"
        class="flex-1 bg-surface-700 text-tgray-100 text-sm font-mono rounded px-3 py-1.5 border border-surface-600 outline-none focus:border-blue-500 disabled:opacity-40 transition-colors"
      />
      <button
        @click="handleSend"
        :disabled="!isConnected || !command.trim()"
        class="btn-secondary disabled:opacity-40"
      >
        ▶ Send
      </button>
      <label class="flex items-center gap-1.5 text-xs text-tgray-400 cursor-pointer select-none">
        <input type="checkbox" v-model="autoScroll" class="accent-blue-500" />
        Autoscroll
      </label>
      <button
        @click="clear"
        class="text-tgray-500 hover:text-red-400 text-sm transition-colors px-2"
        title="Clear"
      >✕ Clear</button>
    </div>

  </div>
</template>

<script setup>
import { ref, watch, nextTick, onUnmounted } from 'vue'
import { useSerial } from '@/composables/useSerial'

const { isConnected, onRaw, send, sendRaw } = useSerial()

const entries  = ref([])
const command  = ref('')
const autoScroll = ref(true)
const logEl    = ref(null)
const inputEl  = ref(null)

const MAX_LINES = 500

const removeRaw = onRaw((line, dir) => {
  const now = new Date()
  const ts  = now.toTimeString().slice(0, 8)
  entries.value.push({ ts, dir, text: line })
  if (entries.value.length > MAX_LINES) entries.value.shift()
})

onUnmounted(() => removeRaw())

watch(entries, async () => {
  if (!autoScroll.value) return
  await nextTick()
  if (logEl.value) logEl.value.scrollTop = logEl.value.scrollHeight
}, { deep: true })

async function handleSend() {
  const cmd = command.value.trim()
  if (!cmd || !isConnected.value) return
  try {
    const parsed = JSON.parse(cmd)
    await send(parsed)
  } catch {
    await sendRaw(cmd)
  }
  command.value = ''
}

function clear() {
  entries.value = []
}
</script>
