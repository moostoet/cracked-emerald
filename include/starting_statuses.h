#ifndef GUARD_STARTING_STATUSES_H
#define GUARD_STARTING_STATUSES_H

#include "constants/battle.h"

// Macro to unpack STARTING_STATUS_DEFINITIONS into struct fields
#define UNPACK_STARTING_STATUSES_STRUCT(_enum, _fieldName, _typeMaxValue, ...) INVOKE_WITH_(UNPACK_STARTING_STATUSES_STRUCT_, _fieldName, UNPACK_B(_typeMaxValue));
#define UNPACK_STARTING_STATUSES_STRUCT_(_fieldName, _type, ...) _type FIRST(__VA_OPT__(_fieldName:BIT_SIZE(FIRST(__VA_ARGS__)),) _fieldName)

struct StartingStatuses
{
    STARTING_STATUS_DEFINITIONS(UNPACK_STARTING_STATUSES_STRUCT)
};

// Macro to check if any starting status is active
#define UNPACK_STARTING_STATUS_GET_ANY(_enum, _fieldName, ...) if (statuses->_fieldName) return TRUE;

static inline bool32 AnyStartingStatusActive(const struct StartingStatuses *statuses)
{
    STARTING_STATUS_DEFINITIONS(UNPACK_STARTING_STATUS_GET_ANY);
    return FALSE;
}

// Macro to merge starting statuses from one struct into another
#define UNPACK_STARTING_STATUS_MERGE(_enum, _fieldName, ...) dst->_fieldName |= src->_fieldName;

static inline void MergeStartingStatuses(struct StartingStatuses *dst, const struct StartingStatuses *src)
{
    STARTING_STATUS_DEFINITIONS(UNPACK_STARTING_STATUS_MERGE);
}

// Macro to set a starting status field by enum value
#define UNPACK_STARTING_STATUS_SETTER(_enum, _fieldName, ...) case _enum: statuses->_fieldName = TRUE; break;

static inline void SetStartingStatusByEnum(struct StartingStatuses *statuses, enum StartingStatus status)
{
    switch (status)
    {
        STARTING_STATUS_DEFINITIONS(UNPACK_STARTING_STATUS_SETTER)
        case STARTING_STATUS_NONE:
        case STARTING_STATUS_COUNT:
            break;
    }
}

// Macro to get a starting status field by enum value
#define UNPACK_STARTING_STATUS_GETTER(_enum, _fieldName, ...) case _enum: return statuses->_fieldName;

static inline bool32 GetStartingStatusByEnum(const struct StartingStatuses *statuses, enum StartingStatus status)
{
    switch (status)
    {
        STARTING_STATUS_DEFINITIONS(UNPACK_STARTING_STATUS_GETTER)
        case STARTING_STATUS_NONE:
        case STARTING_STATUS_COUNT:
            return FALSE;
    }
    return FALSE;
}

// Macro to clear a starting status field by enum value
#define UNPACK_STARTING_STATUS_CLEARER(_enum, _fieldName, ...) case _enum: statuses->_fieldName = FALSE; break;

static inline void ClearStartingStatusByEnum(struct StartingStatuses *statuses, enum StartingStatus status)
{
    switch (status)
    {
        STARTING_STATUS_DEFINITIONS(UNPACK_STARTING_STATUS_CLEARER)
        case STARTING_STATUS_NONE:
        case STARTING_STATUS_COUNT:
            break;
    }
}

#endif // GUARD_STARTING_STATUSES_H
