<template>
  <div
    class="bg-surface-800 border rounded-xl p-3 flex flex-col gap-2 transition-all"
    :class="pin.mode ? 'border-surface-600' : 'border-surface-700 opacity-60'"
  >
    <!-- Header -->
    <div class="flex items-center justify-between gap-2">
      <span class="font-mono font-bold text-sm">{{ pin.name }}</span>
      <div class="flex items-center gap-1.5">
        <span class="text-xs px-1.5 py-0.5 rounded font-mono" :class="capColor(pin.cap)">{{ pin.cap }}</span>
        <!-- Info tooltip -->
        <div class="relative group">
          <button
            class="text-tgray-600 hover:text-tgray-300 transition-colors leading-none"
            tabindex="0"
            aria-label="Pin capabilities"
          >&#9432;</button>
          <div
            class="absolute right-0 top-6 z-30 hidden group-hover:block group-focus-within:block
                   bg-surface-700 border border-surface-600 rounded-lg px-3 py-2 text-xs
                   text-tgray-300 whitespace-nowrap shadow-xl min-w-max"
          >
            <div v-if="pin.outCaps?.length" class="mb-1">
              <span class="text-tgray-500">OUT: </span>
              <span>{{ pin.outCaps.join(', ') }}</span>
            </div>
            <div v-if="pin.inCaps?.length">
              <span class="text-tgray-500">IN: </span>
              <span>{{ pin.inCaps.join(', ') }}</span>
            </div>
          </div>
        </div>
      </div>
    </div>

    <!-- Mode selector -->
    <select
      v-model="selectedMode"
      @change="setMode"
      class="bg-surface-700 text-sm text-tgray-200 rounded px-2 py-1 border border-surface-600 w-full"
    >
      <option value="">&#8212; Disabled &#8212;</option>
      <optgroup v-if="outModes.length" label="Output">
        <option v-for="m in outModes" :key="m" :value="m">{{ m }}</option>
      </optgroup>
      <optgroup v-if="inModes.length" label="Input">
        <option v-for="m in inModes" :key="m" :value="m">{{ m }}</option>
      </optgroup>
    </select>

    <!-- Value display / control -->
    <div v-if="pin.mode" class="flex flex-col gap-1.5">

      <!-- Digital Input -->
      <div v-if="pin.mode === 'DIN'" class="flex items-center gap-2">
        <span
          class="text-xs px-2 py-0.5 rounded font-mono"
          :class="pin.value ? 'bg-green-700 text-green-100' : 'bg-surface-700 text-tgray-400'"
        >{{ pin.value ? 'HIGH' : 'LOW' }}</span>
      </div>

      <!-- Analog Input -->
      <div v-else-if="pin.mode === 'AIN'" class="flex flex-col gap-1">
        <div class="w-full bg-surface-700 rounded h-2">
          <div
            class="bg-blue-500 h-2 rounded transition-all"
            :style="{ width: (pin.value / ainMax * 100) + '%' }"
          />
        </div>
        <div class="flex justify-between">
          <span class="text-xs text-tgray-400">{{ pin.value }}</span>
          <span class="text-xs text-tgray-600">/ {{ ainMax }}</span>
        </div>
      </div>

      <!-- Digital Output -->
      <div v-else-if="pin.mode === 'DOUT'" class="flex items-center gap-2">
        <button
          @click="toggleOutput"
          class="text-xs px-3 py-0.5 rounded font-mono transition-colors"
          :class="outputValue ? 'bg-green-500 text-white' : 'bg-surface-600 text-tgray-400'"
        >{{ outputValue ? 'HIGH' : 'LOW' }}</button>
      </div>

      <!-- PWM Output -->
      <div v-else-if="pin.mode === 'PWM'" class="flex flex-col gap-1.5">
        <div class="flex items-center gap-2">
          <input
            type="range" :min="0" :max="pwmMax"
            v-model.number="pwmValue"
            @change="setPwm"
            class="flex-1 accent-blue-500"
          />
          <input
            type="number" :min="0" :max="pwmMax"
            v-model.number="pwmValue"
            @change="setPwm"
            class="bg-surface-700 text-tgray-200 rounded px-2 py-0.5 border border-surface-600 w-16 text-xs"
          />
        </div>
        <!-- PWM meta: freq + resolution -->
        <div class="flex gap-3 text-xs text-tgray-500">
          <span>&#x1F4CA; {{ pin.res }}&#8209;bit <span class="text-tgray-600">(max {{ pwmMax }})</span></span>
          <span>&#x1F4F6; {{ formatFreq(pin.freq) }}</span>
        </div>
      </div>

      <!-- DAC Output -->
      <div v-else-if="pin.mode === 'DAC'" class="flex flex-col gap-1.5">
        <div class="flex items-center gap-2">
          <input
            type="range" :min="0" :max="dacMax"
            v-model.number="dacValue"
            @change="setDac"
            class="flex-1 accent-yellow-500"
          />
          <input
            type="number" :min="0" :max="dacMax"
            v-model.number="dacValue"
            @change="setDac"
            class="bg-surface-700 text-tgray-200 rounded px-2 py-0.5 border border-surface-600 w-16 text-xs"
          />
        </div>
        <div class="text-xs text-tgray-500">
          &#x1F4CA; {{ pin.res }}-bit <span class="text-tgray-600">(max {{ dacMax }})</span>
        </div>
      </div>

      <!-- Servo -->
      <div v-else-if="pin.mode === 'SERVO'" class="flex items-center gap-2">
        <input
          type="range" min="0" max="180"
          v-model.number="servoAngle"
          @change="setServo"
          class="flex-1 accent-purple-500"
        />
        <input
          type="number" min="0" max="180"
          v-model.number="servoAngle"
          @change="setServo"
          class="bg-surface-700 text-tgray-200 rounded px-1 py-0.5 border border-surface-600 w-14 text-xs"
        />
        <span class="text-xs text-tgray-400">&#176;</span>
      </div>

      <!-- Generic input read (fallback for unknown input modes) -->
      <div
        v-else-if="isInputMode"
        class="p-2 bg-surface-900 rounded border border-surface-700 text-center"
      >
        <p class="font-mono text-lg text-twhite">{{ pin.value }}</p>
      </div>

    </div>
  </div>
</template>

<script setup>
import { ref, computed, watch } from 'vue'
import { usePinStore } from '@/stores/pinStore'

const props = defineProps({ pin: Object })
const pinStore = usePinStore()

const selectedMode = ref(props.pin.mode || '')
const outputValue  = ref(props.pin.value ?? 0)
const pwmValue     = ref(props.pin.value ?? 0)
const dacValue     = ref(props.pin.value ?? 0)
const servoAngle   = ref(props.pin.value ?? 90)

watch(() => props.pin, (p) => {
  selectedMode.value = p.mode || ''
  if (document.activeElement?.tagName !== 'INPUT') {
    outputValue.value = p.value ?? 0
    pwmValue.value    = p.value ?? 0
    dacValue.value    = p.value ?? 0
    servoAngle.value  = p.value ?? 90
  }
}, { deep: true })

// Derive in/out modes from pinStore structure (inCaps / outCaps)
// Falls back to legacy cap-based logic for compatibility
const MODE_CAPS = {
  DIN:   ['DIO', 'AI'],
  DOUT:  ['DIO', 'AI'],
  AIN:   ['AI'],
  PWM:   ['PWM'],
  DAC:   ['DAC'],
  SERVO: ['SERVO'],
}

const outModes = computed(() => {
  if (props.pin.outCaps?.length) return props.pin.outCaps
  const cap = props.pin.cap
  return Object.entries(MODE_CAPS)
    .filter(([mode, caps]) => caps.includes(cap) && ['DOUT','PWM','DAC','SERVO'].includes(mode))
    .map(([mode]) => mode)
})

const inModes = computed(() => {
  if (props.pin.inCaps?.length) return props.pin.inCaps
  const cap = props.pin.cap
  return Object.entries(MODE_CAPS)
    .filter(([mode, caps]) => caps.includes(cap) && ['DIN','AIN'].includes(mode))
    .map(([mode]) => mode)
})

const allowedModes = computed(() => [...outModes.value, ...inModes.value])

const isInputMode = computed(() =>
  props.pin.mode && !outModes.value.includes(props.pin.mode) && props.pin.mode !== 'DISABLED'
)

// Computed max values from firmware-reported resolution
const pwmMax = computed(() => (1 << (props.pin.res || 8)) - 1)
const dacMax = computed(() => (1 << (props.pin.res || 8)) - 1)
const ainMax = computed(() => (1 << (props.pin.res || 12)) - 1)

function formatFreq(hz) {
  if (!hz) return '? Hz'
  if (hz >= 1000) return (hz / 1000).toFixed(1) + ' kHz'
  return hz + ' Hz'
}

async function setMode() {
  await pinStore.setPin({ pin: props.pin.name, mode: selectedMode.value, value: 0 })
}

async function toggleOutput() {
  outputValue.value = outputValue.value ? 0 : 1
  await pinStore.setValue(props.pin.name, outputValue.value)
}

async function setPwm() {
  await pinStore.setValue(props.pin.name, pwmValue.value)
}

async function setDac() {
  await pinStore.setValue(props.pin.name, dacValue.value)
}

async function setServo() {
  await pinStore.setValue(props.pin.name, servoAngle.value)
}

const CAP_COLORS = {
  DIO:   'bg-blue-900/50 text-blue-300',
  AI:    'bg-orange-900/50 text-orange-300',
  PWM:   'bg-purple-900/50 text-purple-300',
  DAC:   'bg-yellow-900/50 text-yellow-300',
  SERVO: 'bg-pink-900/50 text-pink-300',
}
const capColor = (cap) => CAP_COLORS[cap] ?? 'bg-surface-700 text-tgray-400'
</script>
