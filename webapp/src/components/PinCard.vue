<template>
  <div
    class="bg-surface-800 border rounded-xl p-3 flex flex-col gap-2 transition-all"
    :class="pin.mode ? 'border-surface-600' : 'border-surface-700 opacity-60'"
  >
    <!-- Header -->
    <div class="flex items-center justify-between">
      <span class="font-mono font-bold text-sm">{{ pin.name }}</span>
      <div class="flex flex-wrap gap-1 justify-end">
        <span
          v-for="cap in allCaps"
          :key="cap"
          class="text-[10px] px-1.5 py-0.5 rounded font-mono"
          :class="capColor(cap)"
        >{{ cap }}</span>
      </div>
    </div>

    <!-- Mode selector -->
    <select
      v-model="selectedMode"
      @change="onModeChange"
      class="bg-surface-700 text-sm text-gray-200 rounded px-2 py-1 border border-surface-600 w-full"
    >
      <option value="">— unset —</option>
      <optgroup v-if="pin.inCaps.length" label="Input">
        <option v-for="m in pin.inCaps" :key="m" :value="m">{{ modeLabel(m) }}</option>
      </optgroup>
      <optgroup v-if="pin.outCaps.length" label="Output">
        <option v-for="m in pin.outCaps" :key="m" :value="m">{{ modeLabel(m) }}</option>
      </optgroup>
    </select>

    <!-- Value control -->
    <template v-if="selectedMode">
      <!-- Digital input: read-only badge -->
      <div v-if="isDigitalInput" class="flex items-center gap-2">
        <span
          class="px-3 py-1 rounded text-sm font-mono font-bold"
          :class="pin.value ? 'bg-green-700 text-green-100' : 'bg-surface-700 text-gray-400'"
        >{{ pin.value ? 'HIGH' : 'LOW' }}</span>
      </div>

      <!-- ADC / TOUCH: numeric + bar -->
      <div v-else-if="isAnalogInput" class="flex flex-col gap-1">
        <div class="flex justify-between text-xs text-gray-400">
          <span>Value</span>
          <span class="font-mono">{{ pin.value }} / {{ adcMax }}</span>
        </div>
        <div class="w-full bg-surface-700 rounded h-2">
          <div
            class="bg-blue-500 h-2 rounded transition-all"
            :style="{ width: adcPercent + '%' }"
          />
        </div>
      </div>

      <!-- Digital output: toggle -->
      <div v-else-if="isDigitalOutput" class="flex items-center gap-3">
        <label class="flex items-center gap-2 cursor-pointer">
          <div
            class="relative w-10 h-5 rounded-full transition-colors"
            :class="outputValue ? 'bg-green-500' : 'bg-surface-600'"
            @click="toggleOutput"
          >
            <div
              class="absolute top-0.5 w-4 h-4 bg-white rounded-full shadow transition-transform"
              :class="outputValue ? 'translate-x-5' : 'translate-x-0.5'"
            />
          </div>
          <span class="text-sm text-gray-300">{{ outputValue ? 'HIGH' : 'LOW' }}</span>
        </label>
      </div>

      <!-- PWM -->
      <div v-else-if="selectedMode === 'PWM'" class="flex flex-col gap-1.5">
        <div class="flex justify-between text-xs text-gray-400">
          <span>Duty</span>
          <span class="font-mono">{{ pwmValue }} / {{ pwmMax }}</span>
        </div>
        <input
          type="range" v-model.number="pwmValue"
          :min="0" :max="pwmMax" :step="1"
          class="w-full accent-blue-500"
          @change="onPwmChange"
        />
        <div class="flex items-center gap-2 text-xs">
          <label class="text-gray-400 w-12">Freq Hz</label>
          <input
            type="number" v-model.number="pwmFreq"
            class="bg-surface-700 text-gray-200 rounded px-2 py-0.5 border border-surface-600 w-full text-xs"
            @change="onPwmChange"
          />
          <label class="text-gray-400 w-8">Res</label>
          <select
            v-model.number="pwmRes"
            class="bg-surface-700 text-gray-200 rounded px-1 py-0.5 border border-surface-600 text-xs"
            @change="onPwmChange"
          >
            <option v-for="b in [8,10,12,16]" :key="b" :value="b">{{ b }}-bit</option>
          </select>
        </div>
      </div>

      <!-- DAC -->
      <div v-else-if="selectedMode === 'DAC'" class="flex flex-col gap-1.5">
        <div class="flex justify-between text-xs text-gray-400">
          <span>Value</span>
          <span class="font-mono">{{ dacValue }} / 255</span>
        </div>
        <input
          type="range" v-model.number="dacValue"
          :min="0" :max="255" :step="1"
          class="w-full accent-purple-500"
          @change="onDacChange"
        />
      </div>
    </template>
  </div>
</template>

<script setup>
import { ref, computed, watch } from 'vue'
import { usePinStore } from '@/stores/pinStore'

const props = defineProps({
  pin: { type: Object, required: true }
})

const pinStore = usePinStore()

// ─── Local state mirrors the store
const selectedMode = ref(props.pin.mode ?? '')
const outputValue  = ref(props.pin.value ?? 0)
const pwmValue     = ref(props.pin.value ?? 0)
const pwmFreq      = ref(props.pin.freq ?? 1000)
const pwmRes       = ref(props.pin.res ?? 8)
const dacValue     = ref(props.pin.value ?? 0)

// Keep local state in sync when store updates (e.g. from UPDATE poll)
watch(() => props.pin.mode,  v => { if (v) selectedMode.value = v })
watch(() => props.pin.value, v => {
  outputValue.value = v
  pwmValue.value    = v
  dacValue.value    = v
})
watch(() => props.pin.res,  v => pwmRes.value = v)
watch(() => props.pin.freq, v => pwmFreq.value = v)

// ─── Computed helpers
const allCaps = computed(() => [...props.pin.inCaps, ...props.pin.outCaps])

const isDigitalInput  = computed(() => ['IN','IN_UP','IN_DN'].includes(selectedMode.value))
const isAnalogInput   = computed(() => ['ADC','TOUCH'].includes(selectedMode.value))
const isDigitalOutput = computed(() => ['OUT','OUT_OD'].includes(selectedMode.value))

const adcMax     = computed(() => Math.pow(2, props.pin.res) - 1)
const adcPercent = computed(() => adcMax.value > 0 ? (props.pin.value / adcMax.value * 100).toFixed(1) : 0)
const pwmMax     = computed(() => Math.pow(2, pwmRes.value) - 1)

// ─── Mode label lookup
const MODE_LABELS = {
  IN: 'Digital Input', IN_UP: 'Input Pull-Up', IN_DN: 'Input Pull-Down',
  ADC: 'Analog Input (ADC)', TOUCH: 'Capacitive Touch',
  OUT: 'Digital Output', OUT_OD: 'Output Open-Drain',
  PWM: 'PWM Output', DAC: 'DAC (True Analog)'
}
function modeLabel(m) { return MODE_LABELS[m] ?? m }

// ─── Cap badge colors
function capColor(cap) {
  const map = {
    IN: 'bg-blue-900 text-blue-300',
    IN_UP: 'bg-blue-900 text-blue-300',
    IN_DN: 'bg-blue-900 text-blue-300',
    ADC: 'bg-cyan-900 text-cyan-300',
    TOUCH: 'bg-teal-900 text-teal-300',
    OUT: 'bg-orange-900 text-orange-300',
    OUT_OD: 'bg-orange-900 text-orange-300',
    PWM: 'bg-yellow-900 text-yellow-300',
    DAC: 'bg-purple-900 text-purple-300'
  }
  return map[cap] ?? 'bg-surface-700 text-gray-400'
}

// ─── Actions
async function onModeChange() {
  if (!selectedMode.value) return
  await pinStore.setPin({
    pin: props.pin.name,
    mode: selectedMode.value,
    value: 0,
    res: pwmRes.value,
    freq: pwmFreq.value
  })
}

async function toggleOutput() {
  outputValue.value = outputValue.value ? 0 : 1
  await pinStore.setPin({
    pin: props.pin.name,
    mode: selectedMode.value,
    value: outputValue.value
  })
}

async function onPwmChange() {
  await pinStore.setPin({
    pin: props.pin.name,
    mode: 'PWM',
    value: pwmValue.value,
    res: pwmRes.value,
    freq: pwmFreq.value
  })
}

async function onDacChange() {
  await pinStore.setPin({
    pin: props.pin.name,
    mode: 'DAC',
    value: dacValue.value
  })
}
</script>
