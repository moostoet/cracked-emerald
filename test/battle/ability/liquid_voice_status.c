#include "global.h"
#include "test/battle.h"

SINGLE_BATTLE_TEST("Liquid Voice turns Perish Song into a Water-type move and triggers Water Absorb")
{
    GIVEN {
        PLAYER(SPECIES_PRIMARINA) { Ability(ABILITY_LIQUID_VOICE); }
        OPPONENT(SPECIES_POLITOED) { Ability(ABILITY_WATER_ABSORB); HP(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_PERISH_SONG); }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_PERISH_SONG, player);
        ABILITY_POPUP(opponent, ABILITY_WATER_ABSORB);
        HP_BAR(opponent);
    }
}
