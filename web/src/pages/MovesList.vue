<script setup lang="ts">
import { computed, onMounted } from 'vue'
import { useMoves } from '@/composables/useMoves'
import TypeBadge from '@/components/TypeBadge.vue'
import { Input } from '@/components/ui/input'
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
import { Separator } from '@/components/ui/separator'

const { moves, loaded, filters, filtered, load } = useMoves()

const allTypes = [
  'Normal', 'Fighting', 'Flying', 'Poison', 'Ground', 'Rock',
  'Bug', 'Ghost', 'Steel', 'Fire', 'Water', 'Grass',
  'Electric', 'Psychic', 'Ice', 'Dragon', 'Dark', 'Fairy',
]

const categories = ['Physical', 'Special', 'Status']

const totalCount = computed(() => moves.value.length)
const filteredCount = computed(() => filtered.value.length)

function onTypeChange(value: string) {
  filters.value.type = value === '__all__' ? '' : value
}

function onCategoryChange(value: string) {
  filters.value.category = value === '__all__' ? '' : value
}

function displayPower(power: number): string {
  return power === 0 ? '\u2014' : String(power)
}

function displayAccuracy(accuracy: number): string {
  return accuracy === 0 ? '\u2014' : String(accuracy)
}

onMounted(() => {
  load()
})
</script>

<template>
  <div class="space-y-6">
    <!-- Header -->
    <div>
      <h1 class="text-3xl font-bold tracking-tight">Moves</h1>
      <p class="text-muted-foreground mt-1">
        Browse all moves available in Cracked Emerald.
      </p>
    </div>

    <!-- Filter bar -->
    <div class="flex flex-col gap-3 sm:flex-row sm:flex-wrap sm:items-center">
      <Input
        v-model="filters.search"
        placeholder="Search moves..."
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

      <Select :model-value="filters.category || '__all__'" @update:model-value="onCategoryChange">
        <SelectTrigger class="w-full sm:w-44">
          <SelectValue placeholder="All Categories" />
        </SelectTrigger>
        <SelectContent>
          <SelectItem value="__all__">All Categories</SelectItem>
          <SelectItem v-for="c in categories" :key="c" :value="c">
            {{ c }}
          </SelectItem>
        </SelectContent>
      </Select>
    </div>

    <Separator />

    <!-- Result count -->
    <p v-if="loaded" class="text-sm text-muted-foreground">
      Showing {{ filteredCount }} of {{ totalCount }} moves
    </p>

    <!-- Loading state -->
    <div v-if="!loaded" class="flex items-center justify-center py-20">
      <div class="text-muted-foreground animate-pulse text-lg">Loading moves...</div>
    </div>

    <!-- Data table -->
    <div v-else class="rounded-md border overflow-x-auto">
      <Table>
        <TableHeader>
          <TableRow>
            <TableHead>Name</TableHead>
            <TableHead class="w-24">Type</TableHead>
            <TableHead class="w-24">Category</TableHead>
            <TableHead class="w-20 text-right">Power</TableHead>
            <TableHead class="w-20 text-right">Accuracy</TableHead>
            <TableHead class="w-16 text-right">PP</TableHead>
          </TableRow>
        </TableHeader>
        <TableBody>
          <TableRow v-for="m in filtered" :key="m.id">
            <TableCell class="font-medium" :title="m.description">
              {{ m.name }}
            </TableCell>
            <TableCell>
              <TypeBadge :type="m.type" size="sm" />
            </TableCell>
            <TableCell class="text-sm text-muted-foreground">
              {{ m.category }}
            </TableCell>
            <TableCell class="text-right font-mono">
              {{ displayPower(m.power) }}
            </TableCell>
            <TableCell class="text-right font-mono">
              {{ displayAccuracy(m.accuracy) }}
            </TableCell>
            <TableCell class="text-right font-mono">
              {{ m.pp }}
            </TableCell>
          </TableRow>
          <TableRow v-if="filtered.length === 0">
            <TableCell colspan="6" class="text-center py-12 text-muted-foreground">
              No moves match your filters.
            </TableCell>
          </TableRow>
        </TableBody>
      </Table>
    </div>
  </div>
</template>
