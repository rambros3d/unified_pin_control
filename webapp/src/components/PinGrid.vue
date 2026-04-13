<template>
  <!-- No pins configured yet -->
  <div
    v-if="activePins.length === 0"
    class="flex flex-col items-center justify-center py-24 gap-4 text-center"
  >
    <div class="text-5xl opacity-30">📌</div>
    <p class="text-tgray-500 text-sm">
      No pins configured.<br />
      Use <span class="text-twhite font-medium">+ Add Pin</span> to get started.
    </p>
  </div>

  <!-- Active pin cards -->
  <div
    v-else
    class="grid gap-3"
    :style="{ gridTemplateColumns: 'repeat(auto-fill, minmax(220px, 1fr))' }"
  >
    <PinCard
      v-for="pin in activePins"
      :key="pin.name"
      :pin="pin"
    />
  </div>
</template>

<script setup>
import { computed } from 'vue'
import { usePinStore } from '@/stores/pinStore'
import PinCard from '@/components/PinCard.vue'

const pinStore = usePinStore()

// Only render pins that have been configured (mode is not null/empty)
const activePins = computed(() =>
  pinStore.pinList.filter(p => p.mode && p.mode !== 'DISABLED')
)
</script>
