#include "SC_player_hooks.h"

#include "SC_spell_resolver.h"
#include "SC_store.h"

#include "Config.h"     // sConfigMgr
#include "Log.h"
#include "Player.h"

#include <sstream>
#include <string>
#include <unordered_set>

namespace
{
bool IsEnabled()
{
    return sConfigMgr->GetOption<bool>("SecondaryClass.Enable", true);
}

int32 GetSpellLevelOffset()
{
    return sConfigMgr->GetOption<int32>("SecondaryClass.SpellLevelOffset", 0);
}

std::unordered_set<uint32> GetSpellBlacklist()
{
    std::unordered_set<uint32> blacklist;
    std::string const raw = sConfigMgr->GetOption<std::string>("SecondaryClass.SpellBlacklist", "");
    std::stringstream ss(raw);
    std::string token;
    while (std::getline(ss, token, ','))
    {
        if (!token.empty())
            blacklist.insert(static_cast<uint32>(std::stoul(token)));
    }
    return blacklist;
}
}  // namespace

SecondaryClassPlayerScript::SecondaryClassPlayerScript()
    : PlayerScript("SecondaryClassPlayerScript") { }

// 防丢: worldserver restart / crash may drop granted spells — re-learn any
// recorded spell the player is missing.
void SecondaryClassPlayerScript::OnPlayerLogin(Player* player)
{
    if (!IsEnabled() || !player)
        return;

    uint32 const guid = player->GetGUID().GetCounter();
    auto const secondary = SC::Store::LoadSecondary(guid);
    if (!secondary)
        return;

    std::set<uint32> const recorded = SC::Store::LoadSpells(guid);
    for (uint32 spell : recorded)
    {
        if (!player->HasActiveSpell(spell))
            player->learnSpell(spell);
    }
}

// Sync: at new level, grant any secondary spells now unlocked (spell_level ≤ newLevel+offset)
// that we haven't recorded and the player doesn't already have.
void SecondaryClassPlayerScript::OnPlayerLevelChanged(Player* player, uint8 /*oldlevel*/)
{
    if (!IsEnabled() || !player)
        return;

    uint32 const guid = player->GetGUID().GetCounter();
    auto const secondary = SC::Store::LoadSecondary(guid);
    if (!secondary)
        return;

    std::set<uint32> const recorded = SC::Store::LoadSpells(guid);
    std::set<uint32> const should = SC::SpellResolver::GetSpellsForClassAtLevel(
        *secondary, player->GetLevel(), GetSpellLevelOffset(), GetSpellBlacklist());

    for (uint32 spell : should)
    {
        if (recorded.count(spell) != 0u || player->HasActiveSpell(spell))
            continue;
        player->learnSpell(spell);
        SC::Store::RecordSpell(guid, spell);
    }
}

// Clean the two tables when a character is deleted — otherwise rows orphan.
void SecondaryClassPlayerScript::OnPlayerDelete(ObjectGuid guid, uint32 /*accountId*/)
{
    uint32 const g = guid.GetCounter();
    SC::Store::DeleteSecondary(g);
    SC::Store::DeleteAllSpells(g);
}
