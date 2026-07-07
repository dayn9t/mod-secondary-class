#include "SC_spell_resolver.h"

#include "DBCStores.h"   // sSkillLineAbilityStore
#include "SpellInfo.h"
#include "SpellMgr.h"    // sSpellMgr

namespace SC::SpellResolver
{
std::set<uint32> GetSpellsForClassAtLevel(
    Classes cls,
    uint32 maxLevel,
    int32 levelOffset,
    std::unordered_set<uint32> const& blacklist)
{
    std::set<uint32> result;
    if (cls == CLASS_NONE)
        return result;

    // ClassMask bit = 1 << (classId - 1). CLASS_WARRIOR(1)→bit0, CLASS_DRUID(11)→bit10.
    uint32 const classMask = 1u << (static_cast<uint32>(cls) - 1u);
    int32 const effectiveMax = static_cast<int32>(maxLevel) + levelOffset;

    for (SkillLineAbilityEntry const* ability : sSkillLineAbilityStore)
    {
        if ((ability->ClassMask & classMask) == 0u)
            continue;

        SpellInfo const* info = sSpellMgr->GetSpellInfo(ability->Spell);
        if (!info || info->SpellLevel == 0u)
            continue;

        if (static_cast<int32>(info->SpellLevel) > effectiveMax)
            continue;

        if (blacklist.count(ability->Spell) != 0u)
            continue;

        result.insert(ability->Spell);
    }

    return result;
}
}
