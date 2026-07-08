#ifndef MOD_SC_TALENT_COMM_H
#define MOD_SC_TALENT_COMM_H

#include "PlayerScript.h"

#include <string>

class Player;

// Phase-3 talent comm: intercepts SC-prefixed LANG_ADDON messages.
//
// The addon sends via WHISPER-to-self (only SendAddonMessage channel that works
// for a solo player). Empirically that routes through Player::Whisper, which (a)
// does NOT call OnPlayerBeforeSendChatMessage and (b) sends a CHAT_MSG_WHISPER
// echo to the client that crashes it. Player::Whisper DOES call OnPlayerCanUseChat
// (private-chat overload) BEFORE the echo — so we hook that: detect our prefix,
// reply via CHAT_MSG_ADDON, and return false to abort the echo (no crash).
class SecondaryClassTalentCommScript : public PlayerScript
{
public:
    SecondaryClassTalentCommScript();

    // Private-chat (whisper) path — the overload Player::Whisper calls.
    [[nodiscard]] bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 language, std::string& msg, Player* receiver) override;
};

#endif
