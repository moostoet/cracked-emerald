<script setup lang="ts">
import { ref } from 'vue'
import { RouterLink, RouterView } from 'vue-router'
import SearchBar from '@/components/SearchBar.vue'

const isDark = ref(true)

function toggleTheme() {
  isDark.value = !isDark.value
  document.documentElement.classList.toggle('dark', isDark.value)
}
</script>

<template>
  <div class="min-h-screen bg-background text-foreground">
    <nav class="sticky top-0 z-50 border-b border-border bg-zinc-900 text-white">
      <div class="mx-auto flex h-14 max-w-7xl items-center justify-between px-4">
        <RouterLink to="/" class="text-lg font-bold tracking-tight text-primary">
          Cracked Emerald Pokedex
        </RouterLink>

        <div class="flex items-center gap-6">
          <SearchBar />
          <div class="flex items-center gap-4 text-sm font-medium">
            <RouterLink
              to="/"
              class="transition-colors hover:text-primary"
              active-class="text-primary"
            >
              Pokemon
            </RouterLink>
            <RouterLink
              to="/moves"
              class="transition-colors hover:text-primary"
              active-class="text-primary"
            >
              Moves
            </RouterLink>
            <RouterLink
              to="/abilities"
              class="transition-colors hover:text-primary"
              active-class="text-primary"
            >
              Abilities
            </RouterLink>
            <RouterLink
              to="/encounters"
              class="transition-colors hover:text-primary"
              active-class="text-primary"
            >
              Encounters
            </RouterLink>
            <RouterLink
              to="/trainers"
              class="transition-colors hover:text-primary"
              active-class="text-primary"
            >
              Trainers
            </RouterLink>
            <RouterLink
              to="/types"
              class="transition-colors hover:text-primary"
              active-class="text-primary"
            >
              Type Chart
            </RouterLink>
          </div>

          <button
            class="rounded-md p-2 text-sm transition-colors hover:bg-zinc-800"
            @click="toggleTheme"
            :title="isDark ? 'Switch to light mode' : 'Switch to dark mode'"
          >
            <svg
              v-if="isDark"
              xmlns="http://www.w3.org/2000/svg"
              width="18"
              height="18"
              viewBox="0 0 24 24"
              fill="none"
              stroke="currentColor"
              stroke-width="2"
              stroke-linecap="round"
              stroke-linejoin="round"
            >
              <circle cx="12" cy="12" r="4" />
              <path d="M12 2v2" />
              <path d="M12 20v2" />
              <path d="m4.93 4.93 1.41 1.41" />
              <path d="m17.66 17.66 1.41 1.41" />
              <path d="M2 12h2" />
              <path d="M20 12h2" />
              <path d="m6.34 17.66-1.41 1.41" />
              <path d="m19.07 4.93-1.41 1.41" />
            </svg>
            <svg
              v-else
              xmlns="http://www.w3.org/2000/svg"
              width="18"
              height="18"
              viewBox="0 0 24 24"
              fill="none"
              stroke="currentColor"
              stroke-width="2"
              stroke-linecap="round"
              stroke-linejoin="round"
            >
              <path d="M12 3a6 6 0 0 0 9 9 9 9 0 1 1-9-9Z" />
            </svg>
          </button>
        </div>
      </div>
    </nav>

    <main class="mx-auto max-w-7xl px-4 py-6">
      <RouterView />
    </main>
  </div>
</template>
