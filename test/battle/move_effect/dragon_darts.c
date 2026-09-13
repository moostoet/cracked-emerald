#include "global.h"
#include "test/battle.h"

ASSUMPTIONS
{
    ASSUME(GetMoveTarget(MOVE_DRAGON_DARTS) == TARGET_SMART);
    ASSUME(GetMoveStrikeCount(MOVE_DRAGON_DARTS) == 2);
    ASSUME(GetMoveType(MOVE_DRAGON_DARTS) == TYPE_DRAGON);
    ASSUME(GetMoveCategory(MOVE_DRAGON_DARTS) == DAMAGE_CATEGORY_PHYSICAL);
}

SINGLE_BATTLE_TEST("Dragon Darts strikes twice")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(player, MOVE_DRAGON_DARTS); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, player);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, player);
        NOT MESSAGE("The Pokémon was hit 2 time(s)!");
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts strikes each opponent once in a double battle")
{
    struct BattlePokemon *chosenTarget = NULL;
    struct BattlePokemon *secondaryTarget = NULL;
    PARAMETRIZE { chosenTarget = opponentLeft; secondaryTarget = opponentRight; }
    PARAMETRIZE { chosenTarget = opponentRight; secondaryTarget = opponentLeft; }

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_DRAGON_DARTS, target: chosenTarget); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(chosenTarget);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(secondaryTarget);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts strikes the other opponent twice if one target protects")
{
    struct BattlePokemon *chosenTarget = NULL;
    struct BattlePokemon *secondaryTarget = NULL;
    PARAMETRIZE { chosenTarget = opponentLeft; secondaryTarget = opponentRight; }
    PARAMETRIZE { chosenTarget = opponentRight; secondaryTarget = opponentLeft; }

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(chosenTarget, MOVE_PROTECT); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: opponentLeft); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_PROTECT, chosenTarget);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(secondaryTarget);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(secondaryTarget);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts strikes an opponent twice if the other one is Fairy-type")
{
    struct BattlePokemon *chosenTarget = NULL;
    struct BattlePokemon *finalTarget = NULL;
    enum Species speciesLeft, speciesRight;
    PARAMETRIZE { chosenTarget = opponentLeft;  finalTarget = opponentRight; speciesLeft = SPECIES_FIDOUGH;   speciesRight = SPECIES_WOBBUFFET; }
    PARAMETRIZE { chosenTarget = opponentRight; finalTarget = opponentRight; speciesLeft = SPECIES_FIDOUGH;   speciesRight = SPECIES_WOBBUFFET; }
    PARAMETRIZE { chosenTarget = opponentLeft;  finalTarget = opponentLeft;  speciesLeft = SPECIES_WOBBUFFET; speciesRight = SPECIES_FIDOUGH; }
    PARAMETRIZE { chosenTarget = opponentRight; finalTarget = opponentLeft;  speciesLeft = SPECIES_WOBBUFFET; speciesRight = SPECIES_FIDOUGH; }

    GIVEN {
        ASSUME(IsSpeciesOfType(SPECIES_FIDOUGH, TYPE_FAIRY ));
        ASSUME(GetMoveType(MOVE_DRAGON_DARTS) == TYPE_DRAGON);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(speciesLeft);
        OPPONENT(speciesRight);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_DRAGON_DARTS, target: chosenTarget); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(finalTarget);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(finalTarget);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts strikes an opponent twice if electrified and the other one has Volt Absorb")
{
    struct BattlePokemon *chosenTarget = NULL;
    struct BattlePokemon *finalTarget = NULL;
    enum Ability abilityLeft, abilityRight;
    PARAMETRIZE { chosenTarget = opponentLeft;  finalTarget = opponentLeft;  abilityLeft = ABILITY_WATER_ABSORB; abilityRight = ABILITY_VOLT_ABSORB; }
    PARAMETRIZE { chosenTarget = opponentRight; finalTarget = opponentLeft;  abilityLeft = ABILITY_WATER_ABSORB; abilityRight = ABILITY_VOLT_ABSORB; }
    PARAMETRIZE { chosenTarget = opponentLeft;  finalTarget = opponentRight; abilityLeft = ABILITY_VOLT_ABSORB;  abilityRight = ABILITY_WATER_ABSORB; }
    PARAMETRIZE { chosenTarget = opponentRight; finalTarget = opponentRight; abilityLeft = ABILITY_VOLT_ABSORB;  abilityRight = ABILITY_WATER_ABSORB; }

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_ELECTRIFY) == EFFECT_ELECTRIFY);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_LANTURN) { Ability(abilityLeft); }
        OPPONENT(SPECIES_LANTURN) { Ability(abilityRight); }
    } WHEN {
        TURN { MOVE(opponentRight, MOVE_ELECTRIFY, target: playerLeft); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: chosenTarget); }
    } SCENE {
        ABILITY_POPUP(finalTarget == opponentLeft ? opponentRight : opponentLeft, ABILITY_VOLT_ABSORB);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(finalTarget);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(finalTarget);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts strikes an opponent twice if electrified and the other one has Motor Drive")
{
    struct BattlePokemon *chosenTarget = NULL;
    struct BattlePokemon *finalTarget = NULL;
    enum Ability abilityLeft, abilityRight;
    PARAMETRIZE { chosenTarget = opponentLeft;  finalTarget = opponentLeft;  abilityLeft = ABILITY_VITAL_SPIRIT; abilityRight = ABILITY_MOTOR_DRIVE; }
    PARAMETRIZE { chosenTarget = opponentRight; finalTarget = opponentLeft;  abilityLeft = ABILITY_VITAL_SPIRIT; abilityRight = ABILITY_MOTOR_DRIVE; }
    PARAMETRIZE { chosenTarget = opponentLeft;  finalTarget = opponentRight; abilityLeft = ABILITY_MOTOR_DRIVE;  abilityRight = ABILITY_VITAL_SPIRIT; }
    PARAMETRIZE { chosenTarget = opponentRight; finalTarget = opponentRight; abilityLeft = ABILITY_MOTOR_DRIVE;  abilityRight = ABILITY_VITAL_SPIRIT; }

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_ELECTRIFY) == EFFECT_ELECTRIFY);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_ELECTIVIRE) { Ability(abilityLeft); }
        OPPONENT(SPECIES_ELECTIVIRE) { Ability(abilityRight); }
    } WHEN {
        TURN { MOVE(opponentRight, MOVE_ELECTRIFY, target: playerLeft); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: chosenTarget); }
    } SCENE {
        ABILITY_POPUP(finalTarget == opponentLeft ? opponentRight : opponentLeft, ABILITY_MOTOR_DRIVE);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(finalTarget);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(finalTarget);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts strikes an opponent twice if the other one is in a semi-invulnerable turn")
{
    struct BattlePokemon *chosenTarget = NULL;
    struct BattlePokemon *finalTarget = NULL;
    PARAMETRIZE { chosenTarget = opponentLeft;  finalTarget = opponentRight; }
    PARAMETRIZE { chosenTarget = opponentRight; finalTarget = opponentLeft; }

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FLY) == EFFECT_SEMI_INVULNERABLE);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(chosenTarget, MOVE_FLY, target: playerLeft); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: chosenTarget); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_FLY, chosenTarget);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(finalTarget);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(finalTarget);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts is not effected by Wide Guard")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponentLeft, MOVE_WIDE_GUARD); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: opponentLeft); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_WIDE_GUARD, opponentLeft);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentLeft);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentRight);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts strikes an opponent twice if the other one is fainted")
{
    struct BattlePokemon *chosenTarget = NULL;
    struct BattlePokemon *finalTarget = NULL;
    u32 hpLeft, hpRight;
    PARAMETRIZE { chosenTarget = opponentLeft;  finalTarget = opponentRight; hpLeft = 1; hpRight = 101; }
    PARAMETRIZE { chosenTarget = opponentRight; finalTarget = opponentLeft;  hpLeft = 101; hpRight = 1; }

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { HP(hpLeft); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(hpRight); }
    } WHEN {
        TURN { MOVE(playerRight, MOVE_SONIC_BOOM, target: chosenTarget); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: chosenTarget); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SONIC_BOOM, playerRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(finalTarget);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(finalTarget);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts checks the unchosen target for accuracy")
{
    GIVEN {
        ASSUME(GetMoveAccuracy(MOVE_DRAGON_DARTS) == 100);
        ASSUME(GetItemHoldEffect(ITEM_BRIGHT_POWDER) == HOLD_EFFECT_EVASION_UP);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_MACHAMP) { Ability(ABILITY_NO_GUARD); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(ITEM_BRIGHT_POWDER); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_DRAGON_DARTS, target: opponentLeft, hit: FALSE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentLeft);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentLeft);
        NOT HP_BAR(opponentRight);
    } THEN {
        EXPECT_LT(opponentLeft->hp, opponentLeft->maxHP);
        EXPECT_EQ(opponentRight->hp, opponentRight->maxHP);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts strikes the other opponent twice if the chosen target misses")
{
    struct BattlePokemon *chosenTarget = NULL;
    struct BattlePokemon *finalTarget = NULL;
    enum Item itemLeft, itemRight;
    PARAMETRIZE { chosenTarget = opponentLeft;  finalTarget = opponentRight; itemLeft = ITEM_BRIGHT_POWDER;  itemRight = ITEM_NONE; }
    PARAMETRIZE { chosenTarget = opponentRight; finalTarget = opponentLeft;  itemLeft = ITEM_NONE;           itemRight = ITEM_BRIGHT_POWDER; }

    GIVEN {
        ASSUME(GetMoveAccuracy(MOVE_DRAGON_DARTS) == 100);
        ASSUME(GetItemHoldEffect(ITEM_BRIGHT_POWDER) == HOLD_EFFECT_EVASION_UP);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { Item(itemLeft); }
        OPPONENT(SPECIES_WOBBUFFET) { Item(itemRight); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_DRAGON_DARTS, target: chosenTarget, hit: FALSE); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(finalTarget);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(finalTarget);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts strikes will be both redirected to Follow Me user")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FOLLOW_ME) == EFFECT_FOLLOW_ME);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponentRight, MOVE_FOLLOW_ME); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: opponentLeft); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentRight);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts fails to strike any target if under a Fairy-type follow me user")
{
    GIVEN {
        ASSUME(IsSpeciesOfType(SPECIES_FIDOUGH, TYPE_FAIRY ));
        ASSUME(GetMoveType(MOVE_DRAGON_DARTS) == TYPE_DRAGON);
        ASSUME(GetMoveEffect(MOVE_FOLLOW_ME) == EFFECT_FOLLOW_ME);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_FIDOUGH);
    } WHEN {
        TURN { MOVE(opponentRight, MOVE_FOLLOW_ME); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: opponentLeft); }
    } SCENE {
        NONE_OF {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        }
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts fails to strike the second target if first target fainted and Follow Me was active")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FOLLOW_ME) == EFFECT_FOLLOW_ME);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
    } WHEN {
        TURN { MOVE(opponentRight, MOVE_FOLLOW_ME); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: opponentLeft); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentRight);
        NOT ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts can be absorbed by both opponents and hit neither")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_ELECTRIFY) == EFFECT_ELECTRIFY);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_ELECTIVIRE) { Ability(ABILITY_MOTOR_DRIVE); };
        OPPONENT(SPECIES_ELECTIVIRE) { Ability(ABILITY_MOTOR_DRIVE); };
    } WHEN {
        TURN {
            MOVE(opponentRight, MOVE_ELECTRIFY, target: playerLeft);
            MOVE(playerLeft, MOVE_DRAGON_DARTS, target: opponentLeft); }
    } SCENE {
        NOT ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
    } THEN {
        EXPECT_EQ(opponentLeft->statStages[STAT_SPEED], DEFAULT_STAT_STAGE + 1);
        EXPECT_EQ(opponentRight->statStages[STAT_SPEED], DEFAULT_STAT_STAGE + 1);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts always prints effectiveness message after every hit")
{
    // The messages are possibly incorrect if they should print the target
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_CUFANT);
        OPPONENT(SPECIES_DRATINI);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_DRAGON_DARTS, target: opponentLeft); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentLeft);
        MESSAGE("It's not very effective…");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentRight);
        MESSAGE("It's super effective!");
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts strikes its ally twice without targeting its user")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_DRAGON_DARTS, target: playerRight); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(playerRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(playerRight);
    } THEN {
        EXPECT_EQ(playerLeft->hp, playerLeft->maxHP);
        EXPECT_EQ(opponentLeft->hp, opponentLeft->maxHP);
        EXPECT_EQ(opponentRight->hp, opponentRight->maxHP);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts can hit its user and ally after Ally Switch")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_ALLY_SWITCH) == EFFECT_ALLY_SWITCH);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(playerRight, MOVE_ALLY_SWITCH); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: playerRight); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ALLY_SWITCH, playerRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerRight);
        HP_BAR(playerRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerRight);
        HP_BAR(playerLeft);
    } THEN {
        EXPECT_EQ(opponentLeft->hp, opponentLeft->maxHP);
        EXPECT_EQ(opponentRight->hp, opponentRight->maxHP);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts ignores its user's Telepathy after Ally Switch")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_ALLY_SWITCH) == EFFECT_ALLY_SWITCH);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_TELEPATHY); }
        PLAYER(SPECIES_WYNAUT) { Ability(ABILITY_TELEPATHY); }
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(playerRight, MOVE_ALLY_SWITCH); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: playerRight); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ALLY_SWITCH, playerRight);
        ABILITY_POPUP(playerLeft, ABILITY_TELEPATHY);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerRight);
        HP_BAR(playerRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerRight);
        HP_BAR(playerRight);
    } THEN {
        EXPECT_EQ(playerLeft->hp, playerLeft->maxHP);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts can activate its user's Jaboca Berry after Ally Switch")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_ALLY_SWITCH) == EFFECT_ALLY_SWITCH);
        ASSUME(GetItemHoldEffect(ITEM_JABOCA_BERRY) == HOLD_EFFECT_JABOCA_BERRY);
        PLAYER(SPECIES_WOBBUFFET) { Item(ITEM_JABOCA_BERRY); MaxHP(320); HP(320); }
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(playerRight, MOVE_ALLY_SWITCH); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: playerRight); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ALLY_SWITCH, playerRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerRight);
        HP_BAR(playerRight);
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_HELD_ITEM_BERRY, playerRight);
        HP_BAR(playerRight, damage: 40);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerRight);
        HP_BAR(playerLeft);
    } THEN {
        EXPECT_EQ(playerRight->item, ITEM_NONE);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts consumes PP for each opposing Pressure ability regardless of selected target")
{
    struct BattlePokemon *chosenTarget = NULL;
    enum Ability abilityLeft = ABILITY_NONE, abilityRight = ABILITY_NONE;
    u32 pressureCount = 0;
    for (u32 j = 0; j < 4; j++)
    {
        PARAMETRIZE { chosenTarget = opponentLeft; abilityLeft = j & 1 ? ABILITY_PRESSURE : ABILITY_SHADOW_TAG; abilityRight = j & 2 ? ABILITY_PRESSURE : ABILITY_SHADOW_TAG; pressureCount = !!(j & 1) + !!(j & 2); }
        PARAMETRIZE { chosenTarget = opponentRight; abilityLeft = j & 1 ? ABILITY_PRESSURE : ABILITY_SHADOW_TAG; abilityRight = j & 2 ? ABILITY_PRESSURE : ABILITY_SHADOW_TAG; pressureCount = !!(j & 1) + !!(j & 2); }
    }

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { MovesWithPP({MOVE_DRAGON_DARTS, 10}); }
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET) { Ability(abilityLeft); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(abilityRight); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_DRAGON_DARTS, target: chosenTarget); }
    } THEN {
        EXPECT_EQ(playerLeft->pp[0], 10 - 1 - pressureCount);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts does not consume extra PP when targeting its own Pressure after Ally Switch")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_ALLY_SWITCH) == EFFECT_ALLY_SWITCH);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ABILITY_PRESSURE); MovesWithPP({MOVE_DRAGON_DARTS, 10}); }
        PLAYER(SPECIES_WYNAUT) { Ability(ABILITY_TELEPATHY); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_PRESSURE); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_PRESSURE); }
    } WHEN {
        TURN { MOVE(playerRight, MOVE_ALLY_SWITCH); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: playerRight); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerRight);
        HP_BAR(playerRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerRight);
        HP_BAR(playerRight);
    } THEN {
        EXPECT_EQ(playerRight->pp[0], 9);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts still hits Receiver after it inherits Motor Drive between hits")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_GASTRO_ACID) == EFFECT_GASTRO_ACID);
        ASSUME(GetMoveEffect(MOVE_ELECTRIFY) == EFFECT_ELECTRIFY);
        ASSUME(!gAbilitiesInfo[ABILITY_MOTOR_DRIVE].cantBeCopied);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_ELECTIVIRE) { Ability(ABILITY_MOTOR_DRIVE); HP(1); }
        OPPONENT(SPECIES_PASSIMIAN) { Ability(ABILITY_RECEIVER); }
    } WHEN {
        TURN { MOVE(playerRight, MOVE_GASTRO_ACID, target: opponentLeft); }
        TURN { MOVE(playerRight, MOVE_ELECTRIFY, target: playerLeft); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: opponentLeft); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_GASTRO_ACID, playerRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ELECTRIFY, playerRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentLeft);
        MESSAGE("The opposing Electivire fainted!");
        ABILITY_POPUP(opponentRight, ABILITY_RECEIVER);
        NOT ABILITY_POPUP(opponentRight, ABILITY_MOTOR_DRIVE);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentRight);
    } THEN {
        EXPECT_EQ(opponentRight->ability, ABILITY_MOTOR_DRIVE);
        EXPECT_EQ(opponentRight->statStages[STAT_SPEED], DEFAULT_STAT_STAGE);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts activates Innards Out inherited by Receiver between hits")
{
    GIVEN {
        ASSUME(!gAbilitiesInfo[ABILITY_INNARDS_OUT].cantBeCopied);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_PYUKUMUKU) { Ability(ABILITY_INNARDS_OUT); HP(1); }
        OPPONENT(SPECIES_PASSIMIAN) { Ability(ABILITY_RECEIVER); HP(2); }
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_DRAGON_DARTS, target: opponentLeft); SEND_OUT(opponentLeft, 2); SEND_OUT(opponentRight, 3); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentLeft);
        ABILITY_POPUP(opponentLeft, ABILITY_INNARDS_OUT);
        HP_BAR(playerLeft, damage: 1);
        ABILITY_POPUP(opponentRight, ABILITY_RECEIVER);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentRight);
        ABILITY_POPUP(opponentRight, ABILITY_INNARDS_OUT);
        HP_BAR(playerLeft, damage: 2);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts called by Instruct after Ally Switch does not let Counter reflect self-damage")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_ALLY_SWITCH) == EFFECT_ALLY_SWITCH);
        ASSUME(GetMoveEffect(MOVE_INSTRUCT) == EFFECT_INSTRUCT);
        ASSUME(GetMoveEffect(MOVE_COUNTER) == EFFECT_REFLECT_DAMAGE);
        WITH_CONFIG(B_COUNTER_MIRROR_COAT_ALLY, GEN_5);
        PLAYER(SPECIES_WOBBUFFET) { MaxHP(1000); HP(1000); Speed(4); Moves(MOVE_DRAGON_DARTS, MOVE_ALLY_SWITCH, MOVE_INSTRUCT, MOVE_COUNTER); }
        PLAYER(SPECIES_WYNAUT) { Ability(ABILITY_TELEPATHY); Speed(3); Moves(MOVE_DRAGON_DARTS, MOVE_ALLY_SWITCH, MOVE_INSTRUCT, MOVE_COUNTER); }
        OPPONENT(SPECIES_WOBBUFFET) { Speed(2); }
        OPPONENT(SPECIES_WYNAUT) { Speed(1); }
    } WHEN {
        TURN { MOVE(playerRight, MOVE_ALLY_SWITCH); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: playerRight); }
        TURN { MOVE(playerLeft, MOVE_INSTRUCT, target: playerRight); MOVE(playerRight, MOVE_COUNTER); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ALLY_SWITCH, playerRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerRight);
        HP_BAR(playerRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerRight);
        HP_BAR(playerRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_INSTRUCT, playerLeft);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerRight);
        HP_BAR(playerRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerRight);
        HP_BAR(playerRight);
        MESSAGE("Wobbuffet used Counter!");
        NOT ANIMATION(ANIM_TYPE_MOVE, MOVE_COUNTER, playerRight);
        MESSAGE("But it failed!");
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts follows positions after opposing Ally Switch unless its ability ignores redirection")
{
    struct BattlePokemon *chosenTarget = NULL, *firstTarget = NULL, *secondTarget = NULL;
    enum Ability ability = ABILITY_NONE;

    PARAMETRIZE { ability = ABILITY_SHADOW_TAG; chosenTarget = opponentLeft; firstTarget = opponentLeft; secondTarget = opponentRight; }
    PARAMETRIZE { ability = ABILITY_SHADOW_TAG; chosenTarget = opponentRight; firstTarget = opponentRight; secondTarget = opponentLeft; }
    PARAMETRIZE { ability = ABILITY_STALWART; chosenTarget = opponentLeft; firstTarget = opponentRight; secondTarget = opponentLeft; }
    PARAMETRIZE { ability = ABILITY_STALWART; chosenTarget = opponentRight; firstTarget = opponentLeft; secondTarget = opponentRight; }
    PARAMETRIZE { ability = ABILITY_PROPELLER_TAIL; chosenTarget = opponentLeft; firstTarget = opponentRight; secondTarget = opponentLeft; }
    PARAMETRIZE { ability = ABILITY_PROPELLER_TAIL; chosenTarget = opponentRight; firstTarget = opponentLeft; secondTarget = opponentRight; }

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_ALLY_SWITCH) == EFFECT_ALLY_SWITCH);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ability); }
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(opponentLeft, MOVE_ALLY_SWITCH); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: chosenTarget); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ALLY_SWITCH, opponentLeft);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(firstTarget);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(secondTarget);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts with Stalwart or Propeller Tail keeps targeting its ally after Ally Switch")
{
    enum Ability ability;
    bool32 allyImmune;
    PARAMETRIZE { ability = ABILITY_STALWART; allyImmune = FALSE; }
    PARAMETRIZE { ability = ABILITY_STALWART; allyImmune = TRUE; }
    PARAMETRIZE { ability = ABILITY_PROPELLER_TAIL; allyImmune = FALSE; }
    PARAMETRIZE { ability = ABILITY_PROPELLER_TAIL; allyImmune = TRUE; }

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_ALLY_SWITCH) == EFFECT_ALLY_SWITCH);
        PLAYER(SPECIES_WOBBUFFET) { Ability(ability); }
        PLAYER(SPECIES_WYNAUT) { Ability(allyImmune ? ABILITY_TELEPATHY : ABILITY_SHADOW_TAG); }
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(playerRight, MOVE_ALLY_SWITCH); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: playerRight); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ALLY_SWITCH, playerRight);
        if (allyImmune)
        {
            ABILITY_POPUP(playerLeft, ABILITY_TELEPATHY);
            NOT ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerRight);
        }
        else
        {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerRight);
            HP_BAR(playerLeft);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerRight);
            HP_BAR(playerLeft);
        }
    } THEN {
        EXPECT_EQ(playerRight->hp, playerRight->maxHP);
        if (allyImmune)
            EXPECT_EQ(playerLeft->hp, playerLeft->maxHP);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts checks its user's type immunity when Ally Switch makes it the target")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_ALLY_SWITCH) == EFFECT_ALLY_SWITCH);
        ASSUME(IsSpeciesOfType(SPECIES_FIDOUGH, TYPE_FAIRY));
        PLAYER(SPECIES_FIDOUGH);
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(playerRight, MOVE_ALLY_SWITCH); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: playerRight); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ALLY_SWITCH, playerRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerRight);
        HP_BAR(playerLeft);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerRight);
        HP_BAR(playerLeft);
    } THEN {
        EXPECT_EQ(playerRight->hp, playerRight->maxHP);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts does not retarget its user if its selected ally is immune")
{
    GIVEN {
        ASSUME(IsSpeciesOfType(SPECIES_FIDOUGH, TYPE_FAIRY));
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_FIDOUGH);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_DRAGON_DARTS, target: playerRight); }
    } SCENE {
        NONE_OF {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
            HP_BAR(playerLeft);
            HP_BAR(playerRight);
            HP_BAR(opponentLeft);
            HP_BAR(opponentRight);
        }
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts stops if its user faints to its first hit after Ally Switch")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_ALLY_SWITCH) == EFFECT_ALLY_SWITCH);
        PLAYER(SPECIES_WOBBUFFET) { HP(1); }
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(playerRight, MOVE_ALLY_SWITCH); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: playerRight); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ALLY_SWITCH, playerRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerRight);
        HP_BAR(playerRight);
        MESSAGE("Wobbuffet fainted!");
        NONE_OF {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerRight);
            HP_BAR(playerLeft);
        }
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts retains Soak's type effectiveness through Disguise", s16 damage)
{
    bool32 electrify;
    PARAMETRIZE { electrify = FALSE; }
    PARAMETRIZE { electrify = TRUE; }

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_ELECTRIFY) == EFFECT_ELECTRIFY);
        ASSUME(GetMoveEffect(MOVE_SOAK) == EFFECT_SOAK);
        ASSUME(GetMoveEffect(MOVE_PROTECT) == EFFECT_PROTECT);
        WITH_CONFIG(B_DISGUISE_HP_LOSS, GEN_7);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_MIMIKYU_DISGUISED) { Ability(ABILITY_DISGUISE); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(playerRight, MOVE_SOAK, target: opponentLeft); }
        TURN {
            MOVE(opponentRight, MOVE_PROTECT);
            if (electrify) MOVE(playerRight, MOVE_ELECTRIFY, target: playerLeft);
            MOVE(playerLeft, MOVE_DRAGON_DARTS, target: opponentLeft);
        }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SOAK, playerRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        ABILITY_POPUP(opponentLeft, ABILITY_DISGUISE);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentLeft, captureDamage: &results[i].damage);
        if (electrify) MESSAGE("It's super effective!");
    } FINALLY {
        EXPECT_MUL_EQ(results[0].damage, Q_4_12(2.0), results[1].damage);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts defers knockout stat boosts until after its second target")
{
    enum Ability ability;
    enum Stat stat;
    PARAMETRIZE { ability = ABILITY_MOXIE; stat = STAT_ATK; }
    PARAMETRIZE { ability = ABILITY_CHILLING_NEIGH; stat = STAT_ATK; }
    PARAMETRIZE { ability = ABILITY_GRIM_NEIGH; stat = STAT_SPATK; }
    PARAMETRIZE { ability = ABILITY_BEAST_BOOST; stat = STAT_ATK; }

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ability); Attack(200); Defense(100); SpAttack(100); SpDefense(100); Speed(100); }
        PLAYER(SPECIES_WYNAUT) { Speed(90); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); Speed(80); }
        OPPONENT(SPECIES_WYNAUT) { Speed(70); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_DRAGON_DARTS, target: opponentLeft); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentLeft);
        MESSAGE("The opposing Wobbuffet fainted!");
        NOT ABILITY_POPUP(playerLeft, ability);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentRight);
        ABILITY_POPUP(playerLeft, ability);
    } THEN {
        EXPECT_EQ(playerLeft->statStages[stat], DEFAULT_STAT_STAGE + 1);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts activates Soul-Heart after each fainted target")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_MAGEARNA) { Ability(ABILITY_SOUL_HEART); }
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); }
        OPPONENT(SPECIES_WYNAUT) { HP(1); }
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_DRAGON_DARTS, target: opponentLeft); SEND_OUT(opponentLeft, 2); SEND_OUT(opponentRight, 3); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentLeft);
        MESSAGE("The opposing Wobbuffet fainted!");
        ABILITY_POPUP(playerRight, ABILITY_SOUL_HEART);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentRight);
        MESSAGE("The opposing Wynaut fainted!");
        ABILITY_POPUP(playerRight, ABILITY_SOUL_HEART);
    } THEN {
        EXPECT_EQ(playerRight->statStages[STAT_SPATK], DEFAULT_STAT_STAGE + 2);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts defers Magician until after its second target even if the first target faints")
{
    GIVEN {
        PLAYER(SPECIES_DELPHOX) { Ability(ABILITY_MAGICIAN); }
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET) { HP(1); Item(ITEM_SCOPE_LENS); }
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_DRAGON_DARTS, target: opponentLeft); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentLeft);
        MESSAGE("The opposing Wobbuffet fainted!");
        NOT ABILITY_POPUP(playerLeft, ABILITY_MAGICIAN);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentRight);
        ABILITY_POPUP(playerLeft, ABILITY_MAGICIAN);
    } THEN {
        EXPECT_EQ(playerLeft->item, ITEM_SCOPE_LENS);
        EXPECT_EQ(opponentLeft->item, ITEM_NONE);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts defers Berserk until after its second target")
{
    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Attack(100); }
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_DRAMPA) { Ability(ABILITY_BERSERK); MaxHP(200); HP(101); Defense(100); }
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_DRAGON_DARTS, target: opponentLeft); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentLeft);
        NOT ABILITY_POPUP(opponentLeft, ABILITY_BERSERK);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentRight);
        ABILITY_POPUP(opponentLeft, ABILITY_BERSERK);
    } THEN {
        EXPECT_EQ(opponentLeft->statStages[STAT_SPATK], DEFAULT_STAT_STAGE + 1);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts applies Intimidate activated by Neutralizing Gas fainting before its second hit", s16 damage)
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_MOXIE; }
    PARAMETRIZE { ability = ABILITY_INTIMIDATE; }

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Attack(200); }
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WEEZING) { Ability(ABILITY_NEUTRALIZING_GAS); HP(1); }
        OPPONENT(SPECIES_GYARADOS) { Ability(ability); Defense(100); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_DRAGON_DARTS, target: opponentLeft); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentLeft);
        MESSAGE("The effects of the neutralizing gas wore off!");
        if (ability == ABILITY_INTIMIDATE) ABILITY_POPUP(opponentRight, ABILITY_INTIMIDATE);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentRight, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(1.5), results[0].damage);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts does not recheck Lightning Rod after its first hit removes Neutralizing Gas")
{
    GIVEN {
        ASSUME(GetMoveEffect(MOVE_ELECTRIFY) == EFFECT_ELECTRIFY);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WEEZING) { Ability(ABILITY_NEUTRALIZING_GAS); HP(1); }
        OPPONENT(SPECIES_RAICHU) { Ability(ABILITY_LIGHTNING_ROD); }
    } WHEN {
        TURN { MOVE(playerRight, MOVE_ELECTRIFY, target: playerLeft); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: opponentLeft); }
    } SCENE {
        ABILITY_POPUP(opponentLeft, ABILITY_NEUTRALIZING_GAS);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ELECTRIFY, playerRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentLeft);
        MESSAGE("The effects of the neutralizing gas wore off!");
        MESSAGE("The opposing Weezing fainted!");
        NOT ABILITY_POPUP(opponentRight, ABILITY_LIGHTNING_ROD);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentRight);
    } THEN {
        EXPECT_EQ(opponentRight->statStages[STAT_SPATK], DEFAULT_STAT_STAGE);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts only counts the redirecting target's Pressure when affected by Follow Me or Rage Powder")
{
    struct BattlePokemon *chosenTarget = NULL;
    enum Move redirectMove = MOVE_NONE;
    enum Ability abilityLeft = ABILITY_NONE, abilityRight = ABILITY_NONE;
    u32 pressureCount = 0;

    for (u32 j = 0; j < 8; j++)
    {
        PARAMETRIZE { redirectMove = MOVE_FOLLOW_ME; chosenTarget = j & 4 ? opponentRight : opponentLeft; abilityLeft = j & 1 ? ABILITY_PRESSURE : ABILITY_SHADOW_TAG; abilityRight = j & 2 ? ABILITY_PRESSURE : ABILITY_SHADOW_TAG; pressureCount = !!(j & 2); }
        PARAMETRIZE { redirectMove = MOVE_RAGE_POWDER; chosenTarget = j & 4 ? opponentRight : opponentLeft; abilityLeft = j & 1 ? ABILITY_PRESSURE : ABILITY_SHADOW_TAG; abilityRight = j & 2 ? ABILITY_PRESSURE : ABILITY_SHADOW_TAG; pressureCount = !!(j & 2); }
    }

    GIVEN {
        ASSUME(GetMoveEffect(MOVE_FOLLOW_ME) == EFFECT_FOLLOW_ME);
        ASSUME(GetMoveEffect(MOVE_RAGE_POWDER) == EFFECT_FOLLOW_ME);
        PLAYER(SPECIES_WOBBUFFET) { MovesWithPP({MOVE_DRAGON_DARTS, 10}); }
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET) { Ability(abilityLeft); }
        OPPONENT(SPECIES_WYNAUT) { Ability(abilityRight); }
    } WHEN {
        TURN { MOVE(opponentRight, redirectMove); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: chosenTarget); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, redirectMove, opponentRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentRight);
    } THEN {
        EXPECT_EQ(playerLeft->pp[0], 9 - pressureCount);
        EXPECT_EQ(opponentLeft->hp, opponentLeft->maxHP);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts counts Pressure on opponents avoided due to Protect or type immunity")
{
    struct BattlePokemon *chosenTarget = NULL;
    enum Species species;
    PARAMETRIZE { species = SPECIES_WOBBUFFET; chosenTarget = opponentLeft; }
    PARAMETRIZE { species = SPECIES_WOBBUFFET; chosenTarget = opponentRight; }
    PARAMETRIZE { species = SPECIES_FIDOUGH; chosenTarget = opponentLeft; }
    PARAMETRIZE { species = SPECIES_FIDOUGH; chosenTarget = opponentRight; }

    GIVEN {
        ASSUME(IsSpeciesOfType(SPECIES_FIDOUGH, TYPE_FAIRY));
        PLAYER(SPECIES_WOBBUFFET) { MovesWithPP({MOVE_DRAGON_DARTS, 10}); }
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(species) { Ability(ABILITY_PRESSURE); }
        OPPONENT(SPECIES_WYNAUT) { Ability(ABILITY_PRESSURE); }
    } WHEN {
        TURN {
            if (species == SPECIES_WOBBUFFET) MOVE(opponentLeft, MOVE_PROTECT);
            MOVE(playerLeft, MOVE_DRAGON_DARTS, target: chosenTarget);
        }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentRight);
    } THEN {
        EXPECT_EQ(playerLeft->pp[0], 7);
        EXPECT_EQ(opponentLeft->hp, opponentLeft->maxHP);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts still counts both Pressure abilities when it ignores Follow Me or Rage Powder")
{
    enum Move redirectMove;
    enum Ability ability;
    PARAMETRIZE { redirectMove = MOVE_FOLLOW_ME; ability = ABILITY_STALWART; }
    PARAMETRIZE { redirectMove = MOVE_FOLLOW_ME; ability = ABILITY_PROPELLER_TAIL; }
    PARAMETRIZE { redirectMove = MOVE_RAGE_POWDER; ability = ABILITY_OVERCOAT; }

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(ability); MovesWithPP({MOVE_DRAGON_DARTS, 10}); }
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WOBBUFFET) { Ability(ABILITY_PRESSURE); }
        OPPONENT(SPECIES_WYNAUT) { Ability(ABILITY_PRESSURE); }
    } WHEN {
        TURN { MOVE(opponentRight, redirectMove); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: opponentLeft); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentLeft);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentRight);
    } THEN {
        EXPECT_EQ(playerLeft->pp[0], 7);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts fails after Ally Switch if both the user and its ally are immune")
{
    GIVEN {
        ASSUME(IsSpeciesOfType(SPECIES_FIDOUGH, TYPE_FAIRY));
        PLAYER(SPECIES_FIDOUGH);
        PLAYER(SPECIES_WYNAUT) { Ability(ABILITY_TELEPATHY); }
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(playerRight, MOVE_ALLY_SWITCH); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: playerRight); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ALLY_SWITCH, playerRight);
        NONE_OF {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerRight);
            HP_BAR(playerLeft);
            HP_BAR(playerRight);
            HP_BAR(opponentLeft);
            HP_BAR(opponentRight);
        }
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts with Infiltrator bypasses its user's Substitute after Ally Switch")
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_CLEAR_BODY; }
    PARAMETRIZE { ability = ABILITY_INFILTRATOR; }

    GIVEN {
        WITH_CONFIG(B_INFILTRATOR_SUBSTITUTE, GEN_6);
        ASSUME(GetMoveEffect(MOVE_SUBSTITUTE) == EFFECT_SUBSTITUTE);
        ASSUME(!MoveIgnoresSubstitute(MOVE_DRAGON_DARTS));
        PLAYER(SPECIES_DRAGAPULT) { Ability(ability); Attack(100); Defense(200); Moves(MOVE_SUBSTITUTE, MOVE_DRAGON_DARTS, MOVE_ALLY_SWITCH, MOVE_CELEBRATE); }
        PLAYER(SPECIES_WYNAUT) { Moves(MOVE_SUBSTITUTE, MOVE_DRAGON_DARTS, MOVE_ALLY_SWITCH, MOVE_CELEBRATE); }
        OPPONENT(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WYNAUT);
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_SUBSTITUTE); }
        TURN { MOVE(playerRight, MOVE_ALLY_SWITCH); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: playerRight); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SUBSTITUTE, playerLeft);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_ALLY_SWITCH, playerRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerRight);
        if (ability == ABILITY_INFILTRATOR)
            HP_BAR(playerRight);
        else
            SUB_HIT(playerRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerRight);
        HP_BAR(playerLeft);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts restores Infiltrator before its second hit when Neutralizing Gas faints")
{
    enum Ability ability;
    PARAMETRIZE { ability = ABILITY_CLEAR_BODY; }
    PARAMETRIZE { ability = ABILITY_INFILTRATOR; }

    GIVEN {
        WITH_CONFIG(B_INFILTRATOR_SUBSTITUTE, GEN_6);
        ASSUME(GetMoveEffect(MOVE_SUBSTITUTE) == EFFECT_SUBSTITUTE);
        ASSUME(!MoveIgnoresSubstitute(MOVE_DRAGON_DARTS));
        PLAYER(SPECIES_DRAGAPULT) { Ability(ability); }
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WEEZING) { Ability(ABILITY_NEUTRALIZING_GAS); HP(1); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponentRight, MOVE_SUBSTITUTE); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: opponentLeft); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_SUBSTITUTE, opponentRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentLeft);
        MESSAGE("The effects of the neutralizing gas wore off!");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        if (ability == ABILITY_INFILTRATOR)
            HP_BAR(opponentRight);
        else
            SUB_HIT(opponentRight);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts restores damage abilities before its second hit when Neutralizing Gas faints", s16 damage)
{
    enum Ability abilityAtk, abilityDef;
    PARAMETRIZE { abilityAtk = ABILITY_HUGE_POWER; abilityDef = ABILITY_SHADOW_TAG; }
    PARAMETRIZE { abilityAtk = ABILITY_SHADOW_TAG; abilityDef = ABILITY_SHADOW_TAG; }
    PARAMETRIZE { abilityAtk = ABILITY_SHADOW_TAG; abilityDef = ABILITY_FUR_COAT; }

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(abilityAtk); Attack(100); }
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WEEZING) { Ability(ABILITY_NEUTRALIZING_GAS); HP(1); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(abilityDef); Defense(100); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_DRAGON_DARTS, target: opponentLeft); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentLeft);
        MESSAGE("The effects of the neutralizing gas wore off!");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentRight, captureDamage: &results[i].damage);
    } FINALLY {
        EXPECT_MUL_EQ(results[1].damage, Q_4_12(2), results[0].damage);
        EXPECT_MUL_EQ(results[2].damage, Q_4_12(2), results[1].damage);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts does not recheck Klutz and Ring Target after its first hit removes Neutralizing Gas")
{
    GIVEN {
        ASSUME(IsSpeciesOfType(SPECIES_FIDOUGH, TYPE_FAIRY));
        ASSUME(GetItemHoldEffect(ITEM_RING_TARGET) == HOLD_EFFECT_RING_TARGET);
        PLAYER(SPECIES_WOBBUFFET);
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WEEZING) { Ability(ABILITY_NEUTRALIZING_GAS); HP(1); }
        OPPONENT(SPECIES_FIDOUGH) { Ability(ABILITY_KLUTZ); Item(ITEM_RING_TARGET); }
    } WHEN {
        TURN { MOVE(playerLeft, MOVE_DRAGON_DARTS, target: opponentLeft); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentLeft);
        MESSAGE("The effects of the neutralizing gas wore off!");
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentRight);
    }
}

DOUBLE_BATTLE_TEST("Dragon Darts does not retarget a semi-invulnerable opponent when Neutralizing Gas ends and restores No Guard")
{
    enum Ability abilityAtk, abilityDef;
    PARAMETRIZE { abilityAtk = ABILITY_NO_GUARD; abilityDef = ABILITY_SHADOW_TAG; }
    PARAMETRIZE { abilityAtk = ABILITY_SHADOW_TAG; abilityDef = ABILITY_NO_GUARD; }

    GIVEN {
        PLAYER(SPECIES_WOBBUFFET) { Ability(abilityAtk); }
        PLAYER(SPECIES_WYNAUT);
        OPPONENT(SPECIES_WEEZING) { Ability(ABILITY_NEUTRALIZING_GAS); HP(1); }
        OPPONENT(SPECIES_WOBBUFFET) { Ability(abilityDef); }
    } WHEN {
        TURN { MOVE(opponentRight, MOVE_FLY, target: playerLeft); MOVE(playerLeft, MOVE_DRAGON_DARTS, target: opponentLeft); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_FLY, opponentRight);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
        HP_BAR(opponentLeft);
        MESSAGE("The effects of the neutralizing gas wore off!");
        NONE_OF {
            ANIMATION(ANIM_TYPE_MOVE, MOVE_DRAGON_DARTS, playerLeft);
            HP_BAR(opponentRight);
        }
    } THEN {
        EXPECT_EQ(opponentRight->hp, opponentRight->maxHP);
    }
}
