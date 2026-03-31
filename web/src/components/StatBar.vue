<script setup lang="ts">
import { statColors, statLabels } from '@/lib/typeColors'

defineProps<{
  stat: string
  value: number
}>()

const maxStat = 255
</script>

<template>
  <div class="flex items-center gap-2 text-sm">
    <span class="w-8 text-right font-medium text-muted-foreground">{{ statLabels[stat] ?? stat }}</span>
    <span class="w-8 text-right font-mono">{{ value }}</span>
    <div class="flex-1 h-3 rounded-full bg-muted overflow-hidden">
      <div
        :class="[statColors[stat] ?? 'bg-primary', 'h-full rounded-full transition-all duration-500']"
        :style="{ width: `${Math.min((value / maxStat) * 100, 100)}%` }"
      />
    </div>
  </div>
</template>
