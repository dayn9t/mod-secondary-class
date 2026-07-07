#ifndef MOD_SC_CLASS_COMPAT_H
#define MOD_SC_CLASS_COMPAT_H

#include "SharedDefines.h"  // Classes, Powers

// Functional Core — pure helpers for class/resource compatibility.
// No side effects, deterministic, unit-testable.
namespace SC::ClassCompat
{
    // Base power type for a class. Druid counts as mana (its bear/cat forms are
    // client-side shapeshifts; the player still has a mana bar in caster form).
    Powers GetClassBasePower(Classes cls);

    // A primary+secondary combo is castable on the client iff both share the same
    // base power type — the client draws one power bar according to the primary
    // class, so a rage/energy/runic secondary can't be cast by a mana primary.
    bool IsComboCompatible(Classes primary, Classes secondary);
}

#endif
