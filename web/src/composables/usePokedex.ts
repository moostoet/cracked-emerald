import { ref, computed, type Ref } from 'vue'
import type { Pokemon } from '@/types/pokemon'

const pokemon: Ref<Pokemon[]> = ref([])
const loaded = ref(false)
const filters = ref({
  search: '',
  type: '',
  generation: 0,
  ability: '',
})

export function usePokedex() {

  async function load() {
    if (loaded.value) return
    const res = await fetch(import.meta.env.BASE_URL + 'data/pokemon.json')
    pokemon.value = await res.json()
    loaded.value = true
  }

  const allTypes = computed(() => {
    const types = new Set<string>()
    for (const p of pokemon.value) {
      for (const t of p.types) types.add(t)
    }
    return [...types].sort()
  })

  const allGenerations = computed(() => {
    const gens = new Set<number>()
    for (const p of pokemon.value) gens.add(p.generation)
    return [...gens].sort((a, b) => a - b)
  })

  const allAbilities = computed(() => {
    const abilities = new Set<string>()
    for (const p of pokemon.value) {
      for (const a of p.abilities) {
        if (a) abilities.add(a)
      }
    }
    return [...abilities].sort()
  })

  const filtered = computed(() => {
    let result = pokemon.value
    const { search, type, generation, ability } = filters.value

    if (search) {
      const q = search.toLowerCase()
      result = result.filter(p => p.name.toLowerCase().includes(q))
    }
    if (type) {
      result = result.filter(p => p.types.includes(type))
    }
    if (generation) {
      result = result.filter(p => p.generation === generation)
    }
    if (ability) {
      result = result.filter(p => p.abilities.includes(ability))
    }

    return result
  })

  function getById(id: number): Pokemon | undefined {
    return pokemon.value.find(p => p.id === id)
  }

  function getByName(name: string): Pokemon | undefined {
    return pokemon.value.find(p => p.name.toLowerCase() === name.toLowerCase())
  }

  function getBySpriteId(spriteId: string): Pokemon | undefined {
    return pokemon.value.find(p => p.spriteId === spriteId)
  }

  return {
    pokemon,
    loaded,
    filters,
    filtered,
    allTypes,
    allGenerations,
    allAbilities,
    load,
    getById,
    getByName,
    getBySpriteId,
  }
}
