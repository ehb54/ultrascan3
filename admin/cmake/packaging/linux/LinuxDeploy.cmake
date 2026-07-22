# =============================================================================
# LinuxDeploy.cmake - Linux deployment for UltraScan3 (portable tarball layout)
#
# Stages a self-contained directory tree ready for tar.gz packaging.
# Legacy equivalent: copypkg-lnx.sh
#
# Layout:
#   <STAGE_DIR>/
#     bin/           us, us.sh, us_*, assistant, manual.qhc, manual.qch
#     lib/           UltraScan .so + Qt .so + Qwt .so + OpenSSL .so
#     plugins/       Qt plugins (platforms/, sqldrivers/, imageformats/, ...)
#     etc/           editable config data
#     somo/          SOMO data files
#     README.txt     installation and quick-start instructions
#     license.txt
#     bin64 -> bin   (symlink, legacy compat)
#     lib64 -> lib   (symlink, legacy compat)
#
# Required variables (passed via -D on cmake -P invocation):
#   STAGE_DIR           - Root of the staged directory tree
#   BIN_DIR             - Build-tree bin/ containing us and us_* executables
#   LIB_DIR             - Build-tree lib/ containing libusutils*.so / libusgui*.so
#   ETC_SOURCE_DIR      - Source etc/ directory
#   SOMO_SOURCE_DIR     - Source somo/ directory
#   LICENSE_FILE        - Path to LICENSE.txt
#
# Optional variables:
#   QCH_DIR             - Directory containing manual.qch / manual.qhc
#   VCPKG_LIB_DIR       - vcpkg installed lib/ (Qt/Qwt/OpenSSL .so files)
#   VCPKG_PLUGIN_DIR    - vcpkg installed plugins/ dir (Qt5 / legacy layout)
#   VCPKG_QT6_PLUGIN_DIR - vcpkg Qt6/plugins/ dir (<triplet>/Qt6/plugins);
#                          preferred over VCPKG_PLUGIN_DIR for Qt6 builds
#   ASSISTANT_EXE       - Path to assistant binary
#   SOMO_BIN_DIR        - SoMo bin/ directory
#   SOMO_LIB_DIR        - SoMo lib/ directory
#
# Mirrors: admin/cmake/packaging/windows/WinDeploy.cmake
# =============================================================================

cmake_minimum_required(VERSION 3.16)

# ---------------------------------------------------------------------------
# Guard: Linux only
# ---------------------------------------------------------------------------
if(NOT UNIX OR APPLE)
    message(STATUS "[LinuxDeploy] Skipping -- not a Linux build.")
    return()
endif()

# ---------------------------------------------------------------------------
# Validate required inputs
# ---------------------------------------------------------------------------
if(NOT STAGE_DIR)
    message(FATAL_ERROR "[LinuxDeploy] STAGE_DIR not set")
endif()
if(NOT BIN_DIR OR NOT EXISTS "${BIN_DIR}")
    message(FATAL_ERROR "[LinuxDeploy] BIN_DIR does not exist: ${BIN_DIR}")
endif()

# ---------------------------------------------------------------------------
# Convenience aliases (mirrors WinDeploy.cmake)
# ---------------------------------------------------------------------------
set(S_BIN  "${STAGE_DIR}/bin")
set(S_LIB  "${STAGE_DIR}/lib")
set(S_PLUG "${STAGE_DIR}/plugins")
set(S_ETC  "${STAGE_DIR}/etc")
set(S_SOMO "${STAGE_DIR}/somo")

file(MAKE_DIRECTORY "${S_BIN}")
file(MAKE_DIRECTORY "${S_LIB}")
file(MAKE_DIRECTORY "${S_PLUG}")

# =========================================================================
# 1) Copy us executable into bin/
# =========================================================================
if(EXISTS "${BIN_DIR}/us")
    message(STATUS "[LinuxDeploy] Copying us -> ${S_BIN}/")
    file(COPY "${BIN_DIR}/us" DESTINATION "${S_BIN}"
         FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                          GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
else()
    message(FATAL_ERROR "[LinuxDeploy] 'us' not found in ${BIN_DIR}")
endif()

# =========================================================================
# 1b) Generate us.sh launcher script
#
#     Mirrors the legacy us.sh: sets LD_LIBRARY_PATH so the bundled Qt/Qwt
#     .so files in ../lib/ are found regardless of the user's environment,
#     then exec's the us binary.
#
#     us.sh is the documented entry point for UltraScan III on Linux.
#     Users run:  /install/path/bin/us.sh
#     The .desktop file (section 2b) also points to us.sh.
# =========================================================================
file(WRITE "${S_BIN}/us.sh"
"#!/bin/bash
DIR=\"\$( cd \"\$( dirname \"\${BASH_SOURCE[0]}\" )\" >/dev/null 2>&1 && pwd )\"
export LD_LIBRARY_PATH=\"\$DIR/../lib\${LD_LIBRARY_PATH:+:\$LD_LIBRARY_PATH}\"
export QT_PLUGIN_PATH=\"\$DIR/../plugins\${QT_PLUGIN_PATH:+:\$QT_PLUGIN_PATH}\"
exec \"\$DIR/us\" \"\$@\"
")
execute_process(COMMAND chmod 755 "${S_BIN}/us.sh")
message(STATUS "[LinuxDeploy] Generated us.sh launcher")

# =========================================================================
# 1c) Generate README.txt at the tarball root
# =========================================================================
if(NOT DEFINED US3_VERSION_STRING OR US3_VERSION_STRING STREQUAL "")
    set(US3_VERSION_STRING "(unknown version)")
endif()
file(WRITE "${STAGE_DIR}/README.txt"
"UltraScan III ${US3_VERSION_STRING} - Linux
================================================================================

WEBSITE
  https://ultrascan.aucsolutions.com

INSTALLATION
  Extract this archive to a location of your choice, for example:

    cd /usr/local
    tar Jxf UltraScan3-${US3_VERSION_STRING}-Linux-<arch>.tar.xz

  No further installation steps are required.  The package is self-contained.

STARTING ULTRASCAN III
  From a terminal, run the launcher script in the bin/ subdirectory:

    /usr/local/UltraScan3/bin/us.sh

  You may add the bin/ directory to your PATH for convenience:

    export PATH=/usr/local/UltraScan3/bin:\$PATH
    us.sh

FILE LAYOUT
  bin/        Executables and launcher scripts
  lib/        Bundled shared libraries (Qt, Qwt, OpenSSL, UltraScan)
  plugins/    Qt plugins (platform, image format, SQL drivers)
  etc/        Configuration data, color maps, rotor definitions
  somo/       US-SOMO supporting data files
  license.txt LGPL license

SUPPORT
  https://ultrascan.aucsolutions.com/contacts.php

LICENSE
  UltraScan III is distributed under the GNU Lesser General Public License
  (LGPL).  See license.txt for the full license text.
  NIH funding: GM120600
")
message(STATUS "[LinuxDeploy] Generated README.txt")

# =========================================================================
# 2) Copy companion us_* executables into bin/
# =========================================================================
file(GLOB _extra_bins "${BIN_DIR}/us_*")
foreach(_bin ${_extra_bins})
    if(NOT IS_DIRECTORY "${_bin}")
        get_filename_component(_n "${_bin}" NAME)
        message(STATUS "  Copying program: ${_n}")
        file(COPY "${_bin}" DESTINATION "${S_BIN}"
             FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                              GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    endif()
endforeach()

# =========================================================================
# 3) Copy UltraScan shared libraries into lib/
# =========================================================================
if(LIB_DIR AND EXISTS "${LIB_DIR}")
    file(GLOB _us_libs
        "${LIB_DIR}/libusutils*.so*"
        "${LIB_DIR}/libusgui*.so*"
        "${LIB_DIR}/libusUtils*.so*"
        "${LIB_DIR}/libusGui*.so*"
        "${LIB_DIR}/libus_*.so*"
    )
    foreach(_lib ${_us_libs})
        get_filename_component(_n "${_lib}" NAME)
        message(STATUS "  Copying US3 lib: ${_n}")
        file(COPY "${_lib}" DESTINATION "${S_LIB}")
    endforeach()
else()
    message(STATUS "[LinuxDeploy] LIB_DIR not provided -- UltraScan .so files not staged")
endif()

# =========================================================================
# 4) Copy Qt / Qwt .so files from vcpkg into lib/
#    Legacy equivalent: copypkg-lnx.sh copied QTLIBS5 from ${QTDIR}/lib/
# =========================================================================
if(VCPKG_LIB_DIR AND EXISTS "${VCPKG_LIB_DIR}")
    file(GLOB _qt_so
        "${VCPKG_LIB_DIR}/libQt5*.so*"
        "${VCPKG_LIB_DIR}/libQt6*.so*"
        "${VCPKG_LIB_DIR}/libqwt*.so*"
        "${VCPKG_LIB_DIR}/libQwt*.so*"
    )
    set(_qt_count 0)
    foreach(_so ${_qt_so})
        get_filename_component(_n "${_so}" NAME)
        if(NOT EXISTS "${S_LIB}/${_n}")
            file(COPY "${_so}" DESTINATION "${S_LIB}")
            math(EXPR _qt_count "${_qt_count} + 1")
        endif()
    endforeach()
    message(STATUS "[LinuxDeploy] Staged ${_qt_count} Qt/Qwt .so files -> lib/")

    # OpenSSL runtime -- legacy copypkg-lnx.sh included libssl + libcrypto
    file(GLOB _ssl_so
        "${VCPKG_LIB_DIR}/libssl*.so*"
        "${VCPKG_LIB_DIR}/libcrypto*.so*"
    )
    foreach(_so ${_ssl_so})
        get_filename_component(_n "${_so}" NAME)
        if(NOT EXISTS "${S_LIB}/${_n}")
            message(STATUS "  Copying SSL lib: ${_n}")
            file(COPY "${_so}" DESTINATION "${S_LIB}")
        endif()
    endforeach()

    # Qt transitive runtime dependencies built by vcpkg.
    # These live alongside the Qt .so files in the same vcpkg lib/ dir and are
    # NOT present on the target system (they are vcpkg-private builds).
    # Examples: libdouble-conversion, libicuXX, libpcre2-*, libb2, libzstd,
    #           libmd4c, libharfbuzz, libfreetype, libpng, libbrotli*, etc.
    # We copy everything from vcpkg lib/ that is a shared library, skipping
    # files already staged (Qt/Qwt/OpenSSL covered above).
    file(GLOB _vcpkg_all_so
        "${VCPKG_LIB_DIR}/*.so"
        "${VCPKG_LIB_DIR}/*.so.*"
    )
    set(_vcpkg_deps_count 0)
    foreach(_so ${_vcpkg_all_so})
        get_filename_component(_n "${_so}" NAME)
        if(NOT EXISTS "${S_LIB}/${_n}")
            file(COPY "${_so}" DESTINATION "${S_LIB}")
            math(EXPR _vcpkg_deps_count "${_vcpkg_deps_count} + 1")
        endif()
    endforeach()
    message(STATUS "[LinuxDeploy] Staged ${_vcpkg_deps_count} additional vcpkg runtime libs -> lib/")

    # SQLite runtime -- required by the Qt SQL driver (libqsqlite.so) which is
    # loaded by Qt Assistant at runtime to open the .qhc collection file.
    # Qt6 Assistant also uses SQLite FTS5 for full-text search indexing.
    # The glob in vcpkg lib/ matches libsqlite3.so / libsqlite3.so.0 etc.
    file(GLOB _sqlite_so
        "${VCPKG_LIB_DIR}/libsqlite3*.so*"
    )
    foreach(_so ${_sqlite_so})
        get_filename_component(_n "${_so}" NAME)
        if(NOT EXISTS "${S_LIB}/${_n}")
            message(STATUS "  Copying SQLite lib: ${_n}")
            file(COPY "${_so}" DESTINATION "${S_LIB}")
        endif()
    endforeach()
else()
    message(STATUS "[LinuxDeploy] VCPKG_LIB_DIR not set -- Qt .so files will not be bundled")
    message(STATUS "  The app will rely on system Qt libraries (acceptable for distro packages,")
    message(STATUS "  but portable tarballs require Qt .so files to be bundled).")
endif()

# =========================================================================
# 5) Copy Qt plugins from vcpkg into plugins/
#    Legacy equivalent: copypkg-lnx.sh: ${RSYNC} ${QTDIR}/plugins ${PKGDIR}/lib/
#
#    Qt6 vcpkg installs plugins under <triplet>/Qt6/plugins/ rather than
#    <triplet>/plugins/.  Prefer VCPKG_QT6_PLUGIN_DIR when set (Qt6 builds),
#    fall back to the legacy VCPKG_PLUGIN_DIR path (Qt5 / older vcpkg).
# =========================================================================
set(_effective_plugin_dir "")
if(VCPKG_QT6_PLUGIN_DIR AND EXISTS "${VCPKG_QT6_PLUGIN_DIR}")
    set(_effective_plugin_dir "${VCPKG_QT6_PLUGIN_DIR}")
    message(STATUS "[LinuxDeploy] Using Qt6 plugin dir: ${VCPKG_QT6_PLUGIN_DIR}")
elseif(VCPKG_PLUGIN_DIR AND EXISTS "${VCPKG_PLUGIN_DIR}")
    set(_effective_plugin_dir "${VCPKG_PLUGIN_DIR}")
    message(STATUS "[LinuxDeploy] Using legacy plugin dir: ${VCPKG_PLUGIN_DIR}")
endif()

if(_effective_plugin_dir)
    message(STATUS "[LinuxDeploy] Copying Qt plugins -> plugins/")
    file(COPY "${_effective_plugin_dir}/" DESTINATION "${S_PLUG}")
else()
    message(STATUS "[LinuxDeploy] No Qt plugin dir found -- platform plugins not bundled")
    message(STATUS "  Set VCPKG_QT6_PLUGIN_DIR (Qt6) or VCPKG_PLUGIN_DIR (Qt5).")
    message(STATUS "  The app requires platform plugins (libqxcb.so / libqwayland*.so) to run.")
endif()

# =========================================================================
# 5b) Guarantee sqldrivers/libqsqlite.so is present
#     Qt Assistant search requires SQLite; the bulk plugin copy in section 5
#     should have handled this, but we check explicitly as a safety-net.
#     Qt6 vcpkg puts sqldrivers under <triplet>/Qt6/plugins/sqldrivers so we
#     search VCPKG_QT6_PLUGIN_DIR first, then fall back to VCPKG_PLUGIN_DIR.
# =========================================================================
file(MAKE_DIRECTORY "${S_PLUG}/sqldrivers")
if(NOT EXISTS "${S_PLUG}/sqldrivers/libqsqlite.so")
    set(_QSQLITE_FOUND FALSE)
    set(_QSQLITE_SEARCH_DIRS "")
    if(VCPKG_QT6_PLUGIN_DIR)
        list(APPEND _QSQLITE_SEARCH_DIRS "${VCPKG_QT6_PLUGIN_DIR}/sqldrivers")
    endif()
    if(VCPKG_PLUGIN_DIR)
        list(APPEND _QSQLITE_SEARCH_DIRS "${VCPKG_PLUGIN_DIR}/sqldrivers")
    endif()
    foreach(_d IN LISTS _QSQLITE_SEARCH_DIRS)
        # Match libqsqlite.so or libqsqlite.so.5 / .so.6 etc.
        file(GLOB _candidates "${_d}/libqsqlite.so*")
        if(_candidates)
            list(GET _candidates 0 _src)
            message(STATUS "[LinuxDeploy] Safety-net: copying sqldrivers/libqsqlite.so from ${_d}")
            file(COPY "${_src}" DESTINATION "${S_PLUG}/sqldrivers")
            set(_QSQLITE_FOUND TRUE)
            break()
        endif()
    endforeach()
    if(NOT _QSQLITE_FOUND)
        message(WARNING "[LinuxDeploy] sqldrivers/libqsqlite.so not found -- Qt Assistant search may not work")
    endif()
endif()

# =========================================================================
# 5c) Copy MariaDB Connector/C client-side auth plugins
#
#     libmariadb resolves its plugin directory at runtime relative to the
#     directory containing the loaded .so (via dladdr).  The vcpkg port
#     bakes in the relative path "../plugins/libmariadb", so:
#
#       lib/libmariadb.so.3  ->  lib/../plugins/libmariadb/
#
#     The Qt6 bulk-copy (section 5) uses VCPKG_QT6_PLUGIN_DIR which points
#     to <triplet>/Qt6/plugins/ -- a separate subtree that does NOT contain
#     libmariadb plugins.  Those live in <triplet>/plugins/libmariadb/ and
#     must be staged explicitly.  Source: ${VCPKG_PLUGIN_DIR}/libmariadb/
#     Mirrors: MacDeploy.cmake section 9e
# =========================================================================
set(_LNX_MARIADB_PLUGIN_SRC_DIR "")
if(VCPKG_PLUGIN_DIR AND EXISTS "${VCPKG_PLUGIN_DIR}/libmariadb")
    set(_LNX_MARIADB_PLUGIN_SRC_DIR "${VCPKG_PLUGIN_DIR}/libmariadb")
endif()

if(_LNX_MARIADB_PLUGIN_SRC_DIR)
    file(MAKE_DIRECTORY "${S_PLUG}/libmariadb")
    foreach(_mariadb_plugin IN ITEMS "dialog.so" "mysql_clear_password.so")
        set(_mariadb_src "${_LNX_MARIADB_PLUGIN_SRC_DIR}/${_mariadb_plugin}")
        if(EXISTS "${_mariadb_src}")
            if(NOT EXISTS "${S_PLUG}/libmariadb/${_mariadb_plugin}")
                message(STATUS "[LinuxDeploy] Installing MariaDB auth plugin -> plugins/libmariadb/${_mariadb_plugin}")
                file(COPY "${_mariadb_src}" DESTINATION "${S_PLUG}/libmariadb")
            else()
                message(STATUS "[LinuxDeploy] MariaDB auth plugin already present: plugins/libmariadb/${_mariadb_plugin}")
            endif()
        else()
            message(WARNING "[LinuxDeploy] MariaDB auth plugin not found: ${_mariadb_src}")
        endif()
    endforeach()
else()
    message(WARNING
        "[LinuxDeploy] MariaDB Connector/C plugins dir not found -- PAM auth may fail at runtime. "
        "Expected: \${VCPKG_PLUGIN_DIR}/libmariadb/. "
        "Ensure libmariadb is installed for the active vcpkg triplet.")
endif()

# =========================================================================
# 6) Deploy Qt Assistant binary
#    Legacy equivalent: copypkg-lnx.sh copied assistant from ${QTDIR}/bin/
#
#    After copying, we rewrite the RPATH so the binary finds its Qt .so
#    files from the tarball's lib/ dir at runtime ($ORIGIN/../lib) rather
#    than the stale absolute path into the vcpkg build tree.
#    patchelf is the standard tool for this on Linux; if it is absent we
#    emit a warning (the binary will work only if the vcpkg tree is present,
#    i.e. on the build machine, but not from an installed tarball).
#    Qt5 Assistant uses CLucene (not SQLite/FTS5) for full-text search, so
#    the FTS5 concern that affected Qt6/macOS does not apply here.
#    The .qhc collection file still requires the SQLite Qt driver; that is
#    handled by the sqldrivers safety-net in section 5b above.
# =========================================================================
if(ASSISTANT_EXE AND EXISTS "${ASSISTANT_EXE}")
    message(STATUS "[LinuxDeploy] Copying assistant -> bin/")
    file(COPY "${ASSISTANT_EXE}" DESTINATION "${S_BIN}"
         FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                          GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    # Patch RPATH so assistant finds Qt .so files from the tarball lib/ dir.
    # $ORIGIN resolves to the directory containing the binary (bin/).
    # $ORIGIN/../lib is therefore <tarball-root>/lib/ at runtime.
    set(_ASSISTANT_STAGED "${S_BIN}/assistant")
    find_program(_PATCHELF patchelf)
    if(_PATCHELF AND EXISTS "${_ASSISTANT_STAGED}")
        execute_process(
            COMMAND "${_PATCHELF}" --set-rpath "$ORIGIN/../lib" "${_ASSISTANT_STAGED}"
            RESULT_VARIABLE _PATCHELF_RC
            ERROR_VARIABLE  _PATCHELF_ERR
        )
        if(_PATCHELF_RC EQUAL 0)
            message(STATUS "[LinuxDeploy] Patched RPATH on assistant: $ORIGIN/../lib")
        else()
            message(WARNING "[LinuxDeploy] patchelf failed on assistant (rc=${_PATCHELF_RC}): ${_PATCHELF_ERR}")
        endif()
    elseif(NOT _PATCHELF)
        message(WARNING "[LinuxDeploy] patchelf not found -- assistant RPATH not patched.")
        message(WARNING "  assistant will only work on the build machine (vcpkg tree present).")
        message(WARNING "  Install patchelf: apt-get install patchelf / dnf install patchelf")
    endif()
else()
    message(STATUS "[LinuxDeploy] ASSISTANT_EXE not provided -- help system may not work")
endif()

# =========================================================================
# 7) Copy manual.qch / manual.qhc into bin/
#    Mirrors WinDeploy.cmake section 8
# =========================================================================
if(QCH_DIR)
    foreach(_f manual.qch manual.qhc)
        if(EXISTS "${QCH_DIR}/${_f}")
            message(STATUS "  Copying ${_f} -> bin/")
            file(COPY "${QCH_DIR}/${_f}" DESTINATION "${S_BIN}")
        endif()
    endforeach()
endif()

# =========================================================================
# 8) Copy etc/ and somo/
#    Mirrors WinDeploy.cmake section 9
# =========================================================================
if(ETC_SOURCE_DIR AND EXISTS "${ETC_SOURCE_DIR}")
    message(STATUS "[LinuxDeploy] Copying etc/ -> ${S_ETC}/")
    file(COPY "${ETC_SOURCE_DIR}/" DESTINATION "${S_ETC}")
endif()

if(SOMO_SOURCE_DIR AND EXISTS "${SOMO_SOURCE_DIR}")
    message(STATUS "[LinuxDeploy] Copying somo/ -> ${S_SOMO}/")
    file(COPY "${SOMO_SOURCE_DIR}/" DESTINATION "${S_SOMO}")
endif()

# =========================================================================
# 8b) Stage SoMo binaries and libraries
#     Mirrors WinDeploy.cmake section 9b
# =========================================================================
if(SOMO_BIN_DIR AND EXISTS "${SOMO_BIN_DIR}")
    message(STATUS "[LinuxDeploy] Staging SoMo binaries from ${SOMO_BIN_DIR} -> bin/")
    file(GLOB _somo_bins "${SOMO_BIN_DIR}/us3_*" "${SOMO_BIN_DIR}/us_somo*")
    foreach(_b ${_somo_bins})
        if(NOT IS_DIRECTORY "${_b}")
            get_filename_component(_n "${_b}" NAME)
            message(STATUS "  SoMo binary: ${_n}")
            file(COPY "${_b}" DESTINATION "${S_BIN}"
                 FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                                  GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
        endif()
    endforeach()
else()
    message(STATUS "[LinuxDeploy] SoMo bin dir not provided -- skipping SoMo binary staging")
endif()

if(SOMO_LIB_DIR AND EXISTS "${SOMO_LIB_DIR}")
    message(STATUS "[LinuxDeploy] Staging SoMo libraries from ${SOMO_LIB_DIR} -> lib/")
    file(GLOB _somo_libs "${SOMO_LIB_DIR}/*.so*")
    foreach(_l ${_somo_libs})
        get_filename_component(_n "${_l}" NAME)
        if(NOT EXISTS "${S_LIB}/${_n}")
            message(STATUS "  SoMo lib: ${_n}")
            file(COPY "${_l}" DESTINATION "${S_LIB}")
        endif()
    endforeach()
else()
    message(STATUS "[LinuxDeploy] SoMo lib dir not provided -- skipping SoMo library staging")
endif()

# =========================================================================
# 8c) Dependency-completeness pass via linuxdeploy
#
#     Sections 3-4 above hand-curate which .so files get bundled (UltraScan's
#     own libs, everything vcpkg built for Qt/Qwt/OpenSSL). That approach
#     structurally cannot see runtime libraries that come from the *system*
#     rather than vcpkg -- e.g. libxcb-cursor.so.0, which Qt6's xcb platform
#     plugin dlopens at runtime (required since Qt 6.5) but which vcpkg does
#     not vendor (Qt links against the host's X11/xcb stack by design, same
#     as libX11/libGL). A machine without libxcb-cursor0 installed system-wide
#     will crash on startup with "This application failed to start because no
#     Qt platform plugin could be initialized" even though bootstrap-linux.sh
#     lists libxcb-cursor-dev as a *build*-time header package -- the runtime
#     .so was simply never staged into the tarball.
#
#     linuxdeploy (github.com/linuxdeploy/linuxdeploy) is macdeployqt's Linux
#     analog: given a set of executables, it walks their real ELF dependency
#     graph (like ldd) and bundles anything missing. We use it ONLY for that
#     dependency-discovery capability -- not to restructure this tarball into
#     an AppImage/AppDir (usr/bin, usr/lib, .desktop, icon) layout, which
#     would be a much bigger, riskier change rippling into us.sh, qt.conf,
#     and the mirrored Mac/Win deploy scripts. So: run it against a throwaway
#     scratch AppDir, then merge only the *new* libraries it finds into the
#     existing lib/ -- everything else about the tarball layout is untouched.
#
#     Best-effort: if the download/tool fails for any reason, this step warns
#     and the packaging build continues (mirrors the rasmol fetch policy in
#     MacDeploy.cmake/WinDeploy.cmake -- a missing optional piece degrades a
#     feature, it doesn't fail the whole build).
# =========================================================================
execute_process(COMMAND uname -m OUTPUT_VARIABLE _uname_m OUTPUT_STRIP_TRAILING_WHITESPACE)
if(_uname_m STREQUAL "x86_64")
    set(_LD_ARCH "x86_64")
elseif(_uname_m MATCHES "^(aarch64|arm64)$")
    set(_LD_ARCH "aarch64")
else()
    set(_LD_ARCH "")
endif()

# Cache the downloaded/extracted tool next to the stage dir (build/<preset>/_tools/)
# so repeated packaging runs on the same machine don't re-download it every time.
get_filename_component(_LNX_PRESET_DIR "${STAGE_DIR}" DIRECTORY)  # .../_stage
get_filename_component(_LNX_PRESET_DIR "${_LNX_PRESET_DIR}" DIRECTORY)  # .../<preset>
set(_LD_TOOL_DIR "${_LNX_PRESET_DIR}/_tools/linuxdeploy")
set(_LD_APPIMAGE "${_LD_TOOL_DIR}/linuxdeploy-${_LD_ARCH}.AppImage")
set(_LD_APPRUN   "${_LD_TOOL_DIR}/squashfs-root/AppRun")

if(NOT _LD_ARCH)
    message(WARNING "[LinuxDeploy] Unsupported arch '${_uname_m}' for linuxdeploy -- skipping dependency-completeness pass")
elseif(NOT EXISTS "${_LD_APPRUN}")
    file(MAKE_DIRECTORY "${_LD_TOOL_DIR}")
    if(NOT EXISTS "${_LD_APPIMAGE}")
        message(STATUS "[LinuxDeploy] Fetching linuxdeploy (${_LD_ARCH})...")
        file(DOWNLOAD
            "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-${_LD_ARCH}.AppImage"
            "${_LD_APPIMAGE}"
            STATUS _ld_dl_status)
        list(GET _ld_dl_status 0 _ld_dl_code)
        if(NOT _ld_dl_code EQUAL 0)
            list(GET _ld_dl_status 1 _ld_dl_msg)
            message(WARNING "[LinuxDeploy] Failed to download linuxdeploy (${_ld_dl_msg}) -- skipping dependency-completeness pass")
            file(REMOVE "${_LD_APPIMAGE}")
        endif()
    endif()
    if(EXISTS "${_LD_APPIMAGE}")
        file(CHMOD "${_LD_APPIMAGE}" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                                                  GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
        # --appimage-extract avoids needing FUSE, which is frequently unavailable
        # in Docker/CI containers.
        execute_process(
            COMMAND "${_LD_APPIMAGE}" --appimage-extract
            WORKING_DIRECTORY "${_LD_TOOL_DIR}"
            RESULT_VARIABLE _ld_extract_rc
            OUTPUT_QUIET ERROR_QUIET
        )
        if(NOT _ld_extract_rc EQUAL 0)
            message(WARNING "[LinuxDeploy] Failed to extract linuxdeploy AppImage -- skipping dependency-completeness pass")
        endif()
    endif()
endif()

if(EXISTS "${_LD_APPRUN}")
    # linuxdeploy requires at least one --desktop-file to run at all, even
    # though we have no use for desktop integration in a portable tarball.
    # Generate throwaway placeholder inputs purely to satisfy that requirement.
    set(_LD_SCRATCH "${STAGE_DIR}-linuxdeploy-scratch")
    file(REMOVE_RECURSE "${_LD_SCRATCH}")
    file(MAKE_DIRECTORY "${_LD_SCRATCH}")
    set(_LD_DESKTOP "${_LD_SCRATCH}/us.desktop")
    set(_LD_ICON    "${_LD_SCRATCH}/us.svg")
    file(WRITE "${_LD_DESKTOP}"
"[Desktop Entry]
Type=Application
Name=UltraScan3
Exec=us
Icon=us
Categories=Science;
")
    file(WRITE "${_LD_ICON}" "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"1\" height=\"1\"/>")

    # Collect every executable already staged into bin/ so linuxdeploy resolves
    # dependencies for all of them, not just the main us binary.
    file(GLOB _LD_EXES "${S_BIN}/us" "${S_BIN}/us_*" "${S_BIN}/assistant" "${S_BIN}/us3_*" "${S_BIN}/us_somo*")
    set(_LD_EXE_ARGS "")
    foreach(_exe ${_LD_EXES})
        if(NOT IS_DIRECTORY "${_exe}" AND NOT IS_SYMLINK "${_exe}")
            list(APPEND _LD_EXE_ARGS "--executable" "${_exe}")
        endif()
    endforeach()

    list(LENGTH _LD_EXES _LD_EXE_COUNT)
    set(_LD_APPDIR "${_LD_SCRATCH}/AppDir")
    message(STATUS "[LinuxDeploy] Running linuxdeploy dependency scan over ${_LD_EXE_COUNT} staged executables...")
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E env
            "LD_LIBRARY_PATH=${S_LIB}:${VCPKG_LIB_DIR}:$ENV{LD_LIBRARY_PATH}"
            "${_LD_APPRUN}"
            --appdir "${_LD_APPDIR}"
            --desktop-file "${_LD_DESKTOP}"
            --icon-file "${_LD_ICON}"
            ${_LD_EXE_ARGS}
        RESULT_VARIABLE _ld_run_rc
        OUTPUT_VARIABLE _ld_run_out
        ERROR_VARIABLE  _ld_run_err
    )
    if(NOT _ld_run_rc EQUAL 0)
        message(WARNING "[LinuxDeploy] linuxdeploy dependency scan failed (rc=${_ld_run_rc}): ${_ld_run_err}")
    else()
        # Merge only libraries linuxdeploy found that we don't already have --
        # this is the whole point: pick up system libs (libxcb-cursor.so.0 and
        # anything else) that sections 3-4's vcpkg-only globs cannot see.
        file(GLOB _ld_found_libs "${_LD_APPDIR}/usr/lib/*.so*")
        set(_ld_new_count 0)
        foreach(_lib ${_ld_found_libs})
            get_filename_component(_n "${_lib}" NAME)
            if(NOT EXISTS "${S_LIB}/${_n}")
                message(STATUS "  [linuxdeploy] Bundling missing runtime dependency: ${_n}")
                file(COPY "${_lib}" DESTINATION "${S_LIB}")
                math(EXPR _ld_new_count "${_ld_new_count} + 1")
            endif()
        endforeach()
        message(STATUS "[LinuxDeploy] linuxdeploy found ${_ld_new_count} additional runtime dependency/ies not covered by the vcpkg lib copy")
    endif()

    file(REMOVE_RECURSE "${_LD_SCRATCH}")
else()
    message(STATUS "[LinuxDeploy] linuxdeploy not available -- dependency-completeness pass skipped")
    message(STATUS "  (known gap: libxcb-cursor.so.0 and similar system Qt runtime deps may be missing)")
endif()

# =========================================================================
# 8d) Explicitly bundle libxcb-cursor -- linuxdeploy's "missing" heuristic
#     cannot catch this one
#
#     linuxdeploy (section 8c) and macdeployqt-style tools in general only
#     bundle a library if ldd reports it unresolved *on the build machine*.
#     bootstrap-linux.sh installs libxcb-cursor-dev, which pulls in the
#     runtime libxcb-cursor0/libxcb-cursor.so.0 as an apt dependency -- so on
#     every machine that can build this project, ldd on `us` always resolves
#     it successfully, and linuxdeploy therefore never considers it "missing"
#     and never bundles it. But Qt6's xcb platform plugin still needs it at
#     runtime on the *target* machine (required since Qt 6.5), and plenty of
#     target machines won't have it pre-installed. So: bundle it explicitly
#     by name, the same way libssl/libcrypto/libsqlite3 are explicitly
#     globbed above, rather than relying on a "missing on build machine"
#     heuristic that can never fire for this library.
# =========================================================================
if(NOT EXISTS "${S_LIB}/libxcb-cursor.so.0")
    execute_process(
        COMMAND bash -c "ldconfig -p 2>/dev/null | grep -m1 'libxcb-cursor\\.so' | awk '{print \$NF}'"
        OUTPUT_VARIABLE _xcb_cursor_path
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(_xcb_cursor_path AND EXISTS "${_xcb_cursor_path}")
        message(STATUS "[LinuxDeploy] Explicitly bundling libxcb-cursor from ${_xcb_cursor_path}")
        # -L dereferences the symlink so a real regular file lands in lib/,
        # not a dangling link back to a system-only path.
        execute_process(COMMAND cp -L "${_xcb_cursor_path}" "${S_LIB}/libxcb-cursor.so.0")
    else()
        message(WARNING
            "[LinuxDeploy] libxcb-cursor.so not found on this build machine via ldconfig -- "
            "not bundled. Install libxcb-cursor-dev (Debian/Ubuntu) or "
            "xcb-util-cursor-devel (RHEL/Rocky/Oracle) on the build machine so it can be staged.")
    endif()
else()
    message(STATUS "[LinuxDeploy] libxcb-cursor.so.0 already staged -- skipping explicit bundle")
endif()

# =========================================================================
# 9) Copy license.txt
#    Mirrors WinDeploy.cmake section 10
# =========================================================================
if(LICENSE_FILE AND EXISTS "${LICENSE_FILE}")
    message(STATUS "[LinuxDeploy] Copying license.txt")
    file(COPY "${LICENSE_FILE}" DESTINATION "${STAGE_DIR}")
    get_filename_component(_lic_name "${LICENSE_FILE}" NAME)
    if(NOT "${_lic_name}" STREQUAL "license.txt")
        file(RENAME "${STAGE_DIR}/${_lic_name}" "${STAGE_DIR}/license.txt")
    endif()
endif()

if(VERSION_FILE AND EXISTS "${VERSION_FILE}")
    message(STATUS "[LinuxDeploy] Copying VERSION")
    file(COPY "${VERSION_FILE}" DESTINATION "${STAGE_DIR}")
endif()

# =========================================================================
# 10) Write qt.conf so Qt finds plugins/ relative to the tarball root
#
#     On Linux, Qt looks for qt.conf next to the executable (in bin/).
#     Prefix "." means relative to the directory containing qt.conf.
#     At runtime that directory is bin/, so:
#       Plugins = ../plugins   ->  <tarball-root>/plugins/
#       Libraries = .          ->  bin/  (Qt .so are in lib/ but RPATH handles that)
#
#     Mirrors WinDeploy.cmake section 11
# =========================================================================
message(STATUS "[LinuxDeploy] Writing qt.conf -> bin/")
file(WRITE "${S_BIN}/qt.conf"
    "[Paths]\nPrefix = .\nPlugins = ../plugins\nLibraries = ../lib\n")

# =========================================================================
# 11) Strip binaries and libraries (mirrors legacy copypkg-lnx.sh strip step)
# =========================================================================
find_program(_STRIP strip)
if(_STRIP)
    message(STATUS "[LinuxDeploy] Stripping binaries in bin/")
    file(GLOB _strip_bins "${S_BIN}/us" "${S_BIN}/us_*" "${S_BIN}/assistant")
    foreach(_b ${_strip_bins})
        if(NOT IS_DIRECTORY "${_b}")
            execute_process(COMMAND "${_STRIP}" "${_b}" ERROR_QUIET)
        endif()
    endforeach()

    message(STATUS "[LinuxDeploy] Stripping libraries in lib/")
    file(GLOB _strip_libs "${S_LIB}/*.so" "${S_LIB}/*.so.*")
    # libicudata is a huge data-only blob with almost no symbols to strip;
    # some strip/binutils versions corrupt its ELF program headers
    # ("ELF load command address/offset not properly aligned" at load time),
    # so skip it rather than risk shipping a broken shared library.
    list(FILTER _strip_libs EXCLUDE REGEX "libicudata")
    foreach(_l ${_strip_libs})
        if(NOT IS_SYMLINK "${_l}")
            execute_process(COMMAND "${_STRIP}" "--strip-unneeded" "${_l}" ERROR_QUIET)
        endif()
    endforeach()
else()
    message(STATUS "[LinuxDeploy] 'strip' not found -- skipping symbol stripping")
endif()

# =========================================================================
# 12) Legacy compat symlinks: bin64 -> bin, lib64 -> lib
#     copypkg-lnx.sh created these for 64-bit packages
# =========================================================================
execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink "bin" "${STAGE_DIR}/bin64"
    ERROR_QUIET
)
execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink "lib" "${STAGE_DIR}/lib64"
    ERROR_QUIET
)

# =========================================================================
# Summary
# =========================================================================
message(STATUS "")
message(STATUS "[LinuxDeploy] Linux folder deployment complete: ${STAGE_DIR}")
message(STATUS "  bin/     - Executables + help files")
message(STATUS "  lib/     - UltraScan + Qt + Qwt + OpenSSL .so files")
message(STATUS "  plugins/ - Qt plugins (platforms/, sqldrivers/, ...)")
message(STATUS "  etc/     - Configuration data")
message(STATUS "  somo/    - SOMO data")
message(STATUS "  bin/us.sh    - Shell launcher (documented entry point)")
message(STATUS "  README.txt   - Installation and quick-start instructions")
message(STATUS "  bin64 -> bin, lib64 -> lib (legacy symlinks)")
