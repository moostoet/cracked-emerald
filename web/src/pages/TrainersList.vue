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
import type { Trainer, TrainerMon, TrainerReward, TrainerPokemonReward } from '@/types/pokemon'

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

interface SubAreaGroup {
  subArea: string
  entries: DisplayEntry[]
}

interface LocationGroup {
  location: string
  subAreas: SubAreaGroup[]
  totalEntries: number
  hasMultipleSubAreas: boolean
}

// Group display entries by location, then by sub-area
const groupedByLocation = computed(() => {
  const groups: LocationGroup[] = []
  const locMap = new Map<string, { subMap: Map<string, DisplayEntry[]>; order: string[] }>()

  for (const entry of displayEntries.value) {
    const loc = entry.trainers[0].location || 'Unknown'
    const sub = entry.trainers[0].subArea || ''

    if (!locMap.has(loc)) {
      locMap.set(loc, { subMap: new Map(), order: [] })
    }
    const locData = locMap.get(loc)!
    if (!locData.subMap.has(sub)) {
      locData.subMap.set(sub, [])
      locData.order.push(sub)
    }
    locData.subMap.get(sub)!.push(entry)
  }

  for (const [loc, locData] of locMap) {
    const subAreas: SubAreaGroup[] = locData.order.map(sub => ({
      subArea: sub,
      entries: locData.subMap.get(sub)!,
    }))
    const totalEntries = subAreas.reduce((sum, sa) => sum + sa.entries.length, 0)
    groups.push({
      location: loc,
      subAreas,
      totalEntries,
      hasMultipleSubAreas: subAreas.length > 1 || (subAreas.length === 1 && subAreas[0].subArea !== ''),
    })
  }
  return groups
})

const collapsedSubAreas = ref<Set<string>>(new Set())

function toggleLocation(loc: string) {
  if (collapsedLocations.value.has(loc)) {
    collapsedLocations.value.delete(loc)
  } else {
    collapsedLocations.value.add(loc)
  }
}

function subAreaKey(loc: string, sub: string) {
  return `${loc}::${sub}`
}

function toggleSubArea(loc: string, sub: string) {
  const key = subAreaKey(loc, sub)
  if (collapsedSubAreas.value.has(key)) {
    collapsedSubAreas.value.delete(key)
  } else {
    collapsedSubAreas.value.add(key)
  }
}

function toggleExpand(key: string) {
  if (expandedIds.value.has(key)) {
    expandedIds.value.delete(key)
  } else {
    expandedIds.value.add(key)
  }
}

function getSpriteId(speciesId: number, formSpriteId?: string): string {
  if (formSpriteId) return formSpriteId
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

function entryRewards(entry: DisplayEntry): TrainerReward[] {
  // Dedupe rewards across trainers in a forced double
  const seen = new Set<string>()
  const rewards: TrainerReward[] = []
  for (const t of entry.trainers) {
    for (const r of (t.rewards ?? [])) {
      const key = `${r.item}:${r.amount}`
      if (!seen.has(key)) {
        seen.add(key)
        rewards.push(r)
      }
    }
  }
  return rewards
}

function entryPokemonRewards(entry: DisplayEntry): TrainerPokemonReward[] {
  const seen = new Set<string>()
  const rewards: TrainerPokemonReward[] = []
  for (const t of entry.trainers) {
    for (const r of (t.pokemonRewards ?? [])) {
      const key = `${r.speciesId}:${r.level}:${r.kind}:${r.note ?? ''}`
      if (!seen.has(key)) {
        seen.add(key)
        rewards.push(r)
      }
    }
  }
  return rewards
}

function formatReward(r: TrainerReward): string {
  return r.amount > 1 ? `${r.item} x${r.amount}` : r.item
}

function formatPokemonReward(r: TrainerPokemonReward): string {
  if (r.kind === 'egg') return `${r.species} Egg`
  return r.level > 0 ? `${r.species} Lv${r.level}` : r.species
}

function pokemonRewardNote(entry: DisplayEntry): string {
  return entryPokemonRewards(entry).find(r => r.note)?.note ?? ''
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
          <span class="text-xs text-muted-foreground">({{ group.totalEntries }})</span>
        </button>

        <div v-if="!collapsedLocations.has(group.location)" class="pl-1">
          <!-- Sub-area groups (when multiple sub-areas exist) -->
          <template v-if="group.hasMultipleSubAreas">
            <div v-for="subGroup in group.subAreas" :key="subGroup.subArea" class="space-y-2 mb-3">
              <button
                class="flex items-center gap-2 w-full text-left group"
                @click="toggleSubArea(group.location, subGroup.subArea)"
              >
                <svg xmlns="http://www.w3.org/2000/svg" width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="text-muted-foreground/60 shrink-0 transition-transform" :class="{ '-rotate-90': collapsedSubAreas.has(subAreaKey(group.location, subGroup.subArea)) }"><path d="m6 9 6 6 6-6"/></svg>
                <h3 class="text-sm font-medium text-muted-foreground group-hover:text-foreground transition-colors">{{ subGroup.subArea || group.location }}</h3>
                <span class="text-xs text-muted-foreground/60">({{ subGroup.entries.length }})</span>
              </button>
              <div v-if="!collapsedSubAreas.has(subAreaKey(group.location, subGroup.subArea))" class="space-y-2 pl-4">
          <Card
            v-for="entry in subGroup.entries"
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
                    <Badge v-if="entryRewards(entry).length > 0" variant="outline" class="text-[10px] border-emerald-500/60 text-emerald-500" :title="entryRewards(entry).map(formatReward).join(', ')">
                      <svg xmlns="http://www.w3.org/2000/svg" width="10" height="10" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="mr-0.5"><path d="M20 12v10H4V12"/><path d="M2 7h20v5H2z"/><path d="M12 22V7"/><path d="M12 7H7.5a2.5 2.5 0 0 1 0-5C11 2 12 7 12 7z"/><path d="M12 7h4.5a2.5 2.5 0 0 0 0-5C13 2 12 7 12 7z"/></svg>
                      Items
                    </Badge>
                    <Badge v-if="entryPokemonRewards(entry).length > 0" variant="outline" class="text-[10px] border-sky-500/60 text-sky-500" :title="entryPokemonRewards(entry).map(formatPokemonReward).join(', ')">
                      Pokemon
                    </Badge>
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
                    <SpriteImage :sprite-id="getSpriteId(mon.speciesId, mon.formSpriteId)" :name="mon.species" size="sm" />
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
                          <SpriteImage :sprite-id="getSpriteId(mon.speciesId, mon.formSpriteId)" :name="mon.species" size="sm" />
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
                <!-- Rewards section -->
                <div v-if="entryRewards(entry).length > 0" class="mt-3 rounded-lg border border-emerald-500/20 bg-emerald-500/5 p-3">
                  <div class="text-xs font-semibold text-emerald-500 mb-1.5 flex items-center gap-1">
                    <svg xmlns="http://www.w3.org/2000/svg" width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M20 12v10H4V12"/><path d="M2 7h20v5H2z"/><path d="M12 22V7"/><path d="M12 7H7.5a2.5 2.5 0 0 1 0-5C11 2 12 7 12 7z"/><path d="M12 7h4.5a2.5 2.5 0 0 0 0-5C13 2 12 7 12 7z"/></svg>
                    Items Received After Battle
                  </div>
                  <div class="flex flex-wrap gap-2">
                    <span v-for="r in entryRewards(entry)" :key="r.item" class="text-xs px-2 py-1 rounded-md border bg-background">
                      {{ formatReward(r) }}
                    </span>
                  </div>
                </div>
                <div v-if="entryPokemonRewards(entry).length > 0" class="mt-3 rounded-lg border border-sky-500/20 bg-sky-500/5 p-3">
                  <div class="text-xs font-semibold text-sky-500 mb-1.5">Pokemon Received After Battle</div>
                  <div v-if="pokemonRewardNote(entry)" class="text-xs text-muted-foreground mb-2">{{ pokemonRewardNote(entry) }}</div>
                  <div class="flex flex-wrap gap-2">
                    <RouterLink
                      v-for="r in entryPokemonRewards(entry)"
                      :key="`${r.speciesId}:${r.level}:${r.kind}`"
                      :to="{ name: 'pokemon-detail', params: { id: r.speciesId } }"
                      class="flex items-center gap-1.5 text-xs px-2 py-1 rounded-md border bg-background hover:border-primary/50 hover:text-primary transition-colors"
                    >
                      <SpriteImage :sprite-id="getSpriteId(r.speciesId, r.formSpriteId)" :name="r.species" size="sm" />
                      <span>{{ formatPokemonReward(r) }}</span>
                    </RouterLink>
                  </div>
                </div>
              </div>
            </CardContent>
          </Card>
              </div>
            </div>
          </template>

          <!-- No sub-areas: flat list -->
          <template v-else>
            <div class="space-y-2">
              <Card
                v-for="entry in group.subAreas[0].entries"
                :key="entry.key"
                class="transition-colors hover:border-primary/30 cursor-pointer"
                :class="{ 'border-amber-500/40': entry.isForcedDouble }"
                @click="toggleExpand(entry.key)"
              >
                <CardContent class="p-3">
                  <div class="flex items-center gap-3">
                    <div class="flex shrink-0" :class="entry.isForcedDouble ? '-space-x-2' : ''">
                      <template v-for="trainer in entry.trainers" :key="trainer.id">
                        <img v-if="trainer.sprite" :src="trainer.sprite" :alt="displayName(trainer)" class="w-16 h-16 object-contain image-rendering-pixelated" loading="lazy" />
                        <div v-else class="w-16 h-16 rounded bg-muted flex items-center justify-center text-muted-foreground text-xs">?</div>
                      </template>
                    </div>
                    <div class="flex-1 min-w-0">
                      <div class="flex items-baseline gap-2 flex-wrap">
                        <span v-if="entry.isForcedDouble" class="font-semibold text-sm">{{ displayName(entry.trainers[0]) }} &amp; {{ displayName(entry.trainers[1]) }}</span>
                        <span v-else class="font-semibold text-sm">{{ displayName(entry.trainers[0]) }}</span>
                        <Badge v-if="entry.isForcedDouble" variant="outline" class="text-[10px] border-amber-500/60 text-amber-500">Forced Double</Badge>
                        <Badge v-else-if="entry.trainers[0].isDouble" variant="outline" class="text-[10px]">Double</Badge>
                        <Badge v-if="entryRewards(entry).length > 0" variant="outline" class="text-[10px] border-emerald-500/60 text-emerald-500" :title="entryRewards(entry).map(formatReward).join(', ')">
                          <svg xmlns="http://www.w3.org/2000/svg" width="10" height="10" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="mr-0.5"><path d="M20 12v10H4V12"/><path d="M2 7h20v5H2z"/><path d="M12 22V7"/><path d="M12 7H7.5a2.5 2.5 0 0 1 0-5C11 2 12 7 12 7z"/><path d="M12 7h4.5a2.5 2.5 0 0 0 0-5C13 2 12 7 12 7z"/></svg>
                          Items
                        </Badge>
                        <Badge v-if="entryPokemonRewards(entry).length > 0" variant="outline" class="text-[10px] border-sky-500/60 text-sky-500" :title="entryPokemonRewards(entry).map(formatPokemonReward).join(', ')">
                          Pokemon
                        </Badge>
                      </div>
                      <div class="text-xs text-muted-foreground mt-0.5">
                        <span class="font-mono">Lv{{ entryMaxLevel(entry) }}</span>
                        <span v-if="entry.possibleDoubleWith" class="ml-2">· Can double with {{ entry.possibleDoubleWith }}</span>
                      </div>
                    </div>
                    <div class="flex items-center gap-0.5 shrink-0">
                      <RouterLink v-for="mon in entryParty(entry)" :key="mon.speciesId + mon.species" :to="{ name: 'pokemon-detail', params: { id: mon.speciesId } }" @click.stop>
                        <SpriteImage :sprite-id="getSpriteId(mon.speciesId, mon.formSpriteId)" :name="mon.species" size="sm" />
                      </RouterLink>
                    </div>
                    <svg xmlns="http://www.w3.org/2000/svg" width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="text-muted-foreground shrink-0 transition-transform" :class="{ 'rotate-180': expandedIds.has(entry.key) }"><path d="m6 9 6 6 6-6"/></svg>
                  </div>
                  <div v-if="expandedIds.has(entry.key)" class="mt-4" @click.stop>
                    <template v-for="trainer in entry.trainers" :key="trainer.id">
                      <div v-if="entry.isForcedDouble" class="text-xs font-semibold text-muted-foreground mb-2 mt-3 first:mt-0">{{ displayName(trainer) }}</div>
                      <div class="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-3 mb-3">
                        <div v-for="(mon, idx) in trainer.party" :key="idx" class="rounded-lg border bg-muted/30 p-3 space-y-2">
                          <div class="flex items-center gap-2">
                            <RouterLink :to="{ name: 'pokemon-detail', params: { id: mon.speciesId } }"><SpriteImage :sprite-id="getSpriteId(mon.speciesId, mon.formSpriteId)" :name="mon.species" size="sm" /></RouterLink>
                            <div class="min-w-0">
                              <RouterLink :to="{ name: 'pokemon-detail', params: { id: mon.speciesId } }" class="font-semibold text-sm hover:underline hover:text-primary transition-colors block truncate">{{ mon.species }}</RouterLink>
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
                            <div v-for="move in mon.moves" :key="move" class="rounded-md border bg-background px-2 py-1.5">
                              <div class="text-xs font-medium truncate"><MovePopover :move-name="move" /></div>
                              <div class="flex items-center gap-1.5 mt-1">
                                <TypeBadge v-if="getMoveType(move)" :type="getMoveType(move)" size="sm" />
                                <img v-if="getMoveCategory(move)" :src="categoryIconUrl(getMoveCategory(move))" :alt="getMoveCategory(move)" class="h-3.5" loading="lazy" />
                              </div>
                            </div>
                          </div>
                        </div>
                      </div>
                    </template>
                    <!-- Rewards section -->
                    <div v-if="entryRewards(entry).length > 0" class="mt-3 rounded-lg border border-emerald-500/20 bg-emerald-500/5 p-3">
                      <div class="text-xs font-semibold text-emerald-500 mb-1.5 flex items-center gap-1">
                        <svg xmlns="http://www.w3.org/2000/svg" width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M20 12v10H4V12"/><path d="M2 7h20v5H2z"/><path d="M12 22V7"/><path d="M12 7H7.5a2.5 2.5 0 0 1 0-5C11 2 12 7 12 7z"/><path d="M12 7h4.5a2.5 2.5 0 0 0 0-5C13 2 12 7 12 7z"/></svg>
                        Items Received After Battle
                      </div>
                      <div class="flex flex-wrap gap-2">
                        <span v-for="r in entryRewards(entry)" :key="r.item" class="text-xs px-2 py-1 rounded-md border bg-background">
                          {{ formatReward(r) }}
                        </span>
                      </div>
                    </div>
                    <div v-if="entryPokemonRewards(entry).length > 0" class="mt-3 rounded-lg border border-sky-500/20 bg-sky-500/5 p-3">
                      <div class="text-xs font-semibold text-sky-500 mb-1.5">Pokemon Received After Battle</div>
                      <div v-if="pokemonRewardNote(entry)" class="text-xs text-muted-foreground mb-2">{{ pokemonRewardNote(entry) }}</div>
                      <div class="flex flex-wrap gap-2">
                        <RouterLink
                          v-for="r in entryPokemonRewards(entry)"
                          :key="`${r.speciesId}:${r.level}:${r.kind}`"
                          :to="{ name: 'pokemon-detail', params: { id: r.speciesId } }"
                          class="flex items-center gap-1.5 text-xs px-2 py-1 rounded-md border bg-background hover:border-primary/50 hover:text-primary transition-colors"
                        >
                          <SpriteImage :sprite-id="getSpriteId(r.speciesId, r.formSpriteId)" :name="r.species" size="sm" />
                          <span>{{ formatPokemonReward(r) }}</span>
                        </RouterLink>
                      </div>
                    </div>
                  </div>
                </CardContent>
              </Card>
            </div>
          </template>
        </div>
      </div>
    </div>
  </div>
</template>
