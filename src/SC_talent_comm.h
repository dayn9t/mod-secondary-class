#ifndef MOD_SC_TALENT_COMM_H
#define MOD_SC_TALENT_COMM_H

#include "PlayerScript.h"

#include <string>

class Player;

// Phase-3 talent comm: intercepts SC-prefixed LANG_ADDON chat (addon<->server).
// Task 1 (smoke): echoes any "SC\t..." back as "SC\tOK\tPING" to prove the
// round-trip on this core. Replaced with real Q/L/U dispatch in Task 6.
class SecondaryClassTalentCommScript : public PlayerScript
{
public:
    SecondaryClassTalentCommScript();
    void OnPlayerBeforeSendChatMessage(Player* player, uint32& type, uint32& lang, std::string& msg) override;
};

#endif
