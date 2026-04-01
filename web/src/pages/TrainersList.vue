<script setup lang="ts">
import { ref, computed, onMounted } from 'vue'
import { RouterLink } from 'vue-router'
import { useTrainers } from '@/composables/useTrainers'
import { usePokedex } from '@/composables/usePokedex'
import { useMoves } from '@/composables/useMoves'
import SpriteImage from '@/components/SpriteImage.vue'
import TypeBadge from '@/components/TypeBadge.vue'
import MovePopover from '@/components/MovePopover.vue'
import { Card, CardContent } from '@/components/ui/card'
import { Input } from '@/components/ui/input'
import { Badge } from '@/components/ui/badge'
import { Separator } from '@/components/ui/separator'
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select'
import type { Trainer, TrainerMon } from '@/types/pokemon'

const { loaded: trainersLoaded, search, filtered, load: loadTrainers, getTrainerById } = useTrainers()
const { loaded: dexLoaded, load: loadDex, getById } = usePokedex()
const { loaded: movesLoaded, load: loadMoves, getMove } = useMoves()

const loading = computed(() => !trainersLoaded.value || !dexLoaded.value || !movesLoaded.value)

const locationFilter = ref('')
const expandedIds = ref<Set<string>>(new Set())
const collapsedLocations = ref<Set<string>>(new Set())

// A display entry: either a single trainer or a merged forced double
interface DisplayEntry {
  key: string
  trainers: Trainer[]       // 1 for single/possible, 2 for forced double
  isForcedDouble: boolean
  possibleDoubleWith: string | null  // partner name for annotation
}

// Build display entries: merge forced doubles, annotate possible doubles
const displayEntries = computed(() => {
  const entries: DisplayEntry[] = []
  const merged = new Set<string>()  // trainer IDs already merged into a forced double

  for (const t of locationFiltered.value) {
    if (merged.has(t.id)) continue

    if (t.doubleWith?.forced) {
      const partner = getTrainerById(t.doubleWith.trainerId)
      if (partner && !merged.has(partner.id)) {
        merged.add(t.id)
        merged.add(partner.id)
        entries.push({
          key: `double-${t.id}-${partner.id}`,
          trainers: [t, partner],
          isForcedDouble: true,
          possibleDoubleWith: null,
        })
        continue
      }
    }

    let possiblePartnerName: string | null = null
    if (t.doubleWith && !t.doubleWith.forced) {
      const partner = getTrainerById(t.doubleWith.trainerId)
      if (partner) {
        possiblePartnerName = displayName(partner)
      }
    }

    entries.push({
      key: t.id,
      trainers: [t],
      isForcedDouble: false,
      possibleDoubleWith: possiblePartnerName,
    })
  }
  return entries
})

// All unique locations in BFS order
const allLocations = computed(() => {
  const seen = new Set<string>()
  const locs: string[] = []
  for (const t of filtered.value) {
    if (t.location && !seen.has(t.location)) {
      seen.add(t.location)
      locs.push(t.location)
    }
  }
  return locs
})

const locationFiltered = computed(() => {
  if (!locationFilter.value) return filtered.value
  return filtered.value.filter(t => t.location === locationFilter.value)
})

// Group display entries by location
const groupedByLocation = computed(() => {
  const groups: { location: string; entries: DisplayEntry[] }[] = []
  const map = new Map<string, DisplayEntry[]>()

  for (const entry of displayEntries.value) {
    const loc = entry.trainers[0].location || 'Unknown'
    if (!map.has(loc)) {
      const arr: DisplayEntry[] = []
      map.set(loc, arr)
      groups.push({ location: loc, entries: arr })
    }
    map.get(loc)!.push(entry)
  }
  return groups
})

function toggleLocation(loc: string) {
  if (collapsedLocations.value.has(loc)) {
    collapsedLocations.value.delete(loc)
  } else {
    collapsedLocations.value.add(loc)
  }
}

function toggleExpand(key: string) {
  if (expandedIds.value.has(key)) {
    expandedIds.value.delete(key)
  } else {
    expandedIds.value.add(key)
  }
}

function getSpriteId(speciesId: number): string {
  return getById(speciesId)?.spriteId ?? ''
}

function getMoveType(moveName: string): string {
  return getMove(moveName)?.type ?? ''
}

function getMoveCategory(moveName: string): string {
  return getMove(moveName)?.category ?? ''
}

function categoryIconUrl(category: string): string {
  if (!category) return ''
  return `https://play.pokemonshowdown.com/sprites/categories/${category}.png`
}

function displayName(trainer: Trainer): string {
  if (trainer.trainerClass) return `${trainer.trainerClass} ${trainer.name}`
  return trainer.name
}

function entryMaxLevel(entry: DisplayEntry): number {
  return Math.max(...entry.trainers.flatMap(t => t.party.map(m => m.level)))
}

function entryParty(entry: DisplayEntry): TrainerMon[] {
  return entry.trainers.flatMap(t => t.party)
}

function onLocationChange(value: string) {
  locationFilter.value = value === '__all__' ? '' : value
}

onMounted(async () => {
  await Promise.all([loadTrainers(), loadDex(), loadMoves()])
})
</script>

<template>
  <div class="max-w-5xl mx-auto space-y-6">
    <div>
      <h1 class="text-3xl font-bold tracking-tight">Trainers</h1>
      <p class="text-muted-foreground mt-1">
        Browse all trainer battles in approximate game order.
      </p>
    </div>

    <div class="flex flex-col gap-3 sm:flex-row sm:flex-wrap sm:items-center">
      <Input
        v-model="search"
        placeholder="Search trainers or Pokemon..."
        class="sm:w-64"
      />
      <Select :model-value="locationFilter || '__all__'" @update:model-value="onLocationChange">
        <SelectTrigger class="w-full sm:w-52">
          <SelectValue placeholder="All Locations" />
        </SelectTrigger>
        <SelectContent>
          <SelectItem value="__all__">All Locations</SelectItem>
          <SelectItem v-for="loc in allLocations" :key="loc" :value="loc">
            {{ loc }}
          </SelectItem>
        </SelectContent>
      </Select>
    </div>

    <Separator />

    <p v-if="!loading" class="text-sm text-muted-foreground">
      Showing {{ displayEntries.length }} battles across {{ groupedByLocation.length }} locations
    </p>

    <div v-if="loading" class="flex items-center justify-center py-20">
      <div class="text-muted-foreground animate-pulse text-lg">Loading trainers...</div>
    </div>

    <div v-else class="space-y-6">
      <div v-if="groupedByLocation.length === 0" class="text-center py-12 text-muted-foreground">
        No trainers match your search.
      </div>

      <!-- Location groups -->
      <div v-for="group in groupedByLocation" :key="group.location" class="space-y-2">
        <button
          class="flex items-center gap-2 w-full text-left group"
          @click="toggleLocation(group.location)"
        >
          <svg xmlns="http://www.w3.org/2000/svg" width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="text-muted-foreground shrink-0 transition-transform" :class="{ '-rotate-90': collapsedLocations.has(group.location) }"><path d="m6 9 6 6 6-6"/></svg>
          <h2 class="text-lg font-semibold group-hover:text-primary transition-colors">{{ group.location }}</h2>
          <span class="text-xs text-muted-foreground">({{ group.entries.length }})</span>
        </button>

        <div v-if="!collapsedLocations.has(group.location)" class="space-y-2 pl-1">
          <Card
            v-for="entry in group.entries"
            :key="entry.key"
            class="transition-colors hover:border-primary/30 cursor-pointer"
            :class="{ 'border-amber-500/40': entry.isForcedDouble }"
            @click="toggleExpand(entry.key)"
          >
            <CardContent class="p-3">
              <!-- Summary row -->
              <div class="flex items-center gap-3">
                <!-- Trainer sprite(s) -->
                <div class="flex shrink-0" :class="entry.isForcedDouble ? '-space-x-2' : ''">
                  <template v-for="trainer in entry.trainers" :key="trainer.id">
                    <img
                      v-if="trainer.sprite"
                      :src="trainer.sprite"
                      :alt="displayName(trainer)"
                      class="w-16 h-16 object-contain image-rendering-pixelated"
                      loading="lazy"
                    />
                    <div v-else class="w-16 h-16 rounded bg-muted flex items-center justify-center text-muted-foreground text-xs">?</div>
                  </template>
                </div>

                <!-- Name + details -->
                <div class="flex-1 min-w-0">
                  <div class="flex items-baseline gap-2 flex-wrap">
                    <span v-if="entry.isForcedDouble" class="font-semibold text-sm">
                      {{ displayName(entry.trainers[0]) }} &amp; {{ displayName(entry.trainers[1]) }}
                    </span>
                    <span v-else class="font-semibold text-sm">{{ displayName(entry.trainers[0]) }}</span>
                    <Badge v-if="entry.isForcedDouble" variant="outline" class="text-[10px] border-amber-500/60 text-amber-500">Forced Double</Badge>
                    <Badge v-else-if="entry.trainers[0].isDouble" variant="outline" class="text-[10px]">Double</Badge>
                  </div>
                  <div class="text-xs text-muted-foreground mt-0.5">
                    <span class="font-mono">Lv{{ entryMaxLevel(entry) }}</span>
                    <span v-if="entry.possibleDoubleWith" class="ml-2">
                      · Can double with {{ entry.possibleDoubleWith }}
                    </span>
                  </div>
                </div>

                <!-- Pokemon lineup -->
                <div class="flex items-center gap-0.5 shrink-0">
                  <RouterLink
                    v-for="mon in entryParty(entry)"
                    :key="mon.speciesId + mon.species"
                    :to="{ name: 'pokemon-detail', params: { id: mon.speciesId } }"
                    @click.stop
                  >
                    <SpriteImage :sprite-id="getSpriteId(mon.speciesId)" :name="mon.species" size="sm" />
                  </RouterLink>
                </div>

                <!-- Expand chevron -->
                <svg xmlns="http://www.w3.org/2000/svg" width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="text-muted-foreground shrink-0 transition-transform" :class="{ 'rotate-180': expandedIds.has(entry.key) }"><path d="m6 9 6 6 6-6"/></svg>
              </div>

              <!-- Expanded party details -->
              <div v-if="expandedIds.has(entry.key)" class="mt-4" @click.stop>
                <!-- Show trainer name headers for forced doubles -->
                <template v-for="trainer in entry.trainers" :key="trainer.id">
                  <div v-if="entry.isForcedDouble" class="text-xs font-semibold text-muted-foreground mb-2 mt-3 first:mt-0">
                    {{ displayName(trainer) }}
                  </div>
                  <div class="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-3 mb-3">
                    <div
                      v-for="(mon, idx) in trainer.party"
                      :key="idx"
                      class="rounded-lg border bg-muted/30 p-3 space-y-2"
                    >
                      <div class="flex items-center gap-2">
                        <RouterLink :to="{ name: 'pokemon-detail', params: { id: mon.speciesId } }">
                          <SpriteImage :sprite-id="getSpriteId(mon.speciesId)" :name="mon.species" size="sm" />
                        </RouterLink>
                        <div class="min-w-0">
                          <RouterLink
                            :to="{ name: 'pokemon-detail', params: { id: mon.speciesId } }"
                            class="font-semibold text-sm hover:underline hover:text-primary transition-colors block truncate"
                          >
                            {{ mon.species }}
                          </RouterLink>
                          <span class="text-xs font-mono text-muted-foreground">Lv{{ mon.level }}</span>
                        </div>
                      </div>

                      <div class="text-xs space-y-0.5 text-muted-foreground">
                        <div v-if="mon.ability"><span class="text-foreground">Ability:</span> {{ mon.ability }}</div>
                        <div><span class="text-foreground">Item:</span> {{ mon.item || '\u2014' }}</div>
                        <div v-if="mon.nature"><span class="text-foreground">Nature:</span> {{ mon.nature }}</div>
                        <div v-if="mon.teraType"><span class="text-foreground">Tera:</span> {{ mon.teraType }}</div>
                      </div>

                      <div v-if="mon.moves.length > 0" class="grid grid-cols-2 gap-1.5">
                        <div
                          v-for="move in mon.moves"
                          :key="move"
                          class="rounded-md border bg-background px-2 py-1.5"
                        >
                          <div class="text-xs font-medium truncate">
                            <MovePopover :move-name="move" />
                          </div>
                          <div class="flex items-center gap-1.5 mt-1">
                            <TypeBadge v-if="getMoveType(move)" :type="getMoveType(move)" size="sm" />
                            <img
                              v-if="getMoveCategory(move)"
                              :src="categoryIconUrl(getMoveCategory(move))"
                              :alt="getMoveCategory(move)"
                              class="h-3.5"
                              loading="lazy"
                            />
                          </div>
                        </div>
                      </div>
                    </div>
                  </div>
                </template>
              </div>
            </CardContent>
          </Card>
        </div>
      </div>
    </div>
  </div>
</template>
