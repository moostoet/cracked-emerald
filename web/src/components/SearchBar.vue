<script setup lang="ts">
import { ref, computed, watch, onMounted, onUnmounted } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { usePokedex } from '@/composables/usePokedex'
import { Popover, PopoverContent } from '@/components/ui/popover'
import { PopoverAnchor } from 'radix-vue'
import SpriteImage from '@/components/SpriteImage.vue'
import TypeBadge from '@/components/TypeBadge.vue'

const route = useRoute()
const router = useRouter()
const { filters, loaded, pokemon, load } = usePokedex()

const inputRef = ref<HTMLInputElement>()
const localQuery = ref('')
const isOpen = ref(false)
const highlightedIndex = ref(0)

const isListPage = computed(() => route.name === 'pokemon-list')
const isMac = computed(() => navigator.platform.toUpperCase().includes('MAC'))

const typeaheadResults = computed(() => {
  if (!localQuery.value || !loaded.value) return []
  const q = localQuery.value.toLowerCase()
  return pokemon.value
    .filter(p => p.name.toLowerCase().includes(q))
    .slice(0, 8)
})

const showDropdown = computed(() => {
  return !isListPage.value && isOpen.value && localQuery.value.length > 0
})

function onInput(e: Event) {
  const val = (e.target as HTMLInputElement).value
  localQuery.value = val
  highlightedIndex.value = 0

  if (isListPage.value) {
    filters.value.search = val
  }
}

// Sync from filters -> localQuery when navigating to list page
watch(isListPage, (onList) => {
  if (onList) {
    localQuery.value = filters.value.search
  }
})

// When leaving the list page, clear the filter (so list resets)
watch(isListPage, (onList, wasOnList) => {
  if (wasOnList && !onList) {
    localQuery.value = ''
  }
})

function selectResult(pokemonId: number) {
  isOpen.value = false
  localQuery.value = ''
  router.push({ name: 'pokemon-detail', params: { id: pokemonId } })
}

function onKeydown(e: KeyboardEvent) {
  if (!showDropdown.value) return

  const results = typeaheadResults.value
  if (e.key === 'ArrowDown') {
    e.preventDefault()
    highlightedIndex.value = (highlightedIndex.value + 1) % results.length
  } else if (e.key === 'ArrowUp') {
    e.preventDefault()
    highlightedIndex.value = (highlightedIndex.value - 1 + results.length) % results.length
  } else if (e.key === 'Enter' && results.length > 0) {
    e.preventDefault()
    selectResult(results[highlightedIndex.value].id)
  } else if (e.key === 'Escape') {
    e.preventDefault()
    isOpen.value = false
    inputRef.value?.blur()
  }
}

function onFocus() {
  isOpen.value = true
}

function onGlobalKeydown(e: KeyboardEvent) {
  if ((e.metaKey || e.ctrlKey) && e.key === 'k') {
    e.preventDefault()
    inputRef.value?.focus()
  }
}

onMounted(() => {
  load()
  document.addEventListener('keydown', onGlobalKeydown)
})

onUnmounted(() => {
  document.removeEventListener('keydown', onGlobalKeydown)
})

function formatDexNum(num: number): string {
  return '#' + String(num).padStart(3, '0')
}
</script>

<template>
  <Popover v-model:open="isOpen">
    <PopoverAnchor as-child>
      <div class="relative">
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
          class="absolute left-2.5 top-1/2 -translate-y-1/2 text-muted-foreground pointer-events-none"
        >
          <circle cx="11" cy="11" r="8" />
          <path d="m21 21-4.3-4.3" />
        </svg>
        <input
          ref="inputRef"
          type="text"
          :value="localQuery"
          placeholder="Search Pokemon..."
          class="h-8 w-48 sm:w-56 rounded-md border border-input bg-background pl-8 pr-12 text-sm text-foreground ring-offset-background placeholder:text-muted-foreground focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-ring focus-visible:ring-offset-2"
          @input="onInput"
          @focus="onFocus"
          @keydown="onKeydown"
        />
        <kbd class="pointer-events-none absolute right-2 top-1/2 -translate-y-1/2 hidden sm:inline-flex h-5 select-none items-center gap-0.5 rounded border bg-muted px-1.5 font-mono text-[10px] font-medium text-muted-foreground">
          {{ isMac ? '\u2318' : 'Ctrl+' }}K
        </kbd>
      </div>
    </PopoverAnchor>

    <PopoverContent
      v-if="showDropdown"
      class="w-80 p-0 overflow-hidden"
      :side-offset="8"
      align="start"
      @open-auto-focus.prevent
    >
      <div v-if="typeaheadResults.length === 0" class="py-6 text-center text-sm text-muted-foreground">
        No Pokemon found.
      </div>
      <ul v-else class="max-h-80 overflow-y-auto py-1">
        <li
          v-for="(p, idx) in typeaheadResults"
          :key="p.id"
          :class="[
            'flex items-center gap-3 px-3 py-2 cursor-pointer text-sm transition-colors',
            idx === highlightedIndex ? 'bg-accent text-accent-foreground' : 'hover:bg-accent/50'
          ]"
          @mouseenter="highlightedIndex = idx"
          @click="selectResult(p.id)"
        >
          <SpriteImage :sprite-id="p.spriteId" :name="p.name" size="sm" />
          <div class="flex-1 min-w-0">
            <div class="flex items-baseline gap-2">
              <span class="font-medium truncate">{{ p.name }}</span>
              <span class="text-xs text-muted-foreground font-mono">{{ formatDexNum(p.natDexNum) }}</span>
            </div>
            <div class="flex gap-1 mt-0.5">
              <TypeBadge v-for="t in p.types" :key="t" :type="t" size="sm" />
            </div>
          </div>
        </li>
      </ul>
    </PopoverContent>
  </Popover>
</template>
