<script setup lang="ts">
import { type HTMLAttributes, computed } from 'vue'
import { cn } from '@/lib/utils'

const props = defineProps<{
  modelValue?: string | number
  class?: HTMLAttributes['class']
}>()

const emits = defineEmits<{
  (e: 'update:modelValue', payload: string | number): void
}>()

function onInput(event: Event) {
  emits('update:modelValue', (event.target as HTMLInputElement).value)
}

const delegatedProps = computed(() => {
  const { class: _, modelValue: _mv, ...delegated } = props
  return delegated
})
</script>

<template>
  <input
    :value="modelValue"
    v-bind="delegatedProps"
    :class="cn('flex h-10 w-full rounded-md border border-input bg-background px-3 py-2 text-sm ring-offset-background file:border-0 file:bg-transparent file:text-sm file:font-medium placeholder:text-muted-foreground focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring focus-visible:ring-offset-2 disabled:cursor-not-allowed disabled:opacity-50', props.class)"
    @input="onInput"
  />
</template>
