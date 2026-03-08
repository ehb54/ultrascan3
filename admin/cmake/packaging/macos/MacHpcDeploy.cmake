# =============================================================================
# MacHpcDeploy.cmake - macOS HPC deployment for UltraScan3
#
# Stages a minimal headless runtime bundle for HPC use on macOS
# (Apple Silicon compute nodes, AWS, macOS CI workers running batch jobs).
# Mirrors the legacy NO_DB supercomputer build model:
#   - us_mpi_analysis  (bare HPC executable, no .app wrapper needed)
#   - libus_utils      (UltraScan shared dylib)
#   - Qt/OpenSSL dylib runtime files
#   - setup_hpc_env.sh + README
#
# Does NOT include:
#   - 'us' GUI launcher or any .app GUI bundles
#   - Qt Assistant or help docs
#   - desktop LaunchDaemon
#   - plugins tree (headless executables don't need platform plugins)
#
# Layout:
#   <HPC_STAGE_DIR>/
#     bin/              us_mpi_analysis (bare executable, + any other us_* HPC binaries)
#     lib/              libus_utils*.dylib + Qt/OpenSSL dylib runtime files
#     setup_hpc_env.sh  source to set DYLD_LIBRARY_PATH
#     README-HPC.txt
#     license.txt
#
# Required variables:
#   HPC_STAGE_DIR       - Root of the staged directory tree
#   BIN_DIR             - Build-tree bin/ containing us_mpi_analysis
#   LIB_DIR             - Build-tree lib/ containing libusutils*.dylib
#
# Optional variables:
#   LICENSE_FILE        - Path to LICENSE.txt
#   VCPKG_LIB_DIR       - vcpkg lib/ dir (Qt/OpenSSL dylibs)
#   US3_VERSION_STRING  - Version string for README / filename
#
# Note: MACDEPLOYQT is intentionally not used here. The headless HPC model
# uses DYLD_LIBRARY_PATH to find dylibs at runtime; .app bundle machinery
# and macdeployqt rpath rewriting are desktop-app concerns, not HPC concerns.
# =============================================================================

cmake_minimum_required(VERSION 3.16)

if(NOT APPLE)
    message(STATUS "[MacHpcDeploy] Skipping -- not a macOS build.")
    return()
endif()

# ---------------------------------------------------------------------------
# Validate required inputs
# ---------------------------------------------------------------------------
if(NOT HPC_STAGE_DIR)
    message(FATAL_ERROR "[MacHpcDeploy] HPC_STAGE_DIR not set")
endif()

if(NOT DEFINED US3_VERSION_STRING OR US3_VERSION_STRING STREQUAL "")
    set(US3_VERSION_STRING "(unknown version)")
endif()

# ---------------------------------------------------------------------------
# Directory aliases
# ---------------------------------------------------------------------------
set(S_BIN "${HPC_STAGE_DIR}/bin")
set(S_LIB "${HPC_STAGE_DIR}/lib")

file(MAKE_DIRECTORY "${S_BIN}")
file(MAKE_DIRECTORY "${S_LIB}")

# =========================================================================
# 1) Stage us_mpi_analysis (primary HPC binary) and any other us_* binaries
#    Excludes 'us' (GUI launcher) and 'assistant'.
#    Bare executables only — no .app bundles needed for headless HPC use.
# =========================================================================
set(_bin_count 0)

if(BIN_DIR AND EXISTS "${BIN_DIR}")
    # Stage us_mpi_analysis first — it is the required HPC binary
    set(_mpi_bin "${BIN_DIR}/us_mpi_analysis")
    if(EXISTS "${_mpi_bin}")
        message(STATUS "[MacHpcDeploy] Staging us_mpi_analysis")
        file(COPY "${_mpi_bin}" DESTINATION "${S_BIN}"
             FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                              GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
        math(EXPR _bin_count "${_bin_count} + 1")
    else()
        message(WARNING "[MacHpcDeploy] us_mpi_analysis not found in ${BIN_DIR}")
        message(WARNING "[MacHpcDeploy] Ensure the HPC profile built us_mpi_analysis.")
    endif()

    # Stage any other us_* bare executables (not .app bundles)
    file(GLOB _other_bins "${BIN_DIR}/us_*")
    foreach(_b ${_other_bins})
        if(IS_DIRECTORY "${_b}")
            continue()  # skip .app bundles from APP builds if present
        endif()
        get_filename_component(_n "${_b}" NAME)
        if("${_n}" STREQUAL "us_mpi_analysis")
            continue()  # already staged
        endif()
        if("${_n}" STREQUAL "assistant")
            continue()
        endif()
        message(STATUS "[MacHpcDeploy] Staging additional HPC binary: ${_n}")
        file(COPY "${_b}" DESTINATION "${S_BIN}"
             FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                              GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
        math(EXPR _bin_count "${_bin_count} + 1")
    endforeach()
else()
    message(WARNING "[MacHpcDeploy] BIN_DIR does not exist: ${BIN_DIR}")
    message(WARNING "[MacHpcDeploy] No executables staged. "
        "Build the HPC profile first: ./scripts/build.sh HPC")
endif()

message(STATUS "[MacHpcDeploy] Staged ${_bin_count} HPC executables -> bin/")

# =========================================================================
# 2) Stage UltraScan shared libraries (libus_utils + any other US3 dylibs)
# =========================================================================
if(LIB_DIR AND EXISTS "${LIB_DIR}")
    file(GLOB _us_libs
        "${LIB_DIR}/libusutils*.dylib"
        "${LIB_DIR}/libusUtils*.dylib"
        "${LIB_DIR}/libus_utils*.dylib"
    )
    set(_us_lib_count 0)
    foreach(_lib ${_us_libs})
        get_filename_component(_n "${_lib}" NAME)
        get_filename_component(_real "${_lib}" REALPATH)
        message(STATUS "[MacHpcDeploy] Staging US3 lib: ${_n}")
        file(COPY "${_real}" DESTINATION "${S_LIB}")
        # Preserve soname symlinks
        get_filename_component(_real_name "${_real}" NAME)
        if(NOT "${_real_name}" STREQUAL "${_n}")
            execute_process(
                COMMAND ${CMAKE_COMMAND} -E create_symlink "${_real_name}" "${S_LIB}/${_n}"
            )
        endif()
        math(EXPR _us_lib_count "${_us_lib_count} + 1")
    endforeach()
    message(STATUS "[MacHpcDeploy] Staged ${_us_lib_count} US3 dylib files -> lib/")

    # Fix install names on the staged US3 dylibs so they are self-relative
    file(GLOB _staged_us_libs "${S_LIB}/libusutils*.dylib" "${S_LIB}/libus_utils*.dylib")
    foreach(_lib ${_staged_us_libs})
        get_filename_component(_n "${_lib}" NAME)
        # Set the install name to @rpath/<name> so dependents can find it via RPATH
        execute_process(COMMAND install_name_tool -id "@rpath/${_n}" "${_lib}" ERROR_QUIET)
    endforeach()
else()
    message(WARNING "[MacHpcDeploy] LIB_DIR not set or missing -- US3 dylibs not staged")
endif()

# =========================================================================
# 3) Stage Qt + OpenSSL runtime dylibs from vcpkg
#    These are the libraries us_mpi_analysis links against at runtime.
#    We set DYLD_LIBRARY_PATH in setup_hpc_env.sh to point at this dir.
# =========================================================================
if(VCPKG_LIB_DIR AND EXISTS "${VCPKG_LIB_DIR}")
    file(GLOB _vcpkg_dylibs
        "${VCPKG_LIB_DIR}/*.dylib"
    )
    set(_vcpkg_count 0)
    foreach(_dylib ${_vcpkg_dylibs})
        get_filename_component(_n "${_dylib}" NAME)
        if(NOT EXISTS "${S_LIB}/${_n}")
            file(COPY "${_dylib}" DESTINATION "${S_LIB}")
            math(EXPR _vcpkg_count "${_vcpkg_count} + 1")
        endif()
    endforeach()
    message(STATUS "[MacHpcDeploy] Staged ${_vcpkg_count} vcpkg runtime dylibs -> lib/")
else()
    message(STATUS "[MacHpcDeploy] VCPKG_LIB_DIR not set -- Qt/OpenSSL dylibs not bundled")
    message(STATUS "[MacHpcDeploy] Users must provide Qt/OpenSSL via DYLD_LIBRARY_PATH or system install")
endif()

# =========================================================================
# 4) Fix rpaths on staged HPC executables
#    They need to find libus_utils.dylib and Qt dylibs in ../lib/
#    relative to their location in bin/.
# =========================================================================
file(GLOB _hpc_bins "${S_BIN}/us_*")
foreach(_b ${_hpc_bins})
    if(IS_DIRECTORY "${_b}")
        continue()
    endif()
    # Remove existing rpaths that point into the build tree
    execute_process(
        COMMAND bash -c "otool -l '${_b}' 2>/dev/null | grep -A2 LC_RPATH | awk '/path /{print $2}'"
        OUTPUT_VARIABLE _old_rpaths OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(_old_rpaths)
        string(REPLACE "\n" ";" _old_rpath_list "${_old_rpaths}")
        foreach(_rp IN LISTS _old_rpath_list)
            execute_process(COMMAND install_name_tool -delete_rpath "${_rp}" "${_b}" ERROR_QUIET)
        endforeach()
    endif()
    # Add rpath pointing at ../lib/ (relative to bin/)
    execute_process(COMMAND install_name_tool -add_rpath "@executable_path/../lib" "${_b}" ERROR_QUIET)
endforeach()

# Fix rpaths on staged US3 dylibs so they find each other in lib/
file(GLOB _staged_libs "${S_LIB}/*.dylib")
foreach(_lib ${_staged_libs})
    if(IS_SYMLINK "${_lib}")
        continue()
    endif()
    execute_process(
        COMMAND bash -c "otool -l '${_lib}' 2>/dev/null | grep -A2 LC_RPATH | awk '/path /{print $2}'"
        OUTPUT_VARIABLE _lib_rpaths OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(_lib_rpaths)
        string(REPLACE "\n" ";" _lib_rpath_list "${_lib_rpaths}")
        foreach(_rp IN LISTS _lib_rpath_list)
            execute_process(COMMAND install_name_tool -delete_rpath "${_rp}" "${_lib}" ERROR_QUIET)
        endforeach()
    endif()
    execute_process(COMMAND install_name_tool -add_rpath "@loader_path" "${_lib}" ERROR_QUIET)
endforeach()

# =========================================================================
# 5) Ad-hoc codesign staged binaries and dylibs
# =========================================================================
file(GLOB _sign_bins "${S_BIN}/us_*")
foreach(_b ${_sign_bins})
    if(NOT IS_DIRECTORY "${_b}")
        execute_process(COMMAND codesign --force --sign - "${_b}" ERROR_QUIET)
    endif()
endforeach()

file(GLOB _sign_libs "${S_LIB}/*.dylib")
foreach(_lib ${_sign_libs})
    if(NOT IS_SYMLINK "${_lib}")
        execute_process(COMMAND codesign --force --sign - "${_lib}" ERROR_QUIET)
    endif()
endforeach()

message(STATUS "[MacHpcDeploy] Ad-hoc codesigned executables and libraries")

# =========================================================================
# 6) Generate setup_hpc_env.sh
#    Sets DYLD_LIBRARY_PATH to the bundled lib/ dir.
#    No QT_PLUGIN_PATH needed — us_mpi_analysis is headless.
# =========================================================================
file(WRITE "${HPC_STAGE_DIR}/setup_hpc_env.sh"
"#!/bin/bash
# UltraScan3 macOS HPC environment setup - ${US3_VERSION_STRING}
# Source this file before running UltraScan3 HPC programs:
#   source /path/to/UltraScan3-HPC/setup_hpc_env.sh

_US3_HPC_DIR=\"\$( cd \"\$( dirname \"\${BASH_SOURCE[0]}\" )\" >/dev/null 2>&1 && pwd )\"

export DYLD_LIBRARY_PATH=\"\${_US3_HPC_DIR}/lib\${DYLD_LIBRARY_PATH:+:\${DYLD_LIBRARY_PATH}}\"
export US3_HPC_BIN=\"\${_US3_HPC_DIR}/bin\"

echo \"UltraScan3 HPC ${US3_VERSION_STRING} environment loaded from \${_US3_HPC_DIR}\"
echo \"  us_mpi_analysis: \${US3_HPC_BIN}/us_mpi_analysis\"
")
execute_process(COMMAND chmod 755 "${HPC_STAGE_DIR}/setup_hpc_env.sh")
message(STATUS "[MacHpcDeploy] Generated setup_hpc_env.sh")

# =========================================================================
# 7) Generate README-HPC.txt
# =========================================================================
file(WRITE "${HPC_STAGE_DIR}/README-HPC.txt"
"UltraScan3 ${US3_VERSION_STRING} - macOS HPC Build
================================================================================

Minimal headless runtime for HPC/supercomputer use on macOS.
Centered on us_mpi_analysis + libus_utils (legacy supercomputer build model).

CONTENTS
  bin/us_mpi_analysis  UltraScan3 MPI analysis engine (bare executable)
  lib/                 Bundled runtime libraries (libus_utils + Qt/OpenSSL dylibs)
  setup_hpc_env.sh     Environment setup helper
  README-HPC.txt       This file
  license.txt          LGPL license

SETUP
  source /path/to/UltraScan3-HPC/setup_hpc_env.sh

  Or manually:
    export DYLD_LIBRARY_PATH=/path/to/UltraScan3-HPC/lib:\$DYLD_LIBRARY_PATH

USAGE
  mpirun -np <N> /path/to/UltraScan3-HPC/bin/us_mpi_analysis <options>

NOTE
  macOS System Integrity Protection (SIP) may block DYLD_LIBRARY_PATH in some
  contexts. If so, install the dylibs system-wide or use a wrapper script that
  re-exports the variable after disabling SIP for the session.

WEBSITE
  https://ultrascan.aucsolutions.com

LICENSE
  UltraScan III is distributed under the GNU Lesser General Public License
  (LGPL). See license.txt for the full license text.
")
message(STATUS "[MacHpcDeploy] Generated README-HPC.txt")

# =========================================================================
# 8) Copy license.txt
# =========================================================================
if(LICENSE_FILE AND EXISTS "${LICENSE_FILE}")
    file(COPY "${LICENSE_FILE}" DESTINATION "${HPC_STAGE_DIR}")
    get_filename_component(_lic_name "${LICENSE_FILE}" NAME)
    if(NOT "${_lic_name}" STREQUAL "license.txt")
        file(RENAME "${HPC_STAGE_DIR}/${_lic_name}" "${HPC_STAGE_DIR}/license.txt")
    endif()
    message(STATUS "[MacHpcDeploy] Copied license.txt")
endif()

# =========================================================================
# Summary
# =========================================================================
message(STATUS "")
message(STATUS "[MacHpcDeploy] macOS HPC deployment complete: ${HPC_STAGE_DIR}")
message(STATUS "  bin/              - us_mpi_analysis (+ any other HPC executables)")
message(STATUS "  lib/              - libus_utils + Qt/OpenSSL runtime dylibs")
message(STATUS "  setup_hpc_env.sh  - source to set DYLD_LIBRARY_PATH")
message(STATUS "  README-HPC.txt    - quick-start instructions")
