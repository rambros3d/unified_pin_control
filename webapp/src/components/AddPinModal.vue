<template>
  <!-- Backdrop -->
  <Teleport to="body">
    <div
      class="fixed inset-0 bg-black/60 backdrop-blur-sm flex items-center justify-center z-50 p-4"
      @click.self="$emit('close')"
    >
      <div class="bg-surface-800 border border-surface-600 rounded-2xl shadow-2xl w-full max-w-md flex flex-col gap-0 overflow-hidden">

        <!-- Header -->
        <div class="flex items-center justify-between px-5 py-4 border-b border-surface-700">
          <h2 class="text-base font-bold text-twhite">Configure Pin</h2>
          <button
            @click="$emit('close')"
            class="text-tgray-500 hover:text-twhite transition-colors text-xl leading-none"
            aria-label="Close"
          >&times;</button>
        </div>

        <!-- Body -->
        <div class="flex flex-col gap-4 px-5 py-5">

          <!-- Pin selector -->
          <div class="flex flex-col gap-1.5">
            <label class="text-xs font-medium text-tgray-400 uppercase tracking-wide">Pin</label>
            <select
              v-model="selectedPin"
              @change="onPinChange"
              class="bg-surface-700 text-tgray-100 rounded-lg px-3 py-2 border border-surface-600 text-sm outline-none focus:border-blue-500 transition-colors"
            >
              <option value="">Select a pin…</option>
              <optgroup label="Inactive Pins">
                <option
                  v-for="pin in inactivePins"
                  :key="pin.name"
                  :value="pin.name"
                >{{ pin.name }} &mdash; {{ pin.cap }}</option>
              </optgroup>
              <optgroup v-if="activePins.length" label="Active Pins (reconfigure)">
                <option
                  v-for="pin in activePins"
                  :key="pin.name"
                  :value="pin.name"
                >{{ pin.name }} &mdash; {{ pin.mode }}</option>
              </optgroup>
            </select>
          </div>

          <!-- Mode selector -->
          <div v-if="selectedPin" class="flex flex-col gap-1.5">
            <label class="text-xs font-medium text-tgray-400 uppercase tracking-wide">Mode</label>
            <div class="grid grid-cols-3 gap-2">
              <button
                v-for="mode in availableModes"
                :key="mode"
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
          </div>

          <!-- Initial value for output modes -->
          <div v-if="selectedMode && isOutputMode" class="flex flex-col gap-1.5">
            <label class="text-xs font-medium text-tgray-400 uppercase tracking-wide">
              Initial Value
            </label>

            <!-- DOUT: toggle -->
            <div v-if="selectedMode === 'DOUT'" class="flex gap-2">
              <button
                @click="initValue = 0"
                class="flex-1 py-1.5 rounded-lg border text-sm transition-all"
                :class="initValue === 0 ? 'bg-surface-600 border-blue-500 text-twhite' : 'bg-surface-700 border-surface-600 text-tgray-400'"
              >LOW (0)</button>
              <button
                @click="initValue = 1"
                class="flex-1 py-1.5 rounded-lg border text-sm transition-all"
                :class="initValue === 1 ? 'bg-green-700 border-green-500 text-white' : 'bg-surface-700 border-surface-600 text-tgray-400'"
              >HIGH (1)</button>
            </div>

            <!-- PWM / DAC: slider -->
            <div v-else class="flex items-center gap-3">
              <input
                type="range" min="0" :max="outputMax"
                v-model.number="initValue"
                class="flex-1 accent-blue-500"
              />
              <input
                type="number" min="0" :max="outputMax"
                v-model.number="initValue"
                class="w-20 bg-surface-700 text-tgray-100 rounded px-2 py-1 border border-surface-600 text-sm"
              />
              <span class="text-xs text-tgray-500">/ {{ outputMax }}</span>
            </div>

            <!-- SERVO: 0-180 -->
            <div v-if="selectedMode === 'SERVO'" class="flex items-center gap-3">
              <input
                type="range" min="0" max="180"
                v-model.number="initValue"
                class="flex-1 accent-purple-500"
              />
              <span class="text-sm text-tgray-200 w-12">{{ initValue }}&deg;</span>
            </div>
          </div>

          <!-- Info row -->
          <div v-if="selectedMode" class="flex items-center gap-2 text-xs text-tgray-500 bg-surface-900 rounded-lg px-3 py-2">
            <span>{{ MODE_META[selectedMode]?.icon }}</span>
            <span>{{ MODE_META[selectedMode]?.desc }}</span>
          </div>
        </div>

        <!-- Footer -->
        <div class="flex items-center justify-end gap-3 px-5 py-4 border-t border-surface-700">
          <button @click="$emit('close')" class="btn-secondary">Cancel</button>
          <button
            @click="apply"
            :disabled="!selectedPin || !selectedMode"
            class="px-4 py-1.5 rounded-lg text-sm font-semibold transition-colors
                   bg-blue-600 hover:bg-blue-500 text-white
                   disabled:opacity-40 disabled:cursor-not-allowed"
          >
            Apply
          </button>
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

// ─── Mode metadata
const MODE_META = {
  DOUT:  { icon: '💡', desc: 'Digital output — set HIGH or LOW' },
  DIN:   { icon: '🔍', desc: 'Digital input — read HIGH / LOW' },
  AIN:   { icon: '📊', desc: 'Analog input — read ADC value' },
  PWM:   { icon: '🌀', desc: 'PWM output — control duty cycle' },
  DAC:   { icon: '📉', desc: 'DAC output — true analog voltage' },
  SERVO: { icon: '⚙️', desc: 'Servo motor — set angle 0–180°' },
}

const selectedPin  = ref('')
const selectedMode = ref('')
const initValue    = ref(0)

const inactivePins = computed(() => pinStore.pinList.filter(p => !p.mode))
const activePins   = computed(() => pinStore.pinList.filter(p =>  p.mode))

const pinObj = computed(() => pinStore.pins.get(selectedPin.value))

const availableModes = computed(() => {
  if (!pinObj.value) return []
  const out = pinObj.value.outCaps?.length ? pinObj.value.outCaps : []
  const inp = pinObj.value.inCaps?.length  ? pinObj.value.inCaps  : []
  // fallback: derive from legacy cap field
  if (!out.length && !inp.length) {
    const cap = pinObj.value.cap ?? ''
    const all = []
    if (cap.includes('O') || cap === 'DIO') all.push('DOUT')
    if (cap.includes('P'))                  all.push('PWM')
    if (cap.includes('D'))                  all.push('DAC')
    if (cap.includes('S'))                  all.push('SERVO')
    if (cap.includes('I') || cap === 'DIO') all.push('DIN')
    if (cap.includes('A'))                  all.push('AIN')
    return all
  }
  return [...out, ...inp]
})

const isOutputMode = computed(() =>
  ['DOUT', 'PWM', 'DAC', 'SERVO'].includes(selectedMode.value)
)

const outputMax = computed(() => {
  if (selectedMode.value === 'SERVO') return 180
  const res = pinObj.value?.res ?? 8
  return (1 << res) - 1
})

function onPinChange() {
  selectedMode.value = ''
  initValue.value    = 0
}

async function apply() {
  if (!selectedPin.value || !selectedMode.value) return
  await pinStore.setPin({
    pin:   selectedPin.value,
    mode:  selectedMode.value,
    value: initValue.value,
  })
  emit('applied', { pin: selectedPin.value, mode: selectedMode.value })
  emit('close')
}
</script>
