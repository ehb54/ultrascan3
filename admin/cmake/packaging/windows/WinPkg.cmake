# =============================================================================
# WinPkg.cmake
# CMake module: package_windows_nsis custom target
#
# Builds a Windows NSIS installer for UltraScan3 using CPack.
# Output: <build-dir>/UltraScan3-<version>-Windows.exe
#
# Caller requirements:
#   - US3_VERSION_STRING must be set before including this file
#   - deploy_windows target must exist (stages files into _stage/UltraScan3)
#   - CMake install() rules must install the staged tree under bin/, etc/,
#     plugins/, somo/
#
# Include from root CMakeLists.txt after the deploy_windows block:
#   if(WIN32)
#       include(WinPkg)
#   endif()
#
# Mirrors: admin/cmake/packaging/macos/MacOsPkg.cmake
# =============================================================================

# ---------------------------------------------------------------------------
# Guard: Windows only
# ---------------------------------------------------------------------------
if(NOT WIN32)
    return()
endif()

# ---------------------------------------------------------------------------
# Locate NSIS
# ---------------------------------------------------------------------------
find_program(NSIS_EXECUTABLE
    NAMES makensis makensis.exe
    PATHS
        "C:/Program Files (x86)/NSIS"
        "C:/Program Files/NSIS"
        "$ENV{ProgramFiles}/NSIS"
        "$ENV{ProgramFiles\(x86\)}/NSIS"
    DOC "NSIS installer compiler"
)

if(NOT NSIS_EXECUTABLE)
    message(WARNING
        "[WinPkg] NSIS (makensis) not found — package_windows_nsis target will not be available. "
        "Install NSIS from https://nsis.sourceforge.io/ and ensure it is on PATH.")
    return()
endif()

message(STATUS "[WinPkg] Found NSIS: ${NSIS_EXECUTABLE}")

# ---------------------------------------------------------------------------
# Version (mirrors MacOsPkg.cmake convention)
# ---------------------------------------------------------------------------
if(NOT DEFINED US3_VERSION_STRING OR US3_VERSION_STRING STREQUAL "")
    set(US3_VERSION_STRING "${PROJECT_VERSION}")
endif()

# ---------------------------------------------------------------------------
# Paths — all under the build tree so `cmake --build --target clean` works
# ---------------------------------------------------------------------------
set(_PKG_SRC_DIR       "${CMAKE_SOURCE_DIR}/pkg/windows")
set(_PKG_WORK_DIR      "${CMAKE_BINARY_DIR}/pkg/windows")
set(_STAGE_DIR         "${CMAKE_BINARY_DIR}/_stage/UltraScan3")
set(_FINAL_PKG         "${CMAKE_BINARY_DIR}/UltraScan3-${US3_VERSION_STRING}-Windows.exe")

message(STATUS "[WinPkg] NSIS installer target output: ${_FINAL_PKG}")

# ---------------------------------------------------------------------------
# CPack configuration for NSIS
#
# All CPACK_* variables must be set BEFORE include(CPack).  Because this
# module is included() from the root CMakeLists.txt (not a CPack script),
# setting them here affects the CPackConfig.cmake that CMake generates at
# configure time.
# ---------------------------------------------------------------------------

# Generator
set(CPACK_GENERATOR "NSIS" CACHE STRING "CPack generator(s)" FORCE)

# Package identity
set(CPACK_PACKAGE_NAME          "UltraScan3"                                CACHE STRING "" FORCE)
set(CPACK_PACKAGE_VENDOR        "AUC Solutions LLC"                         CACHE STRING "" FORCE)
set(CPACK_PACKAGE_VERSION       "${US3_VERSION_STRING}"                     CACHE STRING "" FORCE)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "UltraScan3 — Analytical Ultracentrifugation Data Analysis"             CACHE STRING "" FORCE)

# Installer target directory — mirrors macOS /Applications/UltraScan3
set(CPACK_PACKAGE_INSTALL_DIRECTORY "UltraScan3"                            CACHE STRING "" FORCE)

# Default install prefix on the target machine: C:\Program Files\UltraScan3
# NSIS expands $PROGRAMFILES at install time (respects 32/64-bit WoW64 redirection).
set(CPACK_NSIS_INSTALL_ROOT      "$PROGRAMFILES64"                          CACHE STRING "" FORCE)

# The tree that CPack packages is the pre-staged directory produced by
# deploy_windows (which runs windeployqt and copies all DLLs/plugins).
# CPACK_INSTALLED_DIRECTORIES points CPack at that tree directly.
set(CPACK_INSTALLED_DIRECTORIES "${_STAGE_DIR};."                           CACHE STRING "" FORCE)

# Disable CPack's internal cmake --install step entirely.
# Without this, CPack runs its own install pass using only targets that have
# explicit install() rules — which misses most programs (they have none).
# Our staged tree from deploy_windows is fully self-contained.
set(CPACK_INSTALL_CMAKE_PROJECTS ""                                         CACHE STRING "" FORCE)

# Output filename (no path — CPack writes to CMAKE_BINARY_DIR by default)
set(CPACK_PACKAGE_FILE_NAME "UltraScan3-${US3_VERSION_STRING}-Windows"     CACHE STRING "" FORCE)

# ---------------------------------------------------------------------------
# NSIS-specific options
# ---------------------------------------------------------------------------

# Display name in Add/Remove Programs
set(CPACK_NSIS_DISPLAY_NAME     "UltraScan3 ${US3_VERSION_STRING}"          CACHE STRING "" FORCE)

# Publisher shown in Add/Remove Programs
set(CPACK_NSIS_PACKAGE_NAME     "UltraScan3"                                CACHE STRING "" FORCE)

# URL for the "More information" link in Add/Remove Programs
set(CPACK_NSIS_URL_INFO_ABOUT   "https://www.ultrascan3.uthscsa.edu/"       CACHE STRING "" FORCE)

# Enable the "Add to PATH" option page (useful for command-line users)
# The bin/ directory is added so us.exe is accessible from a terminal.
set(CPACK_NSIS_MODIFY_PATH      ON                                          CACHE BOOL   "" FORCE)

# Create a Start Menu shortcut to the main launcher
set(CPACK_NSIS_MENU_LINKS
    "bin/us.exe" "UltraScan3"
    CACHE STRING "" FORCE
)

# Create a Desktop shortcut (matches the macOS UltraScan3.app at root of DMG)
set(CPACK_NSIS_CREATE_ICONS_EXTRA
    "CreateShortCut '$DESKTOP\\\\UltraScan3.lnk' '$INSTDIR\\\\bin\\\\us.exe'"
    CACHE STRING "" FORCE
)
set(CPACK_NSIS_DELETE_ICONS_EXTRA
    "Delete '$DESKTOP\\\\UltraScan3.lnk'"
    CACHE STRING "" FORCE
)

# Optional: path to a 150×57 px bitmap for the installer header (MUI_HEADERIMAGE)
# Uncomment and point to your asset once it exists:
# set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/pkg/windows/installer-header.bmp")

# Optional: application icon (.ico) for Add/Remove Programs
# set(CPACK_NSIS_MUI_ICON     "${CMAKE_SOURCE_DIR}/etc/us3-icon.ico")
# set(CPACK_NSIS_MUI_UNIICON  "${CMAKE_SOURCE_DIR}/etc/us3-icon.ico")

# License file shown on the License page of the installer
if(EXISTS "${CMAKE_SOURCE_DIR}/LICENSE.txt")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt" CACHE STRING "" FORCE)
endif()

# Welcome text (plain text, mirrors MacOsPkg's Welcome.txt.in mechanism)
set(_WELCOME_IN  "${_PKG_SRC_DIR}/resources/Welcome.txt.in")
set(_WELCOME_OUT "${_PKG_WORK_DIR}/Welcome.txt")
if(EXISTS "${_WELCOME_IN}")
    file(MAKE_DIRECTORY "${_PKG_WORK_DIR}/resources")
    configure_file("${_WELCOME_IN}" "${_WELCOME_OUT}" @ONLY)
    set(CPACK_RESOURCE_FILE_WELCOME "${_WELCOME_OUT}" CACHE STRING "" FORCE)
elseif(EXISTS "${_PKG_SRC_DIR}/resources/Welcome.txt")
    set(CPACK_RESOURCE_FILE_WELCOME "${_PKG_SRC_DIR}/resources/Welcome.txt" CACHE STRING "" FORCE)
endif()

# ---------------------------------------------------------------------------
# NOTE: include(CPack) is intentionally NOT called here.
# The root CMakeLists.txt calls include(CPack) once, after this module is
# included — that single call picks up all the CPACK_* variables set above.
# Calling include(CPack) a second time would reset variables and produce a
# broken CPackConfig.cmake.
# ---------------------------------------------------------------------------

message(STATUS "[WinPkg] CPack NSIS configuration applied.")
message(STATUS "  Install directory:  %PROGRAMFILES64%\\UltraScan3")
message(STATUS "  Installer output:   ${_FINAL_PKG}")
message(STATUS "  Stage source:       ${_STAGE_DIR}")

# ---------------------------------------------------------------------------
# Custom target: package_windows_nsis
#
# Workflow (mirrors package_macos_pkg):
#   Step 1 — Ensure the staged tree exists (depend on deploy_windows)
#   Step 2 — Run cpack to invoke NSIS and produce the .exe installer
#
# Usage:
#   cmake --build . --target package_windows_nsis
# ---------------------------------------------------------------------------
add_custom_target(package_windows_nsis
    COMMENT "Building Windows NSIS installer: UltraScan3-${US3_VERSION_STRING}-Windows.exe"

    # ------------------------------------------------------------------
    # 1. Ensure pkg/windows working directory exists
    # ------------------------------------------------------------------
    COMMAND ${CMAKE_COMMAND} -E make_directory "${_PKG_WORK_DIR}"

    # ------------------------------------------------------------------
    # 2. Invoke CPack with the NSIS generator.
    #    -G NSIS overrides any multi-generator CPACK_GENERATOR setting.
    #    -C Release selects the Release install component set.
    #    -B points CPack at the build directory for output.
    # ------------------------------------------------------------------
    COMMAND ${CMAKE_COMMAND} -E remove -f "${_FINAL_PKG}"
    COMMAND ${CMAKE_CPACK_COMMAND}
            -G NSIS
            -C Release
            --config "${CMAKE_BINARY_DIR}/CPackConfig.cmake"
            -B "${CMAKE_BINARY_DIR}"

    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="
    COMMAND ${CMAKE_COMMAND} -E echo "NSIS installer ready:"
    COMMAND ${CMAKE_COMMAND} -E echo "  ${_FINAL_PKG}"
    COMMAND ${CMAKE_COMMAND} -E echo "=========================================="

    # deploy_windows must run before packaging to populate the stage dir.
    DEPENDS deploy_windows
    VERBATIM
)
