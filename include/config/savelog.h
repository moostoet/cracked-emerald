#ifndef GUARD_CONFIG_SAVELOG_H
#define GUARD_CONFIG_SAVELOG_H

// Master toggle. When FALSE, all logging compiles to no-ops (zero overhead).
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
// Implemented as a macro that returns TRUE for important flag IDs.
// The compiler optimizes chained == comparisons to efficient code.
//
// To use, uncomment and customize:
// #define SAVELOG_IMPORTANT_FLAGS(id) \
//     ((id) == FLAG_DEFEATED_EVIL_TEAM_LEADER || (id) == FLAG_RECEIVED_STARTER)
#define SAVELOG_IMPORTANT_FLAGS(id) FALSE

#endif // GUARD_CONFIG_SAVELOG_H
