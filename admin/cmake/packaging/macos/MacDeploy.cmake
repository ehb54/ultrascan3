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
#   VCPKG_LIB_DIR         - vcpkg lib/ dir (for libsqlite3)
#   VCPKG_PLUGIN_DIR      - vcpkg plugins/ dir (for sqldrivers/libqsqlite)
#   VCPKG_QT6_PLUGIN_DIR  - vcpkg Qt6/plugins/ dir (dynamic triplet, for platforms/)
#                           When set, passed to macdeployqt as -pluginpath so it
#                           finds .dylib plugins from the dynamic triplet even when
#                           macdeployqt itself comes from the static triplet.
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
# 2.5) packaging: wrap all bare us_* executables into .app bundles
# =========================================================================
message(STATUS "Wrapping staged bare us_* executables into .app bundles (Option 2)...")

execute_process(
        COMMAND bash -c
        "set -euo pipefail
     cd '${S_BIN}'

     shopt -s nullglob

     for f in us_*; do
       # skip directories (including existing .app bundles)
       [ -d \"\$f\" ] && continue

       # skip non-executables
       [ -x \"\$f\" ] || continue

       app=\"\$f.app\"
       exe_in_app=\"\$app/Contents/MacOS/\$f\"

       echo \"  bundling \$f -> \$app\"

       mkdir -p \"\$app/Contents/MacOS\" \"\$app/Contents/Resources\"

       # Minimal Info.plist
       cat > \"\$app/Contents/Info.plist\" <<PLIST
<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">
<plist version=\"1.0\">
<dict>
  <key>CFBundleExecutable</key><string>\$f</string>
  <key>CFBundleIdentifier</key><string>com.aucsolutions.ultrascan3.\$f</string>
  <key>CFBundleName</key><string>\$f</string>
  <key>CFBundlePackageType</key><string>APPL</string>
  <key>CFBundleDevelopmentRegion</key><string>en</string>
  <key>LSMinimumSystemVersion</key><string>11.0</string>
  <key>NSPrincipalClass</key><string>NSApplication</string>
  <key>NSSupportsAutomaticGraphicsSwitching</key><true/>
</dict>
</plist>
PLIST

       # Move executable into the bundle
       mv \"\$f\" \"\$exe_in_app\"
       chmod +x \"\$exe_in_app\"

       # Fix rpath: bare exe used @executable_path/../frameworks; now needs 4-level up
       install_name_tool -delete_rpath \"@executable_path/../frameworks\" \"\$exe_in_app\" 2>/dev/null || true
       install_name_tool -add_rpath \"@executable_path/../../../../frameworks\" \"\$exe_in_app\" 2>/dev/null || true

       # Provide plugin symlinks (legacy-style, robust for Qt on macOS)
       ( cd \"\$app/Contents/MacOS\" &&
         ln -sf ../../../../plugins/platforms platforms &&
         ln -sf ../../../../plugins/imageformats imageformats &&
         ln -sf ../../../../plugins/styles styles &&
         ln -sf ../../../../plugins/iconengines iconengines &&
         ln -sf ../../../../plugins/sqldrivers sqldrivers
       )
     done
    "
        RESULT_VARIABLE wrap_result
)
if(NOT wrap_result EQUAL 0)
    message(FATAL_ERROR "Failed wrapping us_* executables into .app bundles")
endif()

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

# Build list
set(EXTRA_EXEC_ARGS "")

# After wrapping, all tools are .app bundles in bin/.
# Tell macdeployqt about each tool binary so it pulls in all needed Qt deps.
file(GLOB companion_apps "${S_BIN}/us_*.app")
foreach(app ${companion_apps})
    get_filename_component(app_name "${app}" NAME)
    string(REGEX REPLACE "\\.app$" "" base "${app_name}")
    set(app_bin "${app}/Contents/MacOS/${base}")
    if(EXISTS "${app_bin}")
        list(APPEND EXTRA_EXEC_ARGS "-executable=${app_bin}")
    endif()
endforeach()

# macdeployqt uses qt.conf (next to its own binary) to locate Qt libraries and
# plugins.  Our build-tree wrapper has a qt.conf pointing Prefix at the dynamic
# triplet root, so macdeployqt will find .dylib files in lib/ and Qt6/plugins/
# without any -pluginpath override.  (-pluginpath is a Qt5-only flag and errors
# in Qt6, so we must not pass it here.)
set(_MACDEPLOYQT_DEPLOY_ARGS "")

execute_process(
    COMMAND "${MACDEPLOYQT}" "${US_APP}"
            -verbose=1
            ${_MACDEPLOYQT_DEPLOY_ARGS}
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
# 7) Fix rpaths for all bundled companion .app tools
#    Each tool now lives at:
#      bin/<tool>.app/Contents/MacOS/<tool>
#
#    From Contents/MacOS back to top-level frameworks/ is:
#      ../../../../frameworks
#
#    Ensure each bundled tool has:
#      @executable_path/../../../../frameworks
# =========================================================================

# fix @loader_path/../Frameworks/libus_* references in all .app bundles
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

# Force use of dynamically built Assistant from build tree
set(ASSISTANT_APP "${CMAKE_BINARY_DIR}/bin/Assistant.app")

if(NOT EXISTS "${ASSISTANT_APP}")
    message(FATAL_ERROR "Dynamic Assistant.app not found at ${ASSISTANT_APP}")
endif()

message(STATUS "Using dynamic Assistant from ${ASSISTANT_APP}")

# =========================================================================
# 9) Deploy Qt Assistant.app into staging bin/
# =========================================================================
if(ASSISTANT_APP AND EXISTS "${ASSISTANT_APP}")
    message(STATUS "Deploying Qt Assistant into bin/...")

    set(ASSISTANT_DEST "${S_BIN}/Assistant.app")
    if(EXISTS "${ASSISTANT_DEST}")
        file(REMOVE_RECURSE "${ASSISTANT_DEST}")
    endif()
    file(COPY "${ASSISTANT_APP}" DESTINATION "${S_BIN}")

    # Strip broken vcpkg build-tree rpath before macdeployqt
    execute_process(
            COMMAND install_name_tool
            -delete_rpath "@loader_path/../../../../../../lib"
            "${ASSISTANT_DEST}/Contents/MacOS/Assistant"
            ERROR_QUIET
    )

    # ---- Patch Assistant.app bundle id to avoid macOS Installer relocation ----
    set(_ASSIST_PLIST "${S_BIN}/Assistant.app/Contents/Info.plist")

    if(EXISTS "${_ASSIST_PLIST}")
        message(STATUS "Patching Qt Assistant CFBundleIdentifier to avoid Installer relocation")

        execute_process(
                COMMAND /usr/libexec/PlistBuddy
                -c "Set :CFBundleIdentifier com.aucsolutions.ultrascan3.assistant"
                "${_ASSIST_PLIST}"
                RESULT_VARIABLE _pb_rc
        )

        if(NOT _pb_rc EQUAL 0)
            execute_process(
                    COMMAND /usr/libexec/PlistBuddy
                    -c "Add :CFBundleIdentifier string com.aucsolutions.ultrascan3.assistant"
                    "${_ASSIST_PLIST}"
            )
        endif()

        # Optional cosmetics (safe to ignore failures if keys missing)
        execute_process(COMMAND /usr/libexec/PlistBuddy
                -c "Set :CFBundleName UltraScan3 Help"
                "${_ASSIST_PLIST}"
                RESULT_VARIABLE _ign1)
        execute_process(COMMAND /usr/libexec/PlistBuddy
                -c "Set :CFBundleDisplayName UltraScan3 Help"
                "${_ASSIST_PLIST}"
                RESULT_VARIABLE _ign2)

        # Strip any existing signature; we ad-hoc sign later in step 16
        file(REMOVE_RECURSE "${S_BIN}/Assistant.app/Contents/_CodeSignature")
    endif()

    # Run macdeployqt on Assistant.app standalone
    message(STATUS "  Running macdeployqt on Assistant.app...")
    execute_process(
        COMMAND "${MACDEPLOYQT}" "${ASSISTANT_DEST}" -verbose=1
                ${_MACDEPLOYQT_DEPLOY_ARGS}
        RESULT_VARIABLE _r  OUTPUT_VARIABLE _o  ERROR_VARIABLE _e
    )
    if(NOT _r EQUAL 0)
        message(WARNING "macdeployqt on Assistant.app exited with code ${_r}")
        message(STATUS "  stdout: ${_o}")
        message(STATUS "  stderr: ${_e}")
    endif()

    # Ensure libqsqlite.dylib is in Assistant's own PlugIns/sqldrivers/
    # Qt prefers the bundle-internal PlugIns/ over qt.conf-specified paths.
    set(_asst_sql_dir "${ASSISTANT_DEST}/Contents/PlugIns/sqldrivers")
    file(MAKE_DIRECTORY "${_asst_sql_dir}")
    if(NOT EXISTS "${_asst_sql_dir}/libqsqlite.dylib")
        foreach(_sql_candidate IN ITEMS
                "${VCPKG_QT6_PLUGIN_DIR}/sqldrivers/libqsqlite.dylib"
                "${S_PLUG}/sqldrivers/libqsqlite.dylib")
            if(EXISTS "${_sql_candidate}")
                file(COPY "${_sql_candidate}" DESTINATION "${_asst_sql_dir}")
                message(STATUS "  Staged libqsqlite.dylib into Assistant.app/Contents/PlugIns/sqldrivers/")
                break()
            endif()
        endforeach()
    endif()

    set(ASSIST_BIN "${ASSISTANT_DEST}/Contents/MacOS/Assistant")

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

    # Build a prioritised list of candidate directories to search.
    # vcpkg uses a non-standard Qt6/plugins/ layout (not the standard plugins/)
    # and the .dylib lives in the dynamic triplet, while the static triplet
    # only has .a archives.  Search both layout variants for both triplets.
    set(_QSQLITE_SEARCH_DIRS "")

    # Derive installed root from VCPKG_PLUGIN_DIR (which is <root>/<triplet>/plugins)
    # and also try the Qt6/plugins sub-layout used by this vcpkg Qt6 port.
    foreach(_PLUG_DIR IN ITEMS "${VCPKG_PLUGIN_DIR}" "${VCPKG_PLUGIN_DIR_STA}")
        if(_PLUG_DIR)
            # Standard layout: <triplet>/plugins/sqldrivers
            list(APPEND _QSQLITE_SEARCH_DIRS "${_PLUG_DIR}/sqldrivers")
            # vcpkg Qt6 layout:  <triplet>/Qt6/plugins/sqldrivers
            get_filename_component(_TRIPLET_DIR "${_PLUG_DIR}" DIRECTORY)
            list(APPEND _QSQLITE_SEARCH_DIRS "${_TRIPLET_DIR}/Qt6/plugins/sqldrivers")
        endif()
    endforeach()

    # Use dynamic Qt plugin directory directly
    if(VCPKG_QT6_PLUGIN_DIR)
        list(APPEND _QSQLITE_SEARCH_DIRS
                "${VCPKG_QT6_PLUGIN_DIR}/sqldrivers"
        )
    endif()

    foreach(_SEARCH_DIR IN LISTS _QSQLITE_SEARCH_DIRS)
        set(_QSQLITE_SRC "${_SEARCH_DIR}/libqsqlite.dylib")
        if(EXISTS "${_QSQLITE_SRC}")
            message(STATUS "Installing QSQLITE plugin → plugins/sqldrivers/ (from ${_SEARCH_DIR})")
            file(COPY "${_QSQLITE_SRC}" DESTINATION "${S_PLUG}/sqldrivers")
            set(_QSQLITE_FOUND TRUE)
            break()
        endif()
    endforeach()

    if(NOT _QSQLITE_FOUND)
        message(WARNING
            "QSQLITE plugin (libqsqlite.dylib) not found. "
            "Searched: ${_QSQLITE_SEARCH_DIRS}. "
            "Qt Assistant search may not work without it. "
            "Run: ~/vcpkg/vcpkg install qt6-sql:arm64-osx-dynamic")
    endif()
else()
    message(STATUS "QSQLITE plugin already present in plugins/sqldrivers/")
endif()

# =========================================================================
# 9c) Guarantee platform plugins (libqcocoa.dylib) in staged tree
#     macdeployqt should handle this, but when it comes from the static triplet
#     it may deploy nothing.  This safety-net copies from VCPKG_QT6_PLUGIN_DIR.
# =========================================================================
file(MAKE_DIRECTORY "${S_PLUG}/platforms")
if(NOT EXISTS "${S_PLUG}/platforms/libqcocoa.dylib")
    set(_PLATFORMS_SRC "")
    # Search VCPKG_QT6_PLUGIN_DIR first (the definitive dynamic triplet path)
    if(VCPKG_QT6_PLUGIN_DIR AND EXISTS "${VCPKG_QT6_PLUGIN_DIR}/platforms")
        set(_PLATFORMS_SRC "${VCPKG_QT6_PLUGIN_DIR}/platforms")
    else()
        # Fallback: derive from VCPKG_PLUGIN_DIR by trying Qt6/plugins sub-layout
        foreach(_PDIR IN ITEMS "${VCPKG_PLUGIN_DIR}" "${VCPKG_PLUGIN_DIR_STA}")
            if(_PDIR)
                get_filename_component(_PDIR_PARENT "${_PDIR}" DIRECTORY)
                if(EXISTS "${_PDIR_PARENT}/Qt6/plugins/platforms")
                    set(_PLATFORMS_SRC "${_PDIR_PARENT}/Qt6/plugins/platforms")
                    break()
                elseif(EXISTS "${_PDIR}/platforms")
                    set(_PLATFORMS_SRC "${_PDIR}/platforms")
                    break()
                endif()
            endif()
        endforeach()
        # Use dynamic Qt plugin directory directly
        if(NOT _PLATFORMS_SRC AND VCPKG_QT6_PLUGIN_DIR)
            if(EXISTS "${VCPKG_QT6_PLUGIN_DIR}/platforms")
                set(_PLATFORMS_SRC "${VCPKG_QT6_PLUGIN_DIR}/platforms")
            endif()
        endif()

#        This section can be deleted if script works without it
        # Also try deriving the dynamic triplet root from macdeployqt path
#        if(NOT _PLATFORMS_SRC AND MACDEPLOYQT)
#            get_filename_component(_MQ_BIN  "${MACDEPLOYQT}"   DIRECTORY)
#            get_filename_component(_MQ_TQT  "${_MQ_BIN}"       DIRECTORY)
#            get_filename_component(_MQ_TOOL "${_MQ_TQT}"       DIRECTORY)
#            get_filename_component(_MQ_TRIP "${_MQ_TOOL}"      DIRECTORY)
#            get_filename_component(_MQ_ROOT "${_MQ_TRIP}"      DIRECTORY)
#            get_filename_component(_MQ_TNAME "${_MQ_TRIP}"     NAME)
#            string(REPLACE "-osx" "-osx-dynamic" _DYN_T "${_MQ_TNAME}")
#            if(EXISTS "${_MQ_ROOT}/${_DYN_T}/Qt6/plugins/platforms")
#                set(_PLATFORMS_SRC "${_MQ_ROOT}/${_DYN_T}/Qt6/plugins/platforms")
#            endif()
#        endif()
    endif()

    if(_PLATFORMS_SRC)
        message(STATUS "Installing platform plugins → plugins/platforms/ (from ${_PLATFORMS_SRC})")
        file(GLOB _plat_dylibs "${_PLATFORMS_SRC}/*.dylib")
        foreach(_pd ${_plat_dylibs})
            file(COPY "${_pd}" DESTINATION "${S_PLUG}/platforms")
        endforeach()
    else()
        message(WARNING
            "Platform plugins (libqcocoa.dylib) not found. "
            "The application will crash on launch without a platform plugin. "
            "Set VCPKG_QT6_PLUGIN_DIR to arm64-osx-dynamic/Qt6/plugins.")
    endif()
else()
    message(STATUS "Platform plugins already present in plugins/platforms/")
endif()

# =========================================================================
# 9d) Guarantee Qt dylibs in frameworks/
#     macdeployqt from the static triplet cannot find Qt .dylib files because
#     its qt.conf points to arm64-osx/lib/ which contains only .a archives.
#     There is no -libpath flag in macdeployqt to override this.
#     We copy all Qt dylibs directly from VCPKG_LIB_DIR (the dynamic triplet
#     lib/ directory) as a complete safety net.
#     We skip .a archives and only copy real .dylib files (not symlinks to them
#     as those are resolved by file(COPY ...) automatically).
# =========================================================================
if(VCPKG_LIB_DIR AND EXISTS "${VCPKG_LIB_DIR}")
    file(GLOB _ALL_DYLIBS "${VCPKG_LIB_DIR}/libQt*.dylib"
                          "${VCPKG_LIB_DIR}/libqwt*.dylib"
                          "${VCPKG_LIB_DIR}/libQwt*.dylib")
    set(_qt_fw_count 0)
    foreach(_dylib ${_ALL_DYLIBS})
        get_filename_component(_dylib_name "${_dylib}" NAME)
        # Skip .a static archives that may glob through (shouldn't happen but safe)
        if(_dylib_name MATCHES "\.a$")
            continue()
        endif()
        if(NOT EXISTS "${S_FW}/${_dylib_name}")
            file(COPY "${_dylib}" DESTINATION "${S_FW}")
            math(EXPR _qt_fw_count "${_qt_fw_count} + 1")
        endif()
    endforeach()
    if(_qt_fw_count GREATER 0)
        message(STATUS "Installed ${_qt_fw_count} Qt dylibs → frameworks/ (from ${VCPKG_LIB_DIR})")
    else()
        message(STATUS "Qt dylibs already present in frameworks/ (or none found in ${VCPKG_LIB_DIR})")
    endif()
else()
    message(WARNING "VCPKG_LIB_DIR not set -- Qt dylibs will not be deployed. App will crash.")
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

# -----------------------------------------------------------------------------
# qt.conf for "loose" Qt executables in bin/ (us_2dsa, us_pcsa, etc.)
# -----------------------------------------------------------------------------
file(WRITE "${S_BIN}/qt.conf"
        "[Paths]\nPrefix = ..\nPlugins = plugins\nLibraries = frameworks\n")

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
# 15) Stage UltraScan3.app launcher bundle at the *root* of the install
#     (so users can drag it to the Dock easily)
#
#     Source template lives in:  <repo>/pkg/macos/launcher/UltraScan3.app
#     We copy the template, then copy the icon from the staged us.app so
#     the launcher always matches the real app icon.
# =========================================================================

# Compute repo root robustly based on this script location
get_filename_component(_MACDEPLOY_DIR "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)
# Expecting: <repo>/cmake/packaging/macos/MacDeploy.cmake  (or similar)
# Go up 3 levels: macos -> packaging -> cmake -> repo
get_filename_component(_REPO_ROOT "${_MACDEPLOY_DIR}/../../../.." ABSOLUTE)

set(_LAUNCHER_TEMPLATE "${_REPO_ROOT}/pkg/macos/launcher/UltraScan3.app")
set(_LAPP             "${STAGE_DIR}/UltraScan3.app")

if(EXISTS "${_LAUNCHER_TEMPLATE}/Contents/Info.plist")
    message(STATUS "Staging UltraScan3.app launcher at root of install")

    # A) Copy launcher template into stage root
    file(REMOVE_RECURSE "${_LAPP}")
    file(COPY "${_LAUNCHER_TEMPLATE}" DESTINATION "${STAGE_DIR}")

    # Ensure Resources exists (template may omit it)
    file(MAKE_DIRECTORY "${_LAPP}/Contents/Resources")

    # B/C) Copy icon from the staged us.app so it's guaranteed to match
    set(_US_ICON "${STAGE_DIR}/bin/us.app/Contents/Resources/us3-icon.icns")
    set(_ETC_ICON "${STAGE_DIR}/etc/us3-icon.icns")

    if(EXISTS "${_US_ICON}")
        file(COPY "${_US_ICON}" DESTINATION "${_LAPP}/Contents/Resources")
    elseif(EXISTS "${_ETC_ICON}")
        file(COPY "${_ETC_ICON}" DESTINATION "${_LAPP}/Contents/Resources")
    else()
        message(WARNING "Launcher icon not found (checked ${_US_ICON} and ${_ETC_ICON})")
    endif()

    # Optional: ad-hoc sign the launcher bundle (ok for local installs)
    execute_process(COMMAND codesign --force --deep --sign - "${_LAPP}" ERROR_QUIET)

else()
    message(WARNING "UltraScan3.app launcher template not found at ${_LAUNCHER_TEMPLATE} — skipping")
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
