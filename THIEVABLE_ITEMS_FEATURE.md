# Thievable Items NPC Feature

## Overview
This feature implements an NPC that gives players free items based on the wild Pokemon they've encountered on the current route. The items are those that wild Pokemon can hold (itemCommon and itemRare from gSpeciesInfo).

## Files Modified

### C Code
- **src/field_specials.c** - Added 6 special functions for the feature
- **include/field_specials.h** - Added function declarations
- **data/specials.inc** - Registered the special functions

### Scripts
- **data/scripts/thievable_items.inc** - Contains the NPC event script and dialogue

## Implementation Details

### Special Functions

1. **Special_CheckPartyHasCoveOrThief()**
   - Checks if any party Pokemon (non-eggs) knows MOVE_COVET or MOVE_THIEF
   - Returns result in gSpecialVar_Result (TRUE/FALSE)

2. **Special_GetRouteThievableItems()**
   - Gets current map's wild encounter header
   - Iterates through all encounter types (land, water, fishing, rock smash, hidden)
   - Checks all times of day (morning, day, evening, night)
   - For each species, checks if player has SEEN it in Pokedex
   - Collects unique items (itemCommon and itemRare)
   - Stores up to 32 unique items in static buffer
   - Returns count in gSpecialVar_Result

3. **Special_ShowThievableItemsMenu()**
   - Builds a dynamic multichoice menu from collected items
   - Allocates memory for item names
   - Pushes items to the dynamic multichoice stack

4. **Special_HandleThievableItemSelection()**
   - Processes the menu selection result
   - Stores selected item ID in gSpecialVar_0x8004
   - Returns TRUE/FALSE in gSpecialVar_Result
   - Cleans up dynamic multichoice stack

5. **Special_ThievableItems_StartQuantitySelection()**
   - Creates a task for quantity input
   - Uses AdjustQuantityAccordingToDPadInput() for D-pad control
   - Caps at 99 items per request
   - Checks bag space and adjusts max quantity accordingly
   - Stores quantity in gSpecialVar_0x8005
   - Returns TRUE on confirmation, FALSE on cancel

6. **Special_ThievableItems_GiveItems()**
   - Validates bag space for selected item and quantity
   - Uses AddBagItem() to give items to player
   - Returns TRUE on success, FALSE if bag full

### Event Script Flow

```
EventScript_ThievableItemsGuide:
1. Lock and face player
2. Show greeting message
3. Check for Covet/Thief move
   - If not present -> Show "need move" message and exit
4. Get route thievable items
   - If count == 0 -> Show "explore first" message and exit
5. Show item selection menu
6. Handle selection
   - If cancelled -> Show cancelled message and exit
7. Show quantity selection
   - If cancelled -> Show cancelled message and exit
8. Give items
   - If bag full -> Show bag full message and exit
9. Show received message
10. Release and end
```

## How to Use in Maps

### 1. Add the Script Include
In your map's script file, include:
```assembly
.include "data/scripts/thievable_items.inc"
```

### 2. Create NPC Object Event
Add an object event to your map with:
```
script: EventScript_ThievableItemsGuide
```

### Example Usage in Porymap
```
object_event {
    graphics_id: OBJ_EVENT_GFX_MAN_3
    x: 10
    y: 15
    elevation: 0
    movement_type: MOVEMENT_TYPE_FACE_DOWN
    movement_range_x: 0
    movement_range_y: 0
    trainer_type: TRAINER_TYPE_NONE
    trainer_sight_or_berry_tree_id: 0
    script: EventScript_ThievableItemsGuide
    flag: 0
}
```

## Testing Checklist

- [ ] Test with no Covet/Thief move -> Should show "need move" message
- [ ] Test on route with no Pokemon seen -> Should show "explore first" message
- [ ] Test with seen Pokemon -> Should show item menu
- [ ] Test item selection -> Should show quantity menu
- [ ] Test quantity adjustment with D-pad (up/down)
- [ ] Test confirming quantity with A button
- [ ] Test cancelling with B button (at both menu and quantity stages)
- [ ] Test receiving items -> Should add to bag and show confirmation
- [ ] Test with bag full -> Should show "bag full" message
- [ ] Test with multiple encounters types (land, surf, fishing)
- [ ] Verify no duplicate items in menu (same item from multiple Pokemon)
- [ ] Test at different times of day

## Edge Cases Handled

1. **No wild encounters on map** - Returns 0 items, shows "explore" message
2. **Pokemon without held items** - ITEM_NONE is filtered out
3. **Duplicate items** - Only shown once in menu (deduplication logic)
4. **Bag full** - Checks space before giving, reduces max quantity if needed
5. **Eggs in party** - Skipped when checking for moves
6. **Empty party slots** - Checked for SPECIES_NONE
7. **Maximum items** - Buffer limited to 32 unique items
8. **Quantity cap** - Maximum 99 per request

## Technical Notes

### Static Buffers
```c
static u16 sThievableItemsBuffer[MAX_THIEVABLE_ITEMS];  // Max 32 items
static u16 sThievableItemsCount = 0;
static u16 sSelectedItemId = ITEM_NONE;
static s16 sSelectedQuantity = 1;
```

### Key APIs Used
- `GetCurrentMapWildMonHeaderId()` - Get wild encounter data
- `SpeciesToNationalPokedexNum()` - Convert species to dex number
- `GetSetPokedexFlag(dexNum, FLAG_GET_SEEN)` - Check if Pokemon seen
- `MonKnowsMove()` - Check if Pokemon knows a move
- `MultichoiceDynamic_PushElement()` - Build dynamic menu
- `AdjustQuantityAccordingToDPadInput()` - Handle quantity input
- `CheckBagHasSpace()` - Validate bag space
- `AddBagItem()` - Give items to player
- `CopyItemName()` - Get item name string

### Memory Management
- Item names are allocated with `Alloc(32)` for each menu entry
- Dynamic multichoice stack is automatically managed
- Stack is destroyed after menu closes

## Future Enhancements (Optional)

1. Add cooldown timer (daily/hourly limit)
2. Add cost (pay with money or battle points)
3. Filter by item type (berries, held items, etc.)
4. Show Pokemon that hold each item in description
5. Show item rarity or drop rate
6. Add special items for high-level areas
7. Integration with Thief/Covet usage counter
