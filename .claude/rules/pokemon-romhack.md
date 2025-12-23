# Pokemon ROM Hack Conventions

## Static Variables
- Use `EWRAM_DATA` for static variables in C files: `static EWRAM_DATA u16 myVar = 0;`
- EWRAM_DATA sections only allow zero initializers - initialize non-zero values at runtime

## Key Data Structures
- Wild encounters: `gWildMonHeaders[headerId].encounterTypes[timeOfDay].landMonsInfo->wildPokemon[slot].species`
- Species items: `gSpeciesInfo[species].itemCommon` and `gSpeciesInfo[species].itemRare`
- Pokedex seen: `GetSetPokedexFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_SEEN)`
- Move check: `MonKnowsMove(&gPlayerParty[i], MOVE_X)`

## Script System
- Specials registered in `data/specials.inc` with `def_special FunctionName`
- Scripts use `.inc` files with assembly macros from `asm/macros/event.inc`
- Key macros: `msgbox`, `special`, `goto_if_eq`, `waitstate`, `dynmultichoice`

## Encounter Constants
- `LAND_WILD_COUNT`, `WATER_WILD_COUNT`, `FISH_WILD_COUNT`, `ROCK_WILD_COUNT`, `HIDDEN_WILD_COUNT`
- `TIMES_OF_DAY_COUNT` for time-based encounters
- `HEADER_NONE` when no wild encounters on map
