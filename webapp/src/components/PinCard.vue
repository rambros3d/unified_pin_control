<template>
  <div class="bg-surface-800 border border-surface-600 rounded-xl p-3 flex flex-col gap-2">

    <!-- Header -->
    <div class="flex items-center justify-between gap-2">
      <span class="font-mono font-bold text-sm text-twhite">{{ pin.name }}</span>
      <div class="flex items-center gap-1.5">
        <!-- Mode badge -->
        <span class="text-xs px-1.5 py-0.5 rounded font-mono" :class="modeColor">{{ pin.mode }}</span>
        <!-- Caps info tooltip -->
        <div class="relative group">
          <button class="text-tgray-600 hover:text-tgray-300 transition-colors leading-none" aria-label="Pin capabilities">&#9432;</button>
          <div class="absolute right-0 top-6 z-30 hidden group-hover:block group-focus-within:block
                      bg-surface-700 border border-surface-600 rounded-lg px-3 py-2 text-xs
                      text-tgray-300 whitespace-nowrap shadow-xl min-w-max">
            <div v-if="pin.outCaps?.length" class="mb-1">
              <span class="text-tgray-500">OUT: </span>{{ pin.outCaps.join(', ') }}
            </div>
            <div v-if="pin.inCaps?.length">
              <span class="text-tgray-500">IN: </span>{{ pin.inCaps.join(', ') }}
            </div>
          </div>
        </div>
        <!-- Disable / remove button -->
        <button
          @click="disablePin"
          class="text-tgray-600 hover:text-red-400 transition-colors leading-none text-base"
          title="Disable pin"
          aria-label="Disable pin"
        >&#10005;</button>
      </div>
    </div>

    <!-- Value display / control -->
    <div class="flex flex-col gap-1.5">

      <!-- Digital Input (IN / IN_UP / IN_DN) -->
      <div v-if="isDigitalIn" class="flex items-center gap-2">
        <span
          class="text-xs px-2 py-0.5 rounded font-mono"
          :class="pin.value ? 'bg-green-700 text-green-100' : 'bg-surface-700 text-tgray-400'"
        >{{ pin.value ? 'HIGH' : 'LOW' }}</span>
      </div>

      <!-- ADC Input -->
      <div v-else-if="pin.mode === 'ADC'" class="flex flex-col gap-1">
        <div class="w-full bg-surface-700 rounded h-2">
          <div class="bg-blue-500 h-2 rounded transition-all" :style="{ width: (pin.value / ainMax * 100) + '%' }" />
        </div>
        <div class="flex justify-between">
          <span class="text-xs text-tgray-400">{{ pin.value }}</span>
          <span class="text-xs text-tgray-600">/ {{ ainMax }}</span>
        </div>
      </div>

      <!-- TOUCH Input -->
      <div v-else-if="pin.mode === 'TOUCH'" class="flex flex-col gap-1">
        <div class="w-full bg-surface-700 rounded h-2">
          <div class="bg-purple-500 h-2 rounded transition-all" :style="{ width: Math.min(pin.value / 100 * 100, 100) + '%' }" />
        </div>
        <span class="text-xs text-tgray-400">{{ pin.value }}</span>
      </div>

      <!-- Digital Output (OUT / OUT_OD) -->
      <div v-else-if="isDigitalOut" class="flex items-center gap-2">
        <button
          @click="toggleOutput"
          class="text-xs px-3 py-0.5 rounded font-mono transition-colors"
          :class="outputVal ? 'bg-green-600 text-white' : 'bg-surface-600 text-tgray-400'"
        >{{ outputVal ? 'HIGH' : 'LOW' }}</button>
      </div>

      <!-- PWM -->
      <div v-else-if="pin.mode === 'PWM'" class="flex flex-col gap-1.5">
        <div class="flex items-center gap-2">
          <input type="range" :min="0" :max="pwmMax" v-model.number="pwmVal" @change="setPwm" class="flex-1 accent-blue-500" />
          <input type="number" :min="0" :max="pwmMax" v-model.number="pwmVal" @change="setPwm"
            class="bg-surface-700 text-tgray-200 rounded px-2 py-0.5 border border-surface-600 w-16 text-xs" />
        </div>
        <div class="flex gap-3 text-xs text-tgray-500">
          <span>{{ pin.res }}-bit (max {{ pwmMax }})</span>
          <span>{{ formatFreq(pin.freq) }}</span>
        </div>
      </div>

      <!-- DAC -->
      <div v-else-if="pin.mode === 'DAC'" class="flex flex-col gap-1.5">
        <div class="flex items-center gap-2">
          <input type="range" :min="0" :max="dacMax" v-model.number="dacVal" @change="setDac" class="flex-1 accent-yellow-500" />
          <input type="number" :min="0" :max="dacMax" v-model.number="dacVal" @change="setDac"
            class="bg-surface-700 text-tgray-200 rounded px-2 py-0.5 border border-surface-600 w-16 text-xs" />
        </div>
        <div class="text-xs text-tgray-500">{{ pin.res }}-bit (max {{ dacMax }})</div>
      </div>

      <!-- SERVO -->
      <div v-else-if="pin.mode === 'SERVO'" class="flex items-center gap-2">
        <input type="range" min="0" max="180" v-model.number="servoVal" @change="setServo" class="flex-1 accent-purple-500" />
        <input type="number" min="0" max="180" v-model.number="servoVal" @change="setServo"
          class="bg-surface-700 text-tgray-200 rounded px-1 py-0.5 border border-surface-600 w-14 text-xs" />
        <span class="text-xs text-tgray-400">&deg;</span>
      </div>

    </div>
  </div>
</template>

<script setup>
import { ref, computed, watch } from 'vue'
import { usePinStore } from '@/stores/pinStore'

const props = defineProps({ pin: Object })
const pinStore = usePinStore()

const outputVal = ref(props.pin.value ?? 0)
const pwmVal    = ref(props.pin.value ?? 0)
const dacVal    = ref(props.pin.value ?? 0)
const servoVal  = ref(props.pin.value ?? 90)

watch(() => props.pin.value, (v) => {
  if (document.activeElement?.tagName !== 'INPUT') {
    outputVal.value = v ?? 0
    pwmVal.value    = v ?? 0
    dacVal.value    = v ?? 0
    servoVal.value  = v ?? 90
  }
})

const isDigitalIn  = computed(() => ['IN','IN_UP','IN_DN'].includes(props.pin.mode))
const isDigitalOut = computed(() => ['OUT','OUT_OD'].includes(props.pin.mode))

const pwmMax = computed(() => (1 << (props.pin.res || 8)) - 1)
const dacMax = computed(() => (1 << (props.pin.res || 8)) - 1)
const ainMax = computed(() => (1 << 12) - 1)  // ADC is always 12-bit on ESP32

function formatFreq(hz) {
  if (!hz) return '? Hz'
  return hz >= 1000 ? (hz / 1000).toFixed(1) + ' kHz' : hz + ' Hz'
}

const MODE_COLORS = {
  IN:     'bg-blue-900/60 text-blue-300',
  IN_UP:  'bg-blue-900/60 text-blue-300',
  IN_DN:  'bg-blue-900/60 text-blue-300',
  ADC:    'bg-orange-900/60 text-orange-300',
  TOUCH:  'bg-pink-900/60 text-pink-300',
  OUT:    'bg-green-900/60 text-green-300',
  OUT_OD: 'bg-green-900/60 text-green-300',
  PWM:    'bg-purple-900/60 text-purple-300',
  DAC:    'bg-yellow-900/60 text-yellow-300',
  SERVO:  'bg-indigo-900/60 text-indigo-300',
}
const modeColor = computed(() => MODE_COLORS[props.pin.mode] ?? 'bg-surface-700 text-tgray-400')

async function disablePin() {
  await pinStore.disablePin(props.pin.name)
}
async function toggleOutput() {
  outputVal.value = outputVal.value ? 0 : 1
  await pinStore.setValue(props.pin.name, outputVal.value)
}
async function setPwm()   { await pinStore.setValue(props.pin.name, pwmVal.value) }
async function setDac()   { await pinStore.setValue(props.pin.name, dacVal.value) }
async function setServo() { await pinStore.setValue(props.pin.name, servoVal.value) }
</script>
