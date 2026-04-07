# =============================================================================
# LinuxHpcDeploy.cmake - Linux HPC deployment for UltraScan3
#
# Stages a minimal headless runtime bundle for HPC/supercomputer use.
# Mirrors the legacy NO_DB supercomputer build model:
#   - us_mpi_analysis  (the HPC compute binary)
#   - libus_utils      (UltraScan shared library)
#   - Qt/OpenSSL .so runtime files that us_mpi_analysis links against
#   - setup_hpc_env.sh + README
#
# Does NOT include:
#   - 'us' GUI launcher
#   - help docs / Qt Assistant
#   - Qt plugin tree
#   - desktop launchers or desktop-app packaging layout
#
# Layout:
#   <HPC_STAGE_DIR>/
#     bin/              us_mpi_analysis (+ any other us_* HPC executables found)
#     lib/              libus_utils*.so + Qt/OpenSSL .so runtime files
#     setup_hpc_env.sh  source this to set LD_LIBRARY_PATH
#     README-HPC.txt
#     license.txt
#
# Required variables:
#   HPC_STAGE_DIR   - Root of the staged directory tree
#   BIN_DIR         - Build-tree bin/ containing us_mpi_analysis
#   LIB_DIR         - Build-tree lib/ containing libusutils*.so
#
# Optional variables:
#   LICENSE_FILE        - Path to LICENSE.txt
#   VCPKG_LIB_DIR       - vcpkg installed lib/ (Qt/OpenSSL .so runtime files)
#   US3_VERSION_STRING  - Version string for README / filename
# =============================================================================

cmake_minimum_required(VERSION 3.16)

if(NOT UNIX OR APPLE)
    message(STATUS "[LinuxHpcDeploy] Skipping -- not a Linux build.")
    return()
endif()

# ---------------------------------------------------------------------------
# Validate required inputs
# ---------------------------------------------------------------------------
if(NOT HPC_STAGE_DIR)
    message(FATAL_ERROR "[LinuxHpcDeploy] HPC_STAGE_DIR not set")
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
# 1) Stage us_mpi_analysis (primary HPC binary)
#    Also stage any other us_* binaries present (e.g. future HPC tools).
#    Explicitly excludes 'us' (GUI launcher) and 'assistant'.
# =========================================================================
set(_bin_count 0)

if(BIN_DIR AND EXISTS "${BIN_DIR}")
    # Stage us_mpi_analysis first — it is the required HPC binary
    set(_mpi_bin "${BIN_DIR}/us_mpi_analysis")
    if(EXISTS "${_mpi_bin}")
        message(STATUS "[LinuxHpcDeploy] Staging us_mpi_analysis")
        file(COPY "${_mpi_bin}" DESTINATION "${S_BIN}"
             FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                              GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
        math(EXPR _bin_count "${_bin_count} + 1")
    else()
        message(WARNING "[LinuxHpcDeploy] us_mpi_analysis not found in ${BIN_DIR}")
        message(WARNING "[LinuxHpcDeploy] Ensure the HPC profile built us_mpi_analysis.")
    endif()

    # Stage any other us_* binaries (excluding GUI launcher and assistant)
    file(GLOB _other_bins "${BIN_DIR}/us_*")
    foreach(_b ${_other_bins})
        if(IS_DIRECTORY "${_b}")
            continue()
        endif()
        get_filename_component(_n "${_b}" NAME)
        if("${_n}" STREQUAL "us_mpi_analysis")
            continue()  # already staged
        endif()
        if("${_n}" STREQUAL "assistant")
            continue()  # never stage Qt Assistant
        endif()
        message(STATUS "[LinuxHpcDeploy] Staging additional HPC binary: ${_n}")
        file(COPY "${_b}" DESTINATION "${S_BIN}"
             FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                              GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
        math(EXPR _bin_count "${_bin_count} + 1")
    endforeach()
else()
    message(WARNING "[LinuxHpcDeploy] BIN_DIR does not exist: ${BIN_DIR}")
    message(WARNING "[LinuxHpcDeploy] No executables staged. "
        "Build the HPC profile first: ./scripts/build.sh HPC")
endif()

message(STATUS "[LinuxHpcDeploy] Staged ${_bin_count} HPC executables -> bin/")

# =========================================================================
# 2) Stage UltraScan shared libraries (libus_utils + any other US3 .so)
# =========================================================================
if(LIB_DIR AND EXISTS "${LIB_DIR}")
    file(GLOB _us_libs
        "${LIB_DIR}/libusutils*.so*"
        "${LIB_DIR}/libusUtils*.so*"
        "${LIB_DIR}/libus_utils*.so*"
    )
    set(_us_lib_count 0)
    foreach(_lib ${_us_libs})
        get_filename_component(_n "${_lib}" NAME)
        message(STATUS "[LinuxHpcDeploy] Staging US3 lib: ${_n}")
        file(COPY "${_lib}" DESTINATION "${S_LIB}")
        math(EXPR _us_lib_count "${_us_lib_count} + 1")
    endforeach()
    message(STATUS "[LinuxHpcDeploy] Staged ${_us_lib_count} US3 .so files -> lib/")
else()
    message(WARNING "[LinuxHpcDeploy] LIB_DIR not set or missing -- US3 .so files not staged")
endif()

# =========================================================================
# 3) Stage Qt + OpenSSL runtime .so files from vcpkg
#    These are the libraries us_mpi_analysis links against at runtime.
#    We copy the full vcpkg lib/ to ensure all transitive deps are present;
#    this matches the legacy LD_LIBRARY_PATH-based deployment model.
#    No plugin tree is needed — us_mpi_analysis is headless.
# =========================================================================
if(VCPKG_LIB_DIR AND EXISTS "${VCPKG_LIB_DIR}")
    file(GLOB _vcpkg_so
        "${VCPKG_LIB_DIR}/*.so"
        "${VCPKG_LIB_DIR}/*.so.*"
    )
    set(_vcpkg_count 0)
    foreach(_so ${_vcpkg_so})
        get_filename_component(_n "${_so}" NAME)
        if(NOT EXISTS "${S_LIB}/${_n}")
            file(COPY "${_so}" DESTINATION "${S_LIB}")
            math(EXPR _vcpkg_count "${_vcpkg_count} + 1")
        endif()
    endforeach()
    message(STATUS "[LinuxHpcDeploy] Staged ${_vcpkg_count} vcpkg runtime .so files -> lib/")
else()
    message(STATUS "[LinuxHpcDeploy] VCPKG_LIB_DIR not set -- Qt/OpenSSL .so files not bundled")
    message(STATUS "[LinuxHpcDeploy] Users must provide Qt/OpenSSL via LD_LIBRARY_PATH or system install")
endif()

# =========================================================================
# 4) Strip binaries and libraries (release builds)
# =========================================================================
find_program(_STRIP strip)
if(_STRIP)
    file(GLOB _strip_bins "${S_BIN}/us_*")
    foreach(_b ${_strip_bins})
        if(NOT IS_DIRECTORY "${_b}")
            execute_process(COMMAND "${_STRIP}" "${_b}" ERROR_QUIET)
        endif()
    endforeach()

    file(GLOB _strip_libs "${S_LIB}/*.so" "${S_LIB}/*.so.*")
    foreach(_l ${_strip_libs})
        if(NOT IS_SYMLINK "${_l}")
            execute_process(COMMAND "${_STRIP}" "--strip-unneeded" "${_l}" ERROR_QUIET)
        endif()
    endforeach()
    message(STATUS "[LinuxHpcDeploy] Stripped binaries and libraries")
endif()

# =========================================================================
# 5) Generate setup_hpc_env.sh
#    Sets LD_LIBRARY_PATH to the bundled lib/ dir.
#    No QT_PLUGIN_PATH needed — us_mpi_analysis is headless.
# =========================================================================
file(WRITE "${HPC_STAGE_DIR}/setup_hpc_env.sh"
"#!/bin/bash
# UltraScan3 HPC environment setup - ${US3_VERSION_STRING}
# Source this file before running UltraScan3 HPC programs:
#   source /path/to/UltraScan3-HPC/setup_hpc_env.sh

_US3_HPC_DIR=\"\$( cd \"\$( dirname \"\${BASH_SOURCE[0]}\" )\" >/dev/null 2>&1 && pwd )\"

export LD_LIBRARY_PATH=\"\${_US3_HPC_DIR}/lib\${LD_LIBRARY_PATH:+:\${LD_LIBRARY_PATH}}\"
export US3_HPC_BIN=\"\${_US3_HPC_DIR}/bin\"

echo \"UltraScan3 HPC ${US3_VERSION_STRING} environment loaded from \${_US3_HPC_DIR}\"
echo \"  us_mpi_analysis: \${US3_HPC_BIN}/us_mpi_analysis\"
")
execute_process(COMMAND chmod 755 "${HPC_STAGE_DIR}/setup_hpc_env.sh")
message(STATUS "[LinuxHpcDeploy] Generated setup_hpc_env.sh")

# =========================================================================
# 6) Generate README-HPC.txt
# =========================================================================
file(WRITE "${HPC_STAGE_DIR}/README-HPC.txt"
"UltraScan3 ${US3_VERSION_STRING} - Linux HPC Build
================================================================================

Minimal headless runtime for HPC/supercomputer use.
Centered on us_mpi_analysis + libus_utils (legacy supercomputer build model).

CONTENTS
  bin/us_mpi_analysis  UltraScan3 MPI analysis engine
  lib/                 Bundled runtime libraries (libus_utils + Qt/OpenSSL)
  setup_hpc_env.sh     Environment setup helper
  README-HPC.txt       This file
  license.txt          LGPL license

SETUP
  source /path/to/UltraScan3-HPC/setup_hpc_env.sh

  Or manually:
    export LD_LIBRARY_PATH=/path/to/UltraScan3-HPC/lib:\$LD_LIBRARY_PATH

USAGE
  mpirun -np <N> /path/to/UltraScan3-HPC/bin/us_mpi_analysis <options>

WEBSITE
  https://ultrascan.aucsolutions.com

LICENSE
  UltraScan III is distributed under the GNU Lesser General Public License
  (LGPL). See license.txt for the full license text.
")
message(STATUS "[LinuxHpcDeploy] Generated README-HPC.txt")

# =========================================================================
# 7) Copy license.txt
# =========================================================================
if(LICENSE_FILE AND EXISTS "${LICENSE_FILE}")
    file(COPY "${LICENSE_FILE}" DESTINATION "${HPC_STAGE_DIR}")
    get_filename_component(_lic_name "${LICENSE_FILE}" NAME)
    if(NOT "${_lic_name}" STREQUAL "license.txt")
        file(RENAME "${HPC_STAGE_DIR}/${_lic_name}" "${HPC_STAGE_DIR}/license.txt")
    endif()
    message(STATUS "[LinuxHpcDeploy] Copied license.txt")
endif()

# =========================================================================
# Summary
# =========================================================================
message(STATUS "")
message(STATUS "[LinuxHpcDeploy] HPC deployment complete: ${HPC_STAGE_DIR}")
message(STATUS "  bin/              - us_mpi_analysis (+ any other HPC executables)")
message(STATUS "  lib/              - libus_utils + Qt/OpenSSL runtime .so files")
message(STATUS "  setup_hpc_env.sh  - source to set LD_LIBRARY_PATH")
message(STATUS "  README-HPC.txt    - quick-start instructions")
