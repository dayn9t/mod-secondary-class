#include "SC_power.h"

#include "SC_class_compat.h"  // GetClassBasePower

#include "Config.h"   // sConfigMgr
#include "ObjectMgr.h"  // sObjectMgr->GetPlayerClassLevelInfo
#include "Player.h"   // PlayerClassLevelInfo

namespace SC::Power
{
bool NeedsSyntheticMana(Classes primary, Classes secondary)
{
    if (primary == CLASS_NONE || secondary == CLASS_NONE)
        return false;

    // Primary draws no mana bar, but the secondary casts mana spells.
    return ClassCompat::GetClassBasePower(primary) != POWER_MANA
        && ClassCompat::GetClassBasePower(secondary) == POWER_MANA;
}

uint32 ComputeSecondaryBaseMana(Classes secondary, uint8 level)
{
    uint32 const configBase = sConfigMgr->GetOption<uint32>("SecondaryClass.SecondaryPowerBase", 0);
    if (configBase > 0)
        return configBase;

    // AC's own per-class-per-level base mana (same call Player::GiveLevel makes).
    PlayerClassLevelInfo info{};
    sObjectMgr->GetPlayerClassLevelInfo(secondary, level, &info);
    return info.basemana;
}

uint32 ComputeSecondaryMp5(Classes secondary, uint8 level)
{
    // Percent of the secondary's base mana, per 5 seconds. Scales with level;
    // low default so the secondary regens slower than a real caster.
    uint32 const pct = sConfigMgr->GetOption<uint32>("SecondaryClass.ManaRegenPct", 3);
    return ComputeSecondaryBaseMana(secondary, level) * pct / 100;
}
}
