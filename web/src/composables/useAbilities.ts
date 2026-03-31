import { ref, computed, type Ref } from 'vue'
import type { Ability } from '@/types/pokemon'

const abilities: Ref<Ability[]> = ref([])
const loaded = ref(false)
const abilityMap = ref<Map<string, Ability>>(new Map())

export function useAbilities() {
  const search = ref('')

  async function load() {
    if (loaded.value) return
    const res = await fetch(import.meta.env.BASE_URL + 'data/abilities.json')
    abilities.value = await res.json()
    const map = new Map<string, Ability>()
    for (const a of abilities.value) {
      map.set(a.name.toLowerCase(), a)
    }
    abilityMap.value = map
    loaded.value = true
  }

  const filtered = computed(() => {
    if (!search.value) return abilities.value
    const q = search.value.toLowerCase()
    return abilities.value.filter(a => a.name.toLowerCase().includes(q))
  })

  function getAbility(name: string): Ability | undefined {
    return abilityMap.value.get(name.toLowerCase())
  }

  return { abilities, loaded, search, filtered, load, getAbility }
}
