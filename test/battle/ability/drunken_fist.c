#include "global.h"
#include "test/battle.h"

ASSUMPTIONS
{
    ASSUME(GetMoveEffect(MOVE_SCRATCH) == EFFECT_HIT);
    ASSUME(MoveMakesContact(MOVE_SCRATCH) == TRUE);
    ASSUME(MoveMakesContact(MOVE_SWIFT) == FALSE);
}

SINGLE_BATTLE_TEST("Drunken Fist raises user's Attack and drops attacker's accuracy on contact")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_DRUNKEN_FIST); }
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, opponent);
        ABILITY_POPUP(player, ABILITY_DRUNKEN_FIST);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, player);
        MESSAGE("Wobbuffet's Attack rose!");
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, opponent);
        MESSAGE("The opposing Wynaut's accuracy fell!");
    }
}

SINGLE_BATTLE_TEST("Drunken Fist does not activate on non-contact moves")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_DRUNKEN_FIST); }
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SWIFT); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SWIFT, opponent);
        NONE_OF {
            ABILITY_POPUP(player, ABILITY_DRUNKEN_FIST);
            MESSAGE("Wobbuffet's Attack rose!");
            MESSAGE("The opposing Wynaut's accuracy fell!");
        }
    }
}

SINGLE_BATTLE_TEST("Drunken Fist handles stat-drop abilities without losing its Attack boost")
{
    enum Ability ability;

    PARAMETRIZE { ability = ABILITY_NONE; }
    PARAMETRIZE { ability = ABILITY_MIRROR_ARMOR; }
    PARAMETRIZE { ability = ABILITY_CONTRARY; }
    PARAMETRIZE { ability = ABILITY_KEEN_EYE; }

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_DRUNKEN_FIST); }
        OPPONENT(SPECIES_WYNAUT) { Ability(ability); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SCRATCH); }
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE + 1);
        EXPECT_EQ(player->statStages[STAT_ACC], DEFAULT_STAT_STAGE - (ability == ABILITY_MIRROR_ARMOR));
        if (ability == ABILITY_CONTRARY)
            EXPECT_EQ(opponent->statStages[STAT_ACC], DEFAULT_STAT_STAGE + 1);
        else if (ability == ABILITY_NONE)
            EXPECT_EQ(opponent->statStages[STAT_ACC], DEFAULT_STAT_STAGE - 1);
        else
            EXPECT_EQ(opponent->statStages[STAT_ACC], DEFAULT_STAT_STAGE);
    }
}

SINGLE_BATTLE_TEST("Drunken Fist activates only once per move against multi-hit contact moves")
{
    GIVEN {
        ASSUME(GetMoveStrikeCount(MOVE_DOUBLE_KICK) == 2);
        ASSUME(MoveMakesContact(MOVE_DOUBLE_KICK) == TRUE);
        ASSUME(MoveMakesContact(MOVE_AERIAL_ACE) == TRUE);
        ASSUME(GetMoveAccuracy(MOVE_AERIAL_ACE) == 0);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_DRUNKEN_FIST); }
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(opponent, MOVE_DOUBLE_KICK); }
        TURN { MOVE(opponent, MOVE_AERIAL_ACE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DOUBLE_KICK, opponent);
        ABILITY_POPUP(player, ABILITY_DRUNKEN_FIST);
        MESSAGE("Wobbuffet's Attack rose!");
        MESSAGE("The opposing Wynaut's accuracy fell!");
        NONE_OF {
            ABILITY_POPUP(player, ABILITY_DRUNKEN_FIST);
        }
        ANIMATION(ANIM_TYPE_MOVE, MOVE_AERIAL_ACE, opponent);
        ABILITY_POPUP(player, ABILITY_DRUNKEN_FIST);
    } THEN {
        EXPECT_EQ(player->statStages[STAT_ATK], DEFAULT_STAT_STAGE + 2);
        EXPECT_EQ(opponent->statStages[STAT_ACC], DEFAULT_STAT_STAGE - 2);
    }
}

DOUBLE_BATTLE_TEST("Drunken Fist activates when a spread contact move hits the holder but not the chosen target")
{
    GIVEN {
        ASSUME(GetMoveTarget(MOVE_BRUTAL_SWING) == TARGET_FOES_AND_ALLY);
        ASSUME(MoveMakesContact(MOVE_BRUTAL_SWING) == TRUE);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_DRUNKEN_FIST); }
        OPPONENT(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_PROTECT); MOVE(opponentLeft, MOVE_BRUTAL_SWING); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_PROTECT, playerLeft);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_BRUTAL_SWING, opponentLeft);
        ABILITY_POPUP(playerRight, ABILITY_DRUNKEN_FIST);
    } THEN {
        EXPECT_EQ(playerLeft->statStages[STAT_ATK], DEFAULT_STAT_STAGE);
        EXPECT_EQ(playerRight->statStages[STAT_ATK], DEFAULT_STAT_STAGE + 1);
        EXPECT_EQ(opponentLeft->statStages[STAT_ACC], DEFAULT_STAT_STAGE - 1);
    }
}
