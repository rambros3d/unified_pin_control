// Lightweight global toast bus — no PrimeVue dependency
import { reactive } from 'vue'

let _nextId = 1

export const toasts = reactive([])

export function useToast() {
  function add({ severity = 'info', summary = '', detail = '', life = 3000 }) {
    const id = _nextId++
    toasts.push({ id, severity, summary, detail })
    if (life > 0) setTimeout(() => dismiss(id), life)
  }

  function dismiss(id) {
    const idx = toasts.findIndex(t => t.id === id)
    if (idx !== -1) toasts.splice(idx, 1)
  }

  return { add, dismiss }
}
