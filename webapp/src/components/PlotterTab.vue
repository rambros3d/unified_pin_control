<template>
  <div class="flex flex-col h-full gap-0">

    <!-- Controls bar -->
    <div class="bg-surface-800 border-b border-surface-700 px-4 py-2 flex flex-wrap items-center gap-4">
      <!-- Channel toggles -->
      <div class="flex flex-wrap items-center gap-3">
        <span class="text-xs text-tgray-400 font-medium">Channels:</span>
        <div v-if="dataKeys.length === 0" class="text-xs text-tgray-500 italic">
          Waiting for numeric data… (e.g. "1.23,45.6,7")
        </div>
        <label
          v-for="(key, i) in dataKeys"
          :key="key"
          class="flex items-center gap-1.5 text-xs cursor-pointer select-none"
        >
          <input
            type="checkbox"
            :checked="visibleKeys[key]"
            @change="visibleKeys[key] = !visibleKeys[key]; redrawChart()"
            class="accent-blue-500"
          />
          <span
            class="w-2.5 h-2.5 rounded-full flex-shrink-0"
            :style="{ background: COLORS[i % COLORS.length] }"
          />
          <span class="text-tgray-300">{{ key }}</span>
        </label>
      </div>

      <div class="flex-1" />

      <!-- Window size -->
      <div class="flex items-center gap-2 text-xs text-tgray-400">
        <span>Window:</span>
        <select
          v-model="windowSize"
          class="bg-surface-700 text-tgray-200 rounded px-2 py-1 border border-surface-600 text-xs"
        >
          <option :value="50">50</option>
          <option :value="100">100</option>
          <option :value="200">200</option>
          <option :value="500">500</option>
        </select>
        <span>pts</span>
      </div>

      <!-- Clear -->
      <button @click="clearData" class="btn-secondary text-xs">
        🗑 Clear Data
      </button>
    </div>

    <!-- Chart area -->
    <div class="flex-1 relative bg-surface-900 p-3">
      <canvas ref="canvasEl" class="w-full h-full" />
    </div>

    <!-- Status bar -->
    <div class="bg-surface-800 border-t border-surface-700 px-4 py-1 flex items-center gap-4 text-xs text-tgray-500">
      <span>Points: {{ chartData.length }}</span>
      <span v-if="lastValues.length">Latest: {{ lastValues.join(' | ') }}</span>
      <span class="ml-auto">Sampling live from serial RX</span>
    </div>
  </div>
</template>

<script setup>
import { ref, watch, onMounted, onUnmounted, nextTick } from 'vue'
import { useSerial } from '@/composables/useSerial'

const { onRaw } = useSerial()

const COLORS = [
  '#60a5fa', '#34d399', '#f59e0b', '#f472b6',
  '#a78bfa', '#fb923c', '#22d3ee', '#a3e635'
]

const canvasEl = ref(null)
const dataKeys = ref([])
const visibleKeys = ref({})
const chartData = ref([])    // Array of {time, val1, val2, ...}
const lastValues = ref([])
const windowSize = ref(100)

let chartInstance = null
let Chart = null

// Load Chart.js dynamically (avoid build-time dep)
onMounted(async () => {
  // Inject Chart.js via script tag if not already present
  if (!window.Chart) {
    await new Promise((resolve, reject) => {
      const s = document.createElement('script')
      s.src = 'https://cdn.jsdelivr.net/npm/chart.js@4/dist/chart.umd.min.js'
      s.onload = resolve
      s.onerror = reject
      document.head.appendChild(s)
    })
  }
  Chart = window.Chart
  initChart()
})

onUnmounted(() => {
  if (chartInstance) { chartInstance.destroy(); chartInstance = null }
  removeRaw()
})

function initChart() {
  if (!canvasEl.value || !Chart) return
  if (chartInstance) chartInstance.destroy()
  chartInstance = new Chart(canvasEl.value, {
    type: 'line',
    data: { labels: [], datasets: [] },
    options: {
      animation: false,
      responsive: true,
      maintainAspectRatio: false,
      interaction: { mode: 'index', intersect: false },
      plugins: {
        legend: { display: false },
        tooltip: {
          backgroundColor: '#1e1d1b',
          borderColor: '#3a3836',
          borderWidth: 1,
          titleColor: '#cdccca',
          bodyColor: '#797876',
        }
      },
      scales: {
        x: {
          ticks: { color: '#5a5957', maxTicksLimit: 8, maxRotation: 0 },
          grid: { color: '#262523' }
        },
        y: {
          ticks: { color: '#5a5957' },
          grid: { color: '#262523' }
        }
      }
    }
  })
}

function redrawChart() {
  if (!chartInstance) return
  const window_data = chartData.value.slice(-windowSize.value)
  const labels = window_data.map((_, i) => i)

  chartInstance.data.labels = labels
  chartInstance.data.datasets = dataKeys.value
    .filter(k => visibleKeys.value[k])
    .map((key, i) => ({
      label: key,
      data: window_data.map(d => d[key] ?? null),
      borderColor: COLORS[i % COLORS.length],
      backgroundColor: COLORS[i % COLORS.length] + '22',
      borderWidth: 1.5,
      pointRadius: 0,
      tension: 0.2,
    }))
  chartInstance.update('none')
}

watch(windowSize, () => redrawChart())

function clearData() {
  chartData.value = []
  dataKeys.value = []
  visibleKeys.value = {}
  lastValues.value = []
  redrawChart()
}

// Listen to raw RX lines and parse CSV numbers
const removeRaw = onRaw((line, dir) => {
  if (dir !== 'rx') return
  const values = line.split(',').map(v => parseFloat(v.trim())).filter(v => !isNaN(v))
  if (!values.length) return

  const point = { time: Date.now() }
  const keys = []
  values.forEach((val, i) => {
    const key = `ch${i + 1}`
    point[key] = val
    keys.push(key)
  })

  // Register new keys
  let changed = false
  for (const k of keys) {
    if (!(k in visibleKeys.value)) {
      visibleKeys.value[k] = true
      changed = true
    }
  }
  if (changed || dataKeys.value.length !== keys.length) {
    dataKeys.value = keys
  }

  lastValues.value = values.map((v, i) => `ch${i + 1}: ${v}`)
  chartData.value.push(point)

  // Keep memory bounded at 2x window max
  const cap = Math.max(windowSize.value * 2, 1000)
  if (chartData.value.length > cap) chartData.value.splice(0, chartData.value.length - cap)

  redrawChart()
})
</script>
