<script setup lang="ts">
import { typeColors } from '@/lib/typeColors'

defineProps<{
  type: string
  size?: 'sm' | 'md'
}>()
</script>

<template>
  <span
    :class="[
      typeColors[type] ?? 'bg-gray-500',
      size === 'sm' ? 'px-1.5 py-0.5 text-[10px]' : 'px-2.5 py-0.5 text-xs',
      'inline-flex items-center rounded font-semibold text-white shadow-sm'
    ]"
  >
    {{ type }}
  </span>
</template>
