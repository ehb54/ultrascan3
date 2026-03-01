# =============================================================================
# MacDeploy.cmake - macOS deployment for UltraScan3 (folder-based layout)
#
# Stages a location-independent folder tree that can live anywhere:
#
#   <STAGE_DIR>/
#     bin/           us.app, us_helpdaemon.app, Assistant.app, manual.qhc, ...
#     etc/           editable config data
#     frameworks/    Qt frameworks + UltraScan dylibs
#     lib/           (reserved)
#     plugins/       Qt plugins (platforms, sqldrivers, …)
#     somo/
#     license.txt
#     uninstall.sh
#
# Expected variables (passed via -D on the command line):
#   STAGE_DIR           - Root of the staged folder tree
#   MACDEPLOYQT         - Path to macdeployqt executable
#   ASSISTANT_APP       - Path to Qt Assistant.app (optional)
#   QCH_DIR             - Directory containing manual.qch / manual.qhc
#   BIN_DIR             - Build-tree bin/ containing us.app, us_* binaries
#   LIB_DIR             - Build-tree lib/ containing libusutils / libusGui dylibs
#   ETC_SOURCE_DIR      - Source etc/ directory
#   SOMO_SOURCE_DIR     - Source somo/ directory
#   LICENSE_FILE        - Path to LICENSE.txt
#   VCPKG_LIB_DIR       - vcpkg lib/ dir (for libsqlite3)
#   VCPKG_PLUGIN_DIR    - vcpkg plugins/ dir (for sqldrivers/libqsqlite)
# =============================================================================

cmake_minimum_required(VERSION 3.16)

if(NOT STAGE_DIR)
    message(FATAL_ERROR "STAGE_DIR not set")
endif()
if(NOT MACDEPLOYQT)
    message(FATAL_ERROR "MACDEPLOYQT not set")
endif()
if(NOT BIN_DIR OR NOT EXISTS "${BIN_DIR}")
    message(FATAL_ERROR "BIN_DIR does not exist: ${BIN_DIR}")
endif()

set(S_BIN  "${STAGE_DIR}/bin")
set(S_FW   "${STAGE_DIR}/frameworks")
set(S_PLUG "${STAGE_DIR}/plugins")
set(S_LIB  "${STAGE_DIR}/lib")
set(S_ETC  "${STAGE_DIR}/etc")
set(S_SOMO "${STAGE_DIR}/somo")

file(MAKE_DIRECTORY "${S_BIN}")
file(MAKE_DIRECTORY "${S_FW}")
file(MAKE_DIRECTORY "${S_PLUG}")
file(MAKE_DIRECTORY "${S_LIB}")

# =========================================================================
# 1) Copy us.app bundle into bin/
# =========================================================================
if(EXISTS "${BIN_DIR}/us.app")
    message(STATUS "Copying us.app → ${S_BIN}/")
    file(COPY "${BIN_DIR}/us.app" DESTINATION "${S_BIN}")
else()
    message(FATAL_ERROR "us.app not found in ${BIN_DIR}")
endif()

# =========================================================================
# 2) Copy companion us_* programs as .app bundles into bin/
#    On macOS with MACOSX_BUNDLE, each is built as us_foo.app.
#    Non-bundle executables are copied as bare binaries.
# =========================================================================
file(GLOB extra_items "${BIN_DIR}/us_*")
foreach(item ${extra_items})
    get_filename_component(item_name "${item}" NAME)
    if(IS_DIRECTORY "${item}" AND item_name MATCHES "\\.app$")
        message(STATUS "  Copying bundle: ${item_name}")
        file(COPY "${item}" DESTINATION "${S_BIN}")
    elseif(NOT IS_DIRECTORY "${item}")
        message(STATUS "  Copying program: ${item_name}")
        file(COPY "${item}" DESTINATION "${S_BIN}"
             FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                              GROUP_READ GROUP_EXECUTE
                              WORLD_READ WORLD_EXECUTE)
    endif()
endforeach()

# =========================================================================
# 3) Copy UltraScan shared libraries into frameworks/
# =========================================================================
if(LIB_DIR AND EXISTS "${LIB_DIR}")
    file(GLOB us_dylibs "${LIB_DIR}/libusutils*.dylib" "${LIB_DIR}/libusgui*.dylib"
                        "${LIB_DIR}/libusUtils*.dylib" "${LIB_DIR}/libusGui*.dylib"
                        "${LIB_DIR}/libus_*.dylib")
    foreach(dylib ${us_dylibs})
        get_filename_component(dylib_name "${dylib}" NAME)
        get_filename_component(real_dylib "${dylib}" REALPATH)
        message(STATUS "  Copying library: ${dylib_name}")
        file(COPY "${real_dylib}" DESTINATION "${S_FW}")
        get_filename_component(real_name "${real_dylib}" NAME)
        if(NOT "${real_name}" STREQUAL "${dylib_name}")
            execute_process(
                COMMAND ${CMAKE_COMMAND} -E create_symlink "${real_name}" "${S_FW}/${dylib_name}"
            )
        endif()
    endforeach()
endif()

# =========================================================================
# 4) Run macdeployqt on us.app
#    This copies Qt frameworks and plugins INTO the .app bundle.
#    We will relocate them to the top-level frameworks/ and plugins/ later.
# =========================================================================
message(STATUS "Running macdeployqt on us.app...")
set(US_APP "${S_BIN}/us.app")

# Build list of extra executables to fix up.
# NOTE: Do NOT pass UltraScan dylibs (libus_*) as -executable arguments.
# macdeployqt would rewrite their load commands using @loader_path relative
# to frameworks/, producing broken paths.  We fix the dylibs manually in
# section 8 after macdeployqt runs.
set(EXTRA_EXEC_ARGS "")
# Add companion bare binaries
file(GLOB companion_bins "${S_BIN}/us_*")
foreach(bin ${companion_bins})
    if(NOT IS_DIRECTORY "${bin}")
        list(APPEND EXTRA_EXEC_ARGS "-executable=${bin}")
    endif()
endforeach()

execute_process(
    COMMAND "${MACDEPLOYQT}" "${US_APP}"
            -verbose=1
            ${EXTRA_EXEC_ARGS}
    RESULT_VARIABLE deploy_result
    OUTPUT_VARIABLE deploy_output
    ERROR_VARIABLE deploy_error
)
if(NOT deploy_result EQUAL 0)
    message(WARNING "macdeployqt on us.app exited with code ${deploy_result}")
    message(STATUS "stdout: ${deploy_output}")
    message(STATUS "stderr: ${deploy_error}")
endif()

# =========================================================================
# 5) Relocate Qt frameworks from us.app/Contents/Frameworks → top-level frameworks/
#    and Qt plugins from us.app/Contents/PlugIns → top-level plugins/
# =========================================================================
message(STATUS "Relocating Qt frameworks and plugins to top-level directories...")

set(APP_FW_DIR "${US_APP}/Contents/Frameworks")
set(APP_PLUG_DIR "${US_APP}/Contents/PlugIns")

if(EXISTS "${APP_FW_DIR}")
    file(GLOB app_fw_items "${APP_FW_DIR}/*")
    foreach(item ${app_fw_items})
        get_filename_component(item_name "${item}" NAME)
        if(NOT EXISTS "${S_FW}/${item_name}")
            message(STATUS "  Moving framework: ${item_name}")
            file(RENAME "${item}" "${S_FW}/${item_name}")
        else()
            file(REMOVE_RECURSE "${item}")
        endif()
    endforeach()
    file(REMOVE_RECURSE "${APP_FW_DIR}")
endif()

if(EXISTS "${APP_PLUG_DIR}")
    file(GLOB_RECURSE app_plug_files "${APP_PLUG_DIR}/*")
    foreach(pf ${app_plug_files})
        # Preserve subdirectory structure (e.g., platforms/, sqldrivers/)
        file(RELATIVE_PATH rel_path "${APP_PLUG_DIR}" "${pf}")
        get_filename_component(rel_dir "${rel_path}" DIRECTORY)
        file(MAKE_DIRECTORY "${S_PLUG}/${rel_dir}")
        if(NOT EXISTS "${S_PLUG}/${rel_path}")
            message(STATUS "  Moving plugin: ${rel_path}")
            file(RENAME "${pf}" "${S_PLUG}/${rel_path}")
        endif()
    endforeach()
    file(REMOVE_RECURSE "${APP_PLUG_DIR}")
endif()

# =========================================================================
# 6) Fix rpaths for us.app to find frameworks/ and plugins/ relative to bundle
#
#    us.app/Contents/MacOS/us  →  frameworks/ is at ../../../frameworks
#    us.app/Contents/MacOS/us  →  plugins/ is at ../../../plugins
#
#    @executable_path/../../.. = bin/
#    @executable_path/../../../frameworks = frameworks/
# =========================================================================
message(STATUS "Fixing rpaths for folder-based layout...")

set(US_BIN_PATH "${US_APP}/Contents/MacOS/us")

# Remove macdeployqt's old rpath pointing to Contents/Frameworks
execute_process(
    COMMAND bash -c "otool -l '${US_BIN_PATH}' | grep -A2 'LC_RPATH' | grep 'path ' | awk '{print $2}'"
    OUTPUT_VARIABLE _existing_rpaths  OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(_existing_rpaths)
    string(REPLACE "\n" ";" _rpath_list "${_existing_rpaths}")
    foreach(_rp IN LISTS _rpath_list)
        message(STATUS "  Removing old rpath from us: ${_rp}")
        execute_process(COMMAND install_name_tool -delete_rpath "${_rp}" "${US_BIN_PATH}" ERROR_QUIET)
    endforeach()
endif()

# Add rpaths for the folder layout
# us.app/Contents/MacOS/us → STAGE_DIR is 4 levels up:
#   MacOS → Contents → us.app → bin → STAGE_DIR
execute_process(COMMAND install_name_tool -add_rpath "@executable_path/../../../../frameworks" "${US_BIN_PATH}" ERROR_QUIET)
execute_process(COMMAND install_name_tool -add_rpath "@executable_path/../../../../plugins" "${US_BIN_PATH}" ERROR_QUIET)

# Fix hardcoded @loader_path/../Frameworks/ references to UltraScan dylibs.
# macdeployqt rewrites Qt lib references but leaves UltraScan dylib references
# as @loader_path/../Frameworks/<name> which breaks the folder-based layout.
# We must rewrite them to @rpath/<name> so the rpath search above resolves them.
execute_process(
    COMMAND bash -c "otool -L '${US_BIN_PATH}' | awk '{print $1}' | grep '@loader_path/'"
    OUTPUT_VARIABLE _us_libus_refs  OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(_us_libus_refs)
    string(REPLACE "\n" ";" _us_libus_list "${_us_libus_refs}")
    foreach(_ref IN LISTS _us_libus_list)
        get_filename_component(_libname "${_ref}" NAME)
        message(STATUS "  Rewriting us ref: ${_ref} -> @rpath/${_libname}")
        execute_process(
            COMMAND install_name_tool -change "${_ref}" "@rpath/${_libname}" "${US_BIN_PATH}"
            ERROR_QUIET
        )
    endforeach()
endif()

# =========================================================================
# 7) Fix rpaths for bare us_* companion binaries in bin/
#    From bin/ to frameworks/ is ../frameworks
# =========================================================================
file(GLOB bare_bins "${S_BIN}/us_*")
foreach(bin ${bare_bins})
    if(IS_DIRECTORY "${bin}")
        continue()
    endif()
    get_filename_component(bin_name "${bin}" NAME)

    # Remove stale rpaths
    execute_process(
        COMMAND bash -c "otool -l '${bin}' | grep -A2 'LC_RPATH' | grep 'path ' | awk '{print $2}'"
        OUTPUT_VARIABLE _existing OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(_existing)
        string(REPLACE "\n" ";" _rlist "${_existing}")
        foreach(_rp IN LISTS _rlist)
            execute_process(COMMAND install_name_tool -delete_rpath "${_rp}" "${bin}" ERROR_QUIET)
        endforeach()
    endif()

    execute_process(COMMAND install_name_tool -add_rpath "@executable_path/../frameworks" "${bin}" ERROR_QUIET)

    # Fix @loader_path/../Frameworks/libus_* references → @rpath/libus_*
    execute_process(
        COMMAND bash -c "otool -L '${bin}' | awk '{print $1}' | grep '@loader_path/'"
        OUTPUT_VARIABLE _libus_refs  OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(_libus_refs)
        string(REPLACE "\n" ";" _libus_list "${_libus_refs}")
        foreach(_ref IN LISTS _libus_list)
            get_filename_component(_libname "${_ref}" NAME)
            execute_process(
                COMMAND install_name_tool -change "${_ref}" "@rpath/${_libname}" "${bin}"
                ERROR_QUIET
            )
        endforeach()
    endif()
endforeach()

# Also fix @loader_path/../Frameworks/libus_* references in all .app bundles
file(GLOB _app_bundles "${S_BIN}/*.app")
foreach(_app IN LISTS _app_bundles)
    get_filename_component(_app_name "${_app}" NAME)
    string(REGEX REPLACE "\.app$" "" _base_name "${_app_name}")
    set(_app_bin "${_app}/Contents/MacOS/${_base_name}")
    if(NOT EXISTS "${_app_bin}")
        continue()
    endif()
    execute_process(
        COMMAND bash -c "otool -L '${_app_bin}' | awk '{print $1}' | grep '@loader_path/'"
        OUTPUT_VARIABLE _app_libus_refs  OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(_app_libus_refs)
        string(REPLACE "\n" ";" _app_libus_list "${_app_libus_refs}")
        foreach(_ref IN LISTS _app_libus_list)
            get_filename_component(_libname "${_ref}" NAME)
            message(STATUS "  Rewriting ${_app_name} ref: ${_ref} -> @rpath/${_libname}")
            execute_process(
                COMMAND install_name_tool -change "${_ref}" "@rpath/${_libname}" "${_app_bin}"
                ERROR_QUIET
            )
        endforeach()
    endif()
    # Ensure rpath covers the top-level frameworks/ for app bundles too
    # From <app>.app/Contents/MacOS/ up to STAGE_DIR is 4 levels:
    #   MacOS → Contents → <app>.app → bin → STAGE_DIR
    execute_process(COMMAND install_name_tool -add_rpath "@executable_path/../../../../frameworks" "${_app_bin}" ERROR_QUIET)
endforeach()

# =========================================================================
# 8) Fix UltraScan dylib install names AND their internal @loader_path deps
# =========================================================================
file(GLOB us_fw_dylibs "${S_FW}/libus_*.dylib" "${S_FW}/libusutils*.dylib"
                       "${S_FW}/libusgui*.dylib" "${S_FW}/libusUtils*.dylib"
                       "${S_FW}/libusGui*.dylib")
foreach(dylib ${us_fw_dylibs})
    get_filename_component(dylib_name "${dylib}" NAME)
    # Fix the dylib's own install name
    execute_process(COMMAND install_name_tool -id "@rpath/${dylib_name}" "${dylib}" ERROR_QUIET)
    # Fix rpath on the dylib itself so it can find its own dependencies
    execute_process(
        COMMAND bash -c "otool -l '${dylib}' | grep -A2 LC_RPATH | awk '/path /{print $2}'"
        OUTPUT_VARIABLE _dylib_rpaths  OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(_dylib_rpaths)
        string(REPLACE "\n" ";" _dylib_rpath_list "${_dylib_rpaths}")
        foreach(_rp IN LISTS _dylib_rpath_list)
            execute_process(COMMAND install_name_tool -delete_rpath "${_rp}" "${dylib}" ERROR_QUIET)
        endforeach()
    endif()
    execute_process(COMMAND install_name_tool -add_rpath "@loader_path" "${dylib}" ERROR_QUIET)
    # Rewrite any @loader_path/../... references to @rpath/<libname>
    execute_process(
        COMMAND bash -c "otool -L '${dylib}' | awk '{print $1}' | grep '@loader_path/'"
        OUTPUT_VARIABLE _dylib_loader_refs  OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(_dylib_loader_refs)
        string(REPLACE "\n" ";" _dylib_loader_list "${_dylib_loader_refs}")
        foreach(_ref IN LISTS _dylib_loader_list)
            get_filename_component(_libname "${_ref}" NAME)
            execute_process(
                COMMAND install_name_tool -change "${_ref}" "@rpath/${_libname}" "${dylib}"
                ERROR_QUIET
            )
        endforeach()
    endif()
endforeach()

# =========================================================================
# 9) Deploy Qt Assistant.app into bin/
# =========================================================================
if(ASSISTANT_APP AND EXISTS "${ASSISTANT_APP}")
    message(STATUS "Deploying Qt Assistant into bin/...")

    set(ASSISTANT_DEST "${S_BIN}/Assistant.app")
    if(EXISTS "${ASSISTANT_DEST}")
        file(REMOVE_RECURSE "${ASSISTANT_DEST}")
    endif()
    file(COPY "${ASSISTANT_APP}" DESTINATION "${S_BIN}")

    # Run macdeployqt on Assistant.app standalone
    message(STATUS "  Running macdeployqt on Assistant.app...")
    execute_process(
        COMMAND "${MACDEPLOYQT}" "${ASSISTANT_DEST}" -verbose=1
        RESULT_VARIABLE _r  OUTPUT_VARIABLE _o  ERROR_VARIABLE _e
    )
    if(NOT _r EQUAL 0)
        message(WARNING "macdeployqt on Assistant.app exited with code ${_r}")
        message(STATUS "  stdout: ${_o}")
        message(STATUS "  stderr: ${_e}")
    endif()

    # Move frameworks from Assistant.app → top-level frameworks/
    set(ASSIST_FW_DIR "${ASSISTANT_DEST}/Contents/Frameworks")
    if(EXISTS "${ASSIST_FW_DIR}")
        file(GLOB assist_libs "${ASSIST_FW_DIR}/*")
        foreach(lib ${assist_libs})
            get_filename_component(lib_name "${lib}" NAME)
            if(NOT EXISTS "${S_FW}/${lib_name}")
                message(STATUS "  Moving ${lib_name} → frameworks/")
                file(RENAME "${lib}" "${S_FW}/${lib_name}")
            else()
                file(REMOVE_RECURSE "${lib}")
            endif()
        endforeach()
        file(REMOVE_RECURSE "${ASSIST_FW_DIR}")
    endif()

    # Move plugins from Assistant.app → top-level plugins/
    set(ASSIST_PLUG_DIR "${ASSISTANT_DEST}/Contents/PlugIns")
    if(EXISTS "${ASSIST_PLUG_DIR}")
        file(GLOB_RECURSE assist_plug_files "${ASSIST_PLUG_DIR}/*")
        foreach(pf ${assist_plug_files})
            file(RELATIVE_PATH rel_path "${ASSIST_PLUG_DIR}" "${pf}")
            get_filename_component(rel_dir "${rel_path}" DIRECTORY)
            file(MAKE_DIRECTORY "${S_PLUG}/${rel_dir}")
            if(NOT EXISTS "${S_PLUG}/${rel_path}")
                message(STATUS "  Moving Assistant plugin: ${rel_path}")
                file(RENAME "${pf}" "${S_PLUG}/${rel_path}")
            endif()
        endforeach()
        file(REMOVE_RECURSE "${ASSIST_PLUG_DIR}")
    endif()

    # Fix Assistant rpaths to point to top-level frameworks/
    # Assistant binary lives at:
    #   <STAGE>/bin/Assistant.app/Contents/MacOS/Assistant
    # Top-level frameworks at:
    #   <STAGE>/frameworks
    # From @executable_path (= .../Contents/MacOS/):
    #   ../../../.. = MacOS → Contents → Assistant.app → bin → <STAGE>
    #   ../../../../frameworks = <STAGE>/frameworks  ✓
    set(ASSIST_BIN "${ASSISTANT_DEST}/Contents/MacOS/Assistant")

    execute_process(
        COMMAND bash -c "otool -l '${ASSIST_BIN}' | grep -A2 'LC_RPATH' | grep 'path ' | awk '{print $2}'"
        OUTPUT_VARIABLE _existing_rpaths  OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(_existing_rpaths)
        string(REPLACE "\n" ";" _rpath_list "${_existing_rpaths}")
        foreach(_rp IN LISTS _rpath_list)
            message(STATUS "  Removing stale rpath from Assistant: ${_rp}")
            execute_process(COMMAND install_name_tool -delete_rpath "${_rp}" "${ASSIST_BIN}" ERROR_QUIET)
        endforeach()
    endif()

    message(STATUS "  Adding rpath: @executable_path/../../../../frameworks")
    execute_process(
        COMMAND install_name_tool -add_rpath "@executable_path/../../../../frameworks" "${ASSIST_BIN}"
        RESULT_VARIABLE _r
    )
    execute_process(
        COMMAND install_name_tool -add_rpath "@executable_path/../../../../plugins" "${ASSIST_BIN}"
        ERROR_QUIET
    )

    # (QSQLITE and libsqlite3 are installed in section 9b below,
    #  AFTER all plugin relocation is complete.)

else()
    message(STATUS "Assistant.app not provided or not found - help system may not work")
endif()

# =========================================================================
# 9b) Guarantee QSQLITE plugin and libsqlite3 in staged tree
#     This runs AFTER macdeployqt + plugin evacuation for both us.app and
#     Assistant.app, so the file won't be clobbered.
#     Legacy equivalent: setup.pl copies ALL plugins from Qt install
#     via "rsync -av plugins $us_dir/" — QSQLITE is always present.
# =========================================================================
file(MAKE_DIRECTORY "${S_PLUG}/sqldrivers")
if(NOT EXISTS "${S_PLUG}/sqldrivers/libqsqlite.dylib")
    set(_QSQLITE_FOUND FALSE)
    if(VCPKG_PLUGIN_DIR)
        set(_QSQLITE_SRC "${VCPKG_PLUGIN_DIR}/sqldrivers/libqsqlite.dylib")
        if(EXISTS "${_QSQLITE_SRC}")
            message(STATUS "Installing QSQLITE plugin → plugins/sqldrivers/ (from vcpkg)")
            file(COPY "${_QSQLITE_SRC}" DESTINATION "${S_PLUG}/sqldrivers")
            set(_QSQLITE_FOUND TRUE)
        endif()
    endif()
    if(NOT _QSQLITE_FOUND)
        message(FATAL_ERROR
            "QSQLITE plugin (libqsqlite.dylib) not found in staged plugins/ "
            "or VCPKG_PLUGIN_DIR (${VCPKG_PLUGIN_DIR}). "
            "Qt Assistant cannot open .qhc files without it.")
    endif()
else()
    message(STATUS "QSQLITE plugin already present in plugins/sqldrivers/")
endif()

# libsqlite3 runtime
if(VCPKG_LIB_DIR)
    file(GLOB _SQLITE_RT "${VCPKG_LIB_DIR}/libsqlite3*.dylib")
    foreach(_slib ${_SQLITE_RT})
        get_filename_component(_sn "${_slib}" NAME)
        if(NOT EXISTS "${S_FW}/${_sn}")
            message(STATUS "Installing ${_sn} → frameworks/")
            file(COPY "${_slib}" DESTINATION "${S_FW}")
        endif()
    endforeach()
endif()

# =========================================================================
# 10) Copy manual.qch / manual.qhc into bin/
# =========================================================================
if(QCH_DIR)
    foreach(qch_file manual.qch manual.qhc)
        if(EXISTS "${QCH_DIR}/${qch_file}")
            message(STATUS "  Copying ${qch_file} → bin/")
            file(COPY "${QCH_DIR}/${qch_file}" DESTINATION "${S_BIN}")
        endif()
    endforeach()
endif()

# =========================================================================
# 11) Copy etc/ and somo/
# =========================================================================
if(ETC_SOURCE_DIR AND EXISTS "${ETC_SOURCE_DIR}")
    message(STATUS "Copying etc/ → ${S_ETC}/")
    file(COPY "${ETC_SOURCE_DIR}/" DESTINATION "${S_ETC}")
endif()

if(SOMO_SOURCE_DIR AND EXISTS "${SOMO_SOURCE_DIR}")
    message(STATUS "Copying somo/ → ${S_SOMO}/")
    file(COPY "${SOMO_SOURCE_DIR}/" DESTINATION "${S_SOMO}")
endif()

# =========================================================================
# 11b) Stage SoMo artifacts from source-tree bin/ and lib/
#      makesomo.sh rsyncs SoMo outputs into $ULTRASCAN/bin and $ULTRASCAN/lib
#      (the source tree root).  We pick them up here so they land in the DMG.
#
#      SoMo binaries are .app bundles; they have already been fixed up by
#      somo_appnames.sh / somo_libnames.sh (run inside makesomo.sh on Darwin).
#      Those scripts use ../../../../lib/ and ../../../../Frameworks/ rpaths
#      which match the staged folder layout:
#        bin/us3_hydrodyn.app/Contents/MacOS/us3_hydrodyn
#           ../../../../lib/      → NOT present in our layout (we use frameworks/)
#           ../../../../Frameworks/ → frameworks/ is two levels up from bin/
#      The rpath fixup in sections 6–8 above covers the main UltraScan binaries;
#      SoMo apps are self-contained bundles and rely on the somo_appnames fixup
#      (already applied by makesomo.sh).  No additional rpath patching needed here.
# =========================================================================
if(SOMO_BIN_DIR AND EXISTS "${SOMO_BIN_DIR}")
    message(STATUS "Staging SoMo binaries from ${SOMO_BIN_DIR} → bin/")
    # Copy .app bundles
    file(GLOB _somo_apps "${SOMO_BIN_DIR}/*.app")
    foreach(_app ${_somo_apps})
        get_filename_component(_app_name "${_app}" NAME)
        message(STATUS "  SoMo bundle: ${_app_name}")
        file(COPY "${_app}" DESTINATION "${S_BIN}")
    endforeach()
    # Copy bare executables (non-directories)
    file(GLOB _somo_bins "${SOMO_BIN_DIR}/*")
    foreach(_bin ${_somo_bins})
        if(NOT IS_DIRECTORY "${_bin}")
            get_filename_component(_bin_name "${_bin}" NAME)
            message(STATUS "  SoMo binary: ${_bin_name}")
            file(COPY "${_bin}" DESTINATION "${S_BIN}"
                 FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                                  GROUP_READ GROUP_EXECUTE
                                  WORLD_READ WORLD_EXECUTE)
        endif()
    endforeach()
else()
    message(STATUS "SoMo bin dir not provided or not yet built — skipping SoMo binary staging")
    message(STATUS "  Run: cmake --build . --target build_somo  before  deploy_macos")
endif()

if(SOMO_LIB_DIR AND EXISTS "${SOMO_LIB_DIR}")
    message(STATUS "Staging SoMo libraries from ${SOMO_LIB_DIR} → frameworks/")
    file(GLOB _somo_libs "${SOMO_LIB_DIR}/*.dylib" "${SOMO_LIB_DIR}/*.so")
    foreach(_lib ${_somo_libs})
        get_filename_component(_lib_name "${_lib}" NAME)
        if(NOT EXISTS "${S_FW}/${_lib_name}")
            message(STATUS "  SoMo library: ${_lib_name}")
            file(COPY "${_lib}" DESTINATION "${S_FW}")
        endif()
    endforeach()
else()
    message(STATUS "SoMo lib dir not provided or not yet built — skipping SoMo library staging")
endif()

# =========================================================================
# 12) Copy license.txt
# =========================================================================
if(LICENSE_FILE AND EXISTS "${LICENSE_FILE}")
    message(STATUS "Copying license.txt")
    file(COPY "${LICENSE_FILE}" DESTINATION "${STAGE_DIR}")
    # Rename to license.txt if needed
    get_filename_component(lic_name "${LICENSE_FILE}" NAME)
    if(NOT "${lic_name}" STREQUAL "license.txt")
        file(RENAME "${STAGE_DIR}/${lic_name}" "${STAGE_DIR}/license.txt")
    endif()
endif()

# =========================================================================
# 13) Create uninstall.sh
# =========================================================================
file(WRITE "${STAGE_DIR}/uninstall.sh" [=[#!/bin/bash
# UltraScan3 Uninstaller
# Removes the UltraScan3 folder and all its contents.

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
echo "This will remove UltraScan3 from:"
echo "  $SCRIPT_DIR"
echo ""
read -rp "Are you sure? [y/N] " answer
if [[ "$answer" =~ ^[Yy]$ ]]; then
    rm -rf "$SCRIPT_DIR"
    echo "UltraScan3 has been removed."
else
    echo "Cancelled."
fi
]=])
execute_process(COMMAND chmod +x "${STAGE_DIR}/uninstall.sh")

# =========================================================================
# 14) Create qt.conf files for each .app so Qt finds plugins at top level
#
#     For bin/us.app/Contents/MacOS/us:
#       Prefix = ../../..      (→ points to STAGE_DIR relative to MacOS/)
#
#     For bin/Assistant.app/Contents/MacOS/Assistant:
#       Prefix = ../../..      (→ points to STAGE_DIR relative to Contents/)
# =========================================================================
message(STATUS "Writing qt.conf files...")

# qt.conf Prefix resolution in Qt5:
# Qt reads qt.conf from <bundle>/Contents/Resources/qt.conf
# but resolves Prefix relative to <bundle>/Contents/ (NOT Resources/).
#
# Layout: STAGE_DIR/bin/<app>.app/Contents/Resources/qt.conf
# From Contents/: ../../.. = Contents → <app>.app → bin → STAGE_DIR  ✓
#
# us.app qt.conf
file(WRITE "${S_BIN}/us.app/Contents/Resources/qt.conf"
"[Paths]\nPrefix = ../../..\nPlugins = plugins\nLibraries = frameworks\n")

# Assistant.app qt.conf
if(EXISTS "${S_BIN}/Assistant.app")
    file(MAKE_DIRECTORY "${S_BIN}/Assistant.app/Contents/Resources")
    file(WRITE "${S_BIN}/Assistant.app/Contents/Resources/qt.conf"
"[Paths]\nPrefix = ../../..\nPlugins = plugins\nLibraries = frameworks\n")
endif()

# us_helpdaemon.app qt.conf
if(EXISTS "${S_BIN}/us_helpdaemon.app")
    file(MAKE_DIRECTORY "${S_BIN}/us_helpdaemon.app/Contents/Resources")
    file(WRITE "${S_BIN}/us_helpdaemon.app/Contents/Resources/qt.conf"
"[Paths]\nPrefix = ../../..\nPlugins = plugins\nLibraries = frameworks\n")
endif()

# =========================================================================
# 15) Create UltraScan.app launcher bundle in bin/
#     This is a thin shell-script wrapper that checks for X11 before
#     handing off to the real us binary.  No Qt involvement needed.
# =========================================================================
set(_LAUNCHER_SRC "")
# Locate the source files relative to this cmake script
get_filename_component(_MACDEPLY_DIR "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)
get_filename_component(_REPO_ROOT    "${_MACDEPLY_DIR}"            DIRECTORY)  # admin/cmake -> admin
get_filename_component(_REPO_ROOT    "${_REPO_ROOT}"                DIRECTORY)  # admin       -> repo root
set(_LAUNCHER_DIR "${_REPO_ROOT}/admin/launcher")

if(EXISTS "${_LAUNCHER_DIR}/UltraScan" AND EXISTS "${_LAUNCHER_DIR}/Info.plist")
    message(STATUS "Creating UltraScan.app launcher bundle in bin/")
    set(_LAPP "${S_BIN}/UltraScan.app")
    file(MAKE_DIRECTORY "${_LAPP}/Contents/MacOS")
    file(MAKE_DIRECTORY "${_LAPP}/Contents/Resources")

    # Copy and make launcher executable
    file(COPY "${_LAUNCHER_DIR}/UltraScan" DESTINATION "${_LAPP}/Contents/MacOS"
         FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                          GROUP_READ GROUP_EXECUTE
                          WORLD_READ WORLD_EXECUTE)

    # Info.plist
    file(COPY "${_LAUNCHER_DIR}/Info.plist" DESTINATION "${_LAPP}/Contents")
else()
    message(WARNING "UltraScan.app launcher sources not found at ${_LAUNCHER_DIR} — skipping")
endif()

# =========================================================================
# 16) Ad-hoc codesign everything — MUST be last, inside-out order
# =========================================================================
message(STATUS "Ad-hoc code signing (inside-out)...")

# Frameworks and dylibs
file(GLOB_RECURSE all_fw_items "${S_FW}/*.framework" "${S_FW}/*.dylib")
foreach(item ${all_fw_items})
    execute_process(COMMAND codesign --force --sign - "${item}" ERROR_QUIET)
endforeach()

# Top-level plugins
file(GLOB_RECURSE all_plugins "${S_PLUG}/*.dylib")
foreach(item ${all_plugins})
    execute_process(COMMAND codesign --force --sign - "${item}" ERROR_QUIET)
endforeach()

# Assistant.app
if(EXISTS "${S_BIN}/Assistant.app")
    file(GLOB_RECURSE assist_plugins "${S_BIN}/Assistant.app/Contents/PlugIns/*.dylib")
    foreach(p ${assist_plugins})
        execute_process(COMMAND codesign --force --sign - "${p}" ERROR_QUIET)
    endforeach()
    execute_process(COMMAND codesign --force --sign - "${S_BIN}/Assistant.app/Contents/MacOS/Assistant")
    execute_process(COMMAND codesign --force --sign - "${S_BIN}/Assistant.app")
endif()

# All .app bundles in bin/
file(GLOB all_app_bundles "${S_BIN}/*.app")
foreach(app ${all_app_bundles})
    get_filename_component(app_name "${app}" NAME)
    if("${app_name}" STREQUAL "Assistant.app")
        continue()  # Already signed above
    endif()
    # Sign the main binary inside each bundle
    string(REGEX REPLACE "\\.app$" "" base_name "${app_name}")
    if(EXISTS "${app}/Contents/MacOS/${base_name}")
        execute_process(COMMAND codesign --force --sign - "${app}/Contents/MacOS/${base_name}")
    endif()
    execute_process(COMMAND codesign --force --sign - "${app}")
endforeach()

# Bare companion binaries in bin/
file(GLOB bare_sign "${S_BIN}/us_*")
foreach(bin ${bare_sign})
    if(NOT IS_DIRECTORY "${bin}")
        execute_process(COMMAND codesign --force --sign - "${bin}")
    endif()
endforeach()

message(STATUS "")
message(STATUS "macOS folder deployment complete: ${STAGE_DIR}")
message(STATUS "  bin/        - Application bundles and help files")
message(STATUS "  frameworks/ - Qt frameworks and UltraScan libraries")
message(STATUS "  plugins/    - Qt plugins")
message(STATUS "  etc/        - Configuration data")
message(STATUS "  somo/       - SOMO data")
