<script setup lang="ts">
import { ref, computed, onMounted } from 'vue'
import { RouterLink } from 'vue-router'
import { useEncounters } from '@/composables/useEncounters'
import { Card, CardContent } from '@/components/ui/card'
import { Input } from '@/components/ui/input'
import { Separator } from '@/components/ui/separator'

const { encounters, loaded, load, locationToSlug } = useEncounters()

const search = ref('')

const filtered = computed(() => {
  if (!search.value) return encounters.value
  const q = search.value.toLowerCase()
  return encounters.value.filter(e => e.location.toLowerCase().includes(q))
})

function methodSummary(pokemon: { method: string }[]): string[] {
  const methods = new Set(pokemon.map(p => p.method))
  return [...methods].sort()
}

function speciesCount(pokemon: { speciesId: number }[]): number {
  return new Set(pokemon.map(p => p.speciesId)).size
}

const methodIcons: Record<string, string> = {
  grass: '\uD83C\uDF3F',
  surfing: '\uD83C\uDFCA',
  fishing: '\uD83C\uDFA3',
  'rock smash': '\uD83E\uDEA8',
  hidden: '\uD83D\uDC41',
}

onMounted(() => { load() })
</script>

<template>
  <div class="space-y-6">
    <div>
      <h1 class="text-3xl font-bold tracking-tight">Encounters</h1>
      <p class="text-muted-foreground mt-1">
        Browse wild Pokemon encounters by location.
      </p>
    </div>

    <Input
      v-model="search"
      placeholder="Search locations..."
      class="sm:w-64"
    />

    <Separator />

    <p v-if="loaded" class="text-sm text-muted-foreground">
      Showing {{ filtered.length }} of {{ encounters.length }} locations
    </p>

    <div v-if="!loaded" class="flex items-center justify-center py-20">
      <div class="text-muted-foreground animate-pulse text-lg">Loading encounters...</div>
    </div>

    <div v-else class="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-3">
      <RouterLink
        v-for="loc in filtered"
        :key="loc.location"
        :to="{ name: 'encounter-route', params: { location: locationToSlug(loc.location) } }"
        class="group"
      >
        <Card class="h-full transition-all duration-150 group-hover:scale-[1.02] group-hover:border-primary/50 group-hover:shadow-lg">
          <CardContent class="p-4 space-y-2">
            <div class="flex items-baseline justify-between gap-2">
              <h3 class="font-semibold truncate">{{ loc.location }}</h3>
              <span class="text-xs text-muted-foreground whitespace-nowrap">{{ speciesCount(loc.pokemon) }} species</span>
            </div>
            <div class="flex flex-wrap gap-1.5">
              <span
                v-for="method in methodSummary(loc.pokemon)"
                :key="method"
                class="inline-flex items-center gap-1 rounded-full bg-muted px-2 py-0.5 text-xs text-muted-foreground"
              >
                <span>{{ methodIcons[method] || '' }}</span>
                {{ method }}
              </span>
            </div>
          </CardContent>
        </Card>
      </RouterLink>

      <div v-if="filtered.length === 0" class="col-span-full text-center py-12 text-muted-foreground">
        No locations match your search.
      </div>
    </div>
  </div>
</template>
