<script setup lang="ts">
import SpriteImage from './SpriteImage.vue'

interface ChainNode {
  id: number
  name: string
  spriteId: string
  evolutions: { method: string; param: string; node: ChainNode }[]
}

defineProps<{
  node: ChainNode
  currentId: number
  formatMethod: (method: string, param: string) => string
  showArrow?: boolean
  arrowMethod?: string
  arrowParam?: string
}>()
</script>

<template>
  <div class="flex items-start gap-1">
    <!-- Arrow from parent -->
    <div v-if="showArrow" class="flex items-center gap-1 self-center">
      <div class="flex flex-col items-center text-muted-foreground">
        <span class="text-lg">→</span>
        <span class="text-[10px] max-w-20 text-center leading-tight">{{ formatMethod(arrowMethod ?? '', arrowParam ?? '') }}</span>
      </div>
    </div>

    <div class="flex items-start gap-1">
      <!-- This node -->
      <RouterLink
        :to="`/pokemon/${node.id}`"
        class="flex flex-col items-center gap-1 rounded-lg p-2 hover:bg-accent transition-colors self-center"
        :class="{ 'ring-2 ring-primary': node.id === currentId }"
      >
        <SpriteImage :sprite-id="node.spriteId" :name="node.name" size="sm" />
        <span class="text-xs font-medium text-center">{{ node.name }}</span>
      </RouterLink>

      <!-- Children: stack vertically if branching -->
      <div
        v-if="node.evolutions.length"
        class="flex flex-col gap-1"
        :class="{ 'justify-center': node.evolutions.length === 1 }"
      >
        <EvolutionNode
          v-for="evo in node.evolutions"
          :key="evo.node.id"
          :node="evo.node"
          :current-id="currentId"
          :format-method="formatMethod"
          show-arrow
          :arrow-method="evo.method"
          :arrow-param="evo.param"
        />
      </div>
    </div>
  </div>
</template>
