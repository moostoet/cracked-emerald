#include "global.h"
#include "test/battle.h"

SINGLE_BATTLE_TEST("MOVE_TARGET_USER_PARTNER: In single battles only affects user")
{
    s16 damage[2];

    GIVEN {
        ASSUME(B_UPDATED_MOVE_DATA >= GEN_8);
        ASSUME(GetMoveTarget(MOVE_HOWL) == MOVE_TARGET_USER_PARTNER);
        ASSUME(GetMoveCategory(MOVE_SCRATCH) == DAMAGE_CATEGORY_PHYSICAL);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
        TURN { MOVE(player, MOVE_HOWL, target: player); }
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
        HP_BAR(opponent, captureDamage: &damage[0]);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_HOWL, player);
        MESSAGE("Wobbuffet's Attack rose!");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, player);
        HP_BAR(opponent, captureDamage: &damage[1]);
    } THEN {
        EXPECT_MUL_EQ(damage[0], Q_4_12(1.5), damage[1]);
    }
}

DOUBLE_BATTLE_TEST("MOVE_TARGET_USER_PARTNER: Affects both user and partner in double battles")
{
    s16 damageLeft[2];
    s16 damageRight[2];

    GIVEN {
        ASSUME(B_UPDATED_MOVE_DATA >= GEN_8);
        ASSUME(GetMoveTarget(MOVE_HOWL) == MOVE_TARGET_USER_PARTNER);
        ASSUME(GetMoveCategory(MOVE_SCRATCH) == DAMAGE_CATEGORY_PHYSICAL);
        PLAYER(SPECIES_WOBBUFFET) { Speed(40); }
        PLAYER(SPECIES_WYNAUT) { Speed(30); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(20); }
        OPPONENT(SPECIES_WYNAUT) { Speed(10); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_SCRATCH, target: opponentLeft); MOVE(playerRight, MOVE_SCRATCH, target: opponentRight); }
        TURN { MOVE(playerLeft, MOVE_HOWL, target: playerLeft); }
        TURN { MOVE(playerLeft, MOVE_SCRATCH, target: opponentLeft); MOVE(playerRight, MOVE_SCRATCH, target: opponentRight); }
    } SCENE {
        // First turn: baseline damage
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, playerLeft);
        HP_BAR(opponentLeft, captureDamage: &damageLeft[0]);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, playerRight);
        HP_BAR(opponentRight, captureDamage: &damageRight[0]);

        // Second turn: Howl boosts both user and partner via MOVE_TARGET_USER_PARTNER
        ANIMATION(ANIM_TYPE_MOVE, MOVE_HOWL, playerLeft);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, playerLeft);
        MESSAGE("Wobbuffet's Attack rose!");
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_STATS_CHANGE, playerRight);
        MESSAGE("Wynaut's Attack rose!");

        // Third turn: verify both were boosted
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, playerLeft);
        HP_BAR(opponentLeft, captureDamage: &damageLeft[1]);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, playerRight);
        HP_BAR(opponentRight, captureDamage: &damageRight[1]);
    } THEN {
        EXPECT_MUL_EQ(damageLeft[0], Q_4_12(1.5), damageLeft[1]);
        EXPECT_MUL_EQ(damageRight[0], Q_4_12(1.5), damageRight[1]);
    }
}

DOUBLE_BATTLE_TEST("MOVE_TARGET_USER_PARTNER: Only affects user when partner is fainted")
{
    s16 damage[2];

    GIVEN {
        ASSUME(B_UPDATED_MOVE_DATA >= GEN_8);
        ASSUME(GetMoveTarget(MOVE_HOWL) == MOVE_TARGET_USER_PARTNER);
        ASSUME(GetMoveCategory(MOVE_SCRATCH) == DAMAGE_CATEGORY_PHYSICAL);
        PLAYER(SPECIES_WOBBUFFET) { Speed(40); }
        PLAYER(SPECIES_WYNAUT) { HP(0); Speed(30); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(20); }
        OPPONENT(SPECIES_WYNAUT) { Speed(10); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_SCRATCH, target: opponentLeft); }
        TURN { MOVE(playerLeft, MOVE_HOWL, target: playerLeft); }
        TURN { MOVE(playerLeft, MOVE_SCRATCH, target: opponentLeft); }
    } SCENE {
        // First turn: baseline damage
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, playerLeft);
        HP_BAR(opponentLeft, captureDamage: &damage[0]);

        // Second turn: Howl boosts only user since partner is fainted
        ANIMATION(ANIM_TYPE_MOVE, MOVE_HOWL, playerLeft);
        MESSAGE("Wobbuffet's Attack rose!");
        NONE_OF {
            MESSAGE("Wynaut's Attack rose!");
        }

        // Third turn: verify user was boosted
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SCRATCH, playerLeft);
        HP_BAR(opponentLeft, captureDamage: &damage[1]);
    } THEN {
        EXPECT_MUL_EQ(damage[0], Q_4_12(1.5), damage[1]);
    }
}
