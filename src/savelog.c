#include "global.h"
#include "savelog.h"
#include "rtc.h"
#include "gba/isagbprint.h"
#include "constants/map_groups.h"

#if SAVELOG_ENABLE == TRUE

#include "data/map_group_count.h"

// Timestamp auto-insert threshold in minutes
#define TIMESTAMP_TICK_INTERVAL  8
#define TIMESTAMP_TICK_MAX       (7 * TIMESTAMP_TICK_INTERVAL) // 56 min max for 1-byte tick

static struct SaveLog *GetSaveLog(void)
{
    return &gSaveBlock3Ptr->saveLog;
}

static u8 GetEventWidth(u8 type)
{
    if (type < 8)
        return 1;
    if (type < 24)
        return 2;
    return 3;
}

static void IncrementEventCounter(struct SaveLog *log)
{
    log->totalEventsLo++;
    if (log->totalEventsLo == 0)
        log->totalEventsHi++;
}

static void WriteEventBytes(struct SaveLog *log, u8 type, u32 payload)
{
    u8 width = GetEventWidth(type);

    // Check if we need to pad with NOPs and wrap
    if (log->writePos + width > SAVELOG_BUFFER_SIZE)
    {
        // Fill remaining bytes with NOP
        while (log->writePos < SAVELOG_BUFFER_SIZE)
        {
            log->buffer[log->writePos] = SAVELOG_NOP_BYTE;
            log->writePos++;
        }
        log->writePos = 0;
        log->flags |= SAVELOG_FLAG_WRAPPED;
    }

    switch (width)
    {
    case 1:
    {
        // Format: 10TTTPPP (T=type 0-7, P=payload 0-7)
        u8 wireType = type;
        log->buffer[log->writePos] = 0x80 | (wireType << 3) | (payload & 0x7);
        break;
    }
    case 2:
    {
        // Format: 0TTTTPPP PPPPPPPP (T=type 0-15, P=payload 0-2047)
        u8 wireType = type - 8;
        u16 encoded = (wireType << 11) | (payload & 0x7FF);
        log->buffer[log->writePos]     = (encoded >> 8) & 0x7F;
        log->buffer[log->writePos + 1] = encoded & 0xFF;
        break;
    }
    case 3:
    {
        // Format: 11TTTTTP PPPPPPPP PPPPPPPP (T=type 0-31, P=payload 0-131071)
        u8 wireType = type - 24;
        u32 encoded = ((u32)wireType << 17) | (payload & 0x1FFFF);
        log->buffer[log->writePos]     = 0xC0 | ((encoded >> 16) & 0x3F);
        log->buffer[log->writePos + 1] = (encoded >> 8) & 0xFF;
        log->buffer[log->writePos + 2] = encoded & 0xFF;
        break;
    }
    }

    log->writePos += width;
    IncrementEventCounter(log);
}

static void TryInsertTimestamp(struct SaveLog *log)
{
    u16 currentMinutes = (u16)RtcGetMinuteCount();
    u16 delta = currentMinutes - log->lastTimestampMinutes;

    if (delta >= TIMESTAMP_TICK_INTERVAL)
    {
        if (delta <= TIMESTAMP_TICK_MAX)
        {
            // 1-byte tick: delta in 8-minute increments
            u8 ticks = delta / TIMESTAMP_TICK_INTERVAL;
            WriteEventBytes(log, SAVELOG_TIMESTAMP_TICK, ticks);
        }
        else
        {
            // 2-byte extended: absolute minutes (mod 2048)
            WriteEventBytes(log, SAVELOG_TIMESTAMP_EXTENDED, currentMinutes & 0x7FF);
        }
        log->lastTimestampMinutes = currentMinutes;
    }
}

void SaveLog_Init(void)
{
    struct SaveLog *log = GetSaveLog();
    u32 minuteCount;

    memset(log, 0, sizeof(struct SaveLog));
    log->version = SAVELOG_VERSION;

    minuteCount = RtcGetMinuteCount();
    log->runStartTimeLo = (u16)(minuteCount & 0xFFFF);
    log->runStartTimeHi = (u16)((minuteCount >> 16) & 0xFFFF);
    log->lastTimestampMinutes = (u16)minuteCount;
}

void SaveLog_LogEvent(u8 type, u32 payload)
{
    struct SaveLog *log = GetSaveLog();

    // Don't log NOP directly — it's only used internally for padding
    if (type == SAVELOG_NOP)
        return;

    // Auto-insert timestamp if enough time has passed
    if (type != SAVELOG_TIMESTAMP_TICK && type != SAVELOG_TIMESTAMP_EXTENDED)
        TryInsertTimestamp(log);

    WriteEventBytes(log, type, payload);
}

void SaveLog_OnLoad(void)
{
    struct SaveLog *log = GetSaveLog();

    if (log->version != SAVELOG_VERSION)
    {
        // Pre-SaveLog save or version mismatch — clean init
        memset(log, 0, sizeof(struct SaveLog));
        log->version = SAVELOG_VERSION;
        log->lastTimestampMinutes = (u16)RtcGetMinuteCount();
    }
}

u16 SaveLog_GetCompactMapId(u8 mapGroup, u8 mapNum)
{
    u16 id = 0;
    u8 i;

    for (i = 0; i < mapGroup && i < MAP_GROUPS_COUNT; i++)
        id += MAP_GROUP_COUNT[i];

    id += mapNum;
    return id;
}

void SaveLog_DumpToConsole(void)
{
    struct SaveLog *log = GetSaveLog();
    u32 totalEvents = ((u32)log->totalEventsHi << 16) | log->totalEventsLo;
    u16 pos;
    u16 end;
    u32 count = 0;

    DebugPrintf("SaveLog v%d - %lu events (%s)", log->version, totalEvents,
                (log->flags & SAVELOG_FLAG_WRAPPED) ? "wrapped" : "linear");

    // Determine read range
    if (log->flags & SAVELOG_FLAG_WRAPPED)
    {
        pos = log->writePos;
        end = log->writePos; // read until we wrap back
    }
    else
    {
        pos = 0;
        end = log->writePos;
    }

    // Read events sequentially
    while (TRUE)
    {
        u8 byte0;
        u8 type;
        u32 payload;
        u8 width;

        // Check termination
        if (log->flags & SAVELOG_FLAG_WRAPPED)
        {
            if (count > 0 && pos == end)
                break;
        }
        else
        {
            if (pos >= end)
                break;
        }

        byte0 = log->buffer[pos];

        if ((byte0 & 0xC0) == 0x80)
        {
            // 1-byte: 10TTTPPP
            type = (byte0 >> 3) & 0x7;
            payload = byte0 & 0x7;
            width = 1;
        }
        else if ((byte0 & 0x80) == 0)
        {
            // 2-byte: 0TTTTPPP PPPPPPPP
            u16 encoded;
            if (pos + 1 >= SAVELOG_BUFFER_SIZE && !(log->flags & SAVELOG_FLAG_WRAPPED))
                break;
            encoded = ((u16)(byte0 & 0x7F) << 8) | log->buffer[(pos + 1) % SAVELOG_BUFFER_SIZE];
            type = 8 + ((encoded >> 11) & 0xF);
            payload = encoded & 0x7FF;
            width = 2;
        }
        else
        {
            // 3-byte: 11TTTTTP PPPPPPPP PPPPPPPP
            u32 encoded;
            if (pos + 2 >= SAVELOG_BUFFER_SIZE && !(log->flags & SAVELOG_FLAG_WRAPPED))
                break;
            encoded = ((u32)(byte0 & 0x3F) << 16)
                    | ((u32)log->buffer[(pos + 1) % SAVELOG_BUFFER_SIZE] << 8)
                    | log->buffer[(pos + 2) % SAVELOG_BUFFER_SIZE];
            type = 24 + ((encoded >> 17) & 0x1F);
            payload = encoded & 0x1FFFF;
            width = 3;
        }

        // Skip NOP padding
        if (type != SAVELOG_NOP)
        {
            DebugPrintf("[%lu] %02X:%04lX", count, type, payload);
            count++;
        }

        pos = (pos + width) % SAVELOG_BUFFER_SIZE;
    }

    DebugPrintf("SaveLog dump complete (%lu events read)", count);
}

#endif // SAVELOG_ENABLE
