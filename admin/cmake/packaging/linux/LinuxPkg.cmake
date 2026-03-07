# =============================================================================
# LinuxPkg.cmake
# CMake module: package_linux_tarball custom target
#
# Builds a portable tar.gz archive for UltraScan3 on Linux.
# Output: <build-dir>/UltraScan3-<version>-Linux-<arch>.tar.gz
#
# Caller requirements:
#   - US3_VERSION_STRING must be set before including this file
#   - deploy_linux target must exist (stages files into _stage/UltraScan3)
#
# Include from root CMakeLists.txt inside the Linux deployment block:
#   if(UNIX AND NOT APPLE)
#       include(LinuxPkg)
#   endif()
#
# Mirrors: admin/cmake/packaging/windows/WinPkg.cmake
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
# Architecture label (x64 or arm64) for the filename
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
set(_STAGE_DIR  "${CMAKE_BINARY_DIR}/_stage/UltraScan3")
set(_PKG_NAME   "UltraScan3-${US3_VERSION_STRING}-Linux-${US3_ARCH_LABEL}")
set(_FINAL_PKG  "${CMAKE_BINARY_DIR}/${_PKG_NAME}.tar.gz")

message(STATUS "[LinuxPkg] Tarball target output: ${_FINAL_PKG}")

# ---------------------------------------------------------------------------
# Custom target: package_linux_tarball
#
# Workflow:
#   Step 1 — Ensure staged tree exists (depends on deploy_linux)
#   Step 2 — Create tar.gz from the staged directory
#
# Usage:
#   cmake --build . --target package_linux_tarball
# ---------------------------------------------------------------------------
add_custom_target(package_linux_tarball
    COMMENT "Building Linux tarball: ${_PKG_NAME}.tar.gz"

    # Remove any previous archive
    COMMAND ${CMAKE_COMMAND} -E remove -f "${_FINAL_PKG}"

    # Create the tarball: cd to the _stage dir parent so the archive root
    # is "UltraScan3/" (matching the legacy us3-Linux64-<ver>.tar.gz layout)
    COMMAND ${CMAKE_COMMAND} -E tar czf "${_FINAL_PKG}"
            --format=gnutar
            -- "UltraScan3"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/_stage"

    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND ${CMAKE_COMMAND} -E echo "Linux tarball ready:"
    COMMAND ${CMAKE_COMMAND} -E echo "  ${_FINAL_PKG}"
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="

    DEPENDS deploy_linux
    VERBATIM
)
