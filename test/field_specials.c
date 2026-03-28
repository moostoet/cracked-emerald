#include "global.h"

#include "event_data.h"
#include "field_specials.h"
#include "pokedex.h"
#include "pokemon.h"
#include "test/test.h"
#include "wild_encounter.h"
#include "constants/maps.h"

static void SetTestMap(u8 mapGroup, u8 mapNum)
{
    gSaveBlock1Ptr->location.mapGroup = mapGroup;
    gSaveBlock1Ptr->location.mapNum = mapNum;
}

TEST("Thief Finder reports unseen encounters separately from empty held-item tables")
{
    ResetPokedex();
    SetTestMap(MAP_GROUP(MAP_OLDALE_TOWN), MAP_NUM(MAP_OLDALE_TOWN));

    EXPECT_NE(GetCurrentMapWildMonHeaderId(), HEADER_NONE);

    Special_GetRouteThievableItems();

    EXPECT_EQ(gSpecialVar_Result, 0);
    EXPECT_EQ(gSpecialVar_0x8006, THIEVABLE_ITEMS_STATUS_NO_ENCOUNTERS_SEEN);
}

TEST("Thief Finder reports Oldale Town as having no held items after an encounter is seen")
{
    ResetPokedex();
    SetTestMap(MAP_GROUP(MAP_OLDALE_TOWN), MAP_NUM(MAP_OLDALE_TOWN));

    EXPECT_NE(GetCurrentMapWildMonHeaderId(), HEADER_NONE);

    GetSetPokedexFlag(SpeciesToNationalPokedexNum(SPECIES_PANSEAR), FLAG_SET_SEEN);
    Special_GetRouteThievableItems();

    EXPECT_EQ(gSpecialVar_Result, 0);
    EXPECT_EQ(gSpecialVar_0x8006, THIEVABLE_ITEMS_STATUS_NO_ITEMS);
}

TEST("Thief Finder still reports held items when the map has eligible encounters")
{
    ResetPokedex();
    SetTestMap(MAP_GROUP(MAP_PETALBURG_WOODS), MAP_NUM(MAP_PETALBURG_WOODS));

    EXPECT_NE(GetCurrentMapWildMonHeaderId(), HEADER_NONE);

    GetSetPokedexFlag(SpeciesToNationalPokedexNum(SPECIES_SHROOMISH), FLAG_SET_SEEN);
    Special_GetRouteThievableItems();

    EXPECT_GT(gSpecialVar_Result, 0);
    EXPECT_EQ(gSpecialVar_0x8006, THIEVABLE_ITEMS_STATUS_HAS_ITEMS);
}
