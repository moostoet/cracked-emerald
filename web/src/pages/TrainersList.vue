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
import type { Trainer } from '@/types/pokemon'

const { loaded: trainersLoaded, search, filtered, load: loadTrainers } = useTrainers()
const { loaded: dexLoaded, load: loadDex, getById } = usePokedex()
const { loaded: movesLoaded, load: loadMoves, getMove } = useMoves()

const loading = computed(() => !trainersLoaded.value || !dexLoaded.value || !movesLoaded.value)

const locationFilter = ref('')

const expandedIds = ref<Set<string>>(new Set())

// All unique locations in BFS order (preserved from the sorted trainer list)
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

// Trainers filtered by location
const locationFiltered = computed(() => {
  if (!locationFilter.value) return filtered.value
  return filtered.value.filter(t => t.location === locationFilter.value)
})

// Group trainers by location (preserving order)
const groupedByLocation = computed(() => {
  const groups: { location: string; trainers: Trainer[] }[] = []
  const map = new Map<string, Trainer[]>()

  for (const t of locationFiltered.value) {
    const loc = t.location || 'Unknown'
    if (!map.has(loc)) {
      const arr: Trainer[] = []
      map.set(loc, arr)
      groups.push({ location: loc, trainers: arr })
    }
    map.get(loc)!.push(t)
  }
  return groups
})

const collapsedLocations = ref<Set<string>>(new Set())

function toggleLocation(loc: string) {
  if (collapsedLocations.value.has(loc)) {
    collapsedLocations.value.delete(loc)
  } else {
    collapsedLocations.value.add(loc)
  }
}

function toggleExpand(trainerId: string) {
  if (expandedIds.value.has(trainerId)) {
    expandedIds.value.delete(trainerId)
  } else {
    expandedIds.value.add(trainerId)
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

function maxLevel(trainer: Trainer): number {
  return Math.max(...trainer.party.map(m => m.level))
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
      Showing {{ locationFiltered.length }} trainers across {{ groupedByLocation.length }} locations
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
        <!-- Location header -->
        <button
          class="flex items-center gap-2 w-full text-left group"
          @click="toggleLocation(group.location)"
        >
          <svg
            xmlns="http://www.w3.org/2000/svg"
            width="14"
            height="14"
            viewBox="0 0 24 24"
            fill="none"
            stroke="currentColor"
            stroke-width="2"
            stroke-linecap="round"
            stroke-linejoin="round"
            class="text-muted-foreground shrink-0 transition-transform"
            :class="{ '-rotate-90': collapsedLocations.has(group.location) }"
          >
            <path d="m6 9 6 6 6-6"/>
          </svg>
          <h2 class="text-lg font-semibold group-hover:text-primary transition-colors">{{ group.location }}</h2>
          <span class="text-xs text-muted-foreground">({{ group.trainers.length }})</span>
        </button>

        <!-- Trainer cards -->
        <div v-if="!collapsedLocations.has(group.location)" class="space-y-2 pl-1">
          <Card
            v-for="trainer in group.trainers"
            :key="trainer.id"
            class="transition-colors hover:border-primary/30 cursor-pointer"
            @click="toggleExpand(trainer.id)"
          >
            <CardContent class="p-3">
              <!-- Summary row -->
              <div class="flex items-center gap-3">
                <!-- Trainer sprite -->
                <img
                  v-if="trainer.sprite"
                  :src="trainer.sprite"
                  :alt="displayName(trainer)"
                  class="w-16 h-16 object-contain image-rendering-pixelated shrink-0"
                  loading="lazy"
                />
                <div v-else class="w-16 h-16 rounded bg-muted flex items-center justify-center text-muted-foreground text-xs shrink-0">?</div>

                <!-- Name + details -->
                <div class="flex-1 min-w-0">
                  <div class="flex items-baseline gap-2 flex-wrap">
                    <span class="font-semibold text-sm">{{ displayName(trainer) }}</span>
                    <Badge v-if="trainer.isDouble" variant="outline" class="text-[10px]">Double</Badge>
                  </div>
                  <span class="text-xs text-muted-foreground font-mono">Lv{{ maxLevel(trainer) }}</span>
                </div>

                <!-- Pokemon lineup -->
                <div class="flex items-center gap-0.5 shrink-0">
                  <RouterLink
                    v-for="mon in trainer.party"
                    :key="mon.speciesId + mon.species"
                    :to="{ name: 'pokemon-detail', params: { id: mon.speciesId } }"
                    @click.stop
                  >
                    <SpriteImage :sprite-id="getSpriteId(mon.speciesId)" :name="mon.species" size="sm" />
                  </RouterLink>
                </div>

                <!-- Expand chevron -->
                <svg
                  xmlns="http://www.w3.org/2000/svg"
                  width="14"
                  height="14"
                  viewBox="0 0 24 24"
                  fill="none"
                  stroke="currentColor"
                  stroke-width="2"
                  stroke-linecap="round"
                  stroke-linejoin="round"
                  class="text-muted-foreground shrink-0 transition-transform"
                  :class="{ 'rotate-180': expandedIds.has(trainer.id) }"
                >
                  <path d="m6 9 6 6 6-6"/>
                </svg>
              </div>

              <!-- Expanded party details -->
              <div v-if="expandedIds.has(trainer.id)" class="mt-4 grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-3" @click.stop>
                <div
                  v-for="(mon, idx) in trainer.party"
                  :key="idx"
                  class="rounded-lg border bg-muted/30 p-3 space-y-2"
                >
                  <!-- Species header -->
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

                  <!-- Details -->
                  <div class="text-xs space-y-0.5 text-muted-foreground">
                    <div v-if="mon.ability"><span class="text-foreground">Ability:</span> {{ mon.ability }}</div>
                    <div v-if="mon.item"><span class="text-foreground">Item:</span> {{ mon.item }}</div>
                    <div v-if="mon.nature"><span class="text-foreground">Nature:</span> {{ mon.nature }}</div>
                    <div v-if="mon.teraType"><span class="text-foreground">Tera:</span> {{ mon.teraType }}</div>
                  </div>

                  <!-- 2x2 Move grid -->
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
            </CardContent>
          </Card>
        </div>
      </div>
    </div>
  </div>
</template>
