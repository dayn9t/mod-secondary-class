#include "SC_player_hooks.h"

#include "SC_power.h"
#include "SC_spell_resolver.h"
#include "SC_store.h"

#include "Config.h"     // sConfigMgr
#include "Player.h"

#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace
{
bool IsEnabled()
{
    return sConfigMgr->GetOption<bool>("SecondaryClass.Enable", true);
}

// Session-only: the mp5 currently injected into each player's m_baseManaRegen.
// Player hooks + commands run on the world thread, so no sync needed.
// ApplyManaRegenBonus is add/subtract (not idempotent), and the injected mp5 now
// scales with level (ComputeSecondaryMp5 = base*pct), so we must subtract exactly
// what was added — not a freshly recomputed value (which differs after a level-up
// and would underflow). m_baseManaRegen resets to 0 on every login, so the entry
// is reset at login (see OnPlayerLogin) to stay in sync.
std::unordered_map<uint32 /*guid*/, uint32 /*mp5*/> g_appliedMp5;

bool IsPowerEnabled()
{
    return sConfigMgr->GetOption<bool>("SecondaryClass.SecondaryPowerEnabled", true);
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

    // Phase-2: InitStatsForLevel reset create mana to the primary class base
    // (0 for rage/energy/runic primaries), and m_baseManaRegen resets to 0 on
    // every login — so re-grant the synthetic mana pool + mp5 from scratch.
    if (IsPowerEnabled())
    {
        g_appliedMp5[guid] = 0;  // fresh session: nothing injected yet
        SC::Shell::ApplySecondaryPower(player, *secondary);
    }

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

    // Phase-2: GiveLevel reset create mana to the primary base; base mana (and
    // thus the %-of-base mp5) scales with level, so rescale pool + swap mp5.
    if (IsPowerEnabled())
        SC::Shell::ApplySecondaryPower(player, *secondary);

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

// Clean the tables when a character is deleted — otherwise rows orphan.
void SecondaryClassPlayerScript::OnPlayerDelete(ObjectGuid guid, uint32 /*accountId*/)
{
    uint32 const g = guid.GetCounter();
    SC::Store::DeleteSecondary(g);
    SC::Store::DeleteAllSpells(g);
    SC::Store::DeleteAllSecondaryTalents(g);
}

namespace SC::Shell
{
// Phase-2 synthetic mana pool. With create mana > 0, AC's native cost (fixed
// ManaCost, see SC_power/CalcPowerCost), pool-max (UpdateMaxPower), and regen
// (Regenerate(POWER_MANA) every tick via RegenerateAll) all work unchanged. We
// seed base + pool and inject a %-of-base mp5 the core class table omits for
// physical primaries. Idempotent: safe on login, set, and every level-up.
void ApplySecondaryPower(Player* player, Classes secondary)
{
    Classes const primary = static_cast<Classes>(player->getClass());
    if (!SC::Power::NeedsSyntheticMana(primary, secondary))
        return;

    uint32 const base = SC::Power::ComputeSecondaryBaseMana(secondary, player->GetLevel());
    if (base == 0)
        return;

    player->SetCreateMana(base);          // create the pool (base mana > 0)
    player->UpdateMaxPower(POWER_MANA);   // pool max = base + intellect bonus (StatSystem.cpp:330)
    player->SetPower(POWER_MANA, player->GetMaxPower(POWER_MANA));  // start full

    // Swap the injected mp5 to the level-correct value. Subtract exactly what we
    // recorded (not a fresh recomputation — that differs after a level-up and
    // would underflow m_baseManaRegen), then add the new value.
    uint32 const guid = player->GetGUID().GetCounter();
    uint32 const want = SC::Power::ComputeSecondaryMp5(secondary, player->GetLevel());
    uint32 const have = g_appliedMp5.count(guid) ? g_appliedMp5[guid] : 0;
    if (have != want)
    {
        if (have > 0)
            player->ApplyManaRegenBonus(static_cast<int32>(have), false);
        if (want > 0)
            player->ApplyManaRegenBonus(static_cast<int32>(want), true);
        g_appliedMp5[guid] = want;
    }
}

void RemoveSecondaryPower(Player* player, Classes secondary)
{
    Classes const primary = static_cast<Classes>(player->getClass());
    if (!SC::Power::NeedsSyntheticMana(primary, secondary))
        return;

    // Undo exactly the mp5 we injected this session (tracked, not recomputed).
    uint32 const guid = player->GetGUID().GetCounter();
    uint32 const have = g_appliedMp5.count(guid) ? g_appliedMp5[guid] : 0;
    if (have > 0)
        player->ApplyManaRegenBonus(static_cast<int32>(have), false);
    g_appliedMp5[guid] = 0;

    player->SetCreateMana(0);             // back to the physical-primary base (0)
    player->UpdateMaxPower(POWER_MANA);   // pool max → 0
    player->SetPower(POWER_MANA, 0);
}
}  // namespace SC::Shell
