<template>
  <div class="h-screen bg-surface-900 text-twhite flex flex-col overflow-hidden">
    <Toast />

    <!-- Connection Bar -->
    <ConnectionBar />

    <!-- Tab Bar — centered -->
    <nav class="flex-shrink-0 bg-surface-800 border-b border-surface-700 flex items-center justify-center px-4 gap-1">
      <button
        v-for="tab in tabs"
        :key="tab.id"
        @click="activeTab = tab.id"
        class="flex items-center gap-1.5 px-4 py-2.5 text-sm font-medium transition-colors border-b-2 -mb-px"
        :class="activeTab === tab.id
          ? 'border-blue-500 text-twhite'
          : 'border-transparent text-tgray-400 hover:text-twhite hover:border-surface-500'"
      >
        <span>{{ tab.icon }}</span>
        <span>{{ tab.label }}</span>
      </button>
    </nav>

    <!-- Main content: takes all remaining height -->
    <main class="flex-1 min-h-0 flex flex-col overflow-hidden">

      <!-- Disconnected splash -->
      <div
        v-if="!isConnected"
        class="flex flex-col items-center justify-center flex-1 gap-6"
      >
        <div class="text-6xl">🔌</div>
        <h1 class="text-3xl font-bold text-twhite">Unified Pin Control</h1>
        <p class="text-tgray-400 text-center max-w-sm">
          Connect your board via USB to begin testing and validating GPIO pins.
        </p>
        <p v-if="lastError" class="text-red-400 text-sm">{{ lastError }}</p>
      </div>

      <!-- Connected views -->
      <template v-else>

        <!-- Pin Control -->
        <div v-show="activeTab === 'pins'" class="flex-1 min-h-0 overflow-y-auto p-4">
          <div class="flex flex-wrap items-center gap-3 mb-4">
            <button @click="onUpdate" class="btn-secondary" :disabled="polling">⟳ Update</button>
            <label class="flex items-center gap-2 text-sm text-tgray-300 cursor-pointer">
              <input type="checkbox" v-model="polling" class="accent-blue-500" />
              Auto-poll
            </label>
            <select v-model="pollInterval" class="bg-surface-700 text-sm text-tgray-300 rounded px-2 py-1 border border-surface-600">
              <option :value="250">250 ms</option>
              <option :value="500">500 ms</option>
              <option :value="1000">1 s</option>
              <option :value="2000">2 s</option>
            </select>
            <div class="flex-1" />
            <button @click="onSaveConfig" class="btn-secondary">💾 Save Config</button>
            <button @click="confirmReset = true" class="btn-danger">⚠ Reset All</button>
          </div>
          <PinGrid />
        </div>

        <!-- Serial Plotter -->
        <div v-show="activeTab === 'plotter'" class="flex-1 min-h-0 overflow-hidden">
          <PlotterTab />
        </div>

        <!-- Serial Terminal: h-full so TerminalTab fills completely -->
        <div v-show="activeTab === 'terminal'" class="flex-1 min-h-0">
          <TerminalTab class="h-full" />
        </div>

        <!-- Firmware Flasher -->
        <div v-show="activeTab === 'flasher'" class="flex-1 min-h-0 overflow-y-auto">
          <FlasherTab />
        </div>

      </template>
    </main>

    <!-- Reset modal -->
    <div v-if="confirmReset" class="fixed inset-0 bg-black/60 flex items-center justify-center z-50">
      <div class="bg-surface-800 rounded-xl p-6 w-80 shadow-2xl">
        <h2 class="text-lg font-bold mb-2">Reset all pins?</h2>
        <p class="text-tgray-400 text-sm mb-4">This will set all pins to their default safe state on the board.</p>
        <div class="flex gap-3 justify-end">
          <button @click="confirmReset = false" class="btn-secondary">Cancel</button>
          <button @click="doReset" class="btn-danger">Reset</button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, watch, onUnmounted } from 'vue'
import { useToast } from 'primevue/usetoast'
import Toast from 'primevue/toast'
import { useSerial } from '@/composables/useSerial'
import { usePinStore } from '@/stores/pinStore'
import ConnectionBar from '@/components/ConnectionBar.vue'
import PinGrid from '@/components/PinGrid.vue'
import PlotterTab from '@/components/PlotterTab.vue'
import TerminalTab from '@/components/TerminalTab.vue'
import FlasherTab from '@/components/FlasherTab.vue'

const toast = useToast()
const { isConnected, lastError, onMessage } = useSerial()
const pinStore = usePinStore()

const tabs = [
  { id: 'pins',     icon: '🔲', label: 'Pin Control' },
  { id: 'plotter',  icon: '📈', label: 'Serial Plotter' },
  { id: 'terminal', icon: '🖥️', label: 'Serial Terminal' },
  { id: 'flasher',  icon: '⚡', label: 'Firmware Flasher' },
]
const activeTab = ref('pins')

const polling = ref(false)
const pollInterval = ref(500)
let pollTimer = null
watch([polling, pollInterval], () => {
  clearInterval(pollTimer)
  if (polling.value) pollTimer = setInterval(() => pinStore.requestUpdate(), pollInterval.value)
})
onUnmounted(() => clearInterval(pollTimer))

const removeHandler = onMessage(async (msg) => {
  if (msg.error) { toast.add({ severity: 'error', summary: 'Board Error', detail: msg.error, life: 4000 }); return }
  if (msg.board)       { pinStore.loadDef(msg); await pinStore.getConfig() }
  else if (msg.config)  pinStore.loadConfig(msg)
  else if (msg.pins)    pinStore.applyUpdate(msg)
  else if (msg.pin)     pinStore.applyAck(msg)
  else if (msg.resp)    toast.add({ severity: 'success', summary: msg.resp, life: 2000 })
})
onUnmounted(() => removeHandler())

async function onUpdate() { await pinStore.requestUpdate() }
async function onSaveConfig() { await pinStore.saveConfig() }
const confirmReset = ref(false)
async function doReset() {
  await pinStore.reset()
  confirmReset.value = false
  toast.add({ severity: 'info', summary: 'Pins reset', life: 2000 })
}
</script>

<style>
.btn-secondary {
  @apply bg-surface-700 hover:bg-surface-600 text-twhite text-sm px-3 py-1.5 rounded-lg transition-colors border border-surface-600;
}
.btn-danger {
  @apply bg-red-700 hover:bg-red-600 text-twhite text-sm px-3 py-1.5 rounded-lg transition-colors;
}
</style>
