<script setup lang="ts">
import { ref, computed, onMounted } from 'vue'
import { useTypes } from '@/composables/useTypes'
import TypeBadge from '@/components/TypeBadge.vue'
import { Separator } from '@/components/ui/separator'
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select'

const { typeChart, loaded, load, getEffectiveness } = useTypes()

const defType1 = ref('')
const defType2 = ref('')

const types = computed(() => typeChart.value?.types ?? [])

function cellValue(attackIdx: number, defIdx: number): number {
  if (!typeChart.value) return 1
  return typeChart.value.effectiveness[attackIdx][defIdx]
}

function cellText(value: number): string {
  if (value === 2) return '2'
  if (value === 0.5) return '\u00BD'
  if (value === 0) return '0'
  return ''
}

function cellClass(value: number): string {
  if (value === 2) return 'bg-green-600/80 text-white'
  if (value === 0.5) return 'bg-red-600/60 text-white'
  if (value === 0) return 'bg-zinc-900 text-zinc-400'
  return ''
}

const selectedDefTypes = computed(() => {
  const result: string[] = []
  if (defType1.value) result.push(defType1.value)
  if (defType2.value) result.push(defType2.value)
  return result
})

const calculatorResults = computed(() => {
  if (selectedDefTypes.value.length === 0) return []
  return types.value.map(atkType => ({
    type: atkType,
    multiplier: getEffectiveness(atkType, selectedDefTypes.value),
  }))
})

function multiplierText(m: number): string {
  if (m === 0) return '0x'
  if (m === 0.25) return '\u00BCx'
  if (m === 0.5) return '\u00BDx'
  if (m === 1) return '1x'
  if (m === 2) return '2x'
  if (m === 4) return '4x'
  return m + 'x'
}

function multiplierClass(m: number): string {
  if (m === 4) return 'bg-green-500 text-white'
  if (m === 2) return 'bg-green-600/80 text-white'
  if (m === 1) return 'bg-muted text-muted-foreground'
  if (m === 0.5) return 'bg-orange-600/70 text-white'
  if (m === 0.25) return 'bg-red-600/70 text-white'
  if (m === 0) return 'bg-zinc-900 text-zinc-400'
  return 'bg-muted text-muted-foreground'
}

function onDefType1Change(value: string) {
  defType1.value = value === '__none__' ? '' : value
}

function onDefType2Change(value: string) {
  defType2.value = value === '__none__' ? '' : value
}

onMounted(() => {
  load()
})
</script>

<template>
  <div class="space-y-6">
    <!-- Header -->
    <div>
      <h1 class="text-3xl font-bold tracking-tight">Type Chart</h1>
      <p class="text-muted-foreground mt-1">
        Type effectiveness matrix and dual-type calculator.
      </p>
    </div>

    <!-- Loading state -->
    <div v-if="!loaded" class="flex items-center justify-center py-20">
      <div class="text-muted-foreground animate-pulse text-lg">Loading type chart...</div>
    </div>

    <template v-else>
      <!-- Section 1: Full type effectiveness matrix -->
      <div>
        <h2 class="text-xl font-semibold mb-3">Type Effectiveness Matrix</h2>
        <p class="text-sm text-muted-foreground mb-4">
          Rows are attacking types, columns are defending types.
        </p>
        <div class="overflow-x-auto rounded-md border">
          <table class="border-collapse">
            <!-- Header row -->
            <thead>
              <tr>
                <th class="sticky left-0 z-10 bg-background w-10 h-10 min-w-10 border-b border-r" />
                <th
                  v-for="defType in types"
                  :key="'h-' + defType"
                  class="w-10 h-10 min-w-10 border-b text-center p-0"
                >
                  <div class="flex items-center justify-center">
                    <TypeBadge :type="defType" size="sm" />
                  </div>
                </th>
              </tr>
            </thead>
            <tbody>
              <tr v-for="(atkType, atkIdx) in types" :key="'r-' + atkType">
                <!-- Row header -->
                <td class="sticky left-0 z-10 bg-background w-10 h-10 min-w-10 border-r p-0">
                  <div class="flex items-center justify-center">
                    <TypeBadge :type="atkType" size="sm" />
                  </div>
                </td>
                <!-- Effectiveness cells -->
                <td
                  v-for="(defType, defIdx) in types"
                  :key="'c-' + atkType + '-' + defType"
                  :class="[
                    cellClass(cellValue(atkIdx, defIdx)),
                    'w-10 h-10 min-w-10 text-center text-xs font-semibold border border-border/30'
                  ]"
                >
                  {{ cellText(cellValue(atkIdx, defIdx)) }}
                </td>
              </tr>
            </tbody>
          </table>
        </div>
      </div>

      <Separator />

      <!-- Section 2: Dual-type calculator -->
      <div>
        <h2 class="text-xl font-semibold mb-3">Dual-Type Calculator</h2>
        <p class="text-sm text-muted-foreground mb-4">
          Select defending types to see how each attacking type fares against them.
        </p>

        <div class="flex flex-col gap-3 sm:flex-row sm:items-center mb-6">
          <Select :model-value="defType1 || '__none__'" @update:model-value="onDefType1Change">
            <SelectTrigger class="w-full sm:w-48">
              <SelectValue placeholder="Defending Type 1" />
            </SelectTrigger>
            <SelectContent>
              <SelectItem value="__none__">None</SelectItem>
              <SelectItem v-for="t in types" :key="'d1-' + t" :value="t">
                {{ t }}
              </SelectItem>
            </SelectContent>
          </Select>

          <Select :model-value="defType2 || '__none__'" @update:model-value="onDefType2Change">
            <SelectTrigger class="w-full sm:w-48">
              <SelectValue placeholder="Defending Type 2" />
            </SelectTrigger>
            <SelectContent>
              <SelectItem value="__none__">None</SelectItem>
              <SelectItem v-for="t in types" :key="'d2-' + t" :value="t">
                {{ t }}
              </SelectItem>
            </SelectContent>
          </Select>
        </div>

        <!-- Calculator results -->
        <div v-if="selectedDefTypes.length === 0" class="text-sm text-muted-foreground py-8 text-center">
          Select at least one defending type to see effectiveness.
        </div>
        <div v-else class="grid grid-cols-3 sm:grid-cols-6 lg:grid-cols-9 gap-2">
          <div
            v-for="result in calculatorResults"
            :key="'calc-' + result.type"
            :class="[
              multiplierClass(result.multiplier),
              'flex flex-col items-center gap-1.5 rounded-lg p-3'
            ]"
          >
            <TypeBadge :type="result.type" size="sm" />
            <span class="text-sm font-bold">{{ multiplierText(result.multiplier) }}</span>
          </div>
        </div>
      </div>
    </template>
  </div>
</template>
