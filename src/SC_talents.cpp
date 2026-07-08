#include "SC_talents.h"

#include "DBCStores.h"      // sTalentStore, sTalentTabStore
#include "DBCStructure.h"   // TalentEntry, TalentTabEntry, MAX_TALENT_RANK
#include "Define.h"

namespace SC::Talents
{
ValidateResult ValidateLearn(Classes secondary, uint32 talentId,
                             std::map<uint32, uint8> const& learned,
                             uint32 pointsSpent, uint32 maxPoints)
{
    ValidateResult r;

    TalentEntry const* t = sTalentStore.LookupEntry(talentId);
    if (!t) { r.errCode = "NOTFOUND"; r.errText = "talent not in DBC"; return r; }

    TalentTabEntry const* tab = sTalentTabStore.LookupEntry(t->TalentTab);
    if (!tab) { r.errCode = "NOTFOUND"; r.errText = "talent tab missing"; return r; }

    // Tab must belong to the secondary class (single-bit ClassMask for class tabs).
    uint32 const secMask = 1u << (static_cast<uint32>(secondary) - 1u);
    if ((tab->ClassMask & secMask) == 0)
    {
        r.errCode = "NOTSECONDARYTAB";
        r.errText = "talent does not belong to secondary class";
        return r;
    }

    // Current rank the player has in this talent.
    uint8 cur = 0;
    auto it = learned.find(talentId);
    if (it != learned.end())
        cur = it->second;

    if (cur >= MAX_TALENT_RANK)
    {
        r.errCode = "MAXRANK";
        r.errText = "talent already maxed";
        return r;
    }
    r.newRank = cur + 1;

    // Prereq: DependsOn talent must be learned to >= DependsOnRank.
    if (t->DependsOn)
    {
        auto pit = learned.find(t->DependsOn);
        uint8 have = (pit != learned.end()) ? pit->second : 0;
        if (have < t->DependsOnRank)
        {
            r.errCode = "PREREQ";
            r.errText = "prerequisite talent rank not met";
            return r;
        }
    }

    // Tier gate (WotLK rule): to place in tier T (0-indexed Row), the player
    // must have 5*T points already spent in THIS tab.
    uint32 inTab = 0;
    for (auto const& kv : learned)
        if (TalentEntry const* tt = sTalentStore.LookupEntry(kv.first); tt && tt->TalentTab == t->TalentTab)
            inTab += kv.second;

    if (inTab < static_cast<uint32>(t->Row * 5))
    {
        r.errCode = "TIER";
        r.errText = "tier gate: need more points in this tree";
        return r;
    }

    // Talent-point budget.
    if (pointsSpent + 1 > maxPoints)
    {
        r.errCode = "POINTS";
        r.errText = "no secondary talent points available";
        return r;
    }

    r.ok = true;
    return r;
}

ValidateResult ValidateUnlearn(Classes /*secondary*/, uint32 talentId,
                               std::map<uint32, uint8> const& learned)
{
    ValidateResult r;
    auto it = learned.find(talentId);
    if (it == learned.end() || it->second == 0)
    {
        r.errCode = "NOTLEARNED";
        r.errText = "talent not learned";
        return r;
    }
    // NOTE: v1 does not block unlearning a talent that others depend on
    // (WotLK would). Acceptable for a personal server; revisit if needed.
    r.ok = true;
    r.newRank = it->second - 1;
    return r;
}
}
