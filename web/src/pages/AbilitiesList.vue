<script setup lang="ts">
import { computed, onMounted } from 'vue'
import { useAbilities } from '@/composables/useAbilities'
import { Input } from '@/components/ui/input'
import {
  Table,
  TableBody,
  TableCell,
  TableHead,
  TableHeader,
  TableRow,
} from '@/components/ui/table'
import { Separator } from '@/components/ui/separator'

const { abilities, loaded, search, filtered, load } = useAbilities()

const totalCount = computed(() => abilities.value.length)
const filteredCount = computed(() => filtered.value.length)

onMounted(() => {
  load()
})
</script>

<template>
  <div class="space-y-6">
    <!-- Header -->
    <div>
      <h1 class="text-3xl font-bold tracking-tight">Abilities</h1>
      <p class="text-muted-foreground mt-1">
        Browse all abilities available in Cracked Emerald.
      </p>
    </div>

    <!-- Search -->
    <div class="flex flex-col gap-3 sm:flex-row sm:items-center">
      <Input
        v-model="search"
        placeholder="Search abilities..."
        class="sm:w-64"
      />
    </div>

    <Separator />

    <!-- Result count -->
    <p v-if="loaded" class="text-sm text-muted-foreground">
      Showing {{ filteredCount }} of {{ totalCount }} abilities
    </p>

    <!-- Loading state -->
    <div v-if="!loaded" class="flex items-center justify-center py-20">
      <div class="text-muted-foreground animate-pulse text-lg">Loading abilities...</div>
    </div>

    <!-- Data table -->
    <div v-else class="rounded-md border overflow-x-auto">
      <Table>
        <TableHeader>
          <TableRow>
            <TableHead class="w-48">Name</TableHead>
            <TableHead>Description</TableHead>
          </TableRow>
        </TableHeader>
        <TableBody>
          <TableRow v-for="a in filtered" :key="a.id">
            <TableCell class="font-medium">
              {{ a.name }}
            </TableCell>
            <TableCell class="text-sm text-muted-foreground">
              {{ a.description }}
            </TableCell>
          </TableRow>
          <TableRow v-if="filtered.length === 0">
            <TableCell colspan="2" class="text-center py-12 text-muted-foreground">
              No abilities match your search.
            </TableCell>
          </TableRow>
        </TableBody>
      </Table>
    </div>
  </div>
</template>
