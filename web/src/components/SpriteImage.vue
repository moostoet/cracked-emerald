<script setup lang="ts">
import { ref } from 'vue'

const props = defineProps<{
  spriteId: string
  name: string
  size?: 'sm' | 'md' | 'lg'
}>()

const showdownFailed = ref(false)
const localFailed = ref(false)

const sizeClass = {
  sm: 'w-10 h-10',
  md: 'w-20 h-20',
  lg: 'w-32 h-32',
}

const showdownUrl = `https://play.pokemonshowdown.com/sprites/gen5ani/${props.spriteId}.gif`
const localUrl = `${import.meta.env.BASE_URL}sprites/${props.spriteId}.png`
</script>

<template>
  <img
    v-if="!showdownFailed"
    :src="showdownUrl"
    :alt="name"
    :class="[sizeClass[size ?? 'md'], 'object-contain image-rendering-pixelated']"
    loading="lazy"
    @error="showdownFailed = true"
  />
  <img
    v-else-if="!localFailed"
    :src="localUrl"
    :alt="name"
    :class="[sizeClass[size ?? 'md'], 'object-contain image-rendering-pixelated']"
    loading="lazy"
    @error="localFailed = true"
  />
  <div
    v-else
    :class="[sizeClass[size ?? 'md'], 'flex items-center justify-center rounded bg-muted text-muted-foreground text-xs']"
  >
    ?
  </div>
</template>
