# =============================================================================
# MacHpcPkg.cmake
# CMake module: package_macos_hpc_tarball custom target
#
# Builds a portable tar.gz archive of the macOS HPC build.
# Output: <build-dir>/UltraScan3-HPC-<version>-macOS-<arch>.tar.gz
#
# Caller requirements:
#   - US3_VERSION_STRING must be set before including this file
#   - deploy_macos_hpc target must exist (stages files into _hpc_stage/UltraScan3-HPC)
#
# Include from root CMakeLists.txt inside the macOS HPC deployment block.
#
# Mirrors: admin/cmake/packaging/linux/LinuxHpcPkg.cmake (macOS variant)
# =============================================================================

# ---------------------------------------------------------------------------
# Guard: macOS only
# ---------------------------------------------------------------------------
if(NOT APPLE)
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
    if(_uname_m MATCHES "arm64|aarch64")
        set(US3_ARCH_LABEL "arm64")
    else()
        set(US3_ARCH_LABEL "x64")
    endif()
endif()

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
set(_MAC_HPC_STAGE_DIR "${CMAKE_BINARY_DIR}/_hpc_stage/UltraScan3-HPC")
set(_MAC_HPC_PKG_NAME  "UltraScan3-HPC-${US3_VERSION_STRING}-macOS-${US3_ARCH_LABEL}")
set(_MAC_HPC_FINAL_PKG "${CMAKE_BINARY_DIR}/${_MAC_HPC_PKG_NAME}.tar.gz")

message(STATUS "[MacHpcPkg] HPC tarball target output: ${_MAC_HPC_FINAL_PKG}")

# ---------------------------------------------------------------------------
# Custom target: package_macos_hpc_tarball
# ---------------------------------------------------------------------------
add_custom_target(package_macos_hpc_tarball
    COMMENT "Building macOS HPC tarball: ${_MAC_HPC_PKG_NAME}.tar.gz"

    COMMAND ${CMAKE_COMMAND} -E remove -f "${_MAC_HPC_FINAL_PKG}"

    COMMAND ${CMAKE_COMMAND} -E tar czf "${_MAC_HPC_FINAL_PKG}"
            --format=gnutar
            -- "UltraScan3-HPC"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/_hpc_stage"

    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND ${CMAKE_COMMAND} -E echo "macOS HPC tarball ready:"
    COMMAND ${CMAKE_COMMAND} -E echo "  ${_MAC_HPC_FINAL_PKG}"
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="

    DEPENDS deploy_macos_hpc
    VERBATIM
)
