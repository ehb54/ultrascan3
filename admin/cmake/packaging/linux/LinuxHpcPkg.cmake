# =============================================================================
# LinuxHpcPkg.cmake
# CMake module: package_linux_hpc_tarball custom target
#
# Builds a portable tar.gz archive for UltraScan3 HPC on Linux.
# Output: <build-dir>/UltraScan3-HPC-<version>-Linux-<arch>.tar.gz
#
# Caller requirements:
#   - US3_VERSION_STRING must be set before including this file
#   - deploy_linux_hpc target must exist (stages files into _hpc_stage/UltraScan3-HPC)
#
# Include from root CMakeLists.txt inside the Linux HPC deployment block.
#
# Mirrors: admin/cmake/packaging/linux/LinuxPkg.cmake (HPC variant)
# =============================================================================

# ---------------------------------------------------------------------------
# Guard: Linux only
# ---------------------------------------------------------------------------
if(NOT UNIX OR APPLE)
    return()
endif()

# ---------------------------------------------------------------------------
# Version
# ---------------------------------------------------------------------------
if(NOT DEFINED US3_VERSION_STRING OR US3_VERSION_STRING STREQUAL "")
    set(US3_VERSION_STRING "${PROJECT_VERSION}")
endif()

# ---------------------------------------------------------------------------
# Architecture label
# ---------------------------------------------------------------------------
if(NOT DEFINED US3_ARCH_LABEL)
    execute_process(
        COMMAND uname -m
        OUTPUT_VARIABLE _uname_m
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(_uname_m MATCHES "aarch64|arm64")
        set(US3_ARCH_LABEL "arm64")
    else()
        set(US3_ARCH_LABEL "x64")
    endif()
endif()

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
set(_HPC_STAGE_DIR "${CMAKE_BINARY_DIR}/_hpc_stage/UltraScan3-HPC")
set(_HPC_PKG_NAME  "UltraScan3-HPC-${US3_VERSION_STRING}-Linux-${US3_ARCH_LABEL}")
set(_HPC_FINAL_PKG "${CMAKE_BINARY_DIR}/${_HPC_PKG_NAME}.tar.gz")

message(STATUS "[LinuxHpcPkg] HPC tarball target output: ${_HPC_FINAL_PKG}")

# ---------------------------------------------------------------------------
# Custom target: package_linux_hpc_tarball
# ---------------------------------------------------------------------------
add_custom_target(package_linux_hpc_tarball
    COMMENT "Building Linux HPC tarball: ${_HPC_PKG_NAME}.tar.gz"

    COMMAND ${CMAKE_COMMAND} -E remove -f "${_HPC_FINAL_PKG}"

    COMMAND ${CMAKE_COMMAND} -E tar czf "${_HPC_FINAL_PKG}"
            --format=gnutar
            -- "UltraScan3-HPC"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/_hpc_stage"

    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND ${CMAKE_COMMAND} -E echo "Linux HPC tarball ready:"
    COMMAND ${CMAKE_COMMAND} -E echo "  ${_HPC_FINAL_PKG}"
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="

    DEPENDS deploy_linux_hpc
    VERBATIM
)
