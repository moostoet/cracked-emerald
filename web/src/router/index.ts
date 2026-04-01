import { createRouter, createWebHashHistory } from 'vue-router'
import PokemonList from '@/pages/PokemonList.vue'
import PokemonDetail from '@/pages/PokemonDetail.vue'
import MovesList from '@/pages/MovesList.vue'
import AbilitiesList from '@/pages/AbilitiesList.vue'
import TypeChart from '@/pages/TypeChart.vue'
import EncountersList from '@/pages/EncountersList.vue'
import EncounterRoute from '@/pages/EncounterRoute.vue'
import TrainersList from '@/pages/TrainersList.vue'

const router = createRouter({
  history: createWebHashHistory('/cracked-emerald/pokedex/'),
  routes: [
    {
      path: '/',
      name: 'pokemon-list',
      component: PokemonList,
    },
    {
      path: '/pokemon/:id',
      name: 'pokemon-detail',
      component: PokemonDetail,
    },
    {
      path: '/moves',
      name: 'moves-list',
      component: MovesList,
    },
    {
      path: '/moves/:id',
      name: 'move-detail',
      component: () => import('@/pages/MovesList.vue'),
    },
    {
      path: '/abilities',
      name: 'abilities-list',
      component: AbilitiesList,
    },
    {
      path: '/types',
      name: 'type-chart',
      component: TypeChart,
    },
    {
      path: '/encounters',
      name: 'encounters-list',
      component: EncountersList,
    },
    {
      path: '/encounters/:location',
      name: 'encounter-route',
      component: EncounterRoute,
    },
    {
      path: '/trainers',
      name: 'trainers-list',
      component: TrainersList,
    },
  ],
})

export default router
