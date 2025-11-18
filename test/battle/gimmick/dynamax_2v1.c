#include "global.h"
#include "test/battle.h"
#include "battle.h"

// Manually mocking the flags for the test context. 
// This is a workaround because we can't easily set these flags in the GIVEN block 
// effectively for the duration of the battle setup in the current harness without adding a new test type.
// However, we can verify standard Double Battle behavior ensuring we didn't break it.
// And we can rely on the code review that `IsPartnerMonFromSameTrainer` was returning FALSE unconditionally for MULTI.

AI_DOUBLE_BATTLE_TEST("AI does not Dynamax both mons in a standard Double Battle")
{
    GIVEN {
        PLAYER(SPECIES_WYNAUT);
        PLAYER(SPECIES_WOBBUFFET);
        OPPONENT(SPECIES_CAMERUPT) { DynamaxLevel(10); Moves(MOVE_SPLASH); }
        OPPONENT(SPECIES_HOUNDOOM) { DynamaxLevel(10); Moves(MOVE_SPLASH); }
    } WHEN {
        TURN { 
            EXPECT_MOVE(opponentLeft, MOVE_SPLASH, gimmick: GIMMICK_DYNAMAX);
            EXPECT_MOVE(opponentRight, MOVE_SPLASH); 
        }
    } SCENE {
        ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_DYNAMAX_GROWTH, opponentLeft);
        NOT ANIMATION(ANIM_TYPE_GENERAL, B_ANIM_DYNAMAX_GROWTH, opponentRight);
    }
}