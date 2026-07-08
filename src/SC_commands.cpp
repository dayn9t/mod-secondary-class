#include "SC_commands.h"

#include "SC_class_compat.h"
#include "SC_player_hooks.h"  // SC::Shell::Apply/RemoveSecondaryPower
#include "SC_power.h"
#include "SC_spell_resolver.h"
#include "SC_store.h"
#include "SC_talent_shell.h"
#include "SC_talents.h"

#include "Chat.h"
#include "Config.h"
#include "DBCStores.h"     // sTalentStore
#include "DBCStructure.h"  // TalentEntry
#include "Player.h"
#include "RBAC.h"
#include "ScriptMgr.h"

#include "Chat.h"
#include "Config.h"
#include "Player.h"
#include "RBAC.h"
#include "ScriptMgr.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <sstream>
#include <string>
#include <unordered_set>

using namespace Acore::ChatCommands;

namespace
{
Classes ParseClassName(std::string name)
{
    std::transform(name.begin(), name.end(), name.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    if (name == "warrior")                          return CLASS_WARRIOR;
    if (name == "paladin")                          return CLASS_PALADIN;
    if (name == "hunter")                           return CLASS_HUNTER;
    if (name == "rogue")                            return CLASS_ROGUE;
    if (name == "priest")                           return CLASS_PRIEST;
    if (name == "dk" || name == "deathknight")      return CLASS_DEATH_KNIGHT;
    if (name == "shaman")                           return CLASS_SHAMAN;
    if (name == "mage")                             return CLASS_MAGE;
    if (name == "warlock")                          return CLASS_WARLOCK;
    if (name == "druid")                            return CLASS_DRUID;
    return CLASS_NONE;
}

std::unordered_set<uint32> LoadBlacklist()
{
    std::unordered_set<uint32> blacklist;
    std::string const raw = sConfigMgr->GetOption<std::string>("SecondaryClass.SpellBlacklist", "");
    std::stringstream ss(raw);
    std::string token;
    while (std::getline(ss, token, ','))
        if (!token.empty())
            blacklist.insert(static_cast<uint32>(std::stoul(token)));
    return blacklist;
}

Player* TargetedPlayer(ChatHandler* handler)
{
    return handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;
}

// Reverse of ParseClassName — uppercase canonical name for AllowedCombos lookup.
std::string ClassToName(Classes cls)
{
    switch (cls)
    {
        case CLASS_WARRIOR:      return "WARRIOR";
        case CLASS_PALADIN:      return "PALADIN";
        case CLASS_HUNTER:       return "HUNTER";
        case CLASS_ROGUE:        return "ROGUE";
        case CLASS_PRIEST:       return "PRIEST";
        case CLASS_DEATH_KNIGHT: return "DEATHKNIGHT";
        case CLASS_SHAMAN:       return "SHAMAN";
        case CLASS_MAGE:         return "MAGE";
        case CLASS_WARLOCK:      return "WARLOCK";
        case CLASS_DRUID:        return "DRUID";
        default:                 return "";
    }
}

// AllowedCombos whitelist check (spec §6.3/§7.2). Empty config = all allowed.
bool IsComboAllowed(Classes primary, Classes secondary)
{
    std::string const combos = sConfigMgr->GetOption<std::string>("SecondaryClass.AllowedCombos", "");
    if (combos.empty())
        return true;
    std::string const key = ClassToName(primary) + ":" + ClassToName(secondary);
    return combos.find(key) != std::string::npos;
}
}  // namespace

secondary_commandscript::secondary_commandscript()
    : CommandScript("secondary_commandscript") { }

std::vector<ChatCommandBuilder> secondary_commandscript::GetCommands() const
{
    static ChatCommandTable const secondarySubTable =
    {
        { "set",   HandleSecondarySetCommand,   rbac::RBAC_PERM_COMMAND_ACCOUNT_SET_SECLEVEL, Console::Yes },
        { "unset", HandleSecondaryUnsetCommand, rbac::RBAC_PERM_COMMAND_ACCOUNT_SET_SECLEVEL, Console::Yes },
        { "show",  HandleSecondaryShowCommand,  rbac::RBAC_PERM_COMMAND_ACCOUNT_SET_SECLEVEL, Console::Yes },
        { "powerinfo", HandleSecondaryPowerInfoCommand, rbac::RBAC_PERM_COMMAND_ACCOUNT_SET_SECLEVEL, Console::Yes },
    };
    static ChatCommandTable const scSubTable =
    {
        { "learn",    HandleScLearnCommand,    rbac::RBAC_PERM_COMMAND_ACCOUNT_SET_SECLEVEL, Console::Yes },
        { "validate", HandleScValidateCommand, rbac::RBAC_PERM_COMMAND_ACCOUNT_SET_SECLEVEL, Console::Yes },
        { "show",     HandleScShowCommand,     rbac::RBAC_PERM_COMMAND_ACCOUNT_SET_SECLEVEL, Console::Yes },
        { "reset",    HandleScResetCommand,    rbac::RBAC_PERM_COMMAND_ACCOUNT_SET_SECLEVEL, Console::Yes },
    };
    static ChatCommandTable const commandTable =
    {
        { "secondary", secondarySubTable },
        { "sc",        scSubTable },
    };
    return commandTable;
}

bool secondary_commandscript::HandleSecondarySetCommand(ChatHandler* handler, std::string className)
{
    if (!sConfigMgr->GetOption<bool>("SecondaryClass.Enable", true))
    {
        handler->SendErrorMessage("SecondaryClass module is disabled (SecondaryClass.Enable=0)");
        return false;
    }

    Classes const secondary = ParseClassName(className);
    if (secondary == CLASS_NONE)
    {
        handler->SendErrorMessage(
            "Unknown class '{}'. Options: warrior/paladin/hunter/rogue/priest/dk/shaman/mage/warlock/druid",
            className);
        return false;
    }

    Player* player = TargetedPlayer(handler);
    if (!player)
    {
        handler->SendErrorMessage("No player context");
        return false;
    }

    Classes const primary = static_cast<Classes>(player->getClass());
    if (secondary == primary)
    {
        handler->SendErrorMessage("Secondary class cannot equal primary class");
        return false;
    }

    uint32 const minLevel = sConfigMgr->GetOption<uint32>("SecondaryClass.MinLevel", 1);
    if (player->GetLevel() < minLevel)
    {
        handler->SendErrorMessage("Player level {} below minimum {}", player->GetLevel(), minLevel);
        return false;
    }

    // Cross-resource (e.g. rogue + mage) is the Phase-2 use case: SecondaryPowerEnabled
    // grants a synthetic mana pool so the combo is fully playable. Reject only when
    // neither Phase 2 nor the debug AllowResourceMismatch bypass is on (No Silent
    // Degradation: don't silently let a combo through whose spells can't be cast).
    bool const crossResource = !SC::ClassCompat::IsComboCompatible(primary, secondary);
    bool const powerEnabled = sConfigMgr->GetOption<bool>("SecondaryClass.SecondaryPowerEnabled", true);
    bool const allowMismatch = sConfigMgr->GetOption<bool>("SecondaryClass.AllowResourceMismatch", false);
    if (crossResource && !powerEnabled && !allowMismatch)
    {
        handler->SendErrorMessage(
            "Resource mismatch (primary/secondary power types differ). Enable "
            "SecondaryClass.SecondaryPowerEnabled=1 to grant a synthetic mana pool, "
            "or AllowResourceMismatch=1 to force without one.");
        return false;
    }

    if (!IsComboAllowed(primary, secondary))
    {
        handler->SendErrorMessage(
            "Combo {}:{} is not in SecondaryClass.AllowedCombos whitelist",
            ClassToName(primary), ClassToName(secondary));
        return false;
    }

    uint32 const guid = player->GetGUID().GetCounter();

    // Switching secondary: clear the old one first (spells + Phase-2 power pool).
    auto const oldSecondary = SC::Store::LoadSecondary(guid);
    if (oldSecondary)
    {
        SC::Shell::RemoveSecondaryPower(player, *oldSecondary);
        std::set<uint32> const oldSpells = SC::Store::LoadSpells(guid);
        for (uint32 s : oldSpells)
            if (player->HasActiveSpell(s))
                player->removeSpell(s, SPEC_MASK_ALL, false);
        SC::Store::DeleteAllSpells(guid);
    }

    SC::Store::SaveSecondary(guid, secondary);

    int32 const offset = sConfigMgr->GetOption<int32>("SecondaryClass.SpellLevelOffset", 0);
    std::set<uint32> const spells = SC::SpellResolver::GetSpellsForClassAtLevel(
        secondary, player->GetLevel(), offset, LoadBlacklist());

    uint32 learned = 0;
    for (uint32 s : spells)
    {
        if (player->HasActiveSpell(s))
            continue;
        player->learnSpell(s);
        SC::Store::RecordSpell(guid, s);
        ++learned;
    }

    // Phase-2: grant the synthetic mana pool (idempotent). No-op for same-resource
    // combos (e.g. mage + priest).
    SC::Shell::ApplySecondaryPower(player, secondary);

    handler->SendSysMessage("Secondary class set; learned " + std::to_string(learned) + " spells.");
    return true;
}

bool secondary_commandscript::HandleSecondaryUnsetCommand(ChatHandler* handler)
{
    Player* player = TargetedPlayer(handler);
    if (!player)
    {
        handler->SendErrorMessage("No player context");
        return false;
    }

    uint32 const guid = player->GetGUID().GetCounter();
    auto const secondary = SC::Store::LoadSecondary(guid);
    if (!secondary)
    {
        handler->SendSysMessage("No secondary class set.");
        return true;
    }

    // Phase-2: drop the synthetic mana pool (no-op for same-resource combos).
    SC::Shell::RemoveSecondaryPower(player, *secondary);

    std::set<uint32> const spells = SC::Store::LoadSpells(guid);
    uint32 removed = 0;
    for (uint32 s : spells)
    {
        if (player->HasActiveSpell(s))
        {
            player->removeSpell(s, SPEC_MASK_ALL, false);
            ++removed;
        }
    }
    SC::Store::DeleteAllSpells(guid);
    SC::Store::DeleteSecondary(guid);

    handler->SendSysMessage("Secondary class cleared; removed " + std::to_string(removed) + " spells.");
    return true;
}

bool secondary_commandscript::HandleSecondaryShowCommand(ChatHandler* handler)
{
    Player* player = TargetedPlayer(handler);
    if (!player)
        return false;

    uint32 const guid = player->GetGUID().GetCounter();
    auto const secondary = SC::Store::LoadSecondary(guid);
    if (!secondary)
    {
        handler->SendSysMessage("No secondary class set.");
        return true;
    }

    std::set<uint32> const spells = SC::Store::LoadSpells(guid);
    handler->SendSysMessage("Secondary class id: " + std::to_string(static_cast<uint32>(*secondary))
                            + ", spells recorded: " + std::to_string(spells.size()));
    return true;
}

// Diagnostic: dump all 7 power pools (current/max) + primary power type.
// Answers once-for-all: does the rogue have a real mana pool, and what value
// does stat pin it to? No more guessing from client-side behavior.
bool secondary_commandscript::HandleSecondaryPowerInfoCommand(ChatHandler* handler)
{
    Player* player = TargetedPlayer(handler);
    if (!player)
    {
        handler->SendErrorMessage("No player context");
        return false;
    }

    static char const* const powerNames[MAX_POWERS] =
        { "MANA(0)", "RAGE(1)", "FOCUS(2)", "ENERGY(3)", "HAPPINESS(4)", "RUNE(5)", "RUNIC(6)" };

    Classes const cls = static_cast<Classes>(player->getClass());
    handler->SendSysMessage("=== PowerInfo: class=" + std::to_string(cls)
        + " primaryPowerType=" + std::to_string(player->getPowerType()) + " ===");
    for (int p = 0; p < MAX_POWERS; ++p)
    {
        Powers const pt = Powers(p);
        handler->SendSysMessage(std::string("  ") + powerNames[p]
            + ": " + std::to_string(player->GetPower(pt))
            + "/" + std::to_string(player->GetMaxPower(pt)));
    }
    return true;
}

// ---- Phase-3 talent GM commands (.sc) ----

bool secondary_commandscript::HandleScLearnCommand(ChatHandler* handler, uint32 talentId)
{
    Player* player = TargetedPlayer(handler);
    if (!player)
    {
        handler->SendErrorMessage("No player context");
        return false;
    }
    std::string reply;
    bool const ok = SC::Shell::LearnSecondaryTalent(player, talentId, reply);
    handler->SendSysMessage(reply);
    return ok;
}

bool secondary_commandscript::HandleScValidateCommand(ChatHandler* handler, uint32 talentId)
{
    Player* player = TargetedPlayer(handler);
    if (!player)
    {
        handler->SendErrorMessage("No player context");
        return false;
    }
    uint32 const guid = player->GetGUID().GetCounter();
    auto const secondary = SC::Store::LoadSecondary(guid);
    if (!secondary)
    {
        handler->SendErrorMessage("No secondary class set");
        return false;
    }
    std::map<uint32, uint8> const learned = SC::Store::LoadSecondaryTalents(guid);
    uint32 spent = 0;
    for (auto const& kv : learned)
        spent += kv.second;

    auto const r = SC::Talents::ValidateLearn(
        *secondary, talentId, learned, spent, SC::Shell::MaxSecondaryTalentPoints(player));

    handler->SendSysMessage("validate talent " + std::to_string(talentId)
        + (r.ok ? " -> OK (newRank=" + std::to_string(r.newRank) + ")"
                : " -> ERR " + r.errCode + ": " + r.errText)
        + " | spent=" + std::to_string(spent)
        + " max=" + std::to_string(SC::Shell::MaxSecondaryTalentPoints(player)));
    return true;
}

bool secondary_commandscript::HandleScShowCommand(ChatHandler* handler)
{
    Player* player = TargetedPlayer(handler);
    if (!player)
    {
        handler->SendErrorMessage("No player context");
        return false;
    }
    handler->SendSysMessage(SC::Shell::SerializeTalentState(player));
    return true;
}

bool secondary_commandscript::HandleScResetCommand(ChatHandler* handler)
{
    Player* player = TargetedPlayer(handler);
    if (!player)
    {
        handler->SendErrorMessage("No player context");
        return false;
    }
    std::string reply;
    bool const ok = SC::Shell::ResetSecondaryTalents(player, reply);
    handler->SendSysMessage(reply);
    return ok;
}
