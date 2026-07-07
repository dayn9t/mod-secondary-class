# mod-secondary-class: Secondary class (dual-class) system for AzerothCore WotLK
# Stage 1 skeleton — sources added incrementally; AC_ADD_SCRIPT list kept in sync.
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/SC_loader.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/SC_entry.cpp")
AC_ADD_SCRIPT_LOADER("mod_secondary_class" "${CMAKE_CURRENT_LIST_DIR}/src/SC_loader.h")
