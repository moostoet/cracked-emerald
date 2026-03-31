<script setup lang="ts">
import { ref } from 'vue'

const props = defineProps<{
  spriteId: string
  name: string
  size?: 'sm' | 'md' | 'lg'
}>()

const fallbackStage = ref(0) // 0=animated, 1=static, 2=local, 3=placeholder

const sizeClass = {
  sm: 'w-10 h-10',
  md: 'w-20 h-20',
  lg: 'w-32 h-32',
}

const showdownAnimatedUrl = `https://play.pokemonshowdown.com/sprites/gen5ani/${props.spriteId}.gif`
const showdownStaticUrl = `https://play.pokemonshowdown.com/sprites/gen5/${props.spriteId}.png`
const localUrl = `${import.meta.env.BASE_URL}sprites/${props.spriteId}.png`
</script>

<template>
  <img
    v-if="fallbackStage === 0"
    :src="showdownAnimatedUrl"
    :alt="name"
    :class="[sizeClass[size ?? 'md'], 'object-contain image-rendering-pixelated']"
    loading="lazy"
    @error="fallbackStage = 1"
  />
  <img
    v-else-if="fallbackStage === 1"
    :src="showdownStaticUrl"
    :alt="name"
    :class="[sizeClass[size ?? 'md'], 'object-contain image-rendering-pixelated']"
    loading="lazy"
    @error="fallbackStage = 2"
  />
  <img
    v-else-if="fallbackStage === 2"
    :src="localUrl"
    :alt="name"
    :class="[sizeClass[size ?? 'md'], 'object-contain image-rendering-pixelated']"
    loading="lazy"
    @error="fallbackStage = 3"
  />
  <div
    v-else
    :class="[sizeClass[size ?? 'md'], 'flex items-center justify-center rounded bg-muted text-muted-foreground text-xs']"
  >
    ?
  </div>
</template>
