<template>
  <div
    v-if="pinList.length === 0"
    class="text-center text-gray-500 py-20"
  >
    No pins loaded yet.
  </div>

  <div
    v-else
    class="grid gap-3"
    :style="gridStyle"
  >
    <PinCard
      v-for="pin in pinList"
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
const { pinList } = pinStore

const gridStyle = computed(() => ({
  gridTemplateColumns: 'repeat(auto-fill, minmax(220px, 1fr))'
}))
</script>
