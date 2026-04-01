export interface Pokemon {
  id: number
  name: string
  types: string[]
  baseStats: {
    hp: number
    attack: number
    defense: number
    spAttack: number
    spDefense: number
    speed: number
  }
  abilities: (string | null)[]
  evYield: {
    hp: number
    attack: number
    defense: number
    spAttack: number
    spDefense: number
    speed: number
  }
  catchRate: number
  genderRatio: number
  eggCycles: number
  growthRate: string
  eggGroups: string[]
  height: number
  weight: number
  category: string
  description: string
  generation: number
  natDexNum: number
  evolutions: Evolution[]
  levelUpMoves: LevelUpMove[]
  teachableMoves: string[]
  eggMoves: string[]
  heldItems: string[]
  spriteId: string
  encounters: Encounter[]
}

export interface Evolution {
  method: string
  param: string
  targetId: number
  targetName: string
}

export interface LevelUpMove {
  level: number
  move: string
}

export interface Encounter {
  location: string
  method: string
  minLevel: number
  maxLevel: number
  percentage: number
}

export interface Move {
  id: number
  name: string
  type: string
  category: string
  power: number
  accuracy: number
  pp: number
  priority: number
  description: string
  makesContact: boolean
}

export interface Ability {
  id: number
  name: string
  description: string
}

export interface TypeChart {
  types: string[]
  effectiveness: number[][]
}

export interface EncounterLocation {
  location: string
  pokemon: {
    species: string
    speciesId: number
    method: string
    minLevel: number
    maxLevel: number
    percentage: number
  }[]
}

export interface MapConnections {
  up: string[]
  down: string[]
  left: string[]
  right: string[]
  warps: string[]
}
