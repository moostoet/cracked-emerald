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
import type { Trainer } from '@/types/pokemon'

const { loaded: trainersLoaded, search, filtered, load: loadTrainers } = useTrainers()
const { loaded: dexLoaded, load: loadDex, getById } = usePokedex()
const { load: loadMoves } = useMoves()

const loading = computed(() => !trainersLoaded.value || !dexLoaded.value)

const expandedIds = ref<Set<string>>(new Set())

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

function getSpeciesTypes(speciesId: number): string[] {
  return getById(speciesId)?.types ?? []
}

function maxLevel(trainer: Trainer): number {
  return Math.max(...trainer.party.map(m => m.level))
}

function displayName(trainer: Trainer): string {
  if (trainer.trainerClass) {
    return `${trainer.trainerClass} ${trainer.name}`
  }
  return trainer.name
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

    <Input
      v-model="search"
      placeholder="Search trainers, classes, or Pokemon..."
      class="sm:w-80"
    />

    <Separator />

    <p v-if="!loading" class="text-sm text-muted-foreground">
      Showing {{ filtered.length }} trainers
    </p>

    <div v-if="loading" class="flex items-center justify-center py-20">
      <div class="text-muted-foreground animate-pulse text-lg">Loading trainers...</div>
    </div>

    <div v-else class="space-y-2">
      <div v-if="filtered.length === 0" class="text-center py-12 text-muted-foreground">
        No trainers match your search.
      </div>

      <Card
        v-for="trainer in filtered"
        :key="trainer.id"
        class="transition-colors hover:border-primary/30 cursor-pointer"
        @click="toggleExpand(trainer.id)"
      >
        <CardContent class="p-4">
          <!-- Summary row -->
          <div class="flex items-center gap-4">
            <!-- Party sprite preview -->
            <div class="flex -space-x-2 shrink-0">
              <SpriteImage
                v-for="mon in trainer.party.slice(0, 6)"
                :key="mon.speciesId + mon.species"
                :sprite-id="getSpriteId(mon.speciesId)"
                :name="mon.species"
                size="sm"
              />
            </div>

            <!-- Name + class -->
            <div class="flex-1 min-w-0">
              <div class="flex items-baseline gap-2 flex-wrap">
                <span class="font-semibold">{{ displayName(trainer) }}</span>
                <Badge v-if="trainer.isDouble" variant="outline" class="text-[10px]">Double</Badge>
              </div>
              <div class="text-xs text-muted-foreground mt-0.5">
                {{ trainer.location || 'Unknown location' }}
              </div>
            </div>

            <!-- Level badge -->
            <div class="text-sm text-muted-foreground font-mono shrink-0">
              Lv{{ maxLevel(trainer) }}
            </div>

            <!-- Expand chevron -->
            <svg
              xmlns="http://www.w3.org/2000/svg"
              width="16"
              height="16"
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
          <div v-if="expandedIds.has(trainer.id)" class="mt-4 grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-3">
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
                <div>
                  <RouterLink
                    :to="{ name: 'pokemon-detail', params: { id: mon.speciesId } }"
                    class="font-semibold text-sm hover:underline hover:text-primary transition-colors"
                  >
                    {{ mon.species }}
                  </RouterLink>
                  <div class="flex items-center gap-1.5 mt-0.5">
                    <span class="text-xs font-mono text-muted-foreground">Lv{{ mon.level }}</span>
                    <TypeBadge v-for="t in getSpeciesTypes(mon.speciesId)" :key="t" :type="t" size="sm" />
                  </div>
                </div>
              </div>

              <!-- Details -->
              <div class="text-xs space-y-0.5 text-muted-foreground">
                <div v-if="mon.ability"><span class="text-foreground">Ability:</span> {{ mon.ability }}</div>
                <div v-if="mon.item"><span class="text-foreground">Item:</span> {{ mon.item }}</div>
                <div v-if="mon.nature"><span class="text-foreground">Nature:</span> {{ mon.nature }}</div>
                <div v-if="mon.teraType"><span class="text-foreground">Tera:</span> {{ mon.teraType }}</div>
              </div>

              <!-- Moves -->
              <div v-if="mon.moves.length > 0" class="flex flex-wrap gap-1">
                <MovePopover
                  v-for="move in mon.moves"
                  :key="move"
                  :move-name="move"
                />
              </div>
            </div>
          </div>
        </CardContent>
      </Card>
    </div>
  </div>
</template>
