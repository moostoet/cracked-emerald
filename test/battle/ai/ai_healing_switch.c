#include "global.h"
#include "test/battle.h"

// AI_FLAG_FORCE_SETUP_FIRST_TURN is used to force the AI to use Wish on turn 1.
// This ensures we can reliably test the switch logic on turn 2 when Wish is active.

AI_SINGLE_BATTLE_TEST("Revival Blessing: AI selects a fainted Pokemon to revive")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_SMART_SWITCHING);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_RATICATE) { Moves(MOVE_REVIVAL_BLESSING); }
        OPPONENT(SPECIES_CHARIZARD) { HP(0); } // Fainted
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_REVIVAL_BLESSING); }
        TURN { EXPECT_SEND_OUT(opponent, 1); }
    }
}

AI_SINGLE_BATTLE_TEST("Healing Wish: AI switches into a damaged Pokemon")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_SMART_SWITCHING);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_GARDEVOIR) { Moves(MOVE_HEALING_WISH); }
        OPPONENT(SPECIES_CHANSEY) { HP(1); MaxHP(100); } // Damaged
        OPPONENT(SPECIES_BLISSEY) { HP(100); MaxHP(100); } // Healthy
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_HEALING_WISH); }
        TURN { EXPECT_SEND_OUT(opponent, 1); } // Should pick Chansey
    }
}

AI_SINGLE_BATTLE_TEST("Lunar Dance: AI switches into a statused Pokemon")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_SMART_SWITCHING);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CRESSELIA) { Moves(MOVE_LUNAR_DANCE); }
        OPPONENT(SPECIES_MACHAMP) { Status1(STATUS1_PARALYSIS); HP(100); MaxHP(100); } // Statused but full HP
        OPPONENT(SPECIES_CONKELDURR) { HP(100); MaxHP(100); } // Healthy
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_LUNAR_DANCE); }
        TURN { EXPECT_SEND_OUT(opponent, 1); } // Should pick Machamp
    }
}

AI_SINGLE_BATTLE_TEST("Wish: AI switches into a damaged Pokemon after Wish activates")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_SMART_SWITCHING | AI_FLAG_FORCE_SETUP_FIRST_TURN);
        PLAYER(SPECIES_WOBBUFFET) { Speed(10); }
        OPPONENT(SPECIES_JIRACHI) { Moves(MOVE_WISH, MOVE_U_TURN); Speed(100); }
        OPPONENT(SPECIES_SNORLAX) { HP(50); MaxHP(100); Speed(100); } // Damaged (50%)
        OPPONENT(SPECIES_KANGASKHAN) { HP(100); MaxHP(100); Speed(100); } // Healthy
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_WISH); }
        TURN { EXPECT_MOVE(opponent, MOVE_U_TURN); }
        TURN { EXPECT_SEND_OUT(opponent, 1); } // Should pick Snorlax because Wish is active
    }
}

AI_SINGLE_BATTLE_TEST("Wish: AI does NOT prioritize status (Wish doesn't cure status)")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_SMART_SWITCHING | AI_FLAG_FORCE_SETUP_FIRST_TURN);
        PLAYER(SPECIES_WOBBUFFET) { Speed(10); }
        OPPONENT(SPECIES_JIRACHI) { Moves(MOVE_WISH, MOVE_U_TURN); Speed(100); }
        OPPONENT(SPECIES_SNORLAX) { Status1(STATUS1_PARALYSIS); HP(100); MaxHP(100); Speed(100); } // Statused but full HP. Score should be 0.
        OPPONENT(SPECIES_KANGASKHAN) { HP(95); MaxHP(100); Speed(100); } // Slightly damaged. Score should be > 0 (approx 5).
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_WISH); }
        TURN { EXPECT_MOVE(opponent, MOVE_U_TURN); }
        TURN { EXPECT_SEND_OUT(opponent, 2); } // Should pick Kangaskhan (2) because it has HP damage. Snorlax (1) has status but Wish ignores it.
    }
}

AI_SINGLE_BATTLE_TEST("Wish: AI prioritizes Type Matchup over Statused Mon (Wish doesn't cure status)")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_SMART_SWITCHING | AI_FLAG_FORCE_SETUP_FIRST_TURN);
        PLAYER(SPECIES_MANECTRIC) { Speed(10); Ability(ABILITY_LIGHTNING_ROD); } // Electric + Lightning Rod
        OPPONENT(SPECIES_JIRACHI) { Moves(MOVE_WISH, MOVE_U_TURN); Speed(100); }
        OPPONENT(SPECIES_GYARADOS) { Status1(STATUS1_PARALYSIS); HP(100); MaxHP(100); Speed(100); } // Weak to Electric
        OPPONENT(SPECIES_STEELIX) { HP(100); MaxHP(100); Speed(100); } // Immune to Electric
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_WISH); }
        TURN { EXPECT_MOVE(opponent, MOVE_U_TURN); }
        TURN { EXPECT_SEND_OUT(opponent, 2); } // Should pick Steelix (Immunity) over Gyarados (Statused but weak)
    }
}

AI_SINGLE_BATTLE_TEST("Wish: AI prioritizes slight damage over Status (Wish doesn't cure status)")
{
    GIVEN {
        AI_FLAGS(AI_FLAG_CHECK_BAD_MOVE | AI_FLAG_CHECK_VIABILITY | AI_FLAG_TRY_TO_FAINT | AI_FLAG_SMART_SWITCHING | AI_FLAG_FORCE_SETUP_FIRST_TURN);
        PLAYER(SPECIES_WOBBUFFET) { Speed(10); }
        OPPONENT(SPECIES_JIRACHI) { Moves(MOVE_WISH, MOVE_U_TURN); Speed(100); }
        OPPONENT(SPECIES_SNORLAX) { Status1(STATUS1_PARALYSIS); HP(100); MaxHP(100); Speed(100); } // Statused, Full HP
        OPPONENT(SPECIES_KANGASKHAN) { HP(80); MaxHP(100); Speed(100); } // Slightly damaged (20%)
    } WHEN {
        TURN { EXPECT_MOVE(opponent, MOVE_WISH); }
        TURN { EXPECT_MOVE(opponent, MOVE_U_TURN); }
        TURN { EXPECT_SEND_OUT(opponent, 2); } // Should pick Kangaskhan (Damaged) over Snorlax (Statused) because Wish heals HP
    }
}