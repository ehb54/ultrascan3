# =============================================================================
# WinDeploy.cmake - Windows deployment for UltraScan3 (flat-directory layout)
#
# Stages a self-contained directory tree ready for packaging or xcopy install:
#
#   <STAGE_DIR>/
#     bin/           us.exe, us_*.exe, assistant.exe, UltraScan DLLs, Qt DLLs, manual.qhc, ...
#     etc/           editable config data
#     lib/           reserved / non-runtime library area
#     plugins/       Qt plugins (platforms/, sqldrivers/, imageformats/, …)
#     somo/
#     license.txt
#
# Expected variables (passed via -D on the cmake -P command line):
#   STAGE_DIR           - Root of the staged directory tree
#   WINDEPLOYQT         - Path to windeployqt.exe
#   BIN_DIR             - Build-tree bin/ containing us.exe and us_*.exe
#   LIB_DIR             - Build-tree lib/ (currently unused by this script)
#   ETC_SOURCE_DIR      - Source etc/ directory
#   SOMO_SOURCE_DIR     - Source somo/ directory
#   LICENSE_FILE        - Path to LICENSE.txt
#   QCH_DIR             - Directory containing manual.qch / manual.qhc
#   VCPKG_BIN_DIR       - currently unused
#   VCPKG_PLUGIN_DIR    - vcpkg installed plugins/ dir
#   VCPKG_LIB_DIR       - currently unused#
# Optional:
#   ASSISTANT_EXE       - Path to assistant.exe from the Qt/vcpkg build tree
#   SOMO_BIN_DIR        - Build-tree SoMo bin/ (us3_hydrodyn.exe, …)
#   SOMO_LIB_DIR        - Build-tree SoMo lib/ (SoMo DLLs)
#
# Mirrors: admin/cmake/packaging/macos/MacDeploy.cmake
# =============================================================================

cmake_minimum_required(VERSION 3.16)

# ---------------------------------------------------------------------------
# Guard: Windows only
# ---------------------------------------------------------------------------
if(NOT WIN32)
    message(STATUS "[WinDeploy] Skipping — not a Windows build.")
    return()
endif()

# ---------------------------------------------------------------------------
# Validate required inputs
# ---------------------------------------------------------------------------
if(NOT STAGE_DIR)
    message(FATAL_ERROR "[WinDeploy] STAGE_DIR not set")
endif()
if(NOT WINDEPLOYQT)
    message(FATAL_ERROR "[WinDeploy] WINDEPLOYQT not set")
endif()
if(NOT EXISTS "${WINDEPLOYQT}")
    message(FATAL_ERROR "[WinDeploy] WINDEPLOYQT does not exist: ${WINDEPLOYQT}")
endif()
if(NOT BIN_DIR OR NOT EXISTS "${BIN_DIR}")
    message(FATAL_ERROR "[WinDeploy] BIN_DIR does not exist: ${BIN_DIR}")
endif()

# ---------------------------------------------------------------------------
# Convenience path aliases (mirrors MacDeploy.cmake's S_* variables)
# ---------------------------------------------------------------------------
set(S_BIN  "${STAGE_DIR}/bin")
set(S_LIB  "${STAGE_DIR}/lib")
set(S_PLUG "${STAGE_DIR}/plugins")
set(S_ETC  "${STAGE_DIR}/etc")
set(S_SOMO "${STAGE_DIR}/somo")

if(EXISTS "${STAGE_DIR}")
    message(STATUS "[WinDeploy] Removing existing stage dir: ${STAGE_DIR}")
    file(REMOVE_RECURSE "${STAGE_DIR}")
endif()

file(MAKE_DIRECTORY "${S_BIN}")
file(MAKE_DIRECTORY "${S_LIB}")
file(MAKE_DIRECTORY "${S_PLUG}")

# =========================================================================
# 1) Copy us.exe into bin/
# =========================================================================
if(EXISTS "${BIN_DIR}/us.exe")
    message(STATUS "Copying us.exe → ${S_BIN}/")
    file(COPY "${BIN_DIR}/us.exe" DESTINATION "${S_BIN}")
else()
    message(FATAL_ERROR "[WinDeploy] us.exe not found in ${BIN_DIR}")
endif()

# =========================================================================
# 2) Copy companion us_*.exe executables into bin/
# =========================================================================
file(GLOB extra_exes "${BIN_DIR}/us_*.exe")
foreach(exe ${extra_exes})
    get_filename_component(exe_name "${exe}" NAME)
    message(STATUS "  Copying program: ${exe_name}")
    file(COPY "${exe}" DESTINATION "${S_BIN}")
endforeach()

# =========================================================================
# 3) Copy ONLY UltraScan DLLs from BIN_DIR into staged bin/
#
#    Do NOT copy every DLL from BIN_DIR. That can stage stale or mismatched
#    Qt/runtime DLLs and break plugin loading in the final installer.
#    Qt DLLs and plugins must come from windeployqt only.
# =========================================================================
if(BIN_DIR AND EXISTS "${BIN_DIR}")
    file(GLOB _us_bin_dlls
            "${BIN_DIR}/us*.dll"
            "${BIN_DIR}/libus*.dll"
    )
    foreach(_dll ${_us_bin_dlls})
        get_filename_component(_dll_name "${_dll}" NAME)
        message(STATUS "  Copying UltraScan DLL from build bin/: ${_dll_name}")
        file(COPY "${_dll}" DESTINATION "${S_BIN}")
    endforeach()
    list(LENGTH _us_bin_dlls _dll_count)
    message(STATUS "[WinDeploy] Copied ${_dll_count} UltraScan DLL(s) from build bin/")
else()
    message(WARNING "[WinDeploy] BIN_DIR not set or missing -- no UltraScan DLLs staged from build tree")
endif()

# =========================================================================
# 4) Run windeployqt on us.exe
#
#    windeployqt copies all Qt DLLs, plugin sub-directories, and the
#    Visual C++ / MinGW runtime libraries into the target directory.
#    Using --dir bin/ ensures all DLLs land next to the executable;
#    --plugindir explicitly routes plugins into the staged plugins/ tree.
#
#    Equivalent to macOS: macdeployqt ${S_BIN}/us.app
# =========================================================================
message(STATUS "Running windeployqt on us.exe...")

set(US_EXE "${S_BIN}/us.exe")

execute_process(
    COMMAND "${WINDEPLOYQT}"
            "${US_EXE}"
            --release
            --compiler-runtime
            --dir         "${S_BIN}"
            --plugindir   "${S_PLUG}"
            --no-translations
            --verbose 1
    RESULT_VARIABLE _wdq_result
    OUTPUT_VARIABLE _wdq_output
    ERROR_VARIABLE  _wdq_error
)

if(NOT _wdq_result EQUAL 0)
    message(FATAL_ERROR
            "[WinDeploy] windeployqt failed for us.exe with code ${_wdq_result}\n"
            "stdout:\n${_wdq_output}\n"
            "stderr:\n${_wdq_error}")
else()
    message(STATUS "  windeployqt succeeded")
endif()

# =========================================================================
# 4b) Run windeployqt on each companion us_*.exe
#
#    windeployqt is idempotent — it skips DLLs already present in --dir.
#    Running it per-executable ensures every Qt module actually used by
#    any companion tool is included in the deployment.
# =========================================================================
file(GLOB companion_exes "${S_BIN}/us_*.exe")
foreach(exe ${companion_exes})
    get_filename_component(exe_name "${exe}" NAME)
    message(STATUS "  windeployqt → ${exe_name}")
    execute_process(
        COMMAND "${WINDEPLOYQT}"
                "${exe}"
                --release
                --compiler-runtime
                --dir         "${S_BIN}"
                --plugindir   "${S_PLUG}"
                --no-translations
                --verbose 0
        RESULT_VARIABLE _r
        OUTPUT_VARIABLE _o
        ERROR_VARIABLE  _e
    )
    if(NOT _r EQUAL 0)
        message(WARNING "[WinDeploy] windeployqt on ${exe_name} exited with code ${_r}")
        message(STATUS  "  stderr: ${_e}")
    endif()
endforeach()

# =========================================================================
# 5) Guarantee platforms/qwindows.dll is present
#
#    windeployqt should deploy it, but if it was run from a static Qt
#    build or an unusual prefix it may be missing.  We provide a
#    fallback search against VCPKG_PLUGIN_DIR.
#    Equivalent to macOS section 9c (libqcocoa.dylib safety-net).
# =========================================================================
file(MAKE_DIRECTORY "${S_PLUG}/platforms")
if(NOT EXISTS "${S_PLUG}/platforms/qwindows.dll")
    set(_QWINDOWS_FOUND FALSE)

    # Build a list of candidate directories, mirroring the macOS search
    set(_PLATFORMS_SEARCH_DIRS "")
    if(VCPKG_PLUGIN_DIR)
        list(APPEND _PLATFORMS_SEARCH_DIRS "${VCPKG_PLUGIN_DIR}/platforms")
        # vcpkg Qt6 layout: <triplet>/Qt6/plugins/platforms
        get_filename_component(_VCPKG_TRIPLET_DIR "${VCPKG_PLUGIN_DIR}" DIRECTORY)
        list(APPEND _PLATFORMS_SEARCH_DIRS "${_VCPKG_TRIPLET_DIR}/Qt6/plugins/platforms")
    endif()

    foreach(_SEARCH_DIR IN LISTS _PLATFORMS_SEARCH_DIRS)
        if(EXISTS "${_SEARCH_DIR}/qwindows.dll")
            message(STATUS "[WinDeploy] Safety-net: copying platforms/qwindows.dll from ${_SEARCH_DIR}")
            file(COPY "${_SEARCH_DIR}/qwindows.dll" DESTINATION "${S_PLUG}/platforms")
            set(_QWINDOWS_FOUND TRUE)
            break()
        endif()
    endforeach()

    if(NOT _QWINDOWS_FOUND)
        message(FATAL_ERROR
                "[WinDeploy] platforms/qwindows.dll not found after windeployqt. "
                "The application will fail to start without a platform plugin. "
                "Searched: ${_PLATFORMS_SEARCH_DIRS}. "
                "Set VCPKG_PLUGIN_DIR to the vcpkg installed plugins/ directory.")
    endif()
else()
    message(STATUS "[WinDeploy] platforms/qwindows.dll already present")
endif()

# =========================================================================
# 6) Guarantee sqldrivers/qsqlite.dll is present
#
#    Qt Assistant's search index uses SQLite.  windeployqt only deploys
#    SQL plugins when the executable itself links Qt SQL directly.  We
#    add the same safety-net used on macOS (section 9b).
# =========================================================================
file(MAKE_DIRECTORY "${S_PLUG}/sqldrivers")
if(NOT EXISTS "${S_PLUG}/sqldrivers/qsqlite.dll")
    set(_QSQLITE_FOUND FALSE)

    set(_QSQLITE_SEARCH_DIRS "")
    if(VCPKG_PLUGIN_DIR)
        list(APPEND _QSQLITE_SEARCH_DIRS "${VCPKG_PLUGIN_DIR}/sqldrivers")
        get_filename_component(_VCPKG_TRIPLET_DIR "${VCPKG_PLUGIN_DIR}" DIRECTORY)
        list(APPEND _QSQLITE_SEARCH_DIRS "${_VCPKG_TRIPLET_DIR}/Qt6/plugins/sqldrivers")
    endif()

    foreach(_SEARCH_DIR IN LISTS _QSQLITE_SEARCH_DIRS)
        if(EXISTS "${_SEARCH_DIR}/qsqlite.dll")
            message(STATUS "[WinDeploy] Safety-net: copying sqldrivers/qsqlite.dll from ${_SEARCH_DIR}")
            file(COPY "${_SEARCH_DIR}/qsqlite.dll" DESTINATION "${S_PLUG}/sqldrivers")
            set(_QSQLITE_FOUND TRUE)
            break()
        endif()
    endforeach()

    if(NOT _QSQLITE_FOUND)
        message(WARNING
            "[WinDeploy] sqldrivers/qsqlite.dll not found. "
            "Qt Assistant search will not work without it. "
            "Searched: ${_QSQLITE_SEARCH_DIRS}.")
    endif()
else()
    message(STATUS "[WinDeploy] sqldrivers/qsqlite.dll already present")
endif()

# =========================================================================
# 7) Deploy Qt Assistant (help viewer)
#    Mirrors MacDeploy.cmake section 9.
# =========================================================================
if(ASSISTANT_EXE AND EXISTS "${ASSISTANT_EXE}")
    message(STATUS "[WinDeploy] Deploying Qt Assistant: ${ASSISTANT_EXE}")
    file(COPY "${ASSISTANT_EXE}" DESTINATION "${S_BIN}")
    set(_ASSISTANT_STAGED "${S_BIN}/assistant.exe")

    # Run windeployqt on the staged assistant.exe so it gets its own
    # required DLLs (Qt Help, Qt SQL, WebEngine, etc.)
    message(STATUS "  Running windeployqt on assistant.exe...")
    execute_process(
        COMMAND "${WINDEPLOYQT}"
                "${_ASSISTANT_STAGED}"
                --release
                --compiler-runtime
                --dir         "${S_BIN}"
                --plugindir   "${S_PLUG}"
                --no-translations
                --verbose 0
        RESULT_VARIABLE _r
        ERROR_VARIABLE  _e
    )
    if(NOT _r EQUAL 0)
        message(WARNING "[WinDeploy] windeployqt on assistant.exe exited with code ${_r}")
        message(STATUS  "  stderr: ${_e}")
    endif()
else()
    message(STATUS "[WinDeploy] ASSISTANT_EXE not provided — help system may not work")
endif()

# =========================================================================
# 8) Copy manual.qch / manual.qhc into bin/
#    Mirrors MacDeploy.cmake section 10.
# =========================================================================
if(QCH_DIR)
    foreach(_qch_file manual.qch manual.qhc)
        if(EXISTS "${QCH_DIR}/${_qch_file}")
            message(STATUS "  Copying ${_qch_file} → bin/")
            file(COPY "${QCH_DIR}/${_qch_file}" DESTINATION "${S_BIN}")
        endif()
    endforeach()
endif()

# =========================================================================
# 9) Copy etc/ and somo/
#    Mirrors MacDeploy.cmake section 11.
# =========================================================================
if(ETC_SOURCE_DIR AND EXISTS "${ETC_SOURCE_DIR}")
    message(STATUS "[WinDeploy] Copying etc/ → ${S_ETC}/")
    file(COPY "${ETC_SOURCE_DIR}/" DESTINATION "${S_ETC}")
endif()

if(SOMO_SOURCE_DIR AND EXISTS "${SOMO_SOURCE_DIR}")
    message(STATUS "[WinDeploy] Copying somo/ → ${S_SOMO}/")
    file(COPY "${SOMO_SOURCE_DIR}/" DESTINATION "${S_SOMO}")
endif()

# =========================================================================
# 9b) Stage SoMo binaries and libraries
#     Mirrors MacDeploy.cmake section 11b.
# =========================================================================
if(SOMO_BIN_DIR AND EXISTS "${SOMO_BIN_DIR}")
    message(STATUS "[WinDeploy] Staging SoMo binaries from ${SOMO_BIN_DIR} → bin/")
    file(GLOB _somo_exes "${SOMO_BIN_DIR}/*.exe")
    foreach(_exe ${_somo_exes})
        get_filename_component(_exe_name "${_exe}" NAME)
        message(STATUS "  SoMo binary: ${_exe_name}")
        file(COPY "${_exe}" DESTINATION "${S_BIN}")
    endforeach()
else()
    message(STATUS "[WinDeploy] SoMo bin dir not provided or not yet built — skipping SoMo binary staging")
endif()

if(SOMO_LIB_DIR AND EXISTS "${SOMO_LIB_DIR}")
    message(STATUS "[WinDeploy] Staging SoMo DLLs from ${SOMO_LIB_DIR} → bin/")
    file(GLOB _somo_dlls "${SOMO_LIB_DIR}/*.dll")
    foreach(_dll ${_somo_dlls})
        get_filename_component(_dll_name "${_dll}" NAME)

        if(_dll_name MATCHES "^Qt[56].*\\.dll$"
                OR _dll_name MATCHES "^qwt.*\\.dll$"
                OR _dll_name MATCHES "^icu.*\\.dll$"
                OR _dll_name MATCHES "^libpng.*\\.dll$"
                OR _dll_name MATCHES "^zlib.*\\.dll$"
                OR _dll_name MATCHES "^freetype.*\\.dll$"
                OR _dll_name MATCHES "^harfbuzz.*\\.dll$")
            message(STATUS "  Skipping non-SoMo runtime DLL from SoMo lib dir: ${_dll_name}")
        elseif(NOT EXISTS "${S_BIN}/${_dll_name}")
            message(STATUS "  SoMo DLL: ${_dll_name}")
            file(COPY "${_dll}" DESTINATION "${S_BIN}")
        endif()
    endforeach()
else()
    message(STATUS "[WinDeploy] SoMo lib dir not provided or not yet built — skipping SoMo DLL staging")
endif()

# =========================================================================
# 10) Copy license.txt
#     Mirrors MacDeploy.cmake section 12.
# =========================================================================
if(LICENSE_FILE AND EXISTS "${LICENSE_FILE}")
    message(STATUS "[WinDeploy] Copying license.txt")
    file(COPY "${LICENSE_FILE}" DESTINATION "${STAGE_DIR}")
    get_filename_component(_lic_name "${LICENSE_FILE}" NAME)
    if(NOT "${_lic_name}" STREQUAL "license.txt")
        file(RENAME "${STAGE_DIR}/${_lic_name}" "${STAGE_DIR}/license.txt")
    endif()
endif()

# =========================================================================
# 11) Write a qt.conf alongside us.exe so Qt locates plugins/ and the
#     DLLs in bin/ without relying on registry entries or a PATH search.
#
#     On Windows, Qt looks for qt.conf in the same directory as the .exe.
#     Prefix "." means: relative to the directory that contains qt.conf,
#     which at run-time is bin/.  So:
#       Plugins = ../plugins   →  <install-root>/plugins/
#       Libraries = .          →  bin/   (Qt DLLs are alongside the .exe)
#
#     Mirrors MacDeploy.cmake section 14.
# =========================================================================
message(STATUS "[WinDeploy] Writing qt.conf files...")

set(_QTCONF_CONTENT "[Paths]\nPrefix = .\nPlugins = ../plugins\nLibraries = .\n")

# Write for us.exe
file(WRITE "${S_BIN}/qt.conf" "${_QTCONF_CONTENT}")

# Write for assistant.exe (same directory, same needs)
if(EXISTS "${S_BIN}/assistant.exe")
    # assistant.exe lives in bin/ alongside us.exe; it shares qt.conf
    message(STATUS "  qt.conf covers assistant.exe (shared bin/ location)")
endif()

# =========================================================================
# Summary
# =========================================================================
message(STATUS "")
message(STATUS "[WinDeploy] Windows folder deployment complete: ${STAGE_DIR}")
message(STATUS "  bin/     - Executables, Qt DLLs, runtime DLLs, help files")
message(STATUS "  plugins/ - Qt plugins (platforms/, sqldrivers/, …)")
message(STATUS "  lib/     - Reserved / import libraries (build-time only)")
message(STATUS "  etc/     - Configuration data")
message(STATUS "  somo/    - SOMO data")
