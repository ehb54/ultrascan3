# =============================================================================
# MacOsPkg.cmake
# CMake module: package_macos_pkg custom target
#
# Builds a macOS PKG installer for UltraScan3 using pkgbuild + productbuild.
# Output: <build-dir>/UltraScan3-<version>-macOS.pkg
#
# Caller requirements:
#   - US3_VERSION_STRING must be set before including this file
#   - deploy_macos target must exist (stages files into _stage/UltraScan3)
#   - SOURCE_DIR vars must be set (handled by root CMakeLists.txt)
#
# Include from root CMakeLists.txt after the deploy_macos block:
#   if(APPLE)
#       include(MacOsPkg)
#   endif()
# =============================================================================

# ---------------------------------------------------------------------------
# Guard: macOS only
# ---------------------------------------------------------------------------
if(NOT APPLE)
    return()
endif()

# ---------------------------------------------------------------------------
# Locate required tools
# ---------------------------------------------------------------------------
find_program(PKGBUILD_EXECUTABLE    pkgbuild    REQUIRED)
find_program(PRODUCTBUILD_EXECUTABLE productbuild REQUIRED)

if(NOT PKGBUILD_EXECUTABLE)
    message(WARNING "pkgbuild not found — package_macos_pkg target will not be available")
    return()
endif()
if(NOT PRODUCTBUILD_EXECUTABLE)
    message(WARNING "productbuild not found — package_macos_pkg target will not be available")
    return()
endif()

message(STATUS "Found pkgbuild:     ${PKGBUILD_EXECUTABLE}")
message(STATUS "Found productbuild: ${PRODUCTBUILD_EXECUTABLE}")

# ---------------------------------------------------------------------------
# Paths (all under the build tree so `cmake --build --target clean` works)
# ---------------------------------------------------------------------------
set(_PKG_SRC_DIR         "${CMAKE_SOURCE_DIR}/pkg/macos")
set(_PKG_WORK_DIR        "${CMAKE_BINARY_DIR}/pkg/macos")
set(_PKG_ROOT            "${_PKG_WORK_DIR}/pkgroot")
set(_PKG_SCRIPTS_DIR     "${_PKG_WORK_DIR}/scripts")
set(_PKG_RESOURCES_DIR   "${_PKG_WORK_DIR}/resources")
set(_COMPONENT_PKG       "${_PKG_WORK_DIR}/ultrascan3-component.pkg")
set(_DIST_XML_IN         "${_PKG_SRC_DIR}/distribution.xml.in")
set(_DIST_XML            "${_PKG_WORK_DIR}/distribution.xml")

# Version comes from US3_VERSION_STRING (set in root CMakeLists.txt from us_defines.h)
# Fall back to PROJECT_VERSION if somehow not set.
if(NOT DEFINED US3_VERSION_STRING OR US3_VERSION_STRING STREQUAL "")
    set(US3_VERSION_STRING "${PROJECT_VERSION}")
endif()

set(_FINAL_PKG "${CMAKE_BINARY_DIR}/UltraScan3-${US3_VERSION_STRING}-macOS.pkg")

message(STATUS "PKG installer target output: ${_FINAL_PKG}")

# ---------------------------------------------------------------------------
# Welcome screen text: allow version stamping via Welcome.txt.in
# ---------------------------------------------------------------------------
set(_WELCOME_IN  "${_PKG_SRC_DIR}/resources/Welcome.txt.in")
set(_WELCOME_OUT "${_PKG_WORK_DIR}/Welcome.txt")

if(EXISTS "${_WELCOME_IN}")
    configure_file("${_WELCOME_IN}" "${_WELCOME_OUT}" @ONLY)
else()
    set(_WELCOME_OUT "${_PKG_SRC_DIR}/resources/Welcome.txt")
endif()

# ---------------------------------------------------------------------------
# Generate distribution.xml at configure time from the .in template.
# If the template doesn't exist yet (bootstrap), create a minimal one here
# so the build target always has something to work with.
# ---------------------------------------------------------------------------
if(EXISTS "${_DIST_XML_IN}")
    configure_file("${_DIST_XML_IN}" "${_DIST_XML}" @ONLY)
else()
    # Inline fallback — written to the build tree only.
    # The canonical source is pkg/macos/distribution.xml.in (tracked in git).
    file(MAKE_DIRECTORY "${_PKG_WORK_DIR}")

    set(_DIST_FALLBACK
            "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
            "<installer-gui-script minSpecVersion=\"1\">\n"
            "  <title>UltraScan3</title>\n"
            "  <options customize=\"never\" require-scripts=\"true\" rootVolumeOnly=\"true\" allow-external-scripts=\"no\"/>\n"
            "  <welcome file=\"Welcome.txt\" mime-type=\"text/plain\"/>\n"
            "  <license file=\"License.txt\" mime-type=\"text/plain\"/>\n"
            "  <choices-outline><line choice=\"default\"/></choices-outline>\n"
            "  <choice id=\"default\" visible=\"false\"><pkg-ref id=\"com.aucsolutions.ultrascan3\"/></choice>\n"
            "  <pkg-ref id=\"com.aucsolutions.ultrascan3\" version=\"${US3_VERSION_STRING}\" auth=\"Root\">ultrascan3-component.pkg</pkg-ref>\n"
            "</installer-gui-script>\n"
    )

    file(WRITE "${_DIST_XML}" "${_DIST_FALLBACK}")
    message(STATUS "Generated distribution.xml from inline fallback (add pkg/macos/distribution.xml.in for git tracking)")
endif()

# ---------------------------------------------------------------------------
# Custom target: package_macos_pkg
#
# Step 1 — Prepare build-tree working directories
# Step 2 — Stage pkgroot:
#              pkgroot/Applications/UltraScan3/   ← from deploy_macos staging dir
#              pkgroot/Library/LaunchDaemons/      ← from pkg/macos source
# Step 3 — Prepare scripts/ dir with preinstall + postinstall
# Step 4 — Prepare resources/ dir with Welcome.txt + License.txt
# Step 5 — pkgbuild → component package
# Step 6 — productbuild → final installer PKG
# ---------------------------------------------------------------------------
add_custom_target(package_macos_pkg
    COMMENT "Building macOS PKG installer: UltraScan3-${US3_VERSION_STRING}-macOS.pkg"

    # ------------------------------------------------------------------
    # 1. Create working directories
    # ------------------------------------------------------------------
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_PKG_ROOT}/Applications"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_PKG_ROOT}/Library/LaunchDaemons"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_PKG_SCRIPTS_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_PKG_RESOURCES_DIR}"

    # ------------------------------------------------------------------
    # 2a. Populate pkgroot/Applications/UltraScan3/ from deploy_macos
    #     staging tree (_stage/UltraScan3/).  We use rsync to merge rather
    #     than a plain copy — this handles incremental rebuilds correctly
    #     and preserves executable permissions on .app bundles.
    # ------------------------------------------------------------------
    COMMAND ${CMAKE_COMMAND} -E remove_directory
            "${_PKG_ROOT}/Applications/UltraScan3"
        COMMAND rsync -a --delete
        --exclude='.DS_Store'
        --exclude='._*'
        --exclude='**/.DS_Store'
        --exclude='**/._*'
        "${CMAKE_BINARY_DIR}/_stage/UltraScan3/"
        "${_PKG_ROOT}/Applications/UltraScan3/"

    # ------------------------------------------------------------------
    # 2b. Stage the LaunchDaemon plist into pkgroot so pkgbuild installs
    #     it at /Library/LaunchDaemons/ultrascan_sysctl.plist
    # ------------------------------------------------------------------
    COMMAND ${CMAKE_COMMAND} -E copy
            "${_PKG_SRC_DIR}/ultrascan_sysctl.plist"
            "${_PKG_ROOT}/Library/LaunchDaemons/ultrascan_sysctl.plist"

    # ------------------------------------------------------------------
    # 2c. Stage uninstall.sh into the application root so users have a
    #     supported removal path after installation.
    #     Installed at: /Applications/UltraScan3/uninstall.sh
    # ------------------------------------------------------------------
    COMMAND ${CMAKE_COMMAND} -E copy
            "${_PKG_SRC_DIR}/uninstall.sh"
            "${_PKG_ROOT}/Applications/UltraScan3/uninstall.sh"
    COMMAND chmod 0755
            "${_PKG_ROOT}/Applications/UltraScan3/uninstall.sh"

    # ------------------------------------------------------------------
    # 3. Copy installer scripts (pkgbuild requires them executable).
    #    CMake's file(COPY) preserves permissions; E copy does not on all
    #    platforms so we use a cmake -P script-style copy then chmod.
    # ------------------------------------------------------------------
    COMMAND ${CMAKE_COMMAND} -E copy
            "${_PKG_SRC_DIR}/preinstall"
            "${_PKG_SCRIPTS_DIR}/preinstall"
    COMMAND ${CMAKE_COMMAND} -E copy
            "${_PKG_SRC_DIR}/postinstall"
            "${_PKG_SCRIPTS_DIR}/postinstall"
    COMMAND chmod 0755
            "${_PKG_SCRIPTS_DIR}/preinstall"
            "${_PKG_SCRIPTS_DIR}/postinstall"

    # ------------------------------------------------------------------
    # 4. Prepare resources for productbuild UI screens.
    #    Welcome.txt  — from pkg/macos/resources/
    #    License.txt  — copied from repo root LICENSE.txt
    # ------------------------------------------------------------------
    COMMAND ${CMAKE_COMMAND} -E copy
            "${_WELCOME_OUT}"
            "${_PKG_RESOURCES_DIR}/Welcome.txt"
    COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_SOURCE_DIR}/LICENSE.txt"
            "${_PKG_RESOURCES_DIR}/License.txt"
    COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_SOURCE_DIR}/pkg/macos/resources/background.png"
            "${_PKG_RESOURCES_DIR}/background.png"

    # ------------------------------------------------------------------
    # 5. pkgbuild — create the component package.
    #
    #    --root /        The payload root is /, so paths inside pkgroot/
    #                    become absolute install paths.
    #    --identifier    Reverse-DNS bundle ID (matches receipt, uninstall).
    #    --version       Stamped into the receipt; used by Software Update.
    #    --scripts       Directory containing preinstall / postinstall.
    #    --install-location /  Belt-and-suspenders: also locks install root.
    # ------------------------------------------------------------------
    COMMAND ${CMAKE_COMMAND} -E remove -f "${_COMPONENT_PKG}"
    COMMAND ${PKGBUILD_EXECUTABLE}
            --root             "${_PKG_ROOT}"
            --component-plist  "${CMAKE_SOURCE_DIR}/pkg/macos/component.plist"
            --scripts          "${_PKG_SCRIPTS_DIR}"
            --identifier       "com.aucsolutions.ultrascan3"
            --version          "${US3_VERSION_STRING}"
            --install-location "/"
            "${_COMPONENT_PKG}"

    # ------------------------------------------------------------------
    # 6. productbuild — wrap component pkg with installer UI.
    #
    #    --distribution   The XML that defines title, Welcome, License,
    #                     non-relocatable domains, and pkg-ref.
    #    --resources      Directory containing Welcome.txt and License.txt.
    #    --package-path   Directory where productbuild looks for the
    #                     component .pkg referenced in distribution.xml.
    # ------------------------------------------------------------------
    COMMAND ${CMAKE_COMMAND} -E remove -f "${_FINAL_PKG}"
    COMMAND ${PRODUCTBUILD_EXECUTABLE}
            --distribution  "${_DIST_XML}"
            --resources     "${_PKG_RESOURCES_DIR}"
            --package-path  "${_PKG_WORK_DIR}"
            "${_FINAL_PKG}"

    COMMAND ${CMAKE_COMMAND} -E echo
            "=========================================="
    COMMAND ${CMAKE_COMMAND} -E echo
            "PKG installer ready:"
    COMMAND ${CMAKE_COMMAND} -E echo
            "  ${_FINAL_PKG}"
    COMMAND ${CMAKE_COMMAND} -E echo
            "=========================================="

    DEPENDS deploy_macos
    VERBATIM
)
