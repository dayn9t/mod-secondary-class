#ifndef MOD_SC_COMMANDS_H
#define MOD_SC_COMMANDS_H

#include "CommandScript.h"

#include <string>
#include <vector>

class ChatHandler;

// CommandScript registering `.secondary set|unset|show` (GM-only).
class secondary_commandscript : public CommandScript
{
public:
    secondary_commandscript();
    std::vector<Acore::ChatCommands::ChatCommandBuilder> GetCommands() const override;

private:
    static bool HandleSecondarySetCommand(ChatHandler* handler, std::string className);
    static bool HandleSecondaryUnsetCommand(ChatHandler* handler);
    static bool HandleSecondaryShowCommand(ChatHandler* handler);
    static bool HandleSecondaryPowerInfoCommand(ChatHandler* handler);

    // Phase-3 talent GM commands (.sc learn|validate|show|reset) — bypass the
    // addon for admin/testing; same Shell the addon-message path uses.
    static bool HandleScLearnCommand(ChatHandler* handler, uint32 talentId);
    static bool HandleScValidateCommand(ChatHandler* handler, uint32 talentId);
    static bool HandleScShowCommand(ChatHandler* handler);
    static bool HandleScResetCommand(ChatHandler* handler);
};

#endif
