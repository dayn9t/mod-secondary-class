#ifndef MOD_SC_SPELL_RESOLVER_H
#define MOD_SC_SPELL_RESOLVER_H

#include "SharedDefines.h"  // Classes

#include <cstdint>
#include <set>
#include <unordered_set>

// Functional Core — pure helper resolving which spells a secondary class grants.
// Reads DBC (sSkillLineAbilityStore / sSpellMgr) but has no side effects: same
// inputs → same output. Deterministic, unit-testable with a real DBC load.
namespace SC::SpellResolver
{
    // All spells for `cls` whose SpellInfo::SpellLevel is in (0, maxLevel + levelOffset].
    //  - skips spells with SpellLevel==0 (passives/auras — would pollute spellbook)
    //  - skips blacklisted spell ids
    //  - no internal cache; called rarely (on .secondary set / level up / login)
    std::set<uint32> GetSpellsForClassAtLevel(
        Classes cls,
        uint32 maxLevel,
        int32 levelOffset,
        std::unordered_set<uint32> const& blacklist);
}

#endif
