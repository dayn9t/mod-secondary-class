#include "SC_store.h"

#include "DatabaseEnv.h"  // CharacterDatabase, QueryResult
#include "Field.h"
#include "QueryResult.h"  // ResultSet complete type

namespace SC::Store
{
std::optional<Classes> LoadSecondary(uint32 guid)
{
    QueryResult result = CharacterDatabase.Query(
        "SELECT secondary_class FROM character_secondary_class WHERE guid = {}", guid);
    if (!result)
        return std::nullopt;
    Field* fields = result->Fetch();
    return static_cast<Classes>(fields[0].Get<uint8>());
}

void SaveSecondary(uint32 guid, Classes cls)
{
    CharacterDatabase.Execute(
        "REPLACE INTO character_secondary_class (guid, secondary_class) VALUES ({}, {})",
        guid, static_cast<uint32>(cls));
}

void DeleteSecondary(uint32 guid)
{
    CharacterDatabase.Execute("DELETE FROM character_secondary_class WHERE guid = {}", guid);
}

std::set<uint32> LoadSpells(uint32 guid)
{
    std::set<uint32> spells;
    QueryResult result = CharacterDatabase.Query(
        "SELECT spell_id FROM character_secondary_class_spells WHERE guid = {}", guid);
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            spells.insert(fields[0].Get<uint32>());
        } while (result->NextRow());
    }
    return spells;
}

void RecordSpell(uint32 guid, uint32 spellId)
{
    CharacterDatabase.Execute(
        "INSERT IGNORE INTO character_secondary_class_spells (guid, spell_id) VALUES ({}, {})",
        guid, spellId);
}

void DeleteAllSpells(uint32 guid)
{
    CharacterDatabase.Execute("DELETE FROM character_secondary_class_spells WHERE guid = {}", guid);
}
}
