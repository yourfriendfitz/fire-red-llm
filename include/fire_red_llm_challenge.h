#ifndef GUARD_FIRE_RED_LLM_CHALLENGE_H
#define GUARD_FIRE_RED_LLM_CHALLENGE_H

#include "global.h"

void FireRedLLM_InitChallenge(void);
void FireRedLLM_HealAndEnforceLevelCap(void);
void FireRedLLM_CompleteChallenge(void);
void FireRedLLM_MarkChallengeLost(void);
bool8 FireRedLLM_IsChallengeActive(void);
bool8 FireRedLLM_IsChallengeLost(void);
bool8 FireRedLLM_ShouldRestrictStartMenu(void);

#endif // GUARD_FIRE_RED_LLM_CHALLENGE_H
