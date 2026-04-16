#include "global.h"
#include "battle_factory_screen.h"
#include "battle_frontier.h"
#include "brock_challenge.h"
#include "constants/battle_frontier.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/species.h"
#include "event_data.h"
#include "load_save.h"
#include "pokemon.h"
#include "random.h"

#define BROCK_RENTAL_OFFER_COUNT 6

static void ClearBrockRentalData(void)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(gSaveBlock2Ptr->frontier.rentalMons); i++)
    {
        gSaveBlock2Ptr->frontier.rentalMons[i].monId = 0xFFFF;
        gSaveBlock2Ptr->frontier.rentalMons[i].ivs = 0;
        gSaveBlock2Ptr->frontier.rentalMons[i].personality = 0;
        gSaveBlock2Ptr->frontier.rentalMons[i].abilityNum = 0;
    }
}

const struct TrainerMon gBrockRentalMons[BROCK_RENTAL_MONS_COUNT] =
{
    {
        .species = SPECIES_GEODUDE,
        .moves = {MOVE_ROCK_TOMB, MOVE_MAGNITUDE, MOVE_DEFENSE_CURL, MOVE_TACKLE},
        .heldItem = ITEM_HARD_STONE,
        .nature = NATURE_ADAMANT,
        .ball = BALL_POKE
    },
    {
        .species = SPECIES_SANDSHREW,
        .moves = {MOVE_MUD_SLAP, MOVE_RAPID_SPIN, MOVE_SAND_ATTACK, MOVE_SCRATCH},
        .heldItem = ITEM_SOFT_SAND,
        .nature = NATURE_IMPISH,
        .ball = BALL_POKE
    },
    {
        .species = SPECIES_DIGLETT,
        .moves = {MOVE_MAGNITUDE, MOVE_MUD_SLAP, MOVE_ASTONISH, MOVE_SAND_ATTACK},
        .heldItem = ITEM_SOFT_SAND,
        .nature = NATURE_JOLLY,
        .ball = BALL_POKE
    },
    {
        .species = SPECIES_ONIX,
        .moves = {MOVE_ROCK_TOMB, MOVE_BIND, MOVE_SCREECH, MOVE_TACKLE},
        .heldItem = ITEM_HARD_STONE,
        .nature = NATURE_IMPISH,
        .ball = BALL_POKE
    },
    {
        .species = SPECIES_CUBONE,
        .moves = {MOVE_BONE_CLUB, MOVE_HEADBUTT, MOVE_GROWL, MOVE_FOCUS_ENERGY},
        .heldItem = ITEM_ORAN_BERRY,
        .nature = NATURE_ADAMANT,
        .ball = BALL_POKE
    },
    {
        .species = SPECIES_RHYHORN,
        .moves = {MOVE_HORN_ATTACK, MOVE_ROCK_TOMB, MOVE_TAIL_WHIP, MOVE_FURY_ATTACK},
        .heldItem = ITEM_HARD_STONE,
        .nature = NATURE_ADAMANT,
        .ball = BALL_POKE
    },
    {
        .species = SPECIES_OMANYTE,
        .moves = {MOVE_WATER_GUN, MOVE_MUD_SHOT, MOVE_ROCK_TOMB, MOVE_WITHDRAW},
        .heldItem = ITEM_ORAN_BERRY,
        .nature = NATURE_MODEST,
        .ball = BALL_POKE
    },
    {
        .species = SPECIES_KABUTO,
        .moves = {MOVE_SCRATCH, MOVE_MUD_SHOT, MOVE_HARDEN, MOVE_ABSORB},
        .heldItem = ITEM_ORAN_BERRY,
        .nature = NATURE_ADAMANT,
        .ball = BALL_POKE
    },
    {
        .species = SPECIES_PHANPY,
        .moves = {MOVE_ROLLOUT, MOVE_DEFENSE_CURL, MOVE_MUD_SLAP, MOVE_TACKLE},
        .heldItem = ITEM_SOFT_SAND,
        .nature = NATURE_IMPISH,
        .ball = BALL_POKE
    },
    {
        .species = SPECIES_SWINUB,
        .moves = {MOVE_POWDER_SNOW, MOVE_MUD_SLAP, MOVE_ODOR_SLEUTH, MOVE_ENDURE},
        .heldItem = ITEM_NEVER_MELT_ICE,
        .nature = NATURE_QUIET,
        .ball = BALL_POKE
    },
    {
        .species = SPECIES_NOSEPASS,
        .moves = {MOVE_ROCK_THROW, MOVE_THUNDER_WAVE, MOVE_BLOCK, MOVE_HARDEN},
        .heldItem = ITEM_ORAN_BERRY,
        .nature = NATURE_BOLD,
        .ball = BALL_POKE
    },
    {
        .species = SPECIES_ARON,
        .moves = {MOVE_METAL_CLAW, MOVE_ROCK_TOMB, MOVE_HARDEN, MOVE_MUD_SLAP},
        .heldItem = ITEM_METAL_COAT,
        .nature = NATURE_BRAVE,
        .ball = BALL_POKE
    },
    {
        .species = SPECIES_TRAPINCH,
        .moves = {MOVE_SAND_TOMB, MOVE_BITE, MOVE_MUD_SLAP, MOVE_SAND_ATTACK},
        .heldItem = ITEM_SOFT_SAND,
        .nature = NATURE_ADAMANT,
        .ball = BALL_POKE
    },
    {
        .species = SPECIES_BALTOY,
        .moves = {MOVE_CONFUSION, MOVE_ROCK_TOMB, MOVE_HARDEN, MOVE_MUD_SLAP},
        .heldItem = ITEM_ORAN_BERRY,
        .nature = NATURE_CALM,
        .ball = BALL_POKE
    },
    {
        .species = SPECIES_LILEEP,
        .moves = {MOVE_MEGA_DRAIN, MOVE_ROCK_TOMB, MOVE_CONSTRICT, MOVE_ACID},
        .heldItem = ITEM_MIRACLE_SEED,
        .nature = NATURE_MODEST,
        .ball = BALL_POKE
    },
    {
        .species = SPECIES_ANORITH,
        .moves = {MOVE_ROCK_TOMB, MOVE_FURY_CUTTER, MOVE_SCRATCH, MOVE_HARDEN},
        .heldItem = ITEM_SILVER_POWDER,
        .nature = NATURE_JOLLY,
        .ball = BALL_POKE
    },
    {
        .species = SPECIES_LARVITAR,
        .moves = {MOVE_ROCK_TOMB, MOVE_BITE, MOVE_SANDSTORM, MOVE_LEER},
        .heldItem = ITEM_HARD_STONE,
        .nature = NATURE_ADAMANT,
        .ball = BALL_POKE
    },
    {
        .species = SPECIES_WOOPER,
        .moves = {MOVE_MUD_SHOT, MOVE_WATER_GUN, MOVE_TAIL_WHIP, MOVE_SLAM},
        .heldItem = ITEM_SOFT_SAND,
        .nature = NATURE_RELAXED,
        .ball = BALL_POKE
    },
};

void GenerateBrockRentalMons(void)
{
    u32 i;

    ClearBrockRentalData();

    for (i = 0; i < BROCK_RENTAL_OFFER_COUNT; i++)
    {
        u16 monId;
        bool32 duplicate;
        u32 j;

        do
        {
            duplicate = FALSE;
            monId = Random() % BROCK_RENTAL_MONS_COUNT;
            for (j = 0; j < i; j++)
            {
                if (gSaveBlock2Ptr->frontier.rentalMons[j].monId == monId)
                {
                    duplicate = TRUE;
                    break;
                }
            }
        } while (duplicate);

        gSaveBlock2Ptr->frontier.rentalMons[i].monId = monId;
    }
}

void SelectBrockRentalMons(void)
{
    ZeroPlayerPartyMons();
    DoBrockRentalSelectScreen();
}

void SelectBrockRewardMon(void)
{
    DoBrockRewardSelectScreen();
}

void CheckBrockChallengePerfectClear(void)
{
    u32 i;

    gSpecialVar_Result = TRUE;
    for (i = 0; i < FRONTIER_PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) == SPECIES_NONE
            || GetMonData(&gPlayerParty[i], MON_DATA_HP) == 0)
        {
            gSpecialVar_Result = FALSE;
            break;
        }
    }
}

void RestoreBrockPlayerParty(void)
{
    LoadPlayerParty();
}

void GiveBrockSelectedRental(void)
{
    struct Pokemon rewardMon;
    u16 heldItem = ITEM_NONE;
    u16 species;
    u8 selectedSlot = gSpecialVar_0x8004;

    if (selectedSlot >= FRONTIER_PARTY_SIZE
        || GetMonData(&gPlayerParty[selectedSlot], MON_DATA_SPECIES) == SPECIES_NONE)
    {
        LoadPlayerParty();
        gSpecialVar_Result = MON_CANT_GIVE;
        return;
    }

    rewardMon = gPlayerParty[selectedSlot];
    HealPokemon(&rewardMon);
    SetMonData(&rewardMon, MON_DATA_HELD_ITEM, &heldItem);

    species = GetMonData(&rewardMon, MON_DATA_SPECIES);
    VarSet(VAR_TEMP_TRANSFERRED_SPECIES, species);

    LoadPlayerParty();
    gSpecialVar_Result = GiveCapturedMonToPlayer(&rewardMon);
}
