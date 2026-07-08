#ifndef MOD_SC_TALENT_SHELL_H
#define MOD_SC_TALENT_SHELL_H

#include "Define.h"  // uint32

#include <string>

class Player;

// Imperative Shell (Phase-3) — applies secondary-class talent operations to a
// Player. Talent effects come from the rank spell's passive aura (learnSpell);
// talents are NOT registered in m_talentMap (keeps the native frame clean).
// Server-authoritative: every op produces a wire reply the addon mirrors.
namespace SC::Shell
{
    // Secondary talent-point budget for this player's level:
    //   (level >= 10 ? level - 9 : 0) * SecondaryClass.TalentPointRatio
    uint32 MaxSecondaryTalentPoints(Player const* player);

    // Learn next rank of talentId. On success learns the rank spell, persists,
    // and sets reply = "SC\tOK\t<talentId>\t<newRank>\t<spentTotal>". On failure
    // reply = "SC\tERR\t<code>\t<text>". Returns true on success.
    bool LearnSecondaryTalent(Player* player, uint32 talentId, std::string& reply);

    // Unlearn one rank of talentId (reply same shape as Learn; newRank may be 0).
    bool UnlearnSecondaryTalent(Player* player, uint32 talentId, std::string& reply);

    // Unlearn the entire tree (reset). reply = "SC\tOK\tRESET\t<spentTotal=0>".
    bool ResetSecondaryTalents(Player* player, std::string& reply);

    // Full state for the addon: "SC\tS\t<classID>\t<spentTotal>\t<id>:<rank>,..."
    std::string SerializeTalentState(Player* player);

    // Re-apply all learned secondary talents on login (defensive: spells can be
    // dropped by restart/crash). No-op if the player has no secondary class.
    void ReapplySecondaryTalents(Player* player);
}

#endif
