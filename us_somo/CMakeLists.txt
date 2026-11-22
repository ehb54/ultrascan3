# =============================================================================
# us_somo — thin forwarder
# =============================================================================
# The root UltraScan3 CMakeLists.txt integrates SOMO with:
#
#     option(US3_BUILD_SOMO "Build the SOMO/US-SOMO suite" OFF)
#     if(US3_BUILD_SOMO)
#         add_subdirectory(us_somo)     # <-- this file
#     endif()
#
# matching the existing `# add_subdirectory(us_somo)` placeholder on main.
# The actual build lives one level down, next to the SOMO sources and the
# legacy qmake .pro files, in develop/. Keeping the entry point here gives the
# root a stable path even if develop/ is later renamed or promoted.
# =============================================================================

add_subdirectory(develop)
