# =============================================================================
# LinuxPkg.cmake
# CMake module: package_linux_tarball custom target
#
# Builds a portable tar.xz archive for UltraScan3 on Linux.
# Output: <build-dir>/UltraScan3-<version>-Linux-<arch>.tar.xz
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

include("${CMAKE_CURRENT_LIST_DIR}/LinuxPackageName.cmake")

set(_FINAL_PKG "${CMAKE_BINARY_DIR}/${US3_LINUX_ARCHIVE_NAME}")

message(STATUS "[LinuxPkg] Tarball target output: ${_FINAL_PKG}")

# ---------------------------------------------------------------------------
# Custom target: package_linux_tarball
#
# Workflow:
#   Step 1 — Ensure staged tree exists (depends on deploy_linux)
#   Step 2 — Create tar.xz from the staged directory
#
# Usage:
#   cmake --build . --target package_linux_tarball
# ---------------------------------------------------------------------------
add_custom_target(package_linux_tarball
    COMMENT "Building Linux tarball: ${US3_LINUX_ARCHIVE_NAME}"

    # Remove any previous archive
    COMMAND ${CMAKE_COMMAND} -E remove -f "${_FINAL_PKG}"

    # Create the tarball: cd to the _stage dir parent so the archive root
    # is "UltraScan3/" (matching the legacy us3-Linux64-<ver>.tar.xz layout)
    # Use xz compression to match historical customer-facing .tar.xz naming.
    COMMAND ${CMAKE_COMMAND} -E tar cJf "${_FINAL_PKG}"
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

include("${CMAKE_CURRENT_LIST_DIR}/../PackageOutput.cmake")
us3_record_package_output(package_linux_tarball "${_FINAL_PKG}")
