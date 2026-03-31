<script setup lang="ts">
import { ref, computed, onMounted } from 'vue'
import { RouterLink } from 'vue-router'
import { usePokedex } from '@/composables/usePokedex'
import SpriteImage from '@/components/SpriteImage.vue'
import TypeBadge from '@/components/TypeBadge.vue'
import { Card, CardContent } from '@/components/ui/card'
import { Input } from '@/components/ui/input'
import { Button } from '@/components/ui/button'
import { Separator } from '@/components/ui/separator'
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select'
import {
  Table,
  TableBody,
  TableCell,
  TableHead,
  TableHeader,
  TableRow,
} from '@/components/ui/table'

const { loaded, filtered, filters, allTypes, allGenerations, allAbilities, load, pokemon } = usePokedex()

const viewMode = ref<'grid' | 'table'>('grid')

const totalCount = computed(() => pokemon.value.length)
const filteredCount = computed(() => filtered.value.length)

function bst(p: { baseStats: { hp: number; attack: number; defense: number; spAttack: number; spDefense: number; speed: number } }): number {
  const s = p.baseStats
  return s.hp + s.attack + s.defense + s.spAttack + s.spDefense + s.speed
}

function formatDexNum(num: number): string {
  return '#' + String(num).padStart(3, '0')
}

function onTypeChange(value: string) {
  filters.value.type = value === '__all__' ? '' : value
}

function onGenChange(value: string) {
  filters.value.generation = value === '__all__' ? 0 : Number(value)
}

function onAbilityChange(value: string) {
  filters.value.ability = value === '__all__' ? '' : value
}

onMounted(() => {
  load()
})
</script>

<template>
  <div class="space-y-6">
    <!-- Header -->
    <div>
      <h1 class="text-3xl font-bold tracking-tight">Pokemon</h1>
      <p class="text-muted-foreground mt-1">
        Browse all Pokemon available in Cracked Emerald.
      </p>
    </div>

    <!-- Filter bar -->
    <div class="flex flex-col gap-3 sm:flex-row sm:flex-wrap sm:items-center">
      <Input
        v-model="filters.search"
        placeholder="Search Pokemon..."
        class="sm:w-64"
      />

      <Select :model-value="filters.type || '__all__'" @update:model-value="onTypeChange">
        <SelectTrigger class="w-full sm:w-44">
          <SelectValue placeholder="All Types" />
        </SelectTrigger>
        <SelectContent>
          <SelectItem value="__all__">All Types</SelectItem>
          <SelectItem v-for="t in allTypes" :key="t" :value="t">
            {{ t }}
          </SelectItem>
        </SelectContent>
      </Select>

      <Select :model-value="filters.generation ? String(filters.generation) : '__all__'" @update:model-value="onGenChange">
        <SelectTrigger class="w-full sm:w-36">
          <SelectValue placeholder="All Gens" />
        </SelectTrigger>
        <SelectContent>
          <SelectItem value="__all__">All Gens</SelectItem>
          <SelectItem v-for="g in allGenerations" :key="g" :value="String(g)">
            Gen {{ g }}
          </SelectItem>
        </SelectContent>
      </Select>

      <Select :model-value="filters.ability || '__all__'" @update:model-value="onAbilityChange">
        <SelectTrigger class="w-full sm:w-52">
          <SelectValue placeholder="All Abilities" />
        </SelectTrigger>
        <SelectContent>
          <SelectItem value="__all__">All Abilities</SelectItem>
          <SelectItem v-for="a in allAbilities" :key="a" :value="a">
            {{ a }}
          </SelectItem>
        </SelectContent>
      </Select>

      <!-- Spacer + view toggle -->
      <div class="flex items-center gap-2 sm:ml-auto">
        <Button
          :variant="viewMode === 'grid' ? 'default' : 'outline'"
          size="icon"
          @click="viewMode = 'grid'"
          title="Grid view"
        >
          <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <rect x="3" y="3" width="7" height="7" /><rect x="14" y="3" width="7" height="7" /><rect x="3" y="14" width="7" height="7" /><rect x="14" y="14" width="7" height="7" />
          </svg>
        </Button>
        <Button
          :variant="viewMode === 'table' ? 'default' : 'outline'"
          size="icon"
          @click="viewMode = 'table'"
          title="Table view"
        >
          <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <line x1="8" y1="6" x2="21" y2="6" /><line x1="8" y1="12" x2="21" y2="12" /><line x1="8" y1="18" x2="21" y2="18" /><line x1="3" y1="6" x2="3.01" y2="6" /><line x1="3" y1="12" x2="3.01" y2="12" /><line x1="3" y1="18" x2="3.01" y2="18" />
          </svg>
        </Button>
      </div>
    </div>

    <Separator />

    <!-- Result count -->
    <p v-if="loaded" class="text-sm text-muted-foreground">
      Showing {{ filteredCount }} of {{ totalCount }} Pokemon
    </p>

    <!-- Loading state -->
    <div v-if="!loaded" class="flex items-center justify-center py-20">
      <div class="text-muted-foreground animate-pulse text-lg">Loading Pokemon...</div>
    </div>

    <!-- Grid view -->
    <div
      v-else-if="viewMode === 'grid'"
      class="grid grid-cols-2 sm:grid-cols-3 md:grid-cols-4 lg:grid-cols-5 xl:grid-cols-6 gap-3"
    >
      <RouterLink
        v-for="p in filtered"
        :key="p.id"
        :to="{ name: 'pokemon-detail', params: { id: p.id } }"
        class="group"
      >
        <Card class="h-full transition-all duration-150 group-hover:scale-[1.03] group-hover:border-primary/50 group-hover:shadow-lg">
          <CardContent class="flex flex-col items-center gap-1.5 p-3 pt-4">
            <SpriteImage :sprite-id="p.spriteId" :name="p.name" size="md" />
            <span class="text-xs text-muted-foreground font-mono">{{ formatDexNum(p.natDexNum) }}</span>
            <span class="text-sm font-semibold text-center leading-tight">{{ p.name }}</span>
            <div class="flex flex-wrap justify-center gap-1 mt-0.5">
              <TypeBadge v-for="t in p.types" :key="t" :type="t" size="sm" />
            </div>
          </CardContent>
        </Card>
      </RouterLink>

      <div v-if="filtered.length === 0" class="col-span-full text-center py-12 text-muted-foreground">
        No Pokemon match your filters.
      </div>
    </div>

    <!-- Table view -->
    <div v-else class="rounded-md border overflow-x-auto">
      <Table>
        <TableHeader>
          <TableRow>
            <TableHead class="w-14">Sprite</TableHead>
            <TableHead class="w-16">#</TableHead>
            <TableHead>Name</TableHead>
            <TableHead>Types</TableHead>
            <TableHead class="w-16 text-right">BST</TableHead>
            <TableHead class="w-16 text-right">Gen</TableHead>
          </TableRow>
        </TableHeader>
        <TableBody>
          <TableRow
            v-for="p in filtered"
            :key="p.id"
            class="cursor-pointer hover:bg-muted/50"
          >
            <TableCell>
              <RouterLink :to="{ name: 'pokemon-detail', params: { id: p.id } }">
                <SpriteImage :sprite-id="p.spriteId" :name="p.name" size="sm" />
              </RouterLink>
            </TableCell>
            <TableCell class="font-mono text-muted-foreground">
              <RouterLink :to="{ name: 'pokemon-detail', params: { id: p.id } }" class="hover:underline">
                {{ formatDexNum(p.natDexNum) }}
              </RouterLink>
            </TableCell>
            <TableCell class="font-medium">
              <RouterLink :to="{ name: 'pokemon-detail', params: { id: p.id } }" class="hover:underline">
                {{ p.name }}
              </RouterLink>
            </TableCell>
            <TableCell>
              <div class="flex gap-1">
                <TypeBadge v-for="t in p.types" :key="t" :type="t" size="sm" />
              </div>
            </TableCell>
            <TableCell class="text-right font-mono">{{ bst(p) }}</TableCell>
            <TableCell class="text-right">{{ p.generation }}</TableCell>
          </TableRow>
          <TableRow v-if="filtered.length === 0">
            <TableCell colspan="6" class="text-center py-12 text-muted-foreground">
              No Pokemon match your filters.
            </TableCell>
          </TableRow>
        </TableBody>
      </Table>
    </div>
  </div>
</template>
