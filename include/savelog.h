#ifndef GUARD_SAVELOG_H
#define GUARD_SAVELOG_H

#include "config/savelog.h"

#define SAVELOG_VERSION        1

// Ring buffer flags
#define SAVELOG_FLAG_WRAPPED   (1 << 0)

// NOP padding byte value (1-byte event: prefix 10, type 7, payload 0 = 10111000 = 0xB8)
#define SAVELOG_NOP_BYTE       0xB8

// Battle end results
#define SAVELOG_RESULT_WIN     0
#define SAVELOG_RESULT_LOSS    1
#define SAVELOG_RESULT_FLEE    2
#define SAVELOG_RESULT_CATCH   3

// Flat event type enum. Width determined by value:
//   0-7   -> 1-byte encoding (prefix 10, 3-bit wire type, 3-bit payload)
//   8-23  -> 2-byte encoding (prefix 0, 4-bit wire type, 11-bit payload)
//   24-55 -> 3-byte encoding (prefix 11, 5-bit wire type, 17-bit payload)
enum SaveLogEventType {
    // 1-byte events (0-7)
    SAVELOG_TIMESTAMP_TICK = 0,
    SAVELOG_BADGE_OBTAINED,
    SAVELOG_BLACKOUT,
    SAVELOG_CHAMPION_DEFEATED,
    SAVELOG_CREDITS_ROLLED,
    SAVELOG_BATTLE_ENDED,
    SAVELOG_RESERVED_1B,
    SAVELOG_NOP,

    // 2-byte events (8-23)
    SAVELOG_TRAINER_DEFEATED = 8,
    SAVELOG_POKEMON_CAUGHT,
    SAVELOG_POKEMON_FAINTED,
    SAVELOG_POKEMON_EVOLVED,
    SAVELOG_ITEM_PICKED_UP,
    SAVELOG_ITEM_USED_BATTLE,
    SAVELOG_POKEMON_SWITCHED,
    SAVELOG_MOVE_USED,
    SAVELOG_BATTLE_STARTED_WILD,
    SAVELOG_BATTLE_STARTED_TRAINER,
    SAVELOG_TIMESTAMP_EXTENDED,
    SAVELOG_MAP_ENTERED,

    // 3-byte events (24-55)
    SAVELOG_FLAG_SET = 24,
    SAVELOG_WARP_USED,
    SAVELOG_NPC_INTERACTION,
    SAVELOG_POKEMON_FAINTED_DETAIL,
    SAVELOG_POKEMON_CAUGHT_DETAIL,
    SAVELOG_SHOP_PURCHASE,
    SAVELOG_PC_TRANSACTION,
};

// struct SaveLog is defined in global.h (needed by SaveBlock3)

#if SAVELOG_ENABLE == TRUE
void SaveLog_Init(void);
void SaveLog_LogEvent(u8 type, u32 payload);
void SaveLog_OnLoad(void);
u16 SaveLog_GetCompactMapId(u8 mapGroup, u8 mapNum);
void SaveLog_DumpToConsole(void);
#else
#define SaveLog_Init() ((void)0)
#define SaveLog_LogEvent(type, payload) ((void)0)
#define SaveLog_OnLoad() ((void)0)
#define SaveLog_GetCompactMapId(mapGroup, mapNum) (0)
#define SaveLog_DumpToConsole() ((void)0)
#endif

#endif // GUARD_SAVELOG_H
