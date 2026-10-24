# Spinda-Drunken redesign

Spinda-Drunken is a custom **Normal/Fighting** Spinda that fights like a drunken master: it rolls with blows, gets stronger when it's hit, and throws its attacker's aim off. It appears as Gym Leader Brawly's second Pokémon and can't be obtained by the player.

## Typing and stats

| Stat | Before | After |
|---|---:|---:|
| HP | 83 | 80 |
| Attack | 113 | 100 |
| Defense | 107 | 100 |
| Special Attack | 59 | 55 |
| Special Defense | 83 | 80 |
| Speed | 83 | 85 |
| Total | 528 | 500 |

Drunken Fist supplies the extra Attack in battle, so its base stats no longer need to top Brawly's team. Lucario and his Floatzel ace stay the headline threats. Normal/Fighting is weak to Fighting, Flying, Psychic, and Fairy, and immune to Ghost.

## Abilities

- **Contrary:** reverses its stat changes. Hammer Arm and Superpower in its new level-up list now raise its stats instead of lowering them.
- **Scrappy:** lets its Normal and Fighting moves hit Ghost types.
- **Drunken Fist (hidden, custom):** when a contact move damages it, its Attack rises by one stage and the attacker's accuracy falls by one stage.
  - It triggers at most once per move, so multi-hit moves like Double Kick or Arm Thrust no longer stack several boosts in one turn.
  - It checks the Pokémon that was actually hit, so spread contact moves in double battles trigger it even when the chosen target is protected.
  - Trainer AI now prefers non-contact moves against it when they are otherwise equal.
  - Its description now fits on one line of the summary screen.

## Moves

Spinda-Drunken now has its own learnsets instead of borrowing plain Spinda's. Its level-up list lives in `src/data/pokemon/level_up_learnsets/custom.h`, which is included whatever generation of learnsets the build uses:

Tackle and Copycat at 1, Rock Smash at 5, Teeter Dance at 9, Feint Attack at 13, Dizzy Punch at 17, Revenge at 21, Drain Punch at 25, Sucker Punch at 29, Hammer Arm at 33, Zen Headbutt at 37, Superpower at 41, Double-Edge at 45, Flail at 49, and Thrash at 53.

Its teachable moves are Spinda's plus Bulk Up, Taunt, and Torment. Its egg moves are Fake Out, Encore, Mach Punch, Detect, Smelling Salts, Baton Pass, and Psycho Cut.

## Updated trainer set

| Encounter | Ability / item | Before | After |
|---|---|---|---|
| Gym Leader Brawly, Dewford Gym, level 25 | Drunken Fist / Lum Berry | Drain Punch, Sing, Dizzy Punch, Stone Edge | Drain Punch, Dizzy Punch, Rock Tomb, Counter |

All four moves are now in its learnsets. Counter returns a blow without the coin flip of 55%-accurate Sing, and a sleeping foe never hits it, which wasted Drunken Fist. Rock Tomb replaces Stone Edge as reliable coverage against Flying and Ghost types.

## Warning the player

The Dewford gym guide now explains that every battle in the gym is fought on a Rising Tide, which keeps Fighting-type Pokémon on their feet but slows everything else until every gym trainer is beaten. He also warns that Brawly's Spinda "rolls with the punches", so contact moves fire it up and throw off its attacker's aim.

## Presentation

- It has its own shiny palette: its red markings turn yellow-green, matching shiny Spinda, instead of borrowing plain Spinda's shiny colours, which didn't fit its art.
- Its front sprite now sits on the same ground line as other Pokémon and declares its full 64-pixel width.
- Its species entry, learnsets, and graphics are guarded by `P_FAMILY_SPINDA` and no longer depend on high-definition sprites being enabled.
- The Pokédex website lists it as its own entry, so Brawly's team shows its real typing, stats, abilities, and sprite.

This is a balance proposal implemented in the ROM data. Build and test checks verify the implementation; its difficulty still needs gameplay testing.
