# SaveLog Viewer — Product Requirements Document

## Overview

SaveLog Viewer is a client-side web application that parses `.sav` files from pokeemerald-expansion ROM hacks and displays a human-readable timeline of SaveLog events. Developers receive a `.sav` file from a bug report, drop it into the viewer, and get a chronological event history for debugging.

**Stack**: Vue 3 + Vite + TypeScript + shadcn-vue + Tailwind CSS
**Deployment**: Static site on GitHub Pages (`moostoet.github.io/savelog-viewer`)
**Repository**: `savelog-viewer` (separate from the ROM hack repo)

---

## Problem

The SaveLog system (Phase 1) writes gameplay events to a ring buffer in SaveBlock3. But there's no way to read the data outside of the debug menu's raw hex dump via mGBA console. Developers need a tool that extracts and visualizes the event timeline from a `.sav` file in a format that's actually useful for diagnosing bugs.

## Solution

A browser-based viewer that:
1. Accepts a `.sav` file via drag-and-drop or file picker (no server upload — fully client-side)
2. Locates and validates SaveBlock3 within the GBA save sector layout
3. Decodes variable-width events from the SaveLog ring buffer
4. Resolves numeric IDs to human-readable names (species, maps, items, trainers, flags)
5. Displays an interactive timeline with filtering, a summary dashboard, and a raw hex view

---

## Architecture

### High-Level

```
User drops .sav file
        |
        v
FileReader reads binary into ArrayBuffer
        |
        v
Save parser: locate active slot, validate sectors, extract SaveBlock3
        |
        v
SaveLog decoder: find SaveLog struct, decode ring buffer events
        |
        v
ID resolver: map numeric IDs to names using mapping JSON
        |
        v
Vue components render Timeline / Summary / Raw tabs
```

### Key Design Decisions

- **No backend**: All parsing happens in the browser. No data leaves the user's machine.
- **Endianness**: GBA saves are little-endian. All reads use `DataView` with `littleEndian = true`.
- **Encoding constants**: Hardcoded in TypeScript, versioned by `SAVELOG_VERSION`. If the encoding changes, the version bumps and a new decoder is added alongside the old one.
- **ID mapping**: Ships with a default mapping for vanilla pokeemerald-expansion. Users can upload a custom mapping JSON for their ROM hack.
- **Persistence**: Custom mappings and filter preferences stored in `localStorage`. The `.sav` file is never persisted.

---

## GBA Save File Format

### Sector Layout

A `.sav` file is **128KB** (131,072 bytes) containing **32 sectors** of 4,096 bytes each.

| Sectors | Purpose |
|---------|---------|
| 0-13 | Save Slot 1 |
| 14-27 | Save Slot 2 |
| 28-31 | Hall of Fame / Trainer Hill / Recorded Battle |

Each sector has this byte layout:

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0x0000 | 3,968 | `data` | Main sector data (SaveBlock1, SaveBlock2, or PokemonStorage) |
| 0x0F80 | 116 | `saveBlock3Chunk` | SaveBlock3 fragment |
| 0x0FF4 | 2 | `id` | Sector ID within the slot (0-13), little-endian u16 |
| 0x0FF6 | 2 | `checksum` | CRC of `data` bytes only (0x0000-0x0F7F), little-endian u16 |
| 0x0FF8 | 4 | `signature` | Must equal `0x08012025` for a valid sector, little-endian u32 |
| 0x0FFC | 4 | `counter` | Save counter (increments each save), little-endian u32 |

### Checksum Algorithm

```typescript
function calculateChecksum(data: DataView, offset: number, size: number): number {
  let checksum = 0;
  for (let i = 0; i < size; i += 4) {
    checksum += data.getUint32(offset + i, true);
    checksum = checksum >>> 0; // keep as u32
  }
  return ((checksum >>> 16) + (checksum & 0xFFFF)) & 0xFFFF;
}
```

Checksum is computed over the first 3,968 bytes of each sector (`data` field only, NOT the saveBlock3Chunk or footer).

### Active Slot Detection

1. For each slot (sectors 0-13 and 14-27):
   a. Read each sector's signature, checksum, and counter
   b. Validate: `signature === 0x08012025` AND checksum matches calculated value
   c. Collect valid sectors and their counters
2. A slot is valid if all 14 sectors pass validation
3. If both slots are valid: the slot with the **higher counter** is the active one
4. Counter wraparound: handle `0xFFFFFFFF` → `0` transition (compare with unsigned arithmetic)
5. If only one slot is valid: use that slot
6. If neither is valid: show an error

Display validation status in the UI: "Slot 2 active, all sectors valid" or "Warning: 2 sectors failed checksum."

### SaveBlock3 Reassembly

SaveBlock3 is distributed across all 14 sectors of the active slot, 116 bytes per sector:

```
SaveBlock3 bytes [0..115]    = Sector with id=0,  offset 0x0F80
SaveBlock3 bytes [116..231]  = Sector with id=1,  offset 0x0F80
SaveBlock3 bytes [232..347]  = Sector with id=2,  offset 0x0F80
...
SaveBlock3 bytes [1508..1623] = Sector with id=13, offset 0x0F80
```

**Important**: Sectors may not be in physical order within the slot. Use the `id` field from each sector's footer to determine which chunk it provides, not its position in the file.

Total reassembled size: up to 1,624 bytes.

---

## SaveLog Struct Location

### Offset Within SaveBlock3

The SaveLog struct's position depends on compile-time config options that precede it in `struct SaveBlock3`:

```c
struct SaveBlock3 {
    // Optional fields (config-dependent):
    // OW_USE_FAKE_RTC:           +10 bytes
    // FNPC_ENABLE_NPC_FOLLOWERS: +~24 bytes
    // OW_SHOW_ITEM_DESCRIPTIONS: +~110 bytes
    // USE_DEXNAV_SEARCH_LEVELS:  +~1573 bytes
    u8 dexNavChain;               // always 1 byte
    // APRICORN_TREE_COUNT > 0:   +variable
    struct SaveLog saveLog;       // <-- target
};
```

With **vanilla pokeemerald-expansion defaults** (all optional fields disabled), only `dexNavChain` (1 byte) precedes SaveLog. With 2-byte alignment for the first `u16` field, SaveLog starts at **offset 2**.

The offset is included in the mapping JSON as `saveLogOffset`. Default mapping ships with `saveLogOffset: 2`. Custom mappings override this for non-vanilla configs.

### SaveLog Struct Layout (16-byte header + 1,400-byte buffer)

| Offset | Size | Field | Description |
|--------|------|-------|-------------|
| 0 | 2 | `writePos` | Byte offset of next write position in buffer |
| 2 | 2 | `totalEventsLo` | Low 16 bits of total events written |
| 4 | 2 | `lastTimestampMinutes` | Last logged RTC minute count |
| 6 | 2 | `runStartTimeHi` | Upper 16 bits of RTC at New Game |
| 8 | 2 | `runStartTimeLo` | Lower 16 bits of RTC at New Game |
| 10 | 1 | `version` | Schema version (must be 1) |
| 11 | 1 | `flags` | Bit 0: buffer wrapped |
| 12 | 1 | `totalEventsHi` | High 8 bits of total event counter |
| 13 | 3 | `padding` | Reserved, zero |
| 16 | 1,400 | `buffer` | Ring buffer of variable-width events |

**Validation**: If `version !== 1`, the save file likely doesn't have SaveLog enabled. Show an error rather than decoding garbage.

---

## Event Decoding

### Variable-Width Prefix Code

```
Byte 0 bits [7:6]:
  0x  (bit 7 = 0)  -> 2-byte event
  10  (bits 7:6)    -> 1-byte event
  11  (bits 7:6)    -> 3-byte event
```

### Decoding Algorithm

```typescript
function decodeEvent(buffer: Uint8Array, pos: number): { type: number, payload: number, width: number } {
  const byte0 = buffer[pos];

  if ((byte0 & 0x80) === 0) {
    // 2-byte: 0TTTTPPP PPPPPPPP
    const encoded = (byte0 << 8) | buffer[(pos + 1) % BUFFER_SIZE];
    const wireType = (encoded >> 11) & 0xF;
    return { type: 8 + wireType, payload: encoded & 0x7FF, width: 2 };
  } else if ((byte0 & 0xC0) === 0x80) {
    // 1-byte: 10TTTPPP
    const wireType = (byte0 >> 3) & 0x7;
    return { type: wireType, payload: byte0 & 0x7, width: 1 };
  } else {
    // 3-byte: 11TTTTTP PPPPPPPP PPPPPPPP
    const encoded = ((byte0 & 0x3F) << 16)
                  | (buffer[(pos + 1) % BUFFER_SIZE] << 8)
                  | buffer[(pos + 2) % BUFFER_SIZE];
    const wireType = (encoded >> 17) & 0x1F;
    return { type: 24 + wireType, payload: encoded & 0x1FFFF, width: 3 };
  }
}
```

### Flat Event Type Enum

```typescript
enum SaveLogEvent {
  // 1-byte (0-7)
  TIMESTAMP_TICK = 0,
  BADGE_OBTAINED = 1,
  BLACKOUT = 2,
  CHAMPION_DEFEATED = 3,
  CREDITS_ROLLED = 4,
  BATTLE_ENDED = 5,
  RESERVED_1B = 6,
  NOP = 7,

  // 2-byte (8-23)
  TRAINER_DEFEATED = 8,
  POKEMON_CAUGHT = 9,
  POKEMON_FAINTED = 10,
  POKEMON_EVOLVED = 11,
  ITEM_PICKED_UP = 12,
  ITEM_USED_BATTLE = 13,
  POKEMON_SWITCHED = 14,
  MOVE_USED = 15,
  BATTLE_STARTED_WILD = 16,
  BATTLE_STARTED_TRAINER = 17,
  TIMESTAMP_EXTENDED = 18,
  MAP_ENTERED = 19,

  // 3-byte (24-55)
  FLAG_SET = 24,
  WARP_USED = 25,
  NPC_INTERACTION = 26,
  POKEMON_FAINTED_DETAIL = 27,
  POKEMON_CAUGHT_DETAIL = 28,
  SHOP_PURCHASE = 29,
  PC_TRANSACTION = 30,
}
```

### Ring Buffer Reading Order

1. If `flags & 0x01 === 0` (not wrapped): read `buffer[0]` to `buffer[writePos - 1]`
2. If `flags & 0x01 === 1` (wrapped): read `buffer[writePos]` to end, then `buffer[0]` to `buffer[writePos - 1]`
3. Skip NOP events (type 7) — these are wrap-around padding

---

## ID Mapping

### Default Mapping

Ships bundled in the app, generated from vanilla pokeemerald-expansion data. Format:

```json
{
  "saveLogOffset": 2,
  "species": { "1": "Bulbasaur", "2": "Ivysaur", ... },
  "maps": { "0": "Petalburg City", "1": "Slateport City", "9": "Littleroot Town", ... },
  "items": { "1": "Master Ball", "2": "Ultra Ball", ... },
  "trainers": { "1": "Trainer Name", ... },
  "flags": { "2055": "FLAG_BADGE01_GET", ... },
  "battleResults": { "0": "Win", "1": "Loss", "2": "Flee", "3": "Catch" },
  "badges": ["Stone Badge", "Knuckle Badge", "Dynamo Badge", "Heat Badge",
             "Balance Badge", "Feather Badge", "Mind Badge", "Rain Badge"]
}
```

### Custom Mapping Override

Users can upload a custom mapping JSON via a "Load mapping" button. The custom mapping is merged over the default (partial overrides are fine). Stored in `localStorage` so it persists across sessions.

### Mapping Generation (Separate Tooling)

A Python script (`tools/generate_savelog_mapping.py`) in the pokeemerald-expansion repo reads:
- `data/maps/map_groups.json` → compact map IDs and names
- Species constants → species names
- Item constants → item names
- Trainer data → trainer names
- Flag constants → flag names

And emits a `savelog_mapping.json`. This is NOT part of the viewer repo — it lives with the ROM hack. The viewer ships with a pre-generated default mapping.

---

## UI Design

### Landing State (No File Loaded)

A centered drag-and-drop zone with:
- Drop zone icon and "Drop your .sav file here" text
- "Browse..." button as fallback
- Brief description: "Parse SaveLog events from pokeemerald-expansion save files"

### Loaded State

Header bar with:
- File name and save slot info ("save.sav — Slot 2, all sectors valid")
- "Load another file" button
- "Load custom mapping" button

Tabs below:
- **Timeline** (default)
- **Summary**
- **Raw**

### Timeline Tab

A vertical scrollable list of event entries. Each entry has:
- **Colored left border** indicating category (see color scheme below)
- **Event name** (e.g., "Map Entered", "Battle Started (Wild)")
- **Payload detail** (e.g., "Littleroot Town", "Zigzagoon (#263)")
- **Event index** (sequential number)

**Timestamp separators**: Timestamp events render as thin horizontal bars spanning the full width, showing approximate elapsed time: `~ 0:48 into playthrough`. Regular events flow between them.

**Category filters**: A row of toggle buttons/checkboxes above the timeline to show/hide event categories. Filter state persists in `localStorage`.

### Category Color Scheme

| Category | Events | Color | Tailwind Class |
|----------|--------|-------|----------------|
| Map | Map Entered | Blue | `blue-500` |
| Battle | Battle Started, Battle Ended, Trainer Defeated, Pokemon Switched | Red | `red-500` |
| Pokemon | Pokemon Caught, Fainted, Evolved | Green | `emerald-500` |
| Items | Item Picked Up, Item Used in Battle | Amber | `amber-500` |
| Progression | Badge Obtained, Champion Defeated, Credits Rolled, Blackout, Flag Set | Purple | `violet-500` |
| Timing | Timestamp Tick, Timestamp Extended | Gray | `zinc-400` |

### Summary Tab

At-a-glance dashboard cards:

- **Event Stats**: Total events, buffer utilization (e.g., "423 of ~790 capacity"), wrapped status
- **Play Time**: Estimated from run start + last timestamp
- **Badges**: 0-8 visual indicator (filled/unfilled badge icons or dots)
- **Maps Visited**: Unique count, top 5 most visited
- **Battle Stats**: Total battles, wins, losses, flees, catches
- **Pokemon Stats**: Total caught, total fainted, total evolved
- **Last 5 Events**: Quick preview cards

### Raw Tab

Two sections:

1. **SaveLog Header**: Key-value table of all struct fields (version, flags, writePos, totalEvents, timestamps, etc.)
2. **Hex Dump**: The 1,400-byte ring buffer displayed as hex bytes, with each event's bytes color-highlighted by category. Hover tooltip shows the decoded event for that byte range. Current writePos marked with an indicator.

### Dark Mode

Respects system preference via `prefers-color-scheme`. No manual toggle. shadcn-vue's built-in dark mode theming handles this automatically via Tailwind's `dark:` variant.

---

## File Validation & Error Handling

| Check | Condition | User Message |
|-------|-----------|-------------|
| File size | < 131,072 bytes | "File is too small to be a valid GBA save file (expected 128KB)." |
| File size | > 131,072 bytes | Proceed, read only first 128KB. Show info note. |
| Sector signature | No valid sectors found | "No valid save data found. This may not be a GBA save file." |
| Slot detection | Neither slot fully valid | "Save data appears corrupt. Attempting partial read..." |
| SaveLog version | `version !== 1` | "No SaveLog data found. This save may be from a ROM without SaveLog, or the offset may be incorrect." |
| Mapping file | Invalid JSON or missing fields | "Custom mapping is invalid. Using default mapping." |

---

## File Structure (Repo)

```
savelog-viewer/
  public/
    favicon.ico
  src/
    assets/
      default-mapping.json       # Pre-generated from vanilla pokeemerald-expansion
    lib/
      save-parser.ts             # Sector reading, validation, slot detection, SB3 reassembly
      savelog-decoder.ts          # Ring buffer decoding, event type constants
      id-resolver.ts             # Numeric ID -> name resolution using mapping
      types.ts                   # Shared TypeScript types/interfaces
    components/
      FileDropZone.vue           # Drag-and-drop + file picker
      TimelineView.vue           # Event list with category colors + filters
      SummaryView.vue            # Dashboard cards with stats
      RawView.vue                # Header table + hex dump
      TimelineSeparator.vue      # Timestamp separator bar
      TimelineEntry.vue          # Single event row
      CategoryFilter.vue         # Filter toggle buttons
    App.vue                      # Main layout, tab routing, state management
    main.ts                      # Vue app entry point
  index.html
  package.json
  tsconfig.json
  vite.config.ts
  tailwind.config.ts
  components.json                # shadcn-vue config
  .github/
    workflows/
      deploy.yml                 # Build + deploy to GitHub Pages
```

---

## Success Criteria

1. User can drop a `.sav` file and see a decoded event timeline within 1 second.
2. Events display human-readable names for species, maps, items, and trainers.
3. Category filters allow hiding/showing event types.
4. Summary tab provides useful at-a-glance stats for bug triage.
5. Raw tab shows the hex dump with per-event color highlighting.
6. Custom mapping JSON can be uploaded and persists across sessions.
7. Save files without SaveLog produce a clear, non-scary error message.
8. Works in Chrome, Firefox, and Safari (latest versions).
9. Dark mode respects system preference automatically.
10. Deployed to GitHub Pages and accessible via URL.
