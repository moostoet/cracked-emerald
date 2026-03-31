<script setup lang="ts">
import { computed } from 'vue'
import { useMoves } from '@/composables/useMoves'
import TypeBadge from './TypeBadge.vue'
import { Popover, PopoverContent, PopoverTrigger } from '@/components/ui/popover'

const props = defineProps<{
  moveName: string
}>()

const { getMove } = useMoves()
const move = computed(() => getMove(props.moveName))
</script>

<template>
  <Popover>
    <PopoverTrigger as-child>
      <button class="text-left hover:text-primary underline decoration-dotted underline-offset-2 transition-colors">
        {{ moveName }}
      </button>
    </PopoverTrigger>
    <PopoverContent class="w-72" v-if="move">
      <div class="space-y-2">
        <div class="flex items-center justify-between">
          <span class="font-semibold">{{ move.name }}</span>
          <TypeBadge :type="move.type" size="sm" />
        </div>
        <div class="grid grid-cols-3 gap-2 text-xs text-muted-foreground">
          <div>
            <span class="block font-medium text-foreground">{{ move.category }}</span>
            Category
          </div>
          <div>
            <span class="block font-medium text-foreground">{{ move.power || '---' }}</span>
            Power
          </div>
          <div>
            <span class="block font-medium text-foreground">{{ move.accuracy || '---' }}{{ move.accuracy ? '%' : '' }}</span>
            Accuracy
          </div>
        </div>
        <div class="grid grid-cols-2 gap-2 text-xs text-muted-foreground">
          <div>
            <span class="block font-medium text-foreground">{{ move.pp }}</span>
            PP
          </div>
          <div>
            <span class="block font-medium text-foreground">{{ move.priority >= 0 ? '+' : '' }}{{ move.priority }}</span>
            Priority
          </div>
        </div>
        <p class="text-xs text-muted-foreground border-t border-border pt-2">
          {{ move.description }}
        </p>
      </div>
    </PopoverContent>
  </Popover>
</template>
