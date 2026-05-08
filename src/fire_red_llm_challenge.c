#include "global.h"
#include "event_data.h"
#include "fire_red_llm_challenge.h"
#include "pokemon.h"
#include "script_pokemon_util.h"
#include "constants/fire_red_llm_challenge.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/pokemon.h"
#include "constants/species.h"
#include "constants/vars.h"

#define FIRE_RED_LLM_FIXED_IV 20

static const u16 sFireRedLLMPartySpecies[] = {
    SPECIES_BULBASAUR,
    SPECIES_CHARMANDER,
    SPECIES_SQUIRTLE,
    SPECIES_PIKACHU,
    SPECIES_SANDSHREW,
    SPECIES_NIDORAN_F,
    SPECIES_NIDORAN_M,
    SPECIES_ODDISH,
    SPECIES_POLIWAG,
    SPECIES_MACHOP,
    SPECIES_GEODUDE,
    SPECIES_DODUO,
    SPECIES_CUBONE,
    SPECIES_HORSEA,
    SPECIES_EEVEE,
    SPECIES_CHIKORITA,
    SPECIES_CYNDAQUIL,
    SPECIES_TOTODILE,
    SPECIES_MAREEP,
    SPECIES_WOOPER,
    SPECIES_TREECKO,
    SPECIES_TORCHIC,
    SPECIES_MUDKIP,
    SPECIES_TAILLOW,
    SPECIES_SHROOMISH,
    SPECIES_MAKUHITA,
    SPECIES_ARON,
};

static const u16 sFireRedLLMHeldItems[] = {
    ITEM_CHERI_BERRY,
    ITEM_CHESTO_BERRY,
    ITEM_PECHA_BERRY,
    ITEM_RAWST_BERRY,
    ITEM_ASPEAR_BERRY,
    ITEM_ORAN_BERRY,
    ITEM_SITRUS_BERRY,
    ITEM_QUICK_CLAW,
    ITEM_LEFTOVERS,
};

static u32 AdvanceChallengeRng(u32 *seed)
{
    *seed = *seed * 1103515245 + 12345;
    return *seed;
}

static u16 PickPartySpecies(u32 *seed)
{
    return sFireRedLLMPartySpecies[AdvanceChallengeRng(seed) % ARRAY_COUNT(sFireRedLLMPartySpecies)];
}

static u16 PickHeldItem(u32 *seed)
{
    return sFireRedLLMHeldItems[AdvanceChallengeRng(seed) % ARRAY_COUNT(sFireRedLLMHeldItems)];
}

static void ClampMonToLevelCap(struct Pokemon *mon)
{
    u8 level;
    u16 species;
    u32 exp;
    u32 cappedExp;

    if (!GetMonData(mon, MON_DATA_SANITY_HAS_SPECIES) || GetMonData(mon, MON_DATA_SANITY_IS_EGG))
        return;

    level = GetMonData(mon, MON_DATA_LEVEL);
    species = GetMonData(mon, MON_DATA_SPECIES);
    exp = GetMonData(mon, MON_DATA_EXP);
    cappedExp = gExperienceTables[gSpeciesInfo[species].growthRate][FIRE_RED_LLM_CHALLENGE_LEVEL_CAP];

    if (level > FIRE_RED_LLM_CHALLENGE_LEVEL_CAP || exp > cappedExp)
    {
        level = FIRE_RED_LLM_CHALLENGE_LEVEL_CAP;
        SetMonData(mon, MON_DATA_LEVEL, &level);
        SetMonData(mon, MON_DATA_EXP, &cappedExp);
        CalculateMonStats(mon);
    }
}

static void CreateSeededParty(u16 seed)
{
    u8 i;
    u16 species[2];
    u32 rng = seed;

    species[0] = PickPartySpecies(&rng);
    do
    {
        species[1] = PickPartySpecies(&rng);
    } while (species[1] == species[0]);

    ZeroPlayerPartyMons();

    for (i = 0; i < ARRAY_COUNT(species); i++)
    {
        u16 heldItem;
        u32 personality = AdvanceChallengeRng(&rng);

        CreateMon(&gPlayerParty[i],
                  species[i],
                  FIRE_RED_LLM_CHALLENGE_LEVEL_CAP,
                  FIRE_RED_LLM_FIXED_IV,
                  TRUE,
                  personality,
                  OT_ID_PLAYER_ID,
                  0);

        heldItem = PickHeldItem(&rng);
        SetMonData(&gPlayerParty[i], MON_DATA_HELD_ITEM, &heldItem);
    }

    gPlayerPartyCount = CalculatePlayerPartyCount();
    FireRedLLM_HealAndEnforceLevelCap();
}

void FireRedLLM_InitChallenge(void)
{
    VarSet(VAR_FIRE_RED_LLM_CHALLENGE_STATE, FIRE_RED_LLM_CHALLENGE_STATE_ACTIVE);
    VarSet(VAR_FIRE_RED_LLM_CHALLENGE_SEED, FIRE_RED_LLM_CHALLENGE_SEED);
    VarSet(VAR_FIRE_RED_LLM_CHALLENGE_SCENE, FIRE_RED_LLM_CHALLENGE_SCENE_INTRO);

    FlagSet(FLAG_FIRE_RED_LLM_CHALLENGE_STARTED);
    FlagClear(FLAG_FIRE_RED_LLM_CHALLENGE_COMPLETE);
    FlagClear(FLAG_FIRE_RED_LLM_CHALLENGE_LOST);
    FlagSet(FLAG_SYS_POKEMON_GET);

    CreateSeededParty(FIRE_RED_LLM_CHALLENGE_SEED);
}

void FireRedLLM_HealAndEnforceLevelCap(void)
{
    u8 i;

    for (i = 0; i < PARTY_SIZE; i++)
        ClampMonToLevelCap(&gPlayerParty[i]);

    HealPlayerParty();
}

void FireRedLLM_CompleteChallenge(void)
{
    VarSet(VAR_FIRE_RED_LLM_CHALLENGE_STATE, FIRE_RED_LLM_CHALLENGE_STATE_WON);
    FlagSet(FLAG_FIRE_RED_LLM_CHALLENGE_COMPLETE);
    FlagClear(FLAG_FIRE_RED_LLM_CHALLENGE_LOST);
    FireRedLLM_HealAndEnforceLevelCap();
}

void FireRedLLM_MarkChallengeLost(void)
{
    VarSet(VAR_FIRE_RED_LLM_CHALLENGE_STATE, FIRE_RED_LLM_CHALLENGE_STATE_LOST);
    FlagSet(FLAG_FIRE_RED_LLM_CHALLENGE_LOST);
    FireRedLLM_HealAndEnforceLevelCap();
}

bool8 FireRedLLM_IsChallengeActive(void)
{
    return FlagGet(FLAG_FIRE_RED_LLM_CHALLENGE_STARTED)
        && VarGet(VAR_FIRE_RED_LLM_CHALLENGE_STATE) == FIRE_RED_LLM_CHALLENGE_STATE_ACTIVE;
}

bool8 FireRedLLM_IsChallengeLost(void)
{
    return FlagGet(FLAG_FIRE_RED_LLM_CHALLENGE_LOST)
        && VarGet(VAR_FIRE_RED_LLM_CHALLENGE_STATE) == FIRE_RED_LLM_CHALLENGE_STATE_LOST;
}

bool8 FireRedLLM_ShouldRestrictStartMenu(void)
{
    return FlagGet(FLAG_FIRE_RED_LLM_CHALLENGE_STARTED)
        && !FlagGet(FLAG_FIRE_RED_LLM_CHALLENGE_COMPLETE);
}
