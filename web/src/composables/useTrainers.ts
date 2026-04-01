import { ref, computed, type Ref } from 'vue'
import type { Trainer } from '@/types/pokemon'

const trainers: Ref<Trainer[]> = ref([])
const loaded = ref(false)

export function useTrainers() {
  const search = ref('')

  async function load() {
    if (loaded.value) return
    const res = await fetch(import.meta.env.BASE_URL + 'data/trainers.json')
    trainers.value = await res.json()
    loaded.value = true
  }

  const filtered = computed(() => {
    if (!search.value) return trainers.value
    const q = search.value.toLowerCase()
    return trainers.value.filter(t =>
      t.name.toLowerCase().includes(q)
      || t.trainerClass.toLowerCase().includes(q)
      || t.location.toLowerCase().includes(q)
      || t.party.some(m => m.species.toLowerCase().includes(q))
    )
  })

  function getTrainerById(id: string): Trainer | undefined {
    return trainers.value.find(t => t.id === id)
  }

  return {
    trainers,
    loaded,
    search,
    filtered,
    load,
    getTrainerById,
  }
}
