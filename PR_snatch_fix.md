## Description

Fixes an issue where Snatch could steal a move that was already snatched by another Pokémon in the same turn during double battles.

In Gen 3-4, if multiple Pokémon used Snatch, the stolen move would be passed along to each successive Snatch user (slowest user gets the effect). Starting in Gen 5, Snatch no longer steals moves that were already stolen by another Pokémon's Snatch on the same turn.

This PR adds the `B_SNATCH` configuration option to control this behavior based on generation settings.

### Changes:
- Added `B_SNATCH` config option in `include/config/battle.h` (defaults to `GEN_LATEST`)
  - When set to Gen 5+, Snatch cannot steal a move that was already snatched this turn
  - When set to Gen 4 or earlier, multiple Snatch users can chain-steal the same move
- Updated snatch check in `Cmd_attackcanceler` (`src/battle_script_commands.c`) to respect the config
- Added comprehensive tests for Snatch behavior in `test/battle/move_effect/snatch.c`:
  - Snatch steals stat-boosting moves from the opponent
  - Snatch does not steal non-snatchable moves
  - Snatch does not steal a move that was already snatched this turn (Gen 5+)
  - Snatch steals from the correct target when multiple snatchable moves are used

## Issue(s) that this PR fixes

Fixes #8349

## Discord contact info

<!-- Add your Discord username here -->
