<script setup lang="ts">
import { computed } from 'vue'
import { useAbilities } from '@/composables/useAbilities'
import { Popover, PopoverContent, PopoverTrigger } from '@/components/ui/popover'

const props = defineProps<{
  abilityName: string
  isHidden?: boolean
}>()

const { getAbility } = useAbilities()
const ability = computed(() => getAbility(props.abilityName))
</script>

<template>
  <Popover>
    <PopoverTrigger as-child>
      <button
        :class="[
          'text-left hover:text-primary underline decoration-dotted underline-offset-2 transition-colors',
          isHidden ? 'italic text-muted-foreground' : ''
        ]"
      >
        {{ abilityName }}
        <span v-if="isHidden" class="text-[10px] ml-1">(HA)</span>
      </button>
    </PopoverTrigger>
    <PopoverContent class="w-64" v-if="ability">
      <div class="space-y-1">
        <span class="font-semibold text-sm">{{ ability.name }}</span>
        <p class="text-xs text-muted-foreground">{{ ability.description }}</p>
      </div>
    </PopoverContent>
  </Popover>
</template>
