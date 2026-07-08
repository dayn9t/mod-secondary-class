#include "SC_talent_comm.h"

#include "Chat.h"          // ChatHandler::BuildChatPacket, CHAT_TAG_NONE
#include "Common.h"        // ChatMsg, Language, CHAT_MSG_ADDON, LANG_ADDON
#include "Log.h"
#include "Player.h"
#include "WorldPacket.h"

SecondaryClassTalentCommScript::SecondaryClassTalentCommScript()
    : PlayerScript("SecondaryClassTalentCommScript") { }

void SecondaryClassTalentCommScript::OnPlayerBeforeSendChatMessage(
    Player* player, uint32& /*type*/, uint32& lang, std::string& msg)
{
    // Only our addon channel (LANG_ADDON) and our prefix.
    if (lang != LANG_ADDON || msg.rfind("SC\t", 0) != 0)
        return;

    LOG_INFO("sc.talent", "[SC-SMOKE] recv from {}: {}", player->GetName(), msg);

    // Reply via CHAT_MSG_ADDON so the addon's CHAT_MSG_ADDON handler fires.
    WorldPacket data;
    ChatHandler::BuildChatPacket(
        data, CHAT_MSG_ADDON, "SC\tOK\tPING", LANG_ADDON, CHAT_TAG_NONE,
        player->GetGUID(), player->GetName());
    player->SendDirectMessage(&data);

    msg.clear();  // hook takes msg by reference: clear to suppress broadcast
}
