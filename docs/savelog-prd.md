# SaveLog MVP — Product Requirements Document

## Overview

SaveLog is a lightweight, embeddable event logging system for pokeemerald-expansion ROM hacks. It records gameplay events into a ring buffer in SaveBlock3, enabling developers to reconstruct what a player was doing when they encountered a bug.

This document defines the MVP scope: **SaveBlock3 ring buffer, variable-width encoding, mGBA debug output.** Competition features (integrity hashing, EWRAM buffering, Lua sidecar, verification codes) and the external `.sav` parser are out of scope for this phase.

---

## Problem

When a player reports a bug in a ROM hack, the developer typically gets "it crashed in a cave" or "something weird happened after I beat Roxanne." There's no way to reconstruct the sequence of events that led to the issue. Developers are left guessing at flag ordering, edge cases, and state corruption with no diagnostic trail.

## Solution

Embed a ring buffer in SaveBlock3 that automatically logs gameplay events (map transitions, catches, faints, trainer battles, item pickups, etc.) as they happen. When a player reports a bug, they send their `.sav` file. The developer extracts the log (via debug tools or a future parser) and gets a chronological timeline of the last ~790 events, which is typically more than enough to identify the root cause.

---

## Target Environment

- **Base**: pokeemerald-expansion (latest stable, currently 1.15.x)
- **Emulator**: mGBA (primary), but nothing emulator-specific in the MVP
- **Save space budget**: SaveBlock3 has a maximum size of **1,624 bytes** (116 bytes × 14 sectors). With default config, only ~4 bytes are used (the `dexNavChain` field + padding).
- **Usable for events**: ~1,400 bytes after header and breathing room (~208 bytes reserved for future expansion features)
- **EWRAM budget**: 34,608 bytes free (not used in MVP, noted for future reference)

---

## Architecture

### High-Level

```
Game events (battle, catch, map change, etc.)
        │
        ▼
SaveLog_LogEvent(type, payload)
        │
        ▼
Ring buffer in SaveBlock3 (~1,400 bytes)
        │
        ▼
Persisted to flash on normal game save
        │
        ▼
Debug output via MgbaPrintf (MVP)
External parser reads .sav → outputs timeline (Phase 2)
```

### What Gets Logged

The system tracks events across three categories. All are enabled by default in the MVP.

**Progression events** (the backbone):
- Map entered (compact 10-bit map ID)
- Badge obtained (badge number, detected via FlagSet hook)
- Champion defeated
- Credits rolled
- Blackout / whiteout
- Story flags set (developer-configurable list of key flag IDs)

**Pokemon events**:
- Pokemon caught (species)
- Pokemon fainted (species)
- Pokemon evolved (result species)

**Battle events**:
- Battle started — wild (species)
- Battle started — trainer (trainer ID)
- Battle ended (result: win / loss / flee / catch)
- Item used in battle (item ID)
- Pokemon switched in (species)
- Item picked up overworld (item ID)
- Trainer defeated (trainer ID)

**Timing events** (injected automatically):
- Timestamp tick (coarse time reference)
- Extended timestamp (absolute minutes since run start)

---

## Variable-Width Event Encoding

Events use a prefix code where the first bits of byte 0 determine the record length. This is the core space optimization — the average event is ~1.77 bytes instead of 4.

### Encoding Format

```
1-byte events:  10xxxxxx
                 ││└────── 3-bit type + 3-bit payload
                 └──────── prefix: 10

2-byte events:  0xxxxxxx xxxxxxxx
                │└──────────────── 4-bit type + 11-bit payload
                └───────────────── prefix: 0

3-byte events:  11xxxxxx xxxxxxxx xxxxxxxx
                 ││└──────────────────────── 5-bit type + 17-bit payload
                 └──────────────────────────── prefix: 11
```

The 2-byte format gets the cheapest prefix (1 bit) because 2-byte events are the most common in a typical playthrough.

### Flat Event Type Enum

The API uses a single flat enum. The function determines encoding width from the type value:

```c
enum SaveLogEventType {
    // 1-byte events (0–7): prefix 10, 3-bit wire type, 3-bit payload
    SAVELOG_TIMESTAMP_TICK = 0,    // delta (×8 min increments, range 0–56 min)
    SAVELOG_BADGE_OBTAINED,        // badge_num (0–7)
    SAVELOG_BLACKOUT,              // zone_hash (coarse location, 0–7)
    SAVELOG_CHAMPION_DEFEATED,     // unused
    SAVELOG_CREDITS_ROLLED,        // unused
    SAVELOG_BATTLE_ENDED,          // result (0=win, 1=loss, 2=flee, 3=catch)
    SAVELOG_RESERVED_1B,           // reserved
    SAVELOG_NOP,                   // padding byte for wrap-around

    // 2-byte events (8–23): prefix 0, 4-bit wire type, 11-bit payload
    SAVELOG_TRAINER_DEFEATED = 8,  // trainer_id (0–2047)
    SAVELOG_POKEMON_CAUGHT,        // species (0–2047)
    SAVELOG_POKEMON_FAINTED,       // species (0–2047)
    SAVELOG_POKEMON_EVOLVED,       // result_species (0–2047)
    SAVELOG_ITEM_PICKED_UP,        // item_id (0–2047)
    SAVELOG_ITEM_USED_BATTLE,      // item_id (0–2047)
    SAVELOG_POKEMON_SWITCHED,      // species (0–2047)
    SAVELOG_MOVE_USED,             // move_id (0–2047)
    SAVELOG_BATTLE_STARTED_WILD,   // species (0–2047)
    SAVELOG_BATTLE_STARTED_TRAINER,// trainer_id (0–2047)
    SAVELOG_TIMESTAMP_EXTENDED,    // minutes_elapsed (0–2047, ~34 hrs)
    SAVELOG_MAP_ENTERED,           // compact_map_id (0–1023), 1 bit unused

    // 3-byte events (24–55): prefix 11, 5-bit wire type, 17-bit payload
    SAVELOG_FLAG_SET = 24,                // flag_id (0–65535), 1 bit unused
    SAVELOG_WARP_USED,                    // src_map(8) + dst_map(8), 1 bit unused
    SAVELOG_NPC_INTERACTION,              // script_id(9) + map_num(8)
    SAVELOG_POKEMON_FAINTED_DETAIL,       // species(11) + killer_idx(6)
    SAVELOG_POKEMON_CAUGHT_DETAIL,        // species(11) + ball(5) + unused(1)
    SAVELOG_SHOP_PURCHASE,                // item_id(11) + qty(6)
    SAVELOG_PC_TRANSACTION,               // species(11) + box(5) + direction(1)
};
```

Width mapping:
- `type < 8` → 1-byte encoding
- `type >= 8 && type < 24` → 2-byte encoding
- `type >= 24` → 3-byte encoding

Wire type derivation:
- 1-byte: `wire_type = type`
- 2-byte: `wire_type = type - 8`
- 3-byte: `wire_type = type - 24`

### Event Type Tables

#### 1-byte events (prefix `10`, 3-bit type = 8 types max, 3-bit payload)

| Wire Type | Enum                       | Payload (3 bits)                          |
|-----------|----------------------------|-------------------------------------------|
| 0         | SAVELOG_TIMESTAMP_TICK      | delta (×8 min increments, range 0–56 min) |
| 1         | SAVELOG_BADGE_OBTAINED      | badge_num (0–7)                           |
| 2         | SAVELOG_BLACKOUT            | zone_hash (coarse location, 0–7)          |
| 3         | SAVELOG_CHAMPION_DEFEATED   | unused                                    |
| 4         | SAVELOG_CREDITS_ROLLED      | unused                                    |
| 5         | SAVELOG_BATTLE_ENDED        | result (0=win, 1=loss, 2=flee, 3=catch)   |
| 6         | SAVELOG_RESERVED_1B         | reserved                                  |
| 7         | SAVELOG_NOP                 | ignored (wrap-around padding)             |

#### 2-byte events (prefix `0`, 4-bit type = 16 types max, 11-bit payload)

| Wire Type | Enum                         | Payload (11 bits)                      |
|-----------|------------------------------|----------------------------------------|
| 0         | SAVELOG_TRAINER_DEFEATED      | trainer_id (0–2047)                    |
| 1         | SAVELOG_POKEMON_CAUGHT        | species (0–2047)                       |
| 2         | SAVELOG_POKEMON_FAINTED       | species (0–2047)                       |
| 3         | SAVELOG_POKEMON_EVOLVED       | result_species (0–2047)                |
| 4         | SAVELOG_ITEM_PICKED_UP        | item_id (0–2047)                       |
| 5         | SAVELOG_ITEM_USED_BATTLE      | item_id (0–2047)                       |
| 6         | SAVELOG_POKEMON_SWITCHED      | species (0–2047)                       |
| 7         | SAVELOG_MOVE_USED             | move_id (0–2047)                       |
| 8         | SAVELOG_BATTLE_STARTED_WILD   | species (0–2047)                       |
| 9         | SAVELOG_BATTLE_STARTED_TRAINER| trainer_id (0–2047)                    |
| 10        | SAVELOG_TIMESTAMP_EXTENDED    | minutes_elapsed (0–2047, ~34 hrs)      |
| 11        | SAVELOG_MAP_ENTERED           | compact_map_id (0–1023), 1 bit unused  |
| 12–15     | (reserved)                   |                                        |

#### 3-byte events (prefix `11`, 5-bit type = 32 types max, 17-bit payload)

| Wire Type | Enum                            | Payload (17 bits)                          |
|-----------|---------------------------------|--------------------------------------------|
| 0         | SAVELOG_FLAG_SET                 | flag_id(16), 1 bit unused                  |
| 1         | SAVELOG_WARP_USED               | src_map(8) + dst_map(8), 1 bit unused      |
| 2         | SAVELOG_NPC_INTERACTION         | script_id(9) + map_num(8)                  |
| 3         | SAVELOG_POKEMON_FAINTED_DETAIL  | species(11) + killer_idx(6)                |
| 4         | SAVELOG_POKEMON_CAUGHT_DETAIL   | species(11) + ball(5) + unused(1)          |
| 5         | SAVELOG_SHOP_PURCHASE           | item_id(11) + qty(6)                       |
| 6         | SAVELOG_PC_TRANSACTION          | species(11) + box(5) + direction(1)        |
| 7–31      | (reserved)                     |                                            |

### Notes on the Encoding

- **Species IDs**: The expansion currently has ~1,573 species (`NUM_SPECIES`). 11 bits (0–2047) covers this with room for growth. If the expansion ever exceeds 2,047 species, the 3-byte detailed variants can be used instead (17-bit payload).
- **Trainer IDs**: 11 bits covers 2,048 trainers. The codebase has ~416 trainers, well within range.
- **Item IDs**: 874 items (`ITEMS_COUNT`). 11 bits is sufficient.
- **Move IDs**: 848 moves (`MOVES_COUNT`). 11 bits is sufficient.
- **Flag IDs**: Up to 2,400 flags (`FLAGS_COUNT`). The 3-byte flag event uses 16 bits (0–65,535), which is sufficient.
- **Map IDs**: 935 maps across 75 groups. The 2-byte map event uses a compact 10-bit sequential ID (0–1023) generated at build time. See [Compact Map ID Mapping](#compact-map-id-mapping).
- **Ball types**: 28 (`POKEBALL_COUNT`). The detailed catch event uses 5 bits (0–31), which is sufficient.
- **Timestamps**: The 1-byte tick covers gaps up to 56 minutes at 8-minute granularity. For gaps exceeding 56 minutes, the 2-byte extended timestamp is used instead (covers ~34 hours). The system auto-inserts a timestamp event when >8 minutes have elapsed since the last event.
- **NOP padding**: The 1-byte NOP event (type 7, `10111xxx`, bytes `0xB8–0xBF`) is used to fill dead bytes at the end of the buffer during wrap-around. The parser skips NOP events.
- **Reserved slots**: Intentionally left open for developers to add custom event types without changing the encoding scheme. 4 reserved 2-byte slots and 25 reserved 3-byte slots provide ample room.
- **Duplicate event types**: Some events appear in both 2-byte and 3-byte forms (e.g., Pokemon fainted vs Pokemon fainted detailed). The compact form logs just the species; the detailed form adds context. The config controls which form is used.

---

## Compact Map ID Mapping

Since the native map encoding is 16 bits (`map_group(8) + map_num(8)`) but most map events only need 10 bits (935 maps < 1024), a build-time mapping assigns each map a sequential compact ID.

### Build Script

A build script reads `data/maps/map_groups.json` and assigns sequential IDs 0–934:

```
LITTLEROOT_TOWN → 0
OLDALE_TOWN → 1
ROUTE_101 → 2
...
```

This generates:
1. **C header** (`src/data/savelog_map_ids.h`): a `const u16 gSaveLogMapId[MAP_GROUPS_COUNT][MAX_MAP_NUM]` lookup table (~1.9KB ROM), or a flat array indexed by the combined map ID.
2. **JSON mapping** (for the future Phase 2 parser): `{ "0": "LITTLEROOT_TOWN", "1": "OLDALE_TOWN", ... }`.

### ROM-Side Usage

```c
u16 compactId = SaveLog_GetCompactMapId(mapGroup, mapNum);
SaveLog_LogEvent(SAVELOG_MAP_ENTERED, compactId);
```

The lookup is a single array access — negligible cost.

### ROM Cost

935 entries × 2 bytes = ~1.9KB ROM. Trivial relative to the ROM's total size.

---

## Data Structures

### SaveBlock3 Layout

```c
// savelog.h

#define SAVELOG_VERSION        1
#define SAVELOG_BUFFER_SIZE    1400  // bytes, not event count

struct SaveLog {
    u16 writePos;                       // byte offset of next write position
    u16 totalEventsLo;                  // low 16 bits of total events written
    u16 lastTimestampMinutes;           // for delta calculation (RTC minutes)
    u16 runStartTimeHi;                 // upper bits of RTC snapshot at New Game
    u16 runStartTimeLo;                 // lower bits of RTC snapshot at New Game
    u8  version;                        // schema version for parser compat
    u8  flags;                          // bit 0: buffer wrapped, bits 1-7: reserved
    u8  totalEventsHi;                  // high 8 bits (24-bit total counter, max ~16M)
    u8  padding[3];                     // reserved, zero-init (explicit, no hidden padding)
    u8  buffer[SAVELOG_BUFFER_SIZE];    // ring buffer of variable-width events
};
// Header: 16 bytes (5×u16 + 3×u8 + 3 padding, naturally aligned)
// Total: 16 + 1,400 = 1,416 bytes
// Fits within 1,624 bytes of SB3 max with 208 bytes breathing room
```

Fields are ordered u16-first to avoid hidden alignment padding on ARM. This struct is added as a field on `struct SaveBlock3` in `include/global.h`.

### Ring Buffer Mechanics

- **Writing**: `writePos` is a byte offset into `buffer[]`. When a new event is written, its bytes are placed starting at `writePos`, then `writePos` advances by the event's width (1, 2, or 3).
- **Wrap handling**: If `writePos + eventWidth > SAVELOG_BUFFER_SIZE`, the remaining bytes (1 or 2) are filled with `SAVELOG_NOP` events (`0xB8`). Then `writePos` resets to 0, the `wrapped` flag is set, and the event is written at the start.
- **Reading**: If `wrapped` is false, read from `buffer[0]` to `buffer[writePos - 1]` sequentially. If `wrapped` is true, read from `buffer[writePos]` to end, then from `buffer[0]` to `buffer[writePos - 1]`. Skip any NOP events encountered (they are padding from wrap-around).

---

## Configuration

Provide a config header that lets developers toggle categories and control behavior:

```c
// include/config/savelog.h

#ifndef GUARD_CONFIG_SAVELOG_H
#define GUARD_CONFIG_SAVELOG_H

#define SAVELOG_ENABLE                  TRUE

// Event categories (all on by default for MVP)
#define SAVELOG_TRACK_MAPS             TRUE
#define SAVELOG_TRACK_BATTLES          TRUE
#define SAVELOG_TRACK_POKEMON          TRUE
#define SAVELOG_TRACK_ITEMS            TRUE
#define SAVELOG_TRACK_FLAGS            FALSE   // off by default, very high volume
#define SAVELOG_TRACK_NPCS             FALSE   // off by default, high volume
#define SAVELOG_TRACK_MOVES            FALSE   // off by default, extremely high volume

// Use detailed (3-byte) variants where available
#define SAVELOG_DETAILED_CATCHES       FALSE
#define SAVELOG_DETAILED_FAINTS        FALSE

// Developer-defined important flags to track even when TRACK_FLAGS is FALSE.
// These are logged regardless of the TRACK_FLAGS setting.
// Implemented as chained comparisons in FlagSet() for zero overhead when empty.
// Example:
// #define SAVELOG_IMPORTANT_FLAGS(id) \
//     ((id) == FLAG_DEFEATED_EVIL_TEAM_LEADER || (id) == FLAG_RECEIVED_STARTER)
// #define SAVELOG_IMPORTANT_FLAGS(id) FALSE  // default: no important flags

#endif // GUARD_CONFIG_SAVELOG_H
```

When `SAVELOG_ENABLE` is FALSE, all logging functions compile to no-ops (zero overhead).

The `SAVELOG_IMPORTANT_FLAGS` macro takes a flag ID and returns TRUE/FALSE. This is evaluated inline in `FlagSet()` — the compiler optimizes chained `==` comparisons to a jump table or binary search. Zero cost when defined as `FALSE`.

---

## Hook Points

These are the locations in the pokeemerald-expansion source where `SaveLog_LogEvent()` calls need to be inserted. Each hook should be guarded by the relevant config define.

### Map transitions
- **File**: `src/overworld.c`
- **Function**: Inside `LoadMapFromWarp()` or at step 12 of `LoadMapInStepsLocal()` (via `RunFieldCallback()` at line 1851)
- **Access**: `gSaveBlock1Ptr->location.mapGroup` and `gSaveBlock1Ptr->location.mapNum`
- **Event**: 2-byte map entered (compact 10-bit map ID via `SaveLog_GetCompactMapId()`)
- **Config guard**: `SAVELOG_TRACK_MAPS`

### Battle start (wild)
- **File**: `src/battle_setup.c`
- **Function**: `BattleSetup_StartWildBattle()` (line 330) or `DoStandardWildBattle()` (line 350)
- **Access**: `GetMonData(&gEnemyParty[0], MON_DATA_SPECIES)`
- **Event**: 2-byte battle started wild (species)
- **Config guard**: `SAVELOG_TRACK_BATTLES`

### Battle start (trainer)
- **File**: `src/battle_setup.c`
- **Function**: `BattleSetup_StartTrainerBattle()` (line 1300)
- **Access**: `TRAINER_BATTLE_PARAM.opponentA`
- **Event**: 2-byte battle started trainer (trainer ID)
- **Config guard**: `SAVELOG_TRACK_BATTLES`

### Battle end — win
- **File**: `src/battle_main.c`
- **Function**: `HandleEndTurn_BattleWon()` (line 5397)
- **Event**: 1-byte battle ended (result = 0, win)
- **Config guard**: `SAVELOG_TRACK_BATTLES`

### Battle end — loss
- **File**: `src/battle_main.c`
- **Function**: `HandleEndTurn_BattleLost()` (line 5455)
- **Event**: 1-byte battle ended (result = 1, loss)
- **Config guard**: `SAVELOG_TRACK_BATTLES`

### Battle end — flee
- **File**: `src/battle_util.c`
- **Function**: `HandleAction_Run()` (line 641), when `TryRunFromBattle()` returns TRUE
- **Event**: 1-byte battle ended (result = 2, flee)
- **Config guard**: `SAVELOG_TRACK_BATTLES`

### Battle end — catch
- **File**: `src/battle_script_commands.c`
- **Function**: `FinalizeCapture()` (line 10524)
- **Event**: 1-byte battle ended (result = 3, catch)
- **Config guard**: `SAVELOG_TRACK_BATTLES`

### Pokemon caught
- **File**: `src/battle_script_commands.c`
- **Function**: `FinalizeCapture()` (line 10524), after catch is confirmed
- **Access**: Species from caught mon, ball from `ItemIdToBallId(gLastUsedItem)`
- **Event**: 2-byte Pokemon caught (species), or 3-byte detailed if `SAVELOG_DETAILED_CATCHES` (species + ball type)
- **Config guard**: `SAVELOG_TRACK_POKEMON`

### Pokemon fainted
- **File**: `src/battle_move_resolution.c`
- **Function**: `MoveEndFaintBlock()` (line 2544), where `gBattlerFainted` is set
- **Access**: Species from `gBattleMons[gBattlerFainted].species`
- **Event**: 2-byte Pokemon fainted (species), or 3-byte detailed if `SAVELOG_DETAILED_FAINTS`
- **Config guard**: `SAVELOG_TRACK_POKEMON`

### Pokemon evolved
- **File**: `src/evolution_scene.c`
- **Function**: `Task_EvolutionScene()`, `EVOSTATE_END` case (line 826)
- **Guard**: Only log when `!gTasks[taskId].tEvoWasStopped` (cancelled evolutions must NOT be logged)
- **Access**: `gTasks[taskId].tPostEvoSpecies`
- **Event**: 2-byte Pokemon evolved (new species)
- **Config guard**: `SAVELOG_TRACK_POKEMON`

### Trainer defeated
- **File**: `src/battle_setup.c`
- **Function**: `SetBattledTrainersFlags()` (line 1273)
- **Access**: `GetTrainerAFlag()` → trainer ID from `TRAINER_BATTLE_PARAM.opponentA`
- **Event**: 2-byte trainer defeated (trainer ID)
- **Config guard**: `SAVELOG_TRACK_BATTLES`

### Badge obtained + Important flags (via FlagSet hook)
- **File**: `src/event_data.c`
- **Function**: `FlagSet()` (line 241)
- **Badge detection**: If `id >= FLAG_BADGE01_GET && id <= FLAG_BADGE08_GET`, emit 1-byte badge obtained (`id - FLAG_BADGE01_GET`). Always on — core progression marker.
- **Important flag detection**: If `SAVELOG_TRACK_FLAGS` is TRUE, log all flags as 3-byte events. Otherwise, if `SAVELOG_IMPORTANT_FLAGS(id)` returns TRUE, log that specific flag.
- **Event**: 1-byte badge obtained OR 3-byte flag set

### Blackout / whiteout
- **File**: `src/overworld.c`
- **Function**: `DoWhiteOut()` (line 390)
- **Event**: 1-byte blackout
- **Config guard**: Always on

### Item picked up (overworld)
- **File**: `src/item.c`
- **Function**: `AddBagItem()` (line 346)
- **Event**: 2-byte item picked up (item ID)
- **Config guard**: `SAVELOG_TRACK_ITEMS`
- **Note**: This captures all item additions including NPC gifts and shop purchases, not just overworld item balls. Consider filtering by context if this is too noisy.

### Item used in battle
- **File**: `src/battle_util.c`
- **Function**: `HandleAction_UseItem()` (line 533)
- **Access**: `gLastUsedItem`
- **Event**: 2-byte item used in battle (item ID)
- **Config guard**: `SAVELOG_TRACK_ITEMS`

### Pokemon switched in battle
- **File**: `src/battle_util.c`
- **Function**: `HandleAction_Switch()` (line 520)
- **Access**: Species from `gBattleMons` or party data using `gBattleStruct->monToSwitchIntoId[gBattlerAttacker]`
- **Event**: 2-byte Pokemon switched (species)
- **Config guard**: `SAVELOG_TRACK_BATTLES`

### Timestamps
- **Automatic**: Inside `SaveLog_LogEvent()`, before writing any event, check if >8 minutes have elapsed since the last logged timestamp (using `RtcGetMinuteCount()` from `gba/isagbprint.h`... actually `src/rtc.c`). If the gap is ≤56 minutes, auto-insert a 1-byte timestamp tick. If the gap exceeds 56 minutes, auto-insert a 2-byte extended timestamp instead.

---

## Core API

The implementation should expose a minimal API. All functions are internal — no player-facing UI in the MVP.

```c
// savelog.h / savelog.c

// Initialize the log (called on New Game, AFTER ClearSav3())
void SaveLog_Init(void);

// Log an event. Uses the flat enum to determine encoding width.
// This is the single entry point that all hooks call.
void SaveLog_LogEvent(u8 type, u32 payload);

// Called on save load to validate the log version and handle migration.
void SaveLog_OnLoad(void);

// Convert (mapGroup, mapNum) to compact 10-bit ID for map events.
u16 SaveLog_GetCompactMapId(u8 mapGroup, u8 mapNum);

// Dump the last `count` events to mGBA console (debug validation).
void SaveLog_DumpToConsole(u8 count);
```

`SaveLog_LogEvent` should:
1. Check if `SAVELOG_ENABLE` is TRUE (compile-time, so this is free).
2. Check if enough time has elapsed to auto-insert a timestamp.
3. Determine encoding width from the type value (`< 8` → 1-byte, `< 24` → 2-byte, else → 3-byte).
4. Check if `writePos + eventWidth > SAVELOG_BUFFER_SIZE`. If so, fill remaining bytes with NOP (`0xB8`), set `wrapped` flag, reset `writePos` to 0.
5. Encode the event bytes using the prefix code and wire type.
6. Write the bytes to `gSaveBlock3Ptr->saveLog.buffer` at `writePos`.
7. Advance `writePos` by the event width.
8. Increment the 24-bit total event counter.

This function is called during gameplay and must be fast. No dynamic allocation, no string formatting, no flash writes. It's a simple memcpy into the SaveBlock3 struct in RAM. The data persists to flash on the next normal game save.

---

## Debug Output (MVP Validation)

Instead of a full parser, the MVP provides a debug function for validating the ring buffer via mGBA's console.

### `SaveLog_DumpToConsole(u8 count)`

Prints the last `count` events to mGBA console using `MgbaPrintf()` (from `gba/isagbprint.h`). Format: one event per line, raw type + payload in hex:

```
SaveLog v1 — 847 events (wrapped)
[0] 13:MAP 002A
[1] 05:BATTLE_END 00
[2] 08:WILD 0107
[3] 00:TICK 03
...
```

Human-readable name resolution (species names, map names, etc.) is a parser concern, not MVP. The hex dump is sufficient to verify encoding, wrap-around, and event ordering.

This function should be callable via a registered special (`def_special SaveLog_DumpToConsole`) for easy testing from scripts or the debug menu.

---

## Save Integration

### New Game
Call `SaveLog_Init()` during the New Game flow in `src/new_game.c`, inside `NewGameInitData()`. **Critical: must be called AFTER `ClearSav3()` (line 178)**, or the initialization will be wiped. `SaveLog_Init()` zero-initializes the `SaveLog` struct, sets the `version` field, and captures `RtcGetMinuteCount()` as the run start time.

### Save Load
Call `SaveLog_OnLoad()` during save loading in `src/load_save.c`. This checks:
1. If `version` is 0 or doesn't match `SAVELOG_VERSION` → zero-init the struct and set version (migration for pre-SaveLog saves).
2. If `version` matches → do nothing, the log is already valid.

This ensures existing saves are not corrupted. On first load after the feature is added, the log is cleanly initialized as empty.

### Save Write
No special handling. The SaveLog struct is part of SaveBlock3, which is distributed across 14 flash sectors (116 bytes per sector) as part of the normal save flow. The save system handles chunking automatically — no additional flash writes needed.

---

## File Structure

New files to add:

```
include/
  config/
    savelog.h              ← developer config (enable/disable categories)
  savelog.h                ← public API (Init, LogEvent, OnLoad, etc.) + struct + enum
src/
  savelog.c                ← implementation (encoding, ring buffer, timestamp, debug dump)
  data/
    savelog_map_ids.h       ← generated compact map ID lookup table
tools/
  savelog_mapgen.py         ← build script: reads map_groups.json, emits savelog_map_ids.h
```

Modified files:

```
include/global.h           ← add struct SaveLog field to struct SaveBlock3
src/new_game.c             ← call SaveLog_Init() after ClearSav3()
src/load_save.c            ← call SaveLog_OnLoad()
src/overworld.c            ← hooks: map transitions, blackout (DoWhiteOut)
src/battle_setup.c         ← hooks: battle start (wild + trainer), trainer defeated
src/battle_main.c          ← hooks: battle won, battle lost
src/battle_util.c          ← hooks: flee, item used in battle, pokemon switched
src/battle_script_commands.c ← hooks: catch (FinalizeCapture)
src/battle_move_resolution.c ← hook: pokemon fainted (MoveEndFaintBlock)
src/evolution_scene.c      ← hook: evolution complete (guarded by tEvoWasStopped)
src/event_data.c           ← hook: FlagSet() for badges + important flags
src/item.c                 ← hook: item pickup (AddBagItem)
data/specials.inc          ← register SaveLog_DumpToConsole as special
```

---

## What Is NOT in Scope (MVP)

These are explicitly deferred to future phases:

- **External `.sav` parser** — deferred to Phase 2 (see appendix)
- **Data export JSON** — deferred to Phase 2 (needed by parser)
- **Competition mode**: integrity hashing, EWRAM buffering, Lua sidecar file, on-screen verification codes, hash chains, secret salts
- **Absorbing Save B**: event sourcing, checkpoint-based recovery, save system modifications
- **PokemonStorage split buffer**: using the 1,568 free bytes in PokemonStorage for additional event space
- **Move tracking**: the `SAVELOG_TRACK_MOVES` config exists but is off by default; at ~6,000 events per playthrough it would fill the ring buffer almost immediately
- **Player-facing UI**: no in-game log viewer (debug menu integration via MgbaPrintf is the MVP approach)
- **Web-based parser**: CLI only when parser ships

---

## Estimated Event Capacity

Based on variable-width encoding with map events now at 2 bytes (compact IDs):

| Buffer size  | Avg bytes/event | Events (approx) | Coverage                                     |
|-------------|-----------------|------------------|-----------------------------------------------|
| 1,400 bytes | ~1.77           | ~790 events      | Last ~790 actions as ring buffer              |

Typical playthrough event mix estimate:
- Map transitions: ~200 × 2 bytes = 400
- Battle started: ~150 × 2 bytes = 300
- Battle ended: ~150 × 1 byte = 150
- Pokemon caught: ~30 × 2 bytes = 60
- Pokemon fainted: ~50 × 2 bytes = 100
- Trainer defeated: ~80 × 2 bytes = 160
- Badge obtained: ~8 × 1 byte = 8
- Blackout: ~5 × 1 byte = 5
- Item picked up: ~50 × 2 bytes = 100
- Timestamps: ~30 × ~1.5 bytes avg = 45
- Misc: ~20 × 2 bytes = 40

**Total: ~773 events, ~1,368 bytes → ~1.77 bytes/event**

The 790-event ring buffer will retain the majority of a full playthrough's core events. For bug reporting (where the last 50–100 events are the most valuable), this is more than sufficient.

---

## Success Criteria

1. A developer can add SaveLog to their pokeemerald-expansion project by merging the feature branch.
2. With zero configuration, core events (maps, battles, catches, badges, blackouts) are automatically logged to SaveBlock3.
3. Existing saves are not corrupted — the migration path cleanly initializes the log on first load.
4. The ring buffer does not cause any frame drops, save lag, or gameplay-visible side effects.
5. `SaveLog_DumpToConsole()` correctly outputs the last N events in chronological order via MgbaPrintf.
6. The struct fits within SaveBlock3's free space with ≥200 bytes breathing room for future expansion.
7. NOP padding correctly fills dead bytes on wrap-around; no garbage decoding at the seam.
8. The compact map ID build script generates correct mappings from `map_groups.json`.

---

## Appendix: Phase 2 — External Parser (Out of Scope)

The following is deferred to a separate PRD and implementation phase.

### Parser Overview

A Node.js CLI tool that reads a `.sav` file and outputs the event timeline.

### Save File Sector Layout

The parser must understand the GBA save format:
- A `.sav` file contains 32 flash sectors of 4,096 bytes each (128KB total).
- Sectors 0–13 form Save Slot 1; sectors 14–27 form Save Slot 2.
- Each sector contains: 3,968 bytes main data + 116 bytes SaveBlock3 chunk + 12 bytes footer (checksum, sector ID, signature, counter).
- SaveBlock3 is distributed across all 14 sectors in a slot: the parser must extract the 116-byte chunk from each sector and concatenate them to reassemble the full SaveBlock3 struct (up to 1,624 bytes).
- The `SaveLog` struct is located at a fixed offset within the reassembled SaveBlock3 (after any preceding fields like `dexNavChain`).

### Parser Requirements

1. Read the `.sav` file and reassemble SaveBlock3 from 14 sector chunks.
2. Locate the SaveLog struct at the correct offset.
3. Decode variable-width events using the prefix code.
4. Handle ring buffer wrap-around — read from `writePos` forward, skip NOP events.
5. Resolve IDs to names using a JSON mapping file (species, trainers, maps, items, flags).
6. Output a human-readable timeline to stdout.
7. Optionally export JSON for programmatic consumption.

### Data Export

A build script reads the expansion's header files and emits a JSON mapping:

```json
{
  "species": { "252": "Treecko", "263": "Zigzagoon" },
  "trainers": { "261": "Leader Roxanne" },
  "maps": { "0": "Littleroot Town", "1": "Oldale Town" },
  "items": { "13": "Potion" },
  "flags": { "2048": "FLAG_BADGE01_GET" }
}
```

The parser accepts this as a `--data` argument.
