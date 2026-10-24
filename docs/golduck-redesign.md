# Golduck redesign

Golduck is a **Water/Psychic physical attacker** that focuses the psychic power in its forehead into blades. Every blade it learns by level-up is a high critical-hit move, so Focus Energy turns all of them into guaranteed critical hits. Its three abilities give a choice between power, weather control, and speed.

## Typing and stats

| Stat | Before | After |
|---|---:|---:|
| HP | 80 | 80 |
| Attack | 82 | 110 |
| Defense | 78 | 78 |
| Special Attack | 95 | 60 |
| Special Defense | 80 | 80 |
| Speed | 85 | 95 |
| Total | 500 | 503 |

Golduck now yields 2 Attack effort values instead of 2 Special Attack. Water/Psychic is unchanged: it is weak to Grass, Electric, Bug, Ghost, and Dark, and its middling bulk makes a Focus Energy setup turn a real risk.

Psyduck is unchanged. It already learns Zen Headbutt at 18 and Aqua Tail at 24, so it reaches Golduck with physical attacks of both types.

## Abilities

- **Sharpness:** boosts slicing moves by 50%. That covers Aqua Cutter, Psycho Cut, Night Slash, Razor Shell, and Leaf Blade, plus Aerial Ace and Fury Cutter from its teachable moves. It replaces Damp, so a Damp Psyduck evolves into a Sharpness Golduck.
- **Cloud Nine:** unchanged. It cancels weather effects while Golduck is in battle, which makes it a direct answer to Drizzle and Drought teams and to the Mauville Gym's permanent rain.
- **Swift Swim (hidden):** unchanged. It doubles Speed in rain.

These are alternative abilities; Golduck only has one at a time.

## The Focus Energy plan

Aqua Cutter, Psycho Cut, Night Slash, and Leaf Blade, plus its Cross Chop egg move, all have a raised critical-hit ratio. Focus Energy adds two more stages, which reaches stage 3: a guaranteed critical hit under this build's Generation 7+ rules. Critical hits ignore the target's Defense boosts, Reflect, and Aurora Veil, as well as drops to Golduck's own Attack, such as from Intimidate.

With Sharpness, a guaranteed-crit Aqua Cutter hits like a 157-power move before same-type bonus. It costs a turn, though, and the boost ends if Golduck switches out. Taunt, status, and phazing all punish the setup, and Aqua Jet, its only priority move, gets no Sharpness boost.

Trainer AI already rates Focus Energy highly when the user has a high critical-hit move (`src/battle_ai_main.c`, `EFFECT_FOCUS_ENERGY`), so opponents use the plan without special scripting.

## Moves

The active Generation 9 level-up list gives Aqua Cutter on evolution and through the move reminder, Hone Claws through the move reminder, Psyduck's moves up to Aqua Tail at 24, then Night Slash at 28, Focus Energy at 32, Psycho Cut at 36, Razor Shell at 40, and Leaf Blade at 45. Soak, Psych Up, Amnesia, Hydro Pump, and Wonder Room were removed from its level-up list; Psych Up is still a tutor move.

The order also shapes the default movesets of wild Golduck and of trainers that list no moves. From level 37 on, those always include a Water attack, Psycho Cut, and Focus Energy:

| Level | Default moves |
|---|---|
| 37–39 | Aqua Tail, Night Slash, Focus Energy, Psycho Cut |
| 40–44 | Night Slash, Focus Energy, Psycho Cut, Razor Shell |
| 45+ | Focus Energy, Psycho Cut, Razor Shell, Leaf Blade |

Its teachable moves are unchanged because none of the new moves are TMs or tutors in this hack. Physical options there include Waterfall, Brick Break, Dig, Aerial Ace, Ice Punch, Iron Tail, and Fury Cutter.

## Updated trainer sets

| Encounter | Ability / item | Moves | Purpose |
|---|---|---|---|
| Fisherman Dale, Route 110, level 33 | Swift Swim / Damp Rock | Rain Dance, Aqua Cutter, Zen Headbutt, Ice Punch | Sets its own rain for a fast Water attack; Ice Punch covers Grass types. Adamant. |
| Youngster Ben, Mauville Gym, level 34 | Swift Swim / Mystic Water | Focus Energy, Aqua Cutter, Psycho Cut, Ice Punch | Always moves first in the gym's permanent rain; one Focus Energy turn makes its blades crit every time. Adamant. |
| Gentleman Walter, fifth rematch, level 41 | Default | Night Slash, Focus Energy, Psycho Cut, Razor Shell | Replaces Fury Swipes, Disable, Confusion, and Psych Up. |
| Cooltrainer Paul, FireRed/LeafGreen trainer data, level 42 | Default | Aqua Cutter, Psycho Cut, Focus Energy, Night Slash | Replaces Confusion, Scratch, Screech, and Disable. |
| Battle Frontier set 1 | Sharpness / Shell Bell | Focus Energy, Aqua Cutter, Psycho Cut, Night Slash | Adamant, with Attack and Speed investment: guaranteed crits after one setup turn. |
| Battle Frontier set 2 | Cloud Nine / Lum Berry | Hone Claws, Razor Shell, Zen Headbutt, Ice Punch | Adamant, with HP and Attack investment: a weather breaker. |
| Battle Frontier set 3 | Swift Swim / Damp Rock | Rain Dance, Aqua Cutter, Psycho Cut, Cross Chop | Adamant, with Attack and Speed investment: sets its own rain. |
| Battle Frontier set 4 | Sharpness / Scope Lens | Aqua Cutter, Psycho Cut, Leaf Blade, Cross Chop | Jolly, with Attack and Speed investment: every move is high critical-hit, so Scope Lens gives 50% crits without setup. |
| Kindler Bernard, Trainer Hill | Sharpness / Lax Incense | Focus Energy, Aqua Cutter, Psycho Cut, Brick Break | Special Attack effort values moved to Attack. Trainer Hill's legacy data supports only regular ability slots. |
| Camryn, Trainer Tower | Sharpness / Mystic Water | Focus Energy, Aqua Cutter, Psycho Cut, Ice Punch | Replaces a Calm Mind special set. |
| Alberto, Trainer Tower | Cloud Nine / Scope Lens | Cross Chop, Aqua Cutter, Psycho Cut, Ice Punch | Special Attack effort values moved to Attack. |
| Kati & Geb, Trainer Tower | Cloud Nine / Scope Lens | Attract, Aqua Cutter, Dig, Cross Chop | Aqua Cutter replaces Surf. |

The FireRed/LeafGreen trainers who list no moves (Swimmer Abigail, Tamer Vincent, Gentleman Clifford, and Young Couple Eve & Jon) use the new default movesets above. The shared Frontier sets update every facility trainer that draws from them, and Battle Frontier Apprentices build Golduck's moves from its level-up list.

This is a balance proposal implemented in the ROM data. Build and data checks verify the implementation; its difficulty still needs gameplay testing.
