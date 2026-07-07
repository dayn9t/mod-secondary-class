#ifndef MOD_SC_PLAYER_HOOKS_H
#define MOD_SC_PLAYER_HOOKS_H

#include "ObjectGuid.h"
#include "PlayerScript.h"

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

#endif
