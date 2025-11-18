#include "global.h"
#include "test/battle.h"

ASSUMPTIONS
{
    ASSUME(GetMoveEffect(MOVE_PERISH_SONG) == EFFECT_PERISH_SONG);
}

SINGLE_BATTLE_TEST("Perish Song changed to Water activates Water Absorb and leaves the target unaffected")
{
    GIVEN {
        PLAYER(SPECIES_PRIMARINA) { Ability(ABILITY_LIQUID_VOICE); }
        OPPONENT(SPECIES_VAPOREON) { Ability(ABILITY_WATER_ABSORB); HP(50); MaxHP(100); }
    } WHEN {
        TURN { MOVE(player, MOVE_PERISH_SONG); }
        TURN { }
        TURN { }
        TURN { }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_PERISH_SONG, player);
        MESSAGE("All Pokémon that heard the song will faint in three turns!");
        ABILITY_POPUP(opponent, ABILITY_WATER_ABSORB);
        HP_BAR(opponent, damage: -25);

        MESSAGE("Primarina's perish count fell to 0!");
        MESSAGE("Primarina fainted!");
        NONE_OF {
            MESSAGE("The opposing Vaporeon's perish count fell to 0!");
            MESSAGE("The opposing Vaporeon fainted!");
        }
    }
}

SINGLE_BATTLE_TEST("Perish Song user is still affected after Electrify even with Volt Absorb")
{
    GIVEN {
        PLAYER(SPECIES_JOLTEON) { Ability(ABILITY_VOLT_ABSORB); Speed(5); }
        OPPONENT(SPECIES_HELIOLISK) { Speed(10); }
    } WHEN {
        TURN { MOVE(opponent, MOVE_ELECTRIFY, target: player); MOVE(player, MOVE_PERISH_SONG); }
        TURN { }
        TURN { }
        TURN { }
    } SCENE {
        ANIMATION(ANIM_TYPE_MOVE, MOVE_PERISH_SONG, player);
        MESSAGE("All Pokémon that heard the song will faint in three turns!");
        NONE_OF { ABILITY_POPUP(player, ABILITY_VOLT_ABSORB); }

        MESSAGE("Jolteon's perish count fell to 0!");
        MESSAGE("Jolteon fainted!");
        NONE_OF {
            MESSAGE("The opposing Heliolisk's perish count fell to 0!");
            MESSAGE("The opposing Heliolisk fainted!");
        }
    }
}
