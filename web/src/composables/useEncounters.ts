import { ref, type Ref } from 'vue'
import type { EncounterLocation, MapConnections } from '@/types/pokemon'

const encounters: Ref<EncounterLocation[]> = ref([])
const connections: Ref<Record<string, MapConnections>> = ref({})
const loaded = ref(false)

export function useEncounters() {
  async function load() {
    if (loaded.value) return
    const [encRes, connRes] = await Promise.all([
      fetch(import.meta.env.BASE_URL + 'data/encounters.json'),
      fetch(import.meta.env.BASE_URL + 'data/connections.json'),
    ])
    encounters.value = await encRes.json()
    connections.value = await connRes.json()
    loaded.value = true
  }

  function getLocation(name: string): EncounterLocation | undefined {
    return encounters.value.find(e => e.location === name)
  }

  function getConnections(name: string): MapConnections | undefined {
    return connections.value[name]
  }

  function locationToSlug(name: string): string {
    return name.toLowerCase().replace(/ /g, '-')
  }

  function slugToLocation(slug: string): string | undefined {
    return encounters.value.find(
      e => e.location.toLowerCase().replace(/ /g, '-') === slug
    )?.location
  }

  function allLocationSlugs(): Map<string, string> {
    // Build slug -> name map for all known locations (encounters + connections)
    const slugs = new Map<string, string>()
    for (const e of encounters.value) {
      slugs.set(locationToSlug(e.location), e.location)
    }
    // Also include connected locations that may not have encounters
    for (const name of Object.keys(connections.value)) {
      const slug = locationToSlug(name)
      if (!slugs.has(slug)) {
        slugs.set(slug, name)
      }
    }
    return slugs
  }

  return {
    encounters,
    connections,
    loaded,
    load,
    getLocation,
    getConnections,
    locationToSlug,
    slugToLocation,
    allLocationSlugs,
  }
}
