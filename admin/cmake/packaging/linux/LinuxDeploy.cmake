# =============================================================================
# LinuxDeploy.cmake - Linux deployment for UltraScan3 (portable tarball layout)
#
# Stages a self-contained directory tree ready for tar.gz packaging.
# Legacy equivalent: copypkg-lnx.sh
#
# Layout:
#   <STAGE_DIR>/
#     bin/           us, us_*, assistant, manual.qhc, manual.qch
#     lib/           UltraScan .so + Qt .so + Qwt .so + OpenSSL .so
#     plugins/       Qt plugins (platforms/, sqldrivers/, imageformats/, ...)
#     etc/           editable config data
#     somo/          SOMO data files
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
#   VCPKG_PLUGIN_DIR    - vcpkg installed plugins/ dir
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
else()
    message(STATUS "[LinuxDeploy] VCPKG_LIB_DIR not set -- Qt .so files will not be bundled")
    message(STATUS "  The app will rely on system Qt libraries (acceptable for distro packages,")
    message(STATUS "  but portable tarballs require Qt .so files to be bundled).")
endif()

# =========================================================================
# 5) Copy Qt plugins from vcpkg into plugins/
#    Legacy equivalent: copypkg-lnx.sh: ${RSYNC} ${QTDIR}/plugins ${PKGDIR}/lib/
# =========================================================================
if(VCPKG_PLUGIN_DIR AND EXISTS "${VCPKG_PLUGIN_DIR}")
    message(STATUS "[LinuxDeploy] Copying Qt plugins -> plugins/")
    file(COPY "${VCPKG_PLUGIN_DIR}/" DESTINATION "${S_PLUG}")
else()
    # Safety-net: platforms/libqxcb.so is required for any X11 display
    message(STATUS "[LinuxDeploy] VCPKG_PLUGIN_DIR not set -- Qt plugins not bundled")
    message(STATUS "  The app requires platform plugins (libqxcb.so) to run.")
endif()

# =========================================================================
# 5b) Guarantee sqldrivers/libqsqlite.so is present
#     Qt Assistant search requires SQLite; windeployqt/linuxdeployqt equivalents
#     don't always pull it in automatically.
# =========================================================================
file(MAKE_DIRECTORY "${S_PLUG}/sqldrivers")
if(NOT EXISTS "${S_PLUG}/sqldrivers/libqsqlite.so")
    set(_QSQLITE_FOUND FALSE)
    set(_QSQLITE_SEARCH_DIRS "")
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
# 6) Deploy Qt Assistant binary
#    Legacy equivalent: copypkg-lnx.sh copied assistant from ${QTDIR}/bin/
# =========================================================================
if(ASSISTANT_EXE AND EXISTS "${ASSISTANT_EXE}")
    message(STATUS "[LinuxDeploy] Copying assistant -> bin/")
    file(COPY "${ASSISTANT_EXE}" DESTINATION "${S_BIN}"
         FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                          GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
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
message(STATUS "  bin64 -> bin, lib64 -> lib (legacy symlinks)")
