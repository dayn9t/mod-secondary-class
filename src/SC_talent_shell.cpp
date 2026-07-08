#include "SC_talent_shell.h"

#include "SC_store.h"
#include "SC_talents.h"

#include "Config.h"        // sConfigMgr
#include "DBCStores.h"     // sTalentStore
#include "DBCStructure.h"  // TalentEntry
#include "Log.h"
#include "Player.h"
#include "SharedDefines.h"  // Classes, SPEC_MASK_ALL

#include <map>
#include <string>

namespace
{
uint32 SpentTotal(std::map<uint32, uint8> const& learned)
{
    uint32 s = 0;
    for (auto const& kv : learned)
        s += kv.second;
    return s;
}
}  // namespace

namespace SC::Shell
{
uint32 MaxSecondaryTalentPoints(Player const* player)
{
    uint8 const level = player->GetLevel();
    uint32 const base = (level >= 10) ? static_cast<uint32>(level - 9) : 0;
    float const ratio = sConfigMgr->GetOption<float>("SecondaryClass.TalentPointRatio", 0.5f);
    return static_cast<uint32>(base * ratio);
}

bool LearnSecondaryTalent(Player* player, uint32 talentId, std::string& reply)
{
    uint32 const guid = player->GetGUID().GetCounter();
    auto const secondary = SC::Store::LoadSecondary(guid);
    if (!secondary) { reply = "SC\tERR\tNOSECONDARY\tno secondary class set"; return false; }

    std::map<uint32, uint8> learned = SC::Store::LoadSecondaryTalents(guid);
    uint32 const spent = SpentTotal(learned);
    uint32 const maxPoints = MaxSecondaryTalentPoints(player);
    auto const r = SC::Talents::ValidateLearn(*secondary, talentId, learned, spent, maxPoints);
    if (!r.ok) { reply = "SC\tERR\t" + r.errCode + "\t" + r.errText; return false; }

    TalentEntry const* t = sTalentStore.LookupEntry(talentId);
    uint32 const rankSpell = t->RankID[r.newRank - 1];
    player->learnSpell(rankSpell);                       // apply aura; NOT m_talentMap
    SC::Store::UpsertSecondaryTalent(guid, talentId, r.newRank);

    reply = "SC\tOK\t" + std::to_string(talentId) + "\t" + std::to_string(r.newRank)
            + "\t" + std::to_string(spent + 1) + "\t" + std::to_string(maxPoints);
    return true;
}

bool UnlearnSecondaryTalent(Player* player, uint32 talentId, std::string& reply)
{
    uint32 const guid = player->GetGUID().GetCounter();
    auto const secondary = SC::Store::LoadSecondary(guid);
    if (!secondary) { reply = "SC\tERR\tNOSECONDARY\tno secondary class set"; return false; }

    std::map<uint32, uint8> learned = SC::Store::LoadSecondaryTalents(guid);
    auto const r = SC::Talents::ValidateUnlearn(*secondary, talentId, learned);
    if (!r.ok) { reply = "SC\tERR\t" + r.errCode + "\t" + r.errText; return false; }

    TalentEntry const* t = sTalentStore.LookupEntry(talentId);
    uint8 const cur = learned.at(talentId);
    player->removeSpell(t->RankID[cur - 1], SPEC_MASK_ALL, false);
    if (r.newRank > 0)
    {
        player->learnSpell(t->RankID[r.newRank - 1]);   // demote to previous rank
        SC::Store::UpsertSecondaryTalent(guid, talentId, r.newRank);
    }
    else
    {
        SC::Store::DeleteSecondaryTalent(guid, talentId);
    }

    reply = "SC\tOK\t" + std::to_string(talentId) + "\t" + std::to_string(r.newRank)
            + "\t" + std::to_string(SpentTotal(learned) - 1)
            + "\t" + std::to_string(MaxSecondaryTalentPoints(player));
    return true;
}

bool ResetSecondaryTalents(Player* player, std::string& reply)
{
    uint32 const guid = player->GetGUID().GetCounter();
    std::map<uint32, uint8> const learned = SC::Store::LoadSecondaryTalents(guid);
    for (auto const& kv : learned)
        if (TalentEntry const* t = sTalentStore.LookupEntry(kv.first); t && kv.second > 0)
            player->removeSpell(t->RankID[kv.second - 1], SPEC_MASK_ALL, false);

    SC::Store::DeleteAllSecondaryTalents(guid);
    reply = "SC\tOK\tRESET\t0";
    return true;
}

std::string SerializeTalentState(Player* player)
{
    uint32 const guid = player->GetGUID().GetCounter();
    auto const secondary = SC::Store::LoadSecondary(guid);
    if (!secondary)
        return "SC\tS\t0\t0\t0\t";

    std::map<uint32, uint8> const learned = SC::Store::LoadSecondaryTalents(guid);
    std::string list;
    for (auto const& kv : learned)
    {
        if (!list.empty())
            list += ",";
        list += std::to_string(kv.first) + ":" + std::to_string(kv.second);
    }
    return "SC\tS\t" + std::to_string(static_cast<uint32>(*secondary))
         + "\t" + std::to_string(SpentTotal(learned))
         + "\t" + std::to_string(MaxSecondaryTalentPoints(player)) + "\t" + list;
}

void ReapplySecondaryTalents(Player* player)
{
    uint32 const guid = player->GetGUID().GetCounter();
    if (!SC::Store::LoadSecondary(guid))
        return;

    std::map<uint32, uint8> const learned = SC::Store::LoadSecondaryTalents(guid);
    for (auto const& kv : learned)
        if (TalentEntry const* t = sTalentStore.LookupEntry(kv.first); t && kv.second > 0)
        {
            uint32 const spell = t->RankID[kv.second - 1];
            if (!player->HasActiveSpell(spell))
                player->learnSpell(spell);
        }
}
}  // namespace SC::Shell
