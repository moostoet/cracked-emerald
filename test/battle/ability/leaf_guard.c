#include "global.h"
#include "test/battle.h"

SINGLE_BATTLE_TEST("Leaf Guard blocks the first status attempt, but a later status sticks")
{
    GIVEN {
        PLAYER(SPECIES_LEAFEON) { Ability(ABILITY_LEAF_GUARD); }
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_THUNDER_WAVE); MOVE(player, MOVE_CELEBRATE); }
        TURN { MOVE(opponent, MOVE_WILL_O_WISP); }
    } SCENE {
        // First attempt blocked
        ABILITY_POPUP(player, ABILITY_LEAF_GUARD);
        MESSAGE("It doesn't affect Leafeon…");
        NOT STATUS_ICON(player, paralysis: TRUE);
        // Second attempt succeeds after Leaf Guard is consumed
        ANIMATION(ANIM_TYPE_MOVE, MOVE_WILL_O_WISP, opponent);
        STATUS_ICON(player, burn: TRUE);
    }
}

SINGLE_BATTLE_TEST("Leaf Guard refreshes on switch out")
{
    GIVEN {
        PLAYER(SPECIES_LEAFEON) { Ability(ABILITY_LEAF_GUARD); }
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_WOBBUFFET);
    } WHEN {
        TURN { MOVE(opponent, MOVE_TOXIC); }
        TURN { SWITCH(player, 1); }
        TURN { SWITCH(player, 0); }
        TURN { MOVE(opponent, MOVE_THUNDER_WAVE); }
        TURN { MOVE(opponent, MOVE_WILL_O_WISP); }
    } SCENE {
        // First time in: block Toxic
        ABILITY_POPUP(player, ABILITY_LEAF_GUARD);
        MESSAGE("It doesn't affect Leafeon…");
        NOT STATUS_ICON(player, badPoison: TRUE);
        // After switching out and back, block again
        ABILITY_POPUP(player, ABILITY_LEAF_GUARD);
        MESSAGE("It doesn't affect Leafeon…");
        NOT STATUS_ICON(player, paralysis: TRUE);
        // Next status attempt after the block lands
        ANIMATION(ANIM_TYPE_MOVE, MOVE_WILL_O_WISP, opponent);
        STATUS_ICON(player, burn: TRUE);
    }
}

SINGLE_BATTLE_TEST("Leaf Guard blocks the first status from held orbs, then allows the next turn's orb proc")
{
    u16 item;
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
            STATUS_ICON(player, burn: TRUE);
        } else {
            STATUS_ICON(player, badPoison: TRUE);
        }
    }
}
