import { ref, computed, type Ref } from 'vue'
import type { Move } from '@/types/pokemon'

const moves: Ref<Move[]> = ref([])
const loaded = ref(false)
const moveMap = ref<Map<string, Move>>(new Map())

export function useMoves() {
  const filters = ref({
    search: '',
    type: '',
    category: '',
  })

  async function load() {
    if (loaded.value) return
    const res = await fetch(import.meta.env.BASE_URL + 'data/moves.json')
    moves.value = await res.json()
    const map = new Map<string, Move>()
    for (const m of moves.value) {
      map.set(m.name.toLowerCase(), m)
    }
    moveMap.value = map
    loaded.value = true
  }

  const filtered = computed(() => {
    let result = moves.value
    const { search, type, category } = filters.value

    if (search) {
      const q = search.toLowerCase()
      result = result.filter(m => m.name.toLowerCase().includes(q))
    }
    if (type) {
      result = result.filter(m => m.type === type)
    }
    if (category) {
      result = result.filter(m => m.category === category)
    }

    return result
  })

  function getMove(name: string): Move | undefined {
    return moveMap.value.get(name.toLowerCase())
  }

  return { moves, loaded, filters, filtered, load, getMove }
}
