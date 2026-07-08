#ifndef MOD_SC_TALENTS_H
#define MOD_SC_TALENTS_H

#include "SharedDefines.h"  // Classes

#include <cstdint>
#include <map>
#include <string>

// Functional Core (Phase-3) — pure validators for secondary-class talent
// learn/unlearn, driven by the same DBC stores the server loads
// (sTalentStore / sTalentTabStore). No side effects, deterministic.
namespace SC::Talents
{
    struct ValidateResult
    {
        bool ok = false;
        std::string errCode;   // NOTFOUND / NOTSECONDARYTAB / PREREQ / TIER / POINTS / MAXRANK / NOTLEARNED
        std::string errText;   // human-readable reason
        uint8 newRank = 0;     // rank that would be applied (learn: current+1; unlearn: current-1)
    };

    // Validate learning the next rank of talentId for a secondary class.
    //   learned      : current talents (talentId -> rank)
    //   pointsSpent  : total points already spent (sum of ranks)
    //   maxPoints    : the secondary point budget for this level
    ValidateResult ValidateLearn(Classes secondary, uint32 talentId,
                                 std::map<uint32, uint8> const& learned,
                                 uint32 pointsSpent, uint32 maxPoints);

    // Validate unlearning one rank of talentId.
    ValidateResult ValidateUnlearn(Classes secondary, uint32 talentId,
                                   std::map<uint32, uint8> const& learned);
}

#endif
