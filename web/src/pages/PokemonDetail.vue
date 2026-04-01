<script setup lang="ts">
import { ref, computed, watch, onMounted } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import type { Pokemon, Move } from '@/types/pokemon'
import { usePokedex } from '@/composables/usePokedex'
import { useMoves } from '@/composables/useMoves'
import { useAbilities } from '@/composables/useAbilities'
import SpriteImage from '@/components/SpriteImage.vue'
import TypeBadge from '@/components/TypeBadge.vue'
import StatBar from '@/components/StatBar.vue'
import MovePopover from '@/components/MovePopover.vue'
import AbilityPopover from '@/components/AbilityPopover.vue'
import EvolutionChain from '@/components/EvolutionChain.vue'
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card'
import { Tabs, TabsContent, TabsList, TabsTrigger } from '@/components/ui/tabs'
import { Table, TableBody, TableCell, TableHead, TableHeader, TableRow } from '@/components/ui/table'
import { Separator } from '@/components/ui/separator'
import { Button } from '@/components/ui/button'

const route = useRoute()
const router = useRouter()

const { load: loadPokedex, loaded: pokedexLoaded, getById, pokemon: allPokemon } = usePokedex()
const { load: loadMoves, loaded: movesLoaded, getMove } = useMoves()
const { load: loadAbilities, loaded: abilitiesLoaded } = useAbilities()

const currentId = ref(Number(route.params.id))
const mon = computed<Pokemon | undefined>(() => getById(currentId.value))

const loading = computed(() => !pokedexLoaded.value || !movesLoaded.value || !abilitiesLoaded.value)

const bst = computed(() => {
  if (!mon.value) return 0
  const s = mon.value.baseStats
  return s.hp + s.attack + s.defense + s.spAttack + s.spDefense + s.speed
})

const genderDisplay = computed(() => {
  if (!mon.value) return ''
  const ratio = mon.value.genderRatio
  if (ratio === -1) return 'Genderless'
  if (ratio === 0) return '100% Male'
  if (ratio === 100) return '100% Female'
  const female = ratio
  const male = 100 - ratio
  return `${male}% M / ${female}% F`
})

const evYieldDisplay = computed(() => {
  if (!mon.value) return '--'
  const y = mon.value.evYield
  const labels: Record<string, string> = {
    hp: 'HP', attack: 'Atk', defense: 'Def',
    spAttack: 'SpA', spDefense: 'SpD', speed: 'Spe',
  }
  const parts: string[] = []
  for (const [key, label] of Object.entries(labels)) {
    const val = y[key as keyof typeof y]
    if (val > 0) parts.push(`${val} ${label}`)
  }
  return parts.length ? parts.join(', ') : '--'
})

const heightDisplay = computed(() => {
  if (!mon.value) return '--'
  return `${(mon.value.height / 10).toFixed(1)} m`
})

const weightDisplay = computed(() => {
  if (!mon.value) return '--'
  return `${(mon.value.weight / 10).toFixed(1)} kg`
})

const sortedLevelUpMoves = computed(() => {
  if (!mon.value) return []
  return [...mon.value.levelUpMoves].sort((a, b) => a.level - b.level)
})

const hasPrev = computed(() => {
  if (!mon.value) return false
  return !!getById(currentId.value - 1)
})

const hasNext = computed(() => {
  if (!mon.value) return false
  return !!getById(currentId.value + 1)
})

function getMoveData(name: string): Move | undefined {
  return getMove(name)
}

watch(
  () => route.params.id,
  (newId) => {
    currentId.value = Number(newId)
  },
)

onMounted(async () => {
  await Promise.all([loadPokedex(), loadMoves(), loadAbilities()])
})
</script>

<template>
  <div class="max-w-5xl mx-auto px-4 py-6 space-y-6">
    <!-- Loading state -->
    <div v-if="loading" class="flex items-center justify-center min-h-[50vh]">
      <div class="text-center space-y-3">
        <div class="animate-spin h-8 w-8 border-4 border-primary border-t-transparent rounded-full mx-auto" />
        <p class="text-muted-foreground">Loading...</p>
      </div>
    </div>

    <!-- Not found -->
    <div v-else-if="!mon" class="flex items-center justify-center min-h-[50vh]">
      <div class="text-center space-y-3">
        <p class="text-2xl font-bold">Pokemon not found</p>
        <p class="text-muted-foreground">No Pokemon with ID {{ currentId }} exists.</p>
        <Button @click="router.push('/')">Back to Pokedex</Button>
      </div>
    </div>

    <!-- Detail content -->
    <template v-else>
      <!-- Prev/Next nav -->
      <div class="flex items-center justify-between">
        <Button
          variant="outline"
          size="sm"
          :disabled="!hasPrev"
          @click="router.push(`/pokemon/${currentId - 1}`)"
        >
          <span class="mr-1">&larr;</span> Prev
        </Button>
        <RouterLink to="/" class="text-sm text-muted-foreground hover:text-foreground transition-colors">
          Back to Pokedex
        </RouterLink>
        <Button
          variant="outline"
          size="sm"
          :disabled="!hasNext"
          @click="router.push(`/pokemon/${currentId + 1}`)"
        >
          Next <span class="ml-1">&rarr;</span>
        </Button>
      </div>

      <!-- Header row -->
      <Card>
        <CardContent class="pt-6">
          <div class="flex flex-col sm:flex-row items-center sm:items-start gap-6">
            <SpriteImage :sprite-id="mon.spriteId" :name="mon.name" size="lg" />
            <div class="flex-1 space-y-2 text-center sm:text-left">
              <div class="flex flex-col sm:flex-row items-center sm:items-baseline gap-2">
                <h1 class="text-3xl font-bold">{{ mon.name }}</h1>
                <span class="text-lg text-muted-foreground font-mono">#{{ String(mon.natDexNum).padStart(4, '0') }}</span>
              </div>
              <div class="flex flex-wrap items-center justify-center sm:justify-start gap-1.5">
                <TypeBadge v-for="t in mon.types" :key="t" :type="t" size="md" />
              </div>
            </div>
          </div>
        </CardContent>
      </Card>

      <!-- Base Stats -->
      <Card>
        <CardHeader>
          <CardTitle>Base Stats</CardTitle>
        </CardHeader>
        <CardContent class="space-y-2">
          <StatBar stat="hp" :value="mon.baseStats.hp" />
          <StatBar stat="attack" :value="mon.baseStats.attack" />
          <StatBar stat="defense" :value="mon.baseStats.defense" />
          <StatBar stat="spAttack" :value="mon.baseStats.spAttack" />
          <StatBar stat="spDefense" :value="mon.baseStats.spDefense" />
          <StatBar stat="speed" :value="mon.baseStats.speed" />
          <Separator />
          <div class="flex items-center gap-2 text-sm">
            <span class="w-8 text-right font-medium text-muted-foreground">BST</span>
            <span class="w-8 text-right font-mono font-semibold">{{ bst }}</span>
          </div>
        </CardContent>
      </Card>

      <!-- Abilities -->
      <Card>
        <CardHeader>
          <CardTitle>Abilities</CardTitle>
        </CardHeader>
        <CardContent>
          <div class="flex flex-wrap gap-3">
            <template v-for="(ability, index) in mon.abilities" :key="index">
              <AbilityPopover
                v-if="ability"
                :ability-name="ability"
                :is-hidden="index === 2"
              />
            </template>
          </div>
        </CardContent>
      </Card>

      <!-- Evolution Chain -->
      <Card v-if="mon.evolutions.length > 0 || allPokemon.some(p => p.evolutions.some(e => e.targetId === mon!.id))">
        <CardHeader>
          <CardTitle>Evolution Chain</CardTitle>
        </CardHeader>
        <CardContent>
          <EvolutionChain :pokemon="mon" />
        </CardContent>
      </Card>

      <!-- Info Grid -->
      <Card>
        <CardHeader>
          <CardTitle>Details</CardTitle>
        </CardHeader>
        <CardContent>
          <div class="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-x-8 gap-y-3 text-sm">
            <div class="flex justify-between sm:flex-col gap-1">
              <span class="text-muted-foreground">Held Items</span>
              <span v-if="mon.heldItems?.length" class="font-medium flex flex-wrap items-center gap-x-3 gap-y-1">
                <span v-for="item in mon.heldItems" :key="item" class="inline-flex items-center gap-1">
                  <img :src="`https://play.pokemonshowdown.com/sprites/itemicons/${item.toLowerCase().replace(/ /g, '-')}.png`" :alt="item" class="w-6 h-6" />
                  {{ item }}
                </span>
              </span>
              <span v-else class="font-medium">&mdash;</span>
            </div>
            <div class="flex justify-between sm:flex-col gap-1">
              <span class="text-muted-foreground">Egg Groups</span>
              <span class="font-medium">{{ mon.eggGroups.join(', ') || '--' }}</span>
            </div>
            <div class="flex justify-between sm:flex-col gap-1">
              <span class="text-muted-foreground">Gender Ratio</span>
              <span class="font-medium">{{ genderDisplay }}</span>
            </div>
            <div class="flex justify-between sm:flex-col gap-1">
              <span class="text-muted-foreground">Catch Rate</span>
              <span class="font-medium">{{ mon.catchRate }}</span>
            </div>
            <div class="flex justify-between sm:flex-col gap-1">
              <span class="text-muted-foreground">Hatch Cycles</span>
              <span class="font-medium">{{ mon.eggCycles }}</span>
            </div>
            <div class="flex justify-between sm:flex-col gap-1">
              <span class="text-muted-foreground">Growth Rate</span>
              <span class="font-medium">{{ mon.growthRate }}</span>
            </div>
            <div class="flex justify-between sm:flex-col gap-1">
              <span class="text-muted-foreground">EV Yield</span>
              <span class="font-medium">{{ evYieldDisplay }}</span>
            </div>
            <div class="flex justify-between sm:flex-col gap-1">
              <span class="text-muted-foreground">Height</span>
              <span class="font-medium">{{ heightDisplay }}</span>
            </div>
            <div class="flex justify-between sm:flex-col gap-1">
              <span class="text-muted-foreground">Weight</span>
              <span class="font-medium">{{ weightDisplay }}</span>
            </div>
          </div>
        </CardContent>
      </Card>

      <!-- Pokedex Entry -->
      <Card>
        <CardHeader>
          <CardTitle>Pokedex Entry</CardTitle>
        </CardHeader>
        <CardContent class="space-y-2">
          <p class="text-sm font-semibold text-muted-foreground">The {{ mon.category }} Pokemon</p>
          <p class="text-sm leading-relaxed">{{ mon.description }}</p>
        </CardContent>
      </Card>

      <!-- Tabbed section: Moves & Encounters -->
      <Tabs default-value="level-up">
        <TabsList class="w-full flex flex-wrap h-auto">
          <TabsTrigger value="level-up">Level-up Moves</TabsTrigger>
          <TabsTrigger value="tm-tutor">TM & Tutor</TabsTrigger>
          <TabsTrigger value="egg-moves">Egg Moves</TabsTrigger>
          <TabsTrigger v-if="mon.encounters.length > 0" value="encounters">Encounters</TabsTrigger>
        </TabsList>

        <!-- Level-up Moves -->
        <TabsContent value="level-up">
          <Card>
            <CardContent class="pt-6">
              <div v-if="sortedLevelUpMoves.length === 0" class="text-sm text-muted-foreground py-4 text-center">
                No level-up moves.
              </div>
              <div v-else class="overflow-x-auto">
                <Table>
                  <TableHeader>
                    <TableRow>
                      <TableHead class="w-16">Level</TableHead>
                      <TableHead>Move</TableHead>
                      <TableHead class="w-24">Type</TableHead>
                      <TableHead class="w-24">Category</TableHead>
                      <TableHead class="w-16 text-right">Power</TableHead>
                      <TableHead class="w-20 text-right">Accuracy</TableHead>
                    </TableRow>
                  </TableHeader>
                  <TableBody>
                    <TableRow v-for="(lm, idx) in sortedLevelUpMoves" :key="idx">
                      <TableCell class="font-mono">{{ lm.level === 0 ? '—' : lm.level }}</TableCell>
                      <TableCell>
                        <MovePopover :move-name="lm.move" />
                      </TableCell>
                      <TableCell>
                        <TypeBadge v-if="getMoveData(lm.move)" :type="getMoveData(lm.move)!.type" size="sm" />
                      </TableCell>
                      <TableCell class="text-sm">{{ getMoveData(lm.move)?.category ?? '--' }}</TableCell>
                      <TableCell class="text-right font-mono text-sm">{{ getMoveData(lm.move)?.power || '--' }}</TableCell>
                      <TableCell class="text-right font-mono text-sm">
                        {{ getMoveData(lm.move)?.accuracy ? `${getMoveData(lm.move)!.accuracy}%` : '--' }}
                      </TableCell>
                    </TableRow>
                  </TableBody>
                </Table>
              </div>
            </CardContent>
          </Card>
        </TabsContent>

        <!-- TM & Tutor Moves -->
        <TabsContent value="tm-tutor">
          <Card>
            <CardContent class="pt-6">
              <div v-if="mon.teachableMoves.length === 0" class="text-sm text-muted-foreground py-4 text-center">
                No TM or Tutor moves.
              </div>
              <div v-else class="overflow-x-auto">
                <Table>
                  <TableHeader>
                    <TableRow>
                      <TableHead>Move</TableHead>
                      <TableHead class="w-24">Type</TableHead>
                      <TableHead class="w-24">Category</TableHead>
                      <TableHead class="w-16 text-right">Power</TableHead>
                      <TableHead class="w-20 text-right">Accuracy</TableHead>
                    </TableRow>
                  </TableHeader>
                  <TableBody>
                    <TableRow v-for="name in mon.teachableMoves" :key="name">
                      <TableCell>
                        <MovePopover :move-name="name" />
                      </TableCell>
                      <TableCell>
                        <TypeBadge v-if="getMoveData(name)" :type="getMoveData(name)!.type" size="sm" />
                      </TableCell>
                      <TableCell class="text-sm">{{ getMoveData(name)?.category ?? '--' }}</TableCell>
                      <TableCell class="text-right font-mono text-sm">{{ getMoveData(name)?.power || '--' }}</TableCell>
                      <TableCell class="text-right font-mono text-sm">
                        {{ getMoveData(name)?.accuracy ? `${getMoveData(name)!.accuracy}%` : '--' }}
                      </TableCell>
                    </TableRow>
                  </TableBody>
                </Table>
              </div>
            </CardContent>
          </Card>
        </TabsContent>

        <!-- Egg Moves -->
        <TabsContent value="egg-moves">
          <Card>
            <CardContent class="pt-6">
              <div v-if="mon.eggMoves.length === 0" class="text-sm text-muted-foreground py-4 text-center">
                No egg moves.
              </div>
              <div v-else class="overflow-x-auto">
                <Table>
                  <TableHeader>
                    <TableRow>
                      <TableHead>Move</TableHead>
                      <TableHead class="w-24">Type</TableHead>
                      <TableHead class="w-24">Category</TableHead>
                      <TableHead class="w-16 text-right">Power</TableHead>
                      <TableHead class="w-20 text-right">Accuracy</TableHead>
                    </TableRow>
                  </TableHeader>
                  <TableBody>
                    <TableRow v-for="name in mon.eggMoves" :key="name">
                      <TableCell>
                        <MovePopover :move-name="name" />
                      </TableCell>
                      <TableCell>
                        <TypeBadge v-if="getMoveData(name)" :type="getMoveData(name)!.type" size="sm" />
                      </TableCell>
                      <TableCell class="text-sm">{{ getMoveData(name)?.category ?? '--' }}</TableCell>
                      <TableCell class="text-right font-mono text-sm">{{ getMoveData(name)?.power || '--' }}</TableCell>
                      <TableCell class="text-right font-mono text-sm">
                        {{ getMoveData(name)?.accuracy ? `${getMoveData(name)!.accuracy}%` : '--' }}
                      </TableCell>
                    </TableRow>
                  </TableBody>
                </Table>
              </div>
            </CardContent>
          </Card>
        </TabsContent>

        <!-- Encounters -->
        <TabsContent v-if="mon.encounters.length > 0" value="encounters">
          <Card>
            <CardContent class="pt-6">
              <div class="overflow-x-auto">
                <Table>
                  <TableHeader>
                    <TableRow>
                      <TableHead>Location</TableHead>
                      <TableHead class="w-32">Method</TableHead>
                      <TableHead class="w-28">Level Range</TableHead>
                    </TableRow>
                  </TableHeader>
                  <TableBody>
                    <TableRow v-for="(enc, idx) in mon.encounters" :key="idx">
                      <TableCell>{{ enc.location }}</TableCell>
                      <TableCell class="text-sm">{{ enc.method }}</TableCell>
                      <TableCell class="font-mono text-sm">{{ enc.minLevel }}-{{ enc.maxLevel }}</TableCell>
                    </TableRow>
                  </TableBody>
                </Table>
              </div>
            </CardContent>
          </Card>
        </TabsContent>
      </Tabs>
    </template>
  </div>
</template>
