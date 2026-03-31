<script setup lang="ts">
import { computed } from 'vue'
import type { Pokemon } from '@/types/pokemon'
import { usePokedex } from '@/composables/usePokedex'
import SpriteImage from './SpriteImage.vue'

const props = defineProps<{
  pokemon: Pokemon
}>()

const { pokemon: allPokemon, getById } = usePokedex()

interface ChainNode {
  id: number
  name: string
  spriteId: string
  evolutions: { method: string; param: string; node: ChainNode }[]
}

const chain = computed<ChainNode | null>(() => {
  if (!allPokemon.value.length) return null

  // Find the base form (the Pokemon that nothing evolves into, or this Pokemon's earliest ancestor)
  let baseId = props.pokemon.id
  let found = true
  while (found) {
    found = false
    for (const p of allPokemon.value) {
      for (const evo of p.evolutions) {
        if (evo.targetId === baseId) {
          baseId = p.id
          found = true
          break
        }
      }
      if (found) break
    }
  }

  function buildNode(id: number): ChainNode | null {
    const p = getById(id)
    if (!p) return null
    return {
      id: p.id,
      name: p.name,
      spriteId: p.spriteId,
      evolutions: p.evolutions
        .map(evo => {
          const node = buildNode(evo.targetId)
          if (!node) return null
          return { method: evo.method, param: evo.param, node }
        })
        .filter((e): e is NonNullable<typeof e> => e !== null),
    }
  }

  return buildNode(baseId)
})

function formatMethod(method: string, param: string): string {
  if (method === 'Level') return `Lv ${param}`
  if (method === 'Item') return param
  if (method === 'Trade') return param ? `Trade (${param})` : 'Trade'
  if (method === 'Friendship') return 'Friendship'
  if (method === 'Level Friendship Day') return 'Friendship (Day)'
  if (method === 'Level Friendship Night') return 'Friendship (Night)'
  return param ? `${method} (${param})` : method
}
</script>

<template>
  <div v-if="chain" class="flex flex-wrap items-center gap-2">
    <!-- Render the chain recursively -->
    <template v-for="node in [chain]" :key="node.id">
      <div class="flex flex-wrap items-center gap-2">
        <RouterLink
          :to="`/pokemon/${node.id}`"
          class="flex flex-col items-center gap-1 rounded-lg p-2 hover:bg-accent transition-colors"
          :class="{ 'ring-2 ring-primary': node.id === pokemon.id }"
        >
          <SpriteImage :sprite-id="node.spriteId" :name="node.name" size="sm" />
          <span class="text-xs font-medium">{{ node.name }}</span>
        </RouterLink>

        <template v-if="node.evolutions.length">
          <div v-for="evo in node.evolutions" :key="evo.node.id" class="flex flex-wrap items-center gap-2">
            <div class="flex flex-col items-center text-muted-foreground">
              <span class="text-lg">→</span>
              <span class="text-[10px] max-w-20 text-center leading-tight">{{ formatMethod(evo.method, evo.param) }}</span>
            </div>

            <div class="flex flex-wrap items-center gap-2">
              <RouterLink
                :to="`/pokemon/${evo.node.id}`"
                class="flex flex-col items-center gap-1 rounded-lg p-2 hover:bg-accent transition-colors"
                :class="{ 'ring-2 ring-primary': evo.node.id === pokemon.id }"
              >
                <SpriteImage :sprite-id="evo.node.spriteId" :name="evo.node.name" size="sm" />
                <span class="text-xs font-medium">{{ evo.node.name }}</span>
              </RouterLink>

              <!-- Second evolution -->
              <template v-if="evo.node.evolutions.length">
                <div v-for="evo2 in evo.node.evolutions" :key="evo2.node.id" class="flex flex-wrap items-center gap-2">
                  <div class="flex flex-col items-center text-muted-foreground">
                    <span class="text-lg">→</span>
                    <span class="text-[10px] max-w-20 text-center leading-tight">{{ formatMethod(evo2.method, evo2.param) }}</span>
                  </div>
                  <RouterLink
                    :to="`/pokemon/${evo2.node.id}`"
                    class="flex flex-col items-center gap-1 rounded-lg p-2 hover:bg-accent transition-colors"
                    :class="{ 'ring-2 ring-primary': evo2.node.id === pokemon.id }"
                  >
                    <SpriteImage :sprite-id="evo2.node.spriteId" :name="evo2.node.name" size="sm" />
                    <span class="text-xs font-medium">{{ evo2.node.name }}</span>
                  </RouterLink>
                </div>
              </template>
            </div>
          </div>
        </template>
      </div>
    </template>
  </div>
</template>
