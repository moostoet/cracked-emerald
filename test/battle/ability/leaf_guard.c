#include "global.h"
#include "test/battle.h"

SINGLE_BATTLE_TEST("Leaf Guard blocks the first status attempt, but a later status sticks")
{
    enum Move move;
    u16 status;
    PARAMETRIZE { move = MOVE_WILL_O_WISP; status = STATUS1_BURN; }
    PARAMETRIZE { move = MOVE_HYPNOSIS; status = STATUS1_SLEEP; }
    PARAMETRIZE { move = MOVE_THUNDER_WAVE; status = STATUS1_PARALYSIS; }
    PARAMETRIZE { move = MOVE_TOXIC; status = STATUS1_TOXIC_POISON; }
    // PARAMETRIZE { move = MOVE_POWDER_SNOW; status = STATUS1_FREEZE; } // Pointless since you can't freeze in sunlight anyway
    GIVEN {
        PLAYER(SPECIES_LEAFEON) { Ability(ABILITY_LEAF_GUARD); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, move); MOVE(player, MOVE_CELEBRATE); }
        TURN { MOVE(opponent, move); }
    } SCENE {
        // First attempt blocked
        ABILITY_POPUP(player, ABILITY_LEAF_GUARD);
        MESSAGE("It doesn't affect Leafeon…");
        NOT STATUS_ICON(player, status);
        // Second attempt succeeds after Leaf Guard is consumed
        ANIMATION(ANIM_TYPE_MOVE, move, opponent);
        STATUS_ICON(player, status);
    }
}

SINGLE_BATTLE_TEST("Leaf Guard refreshes on switch out")
{
    enum Move move;
    u16 status;
    PARAMETRIZE { move = MOVE_WILL_O_WISP;  status = STATUS1_BURN; }
    PARAMETRIZE { move = MOVE_HYPNOSIS;     status = STATUS1_SLEEP; }
    PARAMETRIZE { move = MOVE_THUNDER_WAVE; status = STATUS1_PARALYSIS; }
    PARAMETRIZE { move = MOVE_TOXIC;        status = STATUS1_TOXIC_POISON; }
    // PARAMETRIZE { move = MOVE_POWDER_SNOW; status = STATUS1_FREEZE; } // Pointless since you can't freeze in sunlight anyway
    GIVEN {
        PLAYER(SPECIES_LEAFEON) { Ability(ABILITY_LEAF_GUARD); }
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, move); }
        TURN { SWITCH(player, 1); }
        TURN { SWITCH(player, 0); }
        TURN { MOVE(opponent, move); }
        TURN { MOVE(opponent, move); }
    } SCENE {
        // First time in: block the first status attempt
        ABILITY_POPUP(player, ABILITY_LEAF_GUARD);
        MESSAGE("It doesn't affect Leafeon…");
        NOT STATUS_ICON(player, status);
        // After switching out and back, block again
        ABILITY_POPUP(player, ABILITY_LEAF_GUARD);
        MESSAGE("It doesn't affect Leafeon…");
        NOT STATUS_ICON(player, status);
        // Next status attempt after the block lands
        ANIMATION(ANIM_TYPE_MOVE, move, opponent);
        STATUS_ICON(player, status);
    }
}

SINGLE_BATTLE_TEST("Leaf Guard blocks the first status from held orbs, then allows the next turn's orb proc")
{
    enum Item item;
    PARAMETRIZE { item = ITEM_FLAME_ORB; }
    PARAMETRIZE { item = ITEM_TOXIC_ORB; }
    GIVEN {
        ASSUME(gItemsInfo[ITEM_FLAME_ORB].holdEffect == HOLD_EFFECT_FLAME_ORB);
        ASSUME(gItemsInfo[ITEM_TOXIC_ORB].holdEffect == HOLD_EFFECT_TOXIC_ORB);
        PLAYER(SPECIES_LEAFEON) { Ability(ABILITY_LEAF_GUARD); Item(item); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN {} // End of turn 1: orb tries and is blocked
        TURN {} // End of turn 2: orb should succeed
    } SCENE {
        // Orb eventually applies status after Leaf Guard is consumed
        if (item == ITEM_FLAME_ORB) {
            NONE_OF { MESSAGE("Leafeon was burned!"); STATUS_ICON(player, burn: TRUE); }
        }
        else {
            NONE_OF { MESSAGE("Leafeon was badly poisoned!"); STATUS_ICON(player, badPoison: TRUE); }
        }
    }
}

SINGLE_BATTLE_TEST("Leaf Guard doesn't prevent status conditions from Flame Orb and Toxic Orb if Cloud Nine/Air Lock is on the field")
{
    enum Item item;
    u32 species;
    enum Ability ability;
    PARAMETRIZE { item = ITEM_FLAME_ORB; species = SPECIES_GOLDUCK;  ability = ABILITY_CLOUD_NINE; }
    PARAMETRIZE { item = ITEM_TOXIC_ORB; species = SPECIES_GOLDUCK;  ability = ABILITY_CLOUD_NINE; }
    PARAMETRIZE { item = ITEM_FLAME_ORB; species = SPECIES_RAYQUAZA; ability = ABILITY_AIR_LOCK; }
    PARAMETRIZE { item = ITEM_TOXIC_ORB; species = SPECIES_RAYQUAZA; ability = ABILITY_AIR_LOCK; }
    GIVEN {
        ASSUME(gItemsInfo[ITEM_FLAME_ORB].holdEffect == HOLD_EFFECT_FLAME_ORB);
        ASSUME(gItemsInfo[ITEM_TOXIC_ORB].holdEffect == HOLD_EFFECT_TOXIC_ORB);
        PLAYER(SPECIES_LEAFEON) { Ability(ABILITY_LEAF_GUARD); Item(item); }
        OPPONENT(species) { Ability(ability); }
    } WHEN {
        TURN { MOVE(player, MOVE_SUNNY_DAY); }
    } SCENE {
        if (item == ITEM_FLAME_ORB) {
            MESSAGE("Leafeon was burned!");
            STATUS_ICON(player, burn: TRUE);
        } else {
            STATUS_ICON(player, badPoison: TRUE);
        }
    }
}

SINGLE_BATTLE_TEST("Leaf Guard prevents Rest during sun (Gen 5+)")
{
    u32 gen;
    PARAMETRIZE { gen = GEN_4; }
    PARAMETRIZE { gen = GEN_5; }
    GIVEN {
        WITH_CONFIG(B_LEAF_GUARD_PREVENTS_REST, gen);
        ASSUME(GetMoveEffect(MOVE_REST) == EFFECT_REST);
        PLAYER(SPECIES_LEAFEON) { Ability(ABILITY_LEAF_GUARD); HP(100); MaxHP(200); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUNNY_DAY); MOVE(player, MOVE_REST); }
    } SCENE {
        if (gen >= GEN_5) {
            NONE_OF {
                ANIMATION(ANIM_TYPE_MOVE, MOVE_REST, player);
                STATUS_ICON(player, sleep: TRUE);
                HP_BAR(player);
            }
        }
        else {
            STATUS_ICON(player, sleep: TRUE);
            ANIMATION(ANIM_TYPE_MOVE, MOVE_REST, player);
            HP_BAR(player);
        }
    }
}

SINGLE_BATTLE_TEST("Leaf Guard doesn't prevent Rest if Cloud Nine/Air Lock is on the field")
{
    u32 species, ability;
    PARAMETRIZE { species = SPECIES_GOLDUCK;  ability = ABILITY_CLOUD_NINE; }
    PARAMETRIZE { species = SPECIES_GOLDUCK;  ability = ABILITY_CLOUD_NINE; }
    PARAMETRIZE { species = SPECIES_RAYQUAZA; ability = ABILITY_AIR_LOCK; }
    PARAMETRIZE { species = SPECIES_RAYQUAZA; ability = ABILITY_AIR_LOCK; }
    GIVEN {
        WITH_CONFIG(B_LEAF_GUARD_PREVENTS_REST, GEN_5);
        ASSUME(GetMoveEffect(MOVE_REST) == EFFECT_REST);
        PLAYER(SPECIES_LEAFEON) { Ability(ABILITY_LEAF_GUARD); HP(100); MaxHP(200); }
        OPPONENT(species) { Ability(ability); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_SUNNY_DAY); MOVE(player, MOVE_REST); }
    } SCENE {
        STATUS_ICON(player, sleep: TRUE);
        ANIMATION(ANIM_TYPE_MOVE, MOVE_REST, player);
        HP_BAR(player);
    }
}
