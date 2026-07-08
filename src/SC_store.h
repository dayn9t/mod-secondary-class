#ifndef MOD_SC_STORE_H
#define MOD_SC_STORE_H

#include "SharedDefines.h"  // Classes

#include <cstdint>
#include <map>
#include <optional>
#include <set>

// Imperative Shell — DB read/write for the secondary-class tables.
// Uses direct (fmt-style) SQL via CharacterDatabase; modules cannot register
// prepared-statement enum ids, so we avoid the prepared API.
namespace SC::Store
{
    // character_secondary_class (state — one row per player)
    std::optional<Classes> LoadSecondary(uint32 guid);
    void SaveSecondary(uint32 guid, Classes cls);
    void DeleteSecondary(uint32 guid);

    // character_secondary_class_spells (detail — which spells we granted)
    std::set<uint32> LoadSpells(uint32 guid);
    void RecordSpell(uint32 guid, uint32 spellId);
    void DeleteAllSpells(uint32 guid);

    // character_secondary_talents (Phase-3 — learned talents: talentId -> rank)
    std::map<uint32, uint8> LoadSecondaryTalents(uint32 guid);
    void UpsertSecondaryTalent(uint32 guid, uint32 talentId, uint8 rank);
    void DeleteSecondaryTalent(uint32 guid, uint32 talentId);
    void DeleteAllSecondaryTalents(uint32 guid);
}

#endif
