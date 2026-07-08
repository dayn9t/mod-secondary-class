#ifndef MOD_SC_TALENT_COMM_H
#define MOD_SC_TALENT_COMM_H

#include "PlayerScript.h"

#include <string>

class Player;

// Phase-3 talent comm: intercepts SC-prefixed LANG_ADDON chat (addon<->server).
//
// Channel choice: the addon sends via GUILD (WHISPER-to-self is a 3.3.5a client
// crash bug; GUILD is the only SendAddonMessage channel a solo player can use
// without a party). GUILD addon messages route through ChatHandler, which calls
// OnPlayerBeforeSendChatMessage (msg by ref) before the guild broadcast -- so we
// detect our prefix, reply via CHAT_MSG_ADDON, and clear msg to suppress the
// (echo) broadcast. Requires the player to be in a guild (.guild create).
class SecondaryClassTalentCommScript : public PlayerScript
{
public:
    SecondaryClassTalentCommScript();
    void OnPlayerBeforeSendChatMessage(Player* player, uint32& type, uint32& lang, std::string& msg) override;
};

#endif
