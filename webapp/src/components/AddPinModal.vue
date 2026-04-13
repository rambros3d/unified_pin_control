<template>
  <Teleport to="body">
    <div
      class="fixed inset-0 bg-black/60 backdrop-blur-sm flex items-center justify-center z-50 p-4"
      @click.self="$emit('close')"
    >
      <div class="bg-surface-800 border border-surface-600 rounded-2xl shadow-2xl w-full max-w-md flex flex-col overflow-hidden">

        <!-- Header -->
        <div class="flex items-center justify-between px-5 py-4 border-b border-surface-700">
          <h2 class="text-base font-bold text-twhite">Configure Pin</h2>
          <button @click="$emit('close')" class="text-tgray-500 hover:text-twhite transition-colors text-xl leading-none" aria-label="Close">&times;</button>
        </div>

        <div class="flex flex-col gap-4 px-5 py-5">

          <!-- Pin selector -->
          <div class="flex flex-col gap-1.5">
            <label class="text-xs font-medium text-tgray-400 uppercase tracking-wide">Pin</label>
            <select
              v-model="selectedPin" @change="onPinChange"
              class="bg-surface-700 text-tgray-100 rounded-lg px-3 py-2 border border-surface-600 text-sm outline-none focus:border-blue-500 transition-colors"
            >
              <option value="">Select a pin…</option>
              <optgroup v-if="inactivePins.length" label="Available Pins">
                <option v-for="pin in inactivePins" :key="pin.name" :value="pin.name">
                  {{ pin.name }} &mdash; {{ capsLabel(pin) }}
                </option>
              </optgroup>
              <optgroup v-if="activePins.length" label="Reconfigure Active Pins">
                <option v-for="pin in activePins" :key="pin.name" :value="pin.name">
                  {{ pin.name }} &mdash; {{ pin.mode }}
                </option>
              </optgroup>
            </select>
          </div>

          <!-- Mode selector -->
          <div v-if="selectedPin" class="flex flex-col gap-1.5">
            <label class="text-xs font-medium text-tgray-400 uppercase tracking-wide">Mode</label>
            <div class="grid grid-cols-3 gap-2">
              <button
                v-for="mode in availableModes" :key="mode"
                @click="selectedMode = mode"
                class="flex flex-col items-center gap-1 px-3 py-2.5 rounded-lg border text-xs font-medium transition-all"
                :class="selectedMode === mode
                  ? 'bg-blue-600 border-blue-500 text-white'
                  : 'bg-surface-700 border-surface-600 text-tgray-300 hover:border-surface-500 hover:text-twhite'"
              >
                <span class="text-base">{{ MODE_META[mode]?.icon ?? '🔧' }}</span>
                <span>{{ mode }}</span>
              </button>
            </div>
            <p v-if="!availableModes.length" class="text-xs text-tgray-500 italic">No supported modes found.</p>
          </div>

          <!-- Initial value for output modes -->
          <div v-if="selectedMode && isOutputMode" class="flex flex-col gap-1.5">
            <label class="text-xs font-medium text-tgray-400 uppercase tracking-wide">Initial Value</label>
            <div v-if="isDigitalOutMode" class="flex gap-2">
              <button @click="initValue = 0"
                class="flex-1 py-1.5 rounded-lg border text-sm transition-all"
                :class="initValue === 0 ? 'bg-surface-600 border-blue-500 text-twhite' : 'bg-surface-700 border-surface-600 text-tgray-400'"
              >LOW (0)</button>
              <button @click="initValue = 1"
                class="flex-1 py-1.5 rounded-lg border text-sm transition-all"
                :class="initValue === 1 ? 'bg-green-700 border-green-500 text-white' : 'bg-surface-700 border-surface-600 text-tgray-400'"
              >HIGH (1)</button>
            </div>
            <div v-else-if="selectedMode === 'SERVO'" class="flex items-center gap-3">
              <input type="range" min="0" max="180" v-model.number="initValue" class="flex-1 accent-purple-500" />
              <span class="text-sm text-tgray-200 w-12">{{ initValue }}&deg;</span>
            </div>
            <div v-else class="flex items-center gap-3">
              <input type="range" min="0" :max="outputMax" v-model.number="initValue" class="flex-1 accent-blue-500" />
              <input type="number" min="0" :max="outputMax" v-model.number="initValue"
                class="w-20 bg-surface-700 text-tgray-100 rounded px-2 py-1 border border-surface-600 text-sm" />
              <span class="text-xs text-tgray-500">/ {{ outputMax }}</span>
            </div>
          </div>

          <!-- Mode description -->
          <div v-if="selectedMode" class="flex items-center gap-2 text-xs text-tgray-500 bg-surface-900 rounded-lg px-3 py-2">
            <span>{{ MODE_META[selectedMode]?.icon }}</span>
            <span>{{ MODE_META[selectedMode]?.desc }}</span>
          </div>
        </div>

        <div class="flex items-center justify-end gap-3 px-5 py-4 border-t border-surface-700">
          <button @click="$emit('close')" class="btn-secondary">Cancel</button>
          <button @click="apply" :disabled="!selectedPin || !selectedMode"
            class="px-4 py-1.5 rounded-lg text-sm font-semibold transition-colors
                   bg-blue-600 hover:bg-blue-500 text-white disabled:opacity-40 disabled:cursor-not-allowed"
          >Apply</button>
        </div>
      </div>
    </div>
  </Teleport>
</template>

<script setup>
import { ref, computed } from 'vue'
import { usePinStore } from '@/stores/pinStore'

const emit = defineEmits(['close', 'applied'])
const pinStore = usePinStore()

// Firmware mode strings exactly as the API uses them
const MODE_META = {
  OUT:    { icon: '💡', desc: 'Digital output — set HIGH or LOW' },
  OUT_OD: { icon: '💡', desc: 'Digital output open-drain' },
  IN:     { icon: '🔍', desc: 'Digital input' },
  IN_UP:  { icon: '🔍', desc: 'Digital input with pull-up resistor' },
  IN_DN:  { icon: '🔍', desc: 'Digital input with pull-down resistor' },
  ADC:    { icon: '📊', desc: 'Analog input — read raw ADC value' },
  PWM:    { icon: '🌀', desc: 'PWM output — control duty cycle' },
  DAC:    { icon: '📉', desc: 'DAC output — true analog voltage' },
  SERVO:  { icon: '⚙️', desc: 'Servo motor — set angle 0–180°' },
  TOUCH:  { icon: '✋', desc: 'Capacitive touch — read touch value' },
}

const selectedPin  = ref('')
const selectedMode = ref('')
const initValue    = ref(0)

const inactivePins = computed(() => pinStore.pinList.filter(p => !p.mode))
const activePins   = computed(() => pinStore.pinList.filter(p =>  p.mode))

function capsLabel(pin) {
  const all = [...(pin.outCaps ?? []), ...(pin.inCaps ?? [])]
  return all.length ? [...new Set(all)].join(', ') : ''
}

const pinObj = computed(() => pinStore.pins.get(selectedPin.value))

const availableModes = computed(() => {
  if (!pinObj.value) return []
  return [...new Set([...(pinObj.value.outCaps ?? []), ...(pinObj.value.inCaps ?? [])])]
})

const isOutputMode    = computed(() => ['OUT','OUT_OD','PWM','DAC','SERVO'].includes(selectedMode.value))
const isDigitalOutMode = computed(() => ['OUT','OUT_OD'].includes(selectedMode.value))

const outputMax = computed(() => {
  if (selectedMode.value === 'SERVO') return 180
  return (1 << (pinObj.value?.res ?? 8)) - 1
})

function onPinChange() { selectedMode.value = ''; initValue.value = 0 }

async function apply() {
  if (!selectedPin.value || !selectedMode.value) return
  await pinStore.setPin({ pin: selectedPin.value, mode: selectedMode.value, value: initValue.value })
  emit('applied', { pin: selectedPin.value, mode: selectedMode.value })
  emit('close')
}
</script>
