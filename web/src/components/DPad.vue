<script setup lang="ts">
import type { MapConnections } from '@/types/pokemon'

defineProps<{
  connections: MapConnections
}>()

const emit = defineEmits<{
  (e: 'navigate', location: string): void
}>()
</script>

<template>
  <div class="inline-grid grid-cols-3 grid-rows-3 gap-1.5 w-fit">
    <!-- Row 1: up -->
    <div />
    <div class="flex flex-col items-center gap-0.5">
      <button
        v-for="loc in connections.up"
        :key="loc"
        class="flex items-center justify-center gap-1 rounded-md border bg-card px-3 py-1.5 text-xs font-medium transition-colors hover:bg-accent hover:text-accent-foreground disabled:opacity-30 disabled:pointer-events-none min-w-[100px]"
        @click="emit('navigate', loc)"
      >
        <svg xmlns="http://www.w3.org/2000/svg" width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round"><path d="m18 15-6-6-6 6"/></svg>
        <span class="truncate max-w-[120px]">{{ loc }}</span>
      </button>
      <div v-if="connections.up.length === 0" class="h-8" />
    </div>
    <div />

    <!-- Row 2: left, center, right -->
    <div class="flex items-center justify-end gap-0.5">
      <button
        v-for="loc in connections.left"
        :key="loc"
        class="flex items-center justify-center gap-1 rounded-md border bg-card px-3 py-1.5 text-xs font-medium transition-colors hover:bg-accent hover:text-accent-foreground disabled:opacity-30 disabled:pointer-events-none min-w-[100px]"
        @click="emit('navigate', loc)"
      >
        <svg xmlns="http://www.w3.org/2000/svg" width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round"><path d="m15 18-6-6 6-6"/></svg>
        <span class="truncate max-w-[120px]">{{ loc }}</span>
      </button>
      <div v-if="connections.left.length === 0" class="w-[100px]" />
    </div>

    <div class="flex items-center justify-center">
      <div class="w-10 h-10 rounded-full border-2 border-muted-foreground/20 flex items-center justify-center">
        <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="text-muted-foreground/40"><circle cx="12" cy="12" r="1"/></svg>
      </div>
    </div>

    <div class="flex items-center justify-start gap-0.5">
      <button
        v-for="loc in connections.right"
        :key="loc"
        class="flex items-center justify-center gap-1 rounded-md border bg-card px-3 py-1.5 text-xs font-medium transition-colors hover:bg-accent hover:text-accent-foreground disabled:opacity-30 disabled:pointer-events-none min-w-[100px]"
        @click="emit('navigate', loc)"
      >
        <span class="truncate max-w-[120px]">{{ loc }}</span>
        <svg xmlns="http://www.w3.org/2000/svg" width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round"><path d="m9 18 6-6-6-6"/></svg>
      </button>
      <div v-if="connections.right.length === 0" class="w-[100px]" />
    </div>

    <!-- Row 3: down -->
    <div />
    <div class="flex flex-col items-center gap-0.5">
      <button
        v-for="loc in connections.down"
        :key="loc"
        class="flex items-center justify-center gap-1 rounded-md border bg-card px-3 py-1.5 text-xs font-medium transition-colors hover:bg-accent hover:text-accent-foreground disabled:opacity-30 disabled:pointer-events-none min-w-[100px]"
        @click="emit('navigate', loc)"
      >
        <span class="truncate max-w-[120px]">{{ loc }}</span>
        <svg xmlns="http://www.w3.org/2000/svg" width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5" stroke-linecap="round" stroke-linejoin="round"><path d="m6 9 6 6 6-6"/></svg>
      </button>
      <div v-if="connections.down.length === 0" class="h-8" />
    </div>
    <div />
  </div>
</template>
