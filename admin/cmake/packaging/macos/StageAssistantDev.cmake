# StageAssistantDev.cmake
# Copies Assistant.app into the build bin/ and writes a qt.conf that points
# at the vcpkg Qt6 plugins directory so libqcocoa.dylib is found at runtime.
# Also patches the rpath so Qt dylibs are found from the vcpkg lib dir.
# Invoked as a post-build step on the `us` target (developer build only).
#
# Inputs (via -D):
#   ASSISTANT_SRC   - path to source Assistant.app
#   BIN_DIR         - build-tree bin/ directory
#   QT6_PLUGINS_DIR - vcpkg Qt6 plugins directory
#   QT6_LIB_DIR     - vcpkg Qt6 lib directory

cmake_minimum_required(VERSION 3.16)

if(NOT EXISTS "${ASSISTANT_SRC}/Contents/MacOS/Assistant")
    message(FATAL_ERROR "StageAssistantDev: Assistant binary not found at ${ASSISTANT_SRC}")
endif()

get_filename_component(BIN_DIR "${BIN_DIR}" ABSOLUTE)
message(STATUS "StageAssistantDev: BIN_DIR resolved to ${BIN_DIR}")
set(_DEST "${BIN_DIR}/Assistant.app")
set(_BIN  "${_DEST}/Contents/MacOS/Assistant")

# Always remove and recopy so qt.conf and rpath are never stale
file(REMOVE_RECURSE "${_DEST}")
file(COPY "${ASSISTANT_SRC}" DESTINATION "${BIN_DIR}")

# Add vcpkg lib dir as rpath so Qt dylibs are found at runtime
execute_process(
    COMMAND install_name_tool -add_rpath "${QT6_LIB_DIR}" "${_BIN}"
    ERROR_QUIET
)

# Ensure libqsqlite.dylib is in Assistant's own PlugIns/sqldrivers/
set(_asst_sql_dir "${_DEST}/Contents/PlugIns/sqldrivers")
file(MAKE_DIRECTORY "${_asst_sql_dir}")
if(NOT EXISTS "${_asst_sql_dir}/libqsqlite.dylib")
    set(_sql_src "${QT6_PLUGINS_DIR}/sqldrivers/libqsqlite.dylib")
    if(EXISTS "${_sql_src}")
        file(COPY "${_sql_src}" DESTINATION "${_asst_sql_dir}")
        message(STATUS "StageAssistantDev: staged libqsqlite.dylib into Assistant PlugIns/sqldrivers/")
    else()
        message(WARNING "StageAssistantDev: libqsqlite.dylib not found at ${_sql_src}")
    endif()
endif()

# Write qt.conf pointing at vcpkg plugins (absolute path)
file(MAKE_DIRECTORY "${_DEST}/Contents/Resources")
file(WRITE "${_DEST}/Contents/Resources/qt.conf"
"[Paths]\nPlugins = ${QT6_PLUGINS_DIR}\nLibraries = ${QT6_LIB_DIR}\n")

message(STATUS "StageAssistantDev: staged ${_DEST}")
message(STATUS "StageAssistantDev: rpath  -> ${QT6_LIB_DIR}")
message(STATUS "StageAssistantDev: qt.conf -> Plugins = ${QT6_PLUGINS_DIR}")
