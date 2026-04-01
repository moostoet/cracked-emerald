<script setup lang="ts">
import { computed, onMounted } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { useEncounters } from '@/composables/useEncounters'
import { usePokedex } from '@/composables/usePokedex'
import DPad from '@/components/DPad.vue'
import SpriteImage from '@/components/SpriteImage.vue'
import TypeBadge from '@/components/TypeBadge.vue'
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card'
import { Tabs, TabsContent, TabsList, TabsTrigger } from '@/components/ui/tabs'
import { Table, TableBody, TableCell, TableHead, TableHeader, TableRow } from '@/components/ui/table'
import { Button } from '@/components/ui/button'

const route = useRoute()
const router = useRouter()
const {
  loaded: encLoaded, load: loadEnc,
  getLocation, getConnections, locationToSlug, allLocationSlugs,
} = useEncounters()
const { loaded: dexLoaded, load: loadDex, getById } = usePokedex()

const loading = computed(() => !encLoaded.value || !dexLoaded.value)

const locationName = computed(() => {
  const slug = route.params.location as string
  const slugMap = allLocationSlugs()
  return slugMap.get(slug) ?? ''
})

const encounterData = computed(() => {
  if (!locationName.value) return null
  return getLocation(locationName.value)
})

const connections = computed(() => {
  if (!locationName.value) return null
  return getConnections(locationName.value)
})

const hasDirectionalConnections = computed(() => {
  if (!connections.value) return false
  const c = connections.value
  return c.up.length > 0 || c.down.length > 0 || c.left.length > 0 || c.right.length > 0
})

// Filter warps to only show locations that have encounters (to reduce noise from houses/shops)
const relevantWarps = computed(() => {
  if (!connections.value) return []
  return connections.value.warps.filter(w => !!getLocation(w) || !!getConnections(w))
})

// Group encounter pokemon by method
const methods = computed(() => {
  if (!encounterData.value) return []
  const grouped = new Map<string, typeof encounterData.value.pokemon>()
  for (const p of encounterData.value.pokemon) {
    if (!grouped.has(p.method)) grouped.set(p.method, [])
    grouped.get(p.method)!.push(p)
  }
  // Sort methods in a logical order
  const order = ['grass', 'surfing', 'fishing', 'rock smash', 'hidden', 'gift']
  return order
    .filter(m => grouped.has(m))
    .map(m => ({
      method: m,
      pokemon: [...grouped.get(m)!].sort((a, b) => b.percentage - a.percentage),
    }))
})

const methodLabels: Record<string, string> = {
  grass: 'Grass',
  surfing: 'Surfing',
  fishing: 'Fishing',
  'rock smash': 'Rock Smash',
  hidden: 'Hidden',
  gift: 'Gift',
}

function getSpriteId(speciesId: number): string {
  const mon = getById(speciesId)
  return mon?.spriteId ?? ''
}

function getSpeciesTypes(speciesId: number): string[] {
  const mon = getById(speciesId)
  return mon?.types ?? []
}

function navigate(location: string) {
  router.push({
    name: 'encounter-route',
    params: { location: locationToSlug(location) },
  })
}

onMounted(async () => {
  await Promise.all([loadEnc(), loadDex()])
})
</script>

<template>
  <div class="max-w-5xl mx-auto space-y-6">
    <!-- Loading -->
    <div v-if="loading" class="flex items-center justify-center min-h-[50vh]">
      <div class="text-center space-y-3">
        <div class="animate-spin h-8 w-8 border-4 border-primary border-t-transparent rounded-full mx-auto" />
        <p class="text-muted-foreground">Loading...</p>
      </div>
    </div>

    <!-- Not found -->
    <div v-else-if="!locationName" class="flex items-center justify-center min-h-[50vh]">
      <div class="text-center space-y-3">
        <p class="text-2xl font-bold">Location not found</p>
        <Button @click="router.push({ name: 'encounters-list' })">Back to Encounters</Button>
      </div>
    </div>

    <template v-else>
      <!-- Back link -->
      <RouterLink
        :to="{ name: 'encounters-list' }"
        class="text-sm text-muted-foreground hover:text-foreground transition-colors"
      >
        &larr; All Locations
      </RouterLink>

      <!-- Header -->
      <h1 class="text-3xl font-bold tracking-tight">{{ locationName }}</h1>

      <!-- D-Pad Navigation -->
      <Card v-if="hasDirectionalConnections || relevantWarps.length > 0">
        <CardHeader>
          <CardTitle class="text-base">Connected Areas</CardTitle>
        </CardHeader>
        <CardContent class="flex flex-col items-center gap-4">
          <DPad
            v-if="hasDirectionalConnections && connections"
            :connections="connections"
            @navigate="navigate"
          />
          <!-- Warp connections -->
          <div v-if="relevantWarps.length > 0" class="flex flex-wrap items-center justify-center gap-2">
            <span v-if="hasDirectionalConnections" class="text-xs text-muted-foreground mr-1">Also:</span>
            <button
              v-for="warp in relevantWarps"
              :key="warp"
              class="rounded-md border bg-card px-3 py-1.5 text-xs font-medium transition-colors hover:bg-accent hover:text-accent-foreground"
              @click="navigate(warp)"
            >
              {{ warp }}
            </button>
          </div>
        </CardContent>
      </Card>

      <!-- Encounters -->
      <div v-if="encounterData && methods.length > 0">
        <Tabs :default-value="methods[0].method">
          <TabsList class="w-full flex flex-wrap h-auto">
            <TabsTrigger v-for="m in methods" :key="m.method" :value="m.method">
              {{ methodLabels[m.method] || m.method }}
            </TabsTrigger>
          </TabsList>

          <TabsContent v-for="m in methods" :key="m.method" :value="m.method">
            <Card>
              <CardContent class="pt-6">
                <div class="overflow-x-auto">
                  <Table>
                    <TableHeader>
                      <TableRow>
                        <TableHead class="w-14">Sprite</TableHead>
                        <TableHead>Pokemon</TableHead>
                        <TableHead class="w-32">Types</TableHead>
                        <TableHead class="w-28 text-right">Level</TableHead>
                        <TableHead v-if="m.method !== 'gift'" class="w-16 text-right">Rate</TableHead>
                      </TableRow>
                    </TableHeader>
                    <TableBody>
                      <TableRow v-for="p in m.pokemon" :key="p.speciesId">
                        <TableCell>
                          <RouterLink :to="{ name: 'pokemon-detail', params: { id: p.speciesId } }">
                            <SpriteImage :sprite-id="getSpriteId(p.speciesId)" :name="p.species" size="sm" />
                          </RouterLink>
                        </TableCell>
                        <TableCell class="font-medium">
                          <RouterLink
                            :to="{ name: 'pokemon-detail', params: { id: p.speciesId } }"
                            class="hover:underline hover:text-primary transition-colors"
                          >
                            {{ p.species }}
                          </RouterLink>
                        </TableCell>
                        <TableCell>
                          <div class="flex gap-1">
                            <TypeBadge v-for="t in getSpeciesTypes(p.speciesId)" :key="t" :type="t" size="sm" />
                          </div>
                        </TableCell>
                        <TableCell class="text-right font-mono text-sm">
                          {{ p.minLevel === 0 && p.maxLevel === 0 ? 'Egg' : p.minLevel === p.maxLevel ? p.minLevel : `${p.minLevel}\u2013${p.maxLevel}` }}
                        </TableCell>
                        <TableCell v-if="m.method !== 'gift'" class="text-right font-mono text-sm">
                          {{ p.percentage }}%
                        </TableCell>
                      </TableRow>
                    </TableBody>
                  </Table>
                </div>
              </CardContent>
            </Card>
          </TabsContent>
        </Tabs>
      </div>

      <!-- No encounters state -->
      <Card v-else-if="!encounterData || methods.length === 0">
        <CardContent class="py-12 text-center text-muted-foreground">
          No wild Pokemon found at this location.
        </CardContent>
      </Card>
    </template>
  </div>
</template>
