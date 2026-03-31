<script setup lang="ts">
defineProps<{
  type: string
  size?: 'sm' | 'md'
}>()
</script>

<template>
  <img
    :src="`https://play.pokemonshowdown.com/sprites/types/${type}.png`"
    :alt="type"
    :class="size === 'sm' ? 'h-4' : 'h-5'"
    class="inline-block"
    loading="lazy"
  />
</template>
