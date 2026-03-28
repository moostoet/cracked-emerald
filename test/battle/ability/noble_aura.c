#include "global.h"
#include "test/battle.h"

SINGLE_BATTLE_TEST("Noble Aura does not reactivate when only the opposing replacement switches in after a KO")
{
    enum Ability ability;

    PARAMETRIZE { ability = ABILITY_NOBLE_AURA; }
    PARAMETRIZE { ability = ABILITY_SHED_SKIN; }

    GIVEN {
        PLAYER(SPECIES_SERVINE) { Ability(ability); Moves(MOVE_SCRATCH); Speed(2); }
        OPPONENT(SPECIES_HERDIER) { HP(1); Speed(1); }
        OPPONENT(SPECIES_AIPOM) { Speed(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); SEND_OUT(opponent, 1); }
    } SCENE {
        if (ability == ABILITY_NOBLE_AURA)
            ABILITY_POPUP(player, ABILITY_NOBLE_AURA);
        HP_BAR(opponent);
    } THEN {
        EXPECT_EQ(opponent->statStages[STAT_SPATK], DEFAULT_STAT_STAGE);
    }
}
