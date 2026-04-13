<template>
  <Teleport to="body">
    <div
      class="fixed bottom-4 left-4 z-[9999] flex flex-col-reverse gap-2 w-80 pointer-events-none"
      aria-live="polite"
      aria-atomic="false"
    >
      <TransitionGroup
        tag="div"
        class="flex flex-col-reverse gap-2"
        enter-active-class="transition-all duration-300 ease-out"
        enter-from-class="opacity-0 translate-y-2"
        enter-to-class="opacity-100 translate-y-0"
        leave-active-class="transition-all duration-200 ease-in"
        leave-from-class="opacity-100 translate-y-0"
        leave-to-class="opacity-0 translate-y-2"
      >
        <div
          v-for="t in toasts"
          :key="t.id"
          class="pointer-events-auto flex items-start gap-3 px-4 py-3 rounded-xl border shadow-xl backdrop-blur-sm"
          :class="styles[t.severity] ?? styles.info"
        >
          <!-- Severity icon -->
          <span class="text-lg leading-none mt-0.5 flex-shrink-0">{{ icons[t.severity] ?? icons.info }}</span>

          <!-- Text -->
          <div class="flex-1 min-w-0">
            <p class="text-sm font-semibold leading-tight">{{ t.summary }}</p>
            <p v-if="t.detail" class="text-xs mt-0.5 opacity-80 break-words">{{ t.detail }}</p>
          </div>

          <!-- Action buttons -->
          <div class="flex items-center gap-1 flex-shrink-0">
            <!-- Copy -->
            <button
              @click="copy(t)"
              :title="copied === t.id ? 'Copied!' : 'Copy'"
              class="w-6 h-6 flex items-center justify-center rounded opacity-50 hover:opacity-100 transition-opacity text-xs"
              aria-label="Copy message"
            >
              <svg v-if="copied !== t.id" xmlns="http://www.w3.org/2000/svg" width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
                <rect x="9" y="9" width="13" height="13" rx="2"/><path d="M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v1"/>
              </svg>
              <svg v-else xmlns="http://www.w3.org/2000/svg" width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round">
                <polyline points="20 6 9 17 4 12"/>
              </svg>
            </button>

            <!-- Dismiss -->
            <button
              @click="dismiss(t.id)"
              class="w-6 h-6 flex items-center justify-center rounded text-base leading-none opacity-50 hover:opacity-100 transition-opacity"
              aria-label="Dismiss"
            >&times;</button>
          </div>
        </div>
      </TransitionGroup>
    </div>
  </Teleport>
</template>

<script setup>
import { ref } from 'vue'
import { toasts, useToast } from '@/composables/useToast'

const { dismiss } = useToast()
const copied = ref(null)

function copy(t) {
  const text = [t.summary, t.detail].filter(Boolean).join('\n')
  navigator.clipboard.writeText(text).then(() => {
    copied.value = t.id
    setTimeout(() => { copied.value = null }, 1500)
  })
}

const icons = {
  success: '✅',
  error:   '❌',
  warn:    '⚠️',
  info:    'ℹ️',
}

const styles = {
  success: 'bg-green-950/90   border-green-700/60  text-green-100',
  error:   'bg-red-950/90     border-red-700/60    text-red-100',
  warn:    'bg-yellow-950/90  border-yellow-700/60 text-yellow-100',
  info:    'bg-surface-800/90 border-surface-600/60 text-tgray-100',
}
</script>
