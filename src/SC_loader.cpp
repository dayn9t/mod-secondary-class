// Secondary class module — script loader.
// AC_ADD_SCRIPT_LOADER("mod_secondary_class") generates a call to
// Addmod_secondary_classScripts(); we forward to the real registration
// in SC_entry.cpp (mirrors mod-playerbots' playerbots_loader.cpp pattern).

#include "SC_loader.h"

void AddSC_secondary_class();  // defined in SC_entry.cpp

void Addmod_secondary_classScripts()
{
    AddSC_secondary_class();
}
