#include "SC_talent_comm.h"

#include "Chat.h"          // ChatHandler::BuildChatPacket, CHAT_TAG_NONE
#include "Common.h"        // ChatMsg, Language, CHAT_MSG_ADDON, LANG_ADDON
#include "Log.h"
#include "Player.h"
#include "WorldPacket.h"

SecondaryClassTalentCommScript::SecondaryClassTalentCommScript()
    : PlayerScript("SecondaryClassTalentCommScript") { }

bool SecondaryClassTalentCommScript::OnPlayerCanUseChat(
    Player* player, uint32 type, uint32 language, std::string& msg, Player* /*receiver*/)
{
    // Only our addon channel + prefix. Allow all other chat through.
    if (language != LANG_ADDON || msg.rfind("SC\t", 0) != 0)
        return true;

    LOG_INFO("sc.talent", "[SC-COMM] recv from {}: type={} msg={}", player->GetName(), type, msg);

    // Reply via CHAT_MSG_ADDON so the addon's CHAT_MSG_ADDON handler fires.
    WorldPacket data;
    ChatHandler::BuildChatPacket(
        data, CHAT_MSG_ADDON, "SC\tOK\tPING", LANG_ADDON, CHAT_TAG_NONE,
        player->GetGUID(), player->GetName());
    player->SendDirectMessage(&data);

    // Abort the whisper: returning false makes Player::Whisper return before it
    // builds/sends the CHAT_MSG_WHISPER echo that crashes the client.
    return false;
}
