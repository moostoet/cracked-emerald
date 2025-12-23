# Fix incorrect "last to move" check for Analytic, Snatch, and Magic Coat

## Description

This PR fixes an issue where Analytic, Snatch, and Magic Coat incorrectly determine whether a battler is the "last to move" in a turn.

### The Problem

The previous implementation checked if a battler's action occurred at position `gBattlersCount - 1` (or `gCurrentTurnActionNumber == gBattlersCount - 1`). This approach fails to account for:

1. **Empty slots in doubles** - When a Pokemon faints and has no replacement, its slot becomes empty but `gBattlersCount` remains unchanged
2. **1v2 or 2v1 battles** - Similar issue where not all slots are filled
3. **Non-move actions** - Actions like switching or using items shouldn't count when determining "last to move"

For example, in a double battle where one Pokemon faints:
- 4 battler slots exist (`gBattlersCount = 4`)
- Only 3 Pokemon remain active
- A Pokemon at turn order position 2 would incorrectly think it's not last (since 2 != 3)
- But if position 3 is an empty/absent slot, position 2 IS actually last to move

### The Solution

Added a new helper function `IsLastMonToMove(u32 battler)` in `src/battle_script_commands.c` that properly determines if a battler is last to move by:

1. Getting the battler's position in turn order
2. Iterating through all subsequent positions
3. Skipping battlers marked as absent (`gAbsentBattlerFlags`)
4. Checking if any remaining battler will use a move (`B_ACTION_USE_MOVE`)
5. Returning `TRUE` only if no other battler after it will use a move

This function is now used by:
- **Analytic** ability (`src/battle_util.c`) - 30% damage boost when moving last
- **Magic Coat** (`src/battle_script_commands.c`) - Fails if user moves last (nothing to reflect)
- **Snatch** (`src/battle_script_commands.c`) - Fails if user moves last (nothing to steal)

### Tests Added

- `test/battle/ability/analytic.c` - Two new double battle tests verifying Analytic correctly activates/doesn't activate with empty slots
- `test/battle/move_effect/snatch.c` - Tests for basic functionality, failure when last, and success with empty partner slot
- `test/battle/move_effect/magic_coat.c` - Tests for failure when last and success with empty partner slot

## Media

N/A - Logic fix with no visual changes.

## Issue(s) that this PR fixes

Fixes #8512

## Discord contact info

mPokemon
