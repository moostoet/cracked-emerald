<script setup lang="ts">
import { computed } from 'vue'
import type { Pokemon } from '@/types/pokemon'
import { usePokedex } from '@/composables/usePokedex'
import EvolutionNode from './EvolutionNode.vue'

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

  // Find the base form by walking backwards through evolutions
  let baseId = props.pokemon.id
  let found = true
  const visited = new Set<number>()
  while (found) {
    found = false
    if (visited.has(baseId)) break
    visited.add(baseId)
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

  const builtIds = new Set<number>()
  function buildNode(id: number): ChainNode | null {
    if (builtIds.has(id)) return null
    builtIds.add(id)
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
  <EvolutionNode
    v-if="chain"
    :node="chain"
    :current-id="pokemon.id"
    :format-method="formatMethod"
  />
</template>
