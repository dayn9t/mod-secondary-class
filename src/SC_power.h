#ifndef MOD_SC_POWER_H
#define MOD_SC_POWER_H

#include "SharedDefines.h"  // Classes, Powers

#include <cstdint>

// Functional Core — pure helpers for the Phase-2 synthetic mana pool.
//
// Problem (Phase-1 exclusion): a physical-primary player (warrior rage / rogue
// energy / DK runic) taking a caster secondary has no mana bar. AC computes mana
// spell cost as ManaCostPercentage * GetCreateMana(); a physical primary's create
// mana is 0, so the cost is 0 and the spells cast for free (verified empirically
// + SpellInfo.cpp:2830). The fix grants a real base mana via SetCreateMana so AC's
// own cost / pool-max / regen logic all take over unchanged.
//
// Pure: no Unit mutation, deterministic, unit-testable. The mutating shell lives
// in SC_player_hooks (SC::Shell::ApplySecondaryPower / RemoveSecondaryPower).
namespace SC::Power
{
    // True iff this primary needs a synthetic mana pool to cast the secondary's
    // mana spells — i.e. the primary draws no mana bar but the secondary is a
    // mana caster. The reverse (caster primary + physical secondary, e.g. mage +
    // rogue) is NOT covered by Phase 2: there is no SetCreateEnergy, and a caster
    // already has a mana bar.
    bool NeedsSyntheticMana(Classes primary, Classes secondary);

    // Base mana to grant for the secondary caster class at the player's level.
    // SecondaryPowerBase config (>0) overrides for tuning; otherwise the exact AC
    // value from player_classlevel_stats (single source of truth — the same table
    // AC itself reads in GiveLevel / InitStatsForLevel).
    uint32 ComputeSecondaryBaseMana(Classes secondary, uint8 level);

    // Passive mp5 injected via Player::ApplyManaRegenBonus, as a configurable
    // PERCENT of the secondary's base mana (scales with level). mp5 = base * pct/100.
    // Scaling keeps regen proportional across levels (unlike a flat constant, which
    // is huge on a tiny low-level pool and trivial at 80). Default pct is low so the
    // secondary regenerates slower than a real caster (which has spirit+int+talents).
    // Drinking/food also refills the pool (SPELL_AURA_MOD_POWER_REGEN[MANA]) and
    // stacks with this. ApplyManaRegenBonus is add/subtract & not idempotent, so the
    // shell tracks the injected amount per-session to swap exactly on level-up/unset.
    uint32 ComputeSecondaryMp5(Classes secondary, uint8 level);
}

#endif
