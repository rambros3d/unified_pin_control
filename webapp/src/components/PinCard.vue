<template>
  <div
    class="bg-surface-800 border rounded-xl p-3 flex flex-col gap-2 transition-all"
    :class="pin.mode ? 'border-surface-600' : 'border-surface-700 opacity-60'"
  >
    <!-- Header -->
    <div class="flex items-center justify-between gap-2">
      <span class="font-mono font-bold text-sm">{{ pin.name }}</span>
      <span class="text-xs px-1.5 py-0.5 rounded font-mono" :class="capColor(pin.cap)">{{ pin.cap }}</span>
    </div>

    <!-- Mode selector -->
    <select
      v-model="selectedMode"
      @change="setMode"
      class="bg-surface-700 text-sm text-tgray-200 rounded px-2 py-1 border border-surface-600 w-full"
    >
      <option value="">— Disabled —</option>
      <option v-for="m in allowedModes" :key="m" :value="m">{{ m }}</option>
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
            :style="{ width: (pin.value / 1023 * 100) + '%' }"
          />
        </div>
        <span class="text-xs text-tgray-400">{{ pin.value }} / 1023</span>
      </div>

      <!-- Digital Output -->
      <div v-else-if="pin.mode === 'DOUT'" class="flex items-center gap-2">
        <button
          @click="toggleOutput"
          class="text-xs px-3 py-0.5 rounded font-mono transition-colors"
          :class="outputValue ? 'bg-green-500' : 'bg-surface-600'"
        >{{ outputValue ? 'HIGH' : 'LOW' }}</button>
      </div>

      <!-- PWM Output -->
      <div v-else-if="pin.mode === 'PWM'" class="flex items-center gap-2">
        <input
          type="range" min="0" max="255"
          v-model.number="pwmValue"
          @change="setPwm"
          class="flex-1 accent-blue-500"
        />
        <input
          type="number" min="0" max="255"
          v-model.number="pwmValue"
          @change="setPwm"
          class="bg-surface-700 text-tgray-200 rounded px-2 py-0.5 border border-surface-600 w-full text-xs"
        />
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
          class="bg-surface-700 text-tgray-200 rounded px-1 py-0.5 border border-surface-600 text-xs"
        />
        <span class="text-xs text-tgray-400">°</span>
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
const servoAngle   = ref(props.pin.value ?? 90)

watch(() => props.pin, (p) => {
  if (document.activeElement?.tagName !== 'INPUT') {
    outputValue.value = p.value ?? 0
    pwmValue.value    = p.value ?? 0
    servoAngle.value  = p.value ?? 90
  }
}, { deep: true })

const MODE_CAPS = {
  DIN:   ['DIO', 'AI'],
  DOUT:  ['DIO', 'AI'],
  AIN:   ['AI'],
  PWM:   ['PWM'],
  SERVO: ['SERVO'],
}

const allowedModes = computed(() => {
  const cap = props.pin.cap
  return Object.entries(MODE_CAPS)
    .filter(([, caps]) => caps.includes(cap))
    .map(([mode]) => mode)
})

async function setMode()  { await pinStore.setMode(props.pin.name, selectedMode.value) }
async function toggleOutput() {
  outputValue.value = outputValue.value ? 0 : 1
  await pinStore.setValue(props.pin.name, outputValue.value)
}
async function setPwm()   { await pinStore.setValue(props.pin.name, pwmValue.value) }
async function setServo() { await pinStore.setValue(props.pin.name, servoAngle.value) }

const CAP_COLORS = {
  DIO:   'bg-blue-900/50 text-blue-300',
  AI:    'bg-orange-900/50 text-orange-300',
  PWM:   'bg-purple-900/50 text-purple-300',
  SERVO: 'bg-pink-900/50 text-pink-300',
}
const capColor = (cap) => {
  const map = CAP_COLORS
  return map[cap] ?? 'bg-surface-700 text-tgray-400'
}
</script>
