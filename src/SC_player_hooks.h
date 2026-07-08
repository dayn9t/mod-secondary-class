#ifndef MOD_SC_PLAYER_HOOKS_H
#define MOD_SC_PLAYER_HOOKS_H

#include "ObjectGuid.h"
#include "PlayerScript.h"
#include "SharedDefines.h"  // Classes

class Player;

// Imperative Shell — PlayerScript hooks wiring the secondary-class system into
// login (防丢: re-grant spells lost on restart), level-up (sync new tier), and
// character delete (clean the two tables to avoid orphan rows).
class SecondaryClassPlayerScript : public PlayerScript
{
public:
    SecondaryClassPlayerScript();

    void OnPlayerLogin(Player* player) override;
    void OnPlayerLevelChanged(Player* player, uint8 oldlevel) override;
    void OnPlayerDelete(ObjectGuid guid, uint32 accountId) override;
};

// Imperative Shell (Phase-2) — mutates Player power state. Thin wrapper over AC's
// own power API so the cost/pool/regen logic stays entirely native.
namespace SC::Shell
{
    // Grant/rescale the synthetic mana pool for a physical-primary player taking a
    // caster secondary (base+pool always rescaled; mp5 swapped to the level-correct
    // value). Idempotent and safe to call on login, set, and every level-up. No-op
    // when the combo needs no synthetic mana or SecondaryPowerEnabled=0.
    void ApplySecondaryPower(Player* player, Classes secondary);

    // Reverse of ApplySecondaryPower: remove injected mp5 and zero the pool. No-op
    // when the combo needed no synthetic mana.
    void RemoveSecondaryPower(Player* player, Classes secondary);
}

#endif
