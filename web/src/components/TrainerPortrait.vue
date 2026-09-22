<script setup lang="ts">
import { computed, ref, watch } from 'vue'

const props = defineProps<{
  src: string
  name: string
}>()

const failed = ref(false)
const portraitUrl = computed(() => {
  if (!props.src) return ''
  // A cached trainers.json may still contain an external portrait URL.
  if (/^https?:\/\//.test(props.src)) return props.src
  return `${import.meta.env.BASE_URL}${props.src}`
})

watch(() => props.src, () => { failed.value = false })
</script>

<template>
  <img
    v-if="portraitUrl && !failed"
    :src="portraitUrl"
    :alt="name"
    class="w-16 h-16 shrink-0 object-contain image-rendering-pixelated"
    loading="lazy"
    @error="failed = true"
  />
  <div
    v-else
    role="img"
    :aria-label="`${name} portrait unavailable`"
    class="w-16 h-16 shrink-0 rounded bg-muted flex items-center justify-center text-muted-foreground text-xs"
  >
    <span aria-hidden="true">?</span>
  </div>
</template>
