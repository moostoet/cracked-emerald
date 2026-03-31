import { ref, type Ref } from 'vue'
import type { TypeChart } from '@/types/pokemon'

const typeChart: Ref<TypeChart | null> = ref(null)
const loaded = ref(false)

export function useTypes() {
  async function load() {
    if (loaded.value) return
    const res = await fetch(import.meta.env.BASE_URL + 'data/types.json')
    typeChart.value = await res.json()
    loaded.value = true
  }

  function getEffectiveness(attackingType: string, defendingTypes: string[]): number {
    if (!typeChart.value) return 1
    const { types, effectiveness } = typeChart.value
    const atkIdx = types.indexOf(attackingType)
    if (atkIdx === -1) return 1

    let multiplier = 1
    for (const defType of defendingTypes) {
      const defIdx = types.indexOf(defType)
      if (defIdx !== -1) {
        multiplier *= effectiveness[atkIdx][defIdx]
      }
    }
    return multiplier
  }

  return { typeChart, loaded, load, getEffectiveness }
}
