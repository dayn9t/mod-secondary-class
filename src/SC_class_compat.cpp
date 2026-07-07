#include "SC_class_compat.h"

namespace SC::ClassCompat
{
Powers GetClassBasePower(Classes cls)
{
    switch (cls)
    {
        case CLASS_WARRIOR:      return POWER_RAGE;
        case CLASS_ROGUE:        return POWER_ENERGY;
        case CLASS_DEATH_KNIGHT: return POWER_RUNIC_POWER;
        // Paladin, Hunter, Priest, Shaman, Mage, Warlock, Druid all use mana.
        default:                 return POWER_MANA;
    }
}

bool IsComboCompatible(Classes primary, Classes secondary)
{
    if (primary == CLASS_NONE || secondary == CLASS_NONE)
        return false;
    if (primary == secondary)
        return false;
    return GetClassBasePower(primary) == GetClassBasePower(secondary);
}
}
