// Secondary class module — script registration entry.

#include "SC_commands.h"
#include "SC_player_hooks.h"
#include "SC_talent_comm.h"
#include "ScriptMgr.h"

void AddSC_secondary_class()
{
    new SecondaryClassPlayerScript();
    new secondary_commandscript();
    new SecondaryClassTalentCommScript();
}
