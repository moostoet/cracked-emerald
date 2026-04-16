#ifndef GUARD_BROCK_CHALLENGE_H
#define GUARD_BROCK_CHALLENGE_H

#include "data.h"

#define BROCK_RENTAL_MONS_COUNT 18

extern const struct TrainerMon gBrockRentalMons[BROCK_RENTAL_MONS_COUNT];

void GenerateBrockRentalMons(void);
void SelectBrockRentalMons(void);
void SelectBrockRewardMon(void);
void CheckBrockChallengePerfectClear(void);
void RestoreBrockPlayerParty(void);
void GiveBrockSelectedRental(void);

#endif // GUARD_BROCK_CHALLENGE_H
