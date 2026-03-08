# MacOsDeployQtAssistant.cmake (minimal, robust)
#
# Purpose:
#   Copy a Qt Assistant.app into DEST_DIR and make it runnable in-place.
#   For dynamic Qt builds, we run macdeployqt and then ensure the QSQLITE
#   driver plugin (needed for QtHelp collection search/index) is present.
#
# Inputs (via -D):
#   ASSISTANT_SOURCE : path to source Assistant.app
#   DEST_DIR         : directory to stage into (e.g. build/.../bin)
#   MACDEPLOYQT      : path to macdeployqt (optional)
#   QSQLITE_PLUGIN   : path to libqsqlite.dylib (optional)
#   QT_LIB_DIR       : used only for fallback rpath mode (optional)

cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED ASSISTANT_SOURCE)
    message(FATAL_ERROR "StageAssistant: ASSISTANT_SOURCE not set")
endif()
if(NOT DEFINED DEST_DIR)
    message(FATAL_ERROR "StageAssistant: DEST_DIR not set")
endif()

# ---- Normalize paths ----
get_filename_component(SRC_APP "${ASSISTANT_SOURCE}" ABSOLUTE)
get_filename_component(DEST_DIR_ABS "${DEST_DIR}" ABSOLUTE)
set(DEST_APP "${DEST_DIR_ABS}/Assistant.app")
set(SRC_BIN  "${SRC_APP}/Contents/MacOS/Assistant")
set(DST_BIN  "${DEST_APP}/Contents/MacOS/Assistant")

if(NOT EXISTS "${SRC_BIN}")
    message(FATAL_ERROR "StageAssistant: source Assistant binary not found: ${SRC_BIN}")
endif()

# ---- Helper: create Qt soname symlinks so plugins can load (libQt6Sql.6.dylib etc) ----
function(_stageassistant_make_qt_soname_links _fw_dir)
    if(NOT IS_DIRECTORY "${_fw_dir}")
        return()
    endif()

    file(GLOB _qt_full "${_fw_dir}/libQt6*.6.*.dylib")
    foreach(_full IN LISTS _qt_full)
        get_filename_component(_base "${_full}" NAME)

        # Derive lib name stem and major
        # Example: libQt6Sql.6.9.1.dylib -> stem=libQt6Sql major=6
        string(REGEX REPLACE "(libQt6[^.]+)\\.([0-9]+)\\..*" "\\1" _stem "${_base}")
        string(REGEX REPLACE "(libQt6[^.]+)\\.([0-9]+)\\..*" "\\2" _maj  "${_base}")

        if(_stem STREQUAL "${_base}")
            # regex didn't match; skip
            continue()
        endif()

        set(_link1 "${_fw_dir}/${_stem}.${_maj}.dylib")
        set(_link2 "${_fw_dir}/${_stem}.dylib")

        if(NOT EXISTS "${_link1}")
            execute_process(COMMAND /bin/ln -sf "${_base}" "${_link1}")
        endif()
        if(NOT EXISTS "${_link2}")
            execute_process(COMMAND /bin/ln -sf "${_base}" "${_link2}")
        endif()
    endforeach()
endfunction()

# ---- Helper: locate vcpkg Qt6 plugins dir from MACDEPLOYQT path ----
function(_stageassistant_guess_qt6_plugins_from_macdeployqt _macdeployqt _out_plugins_root)
    # macdeployqt typically:
    #   .../vcpkg/installed/<triplet>/tools/Qt6/bin/macdeployqt
    # plugins typically:
    #   .../vcpkg/installed/<triplet>/Qt6/plugins
    get_filename_component(_mdq_dir "${_macdeployqt}" DIRECTORY)      # .../tools/Qt6/bin
    get_filename_component(_qt6_dir "${_mdq_dir}" DIRECTORY)          # .../tools/Qt6
    get_filename_component(_tools_dir "${_qt6_dir}" DIRECTORY)        # .../tools
    get_filename_component(_triplet_root "${_tools_dir}" DIRECTORY)   # .../<triplet>

    set(_guess "${_triplet_root}/Qt6/plugins")
    if(EXISTS "${_guess}")
        set(${_out_plugins_root} "${_guess}" PARENT_SCOPE)
    else()
        set(${_out_plugins_root} "" PARENT_SCOPE)
    endif()
endfunction()

# ---- Decide whether we need to (re)deploy ----
set(_need_stage TRUE)
if(EXISTS "${DST_BIN}")
    # If source binary is newer, restage.
    if("${DST_BIN}" IS_NEWER_THAN "${SRC_BIN}")
        # Still need to ensure required plugins exist.
        set(_need_stage FALSE)
    endif()
endif()

# Required runtime pieces for working QtHelp search/index:
#   - libqcocoa (platform)
#   - libqsqlite (sql driver) + libsqlite3 runtime
set(_have_qcocoa FALSE)
set(_have_qsqlite FALSE)
if(EXISTS "${DEST_APP}/Contents/PlugIns/platforms/libqcocoa.dylib")
    set(_have_qcocoa TRUE)
endif()
if(EXISTS "${DEST_APP}/Contents/PlugIns/sqldrivers/libqsqlite.dylib")
    set(_have_qsqlite TRUE)
endif()

if(NOT _have_qcocoa OR NOT _have_qsqlite)
    set(_need_stage TRUE)
endif()

if(NOT _need_stage)
    message(STATUS "StageAssistant: ${DEST_APP} binary is up to date")
    message(STATUS "StageAssistant: required plugins already present (qcocoa + qsqlite)")
    return()
endif()

message(STATUS "StageAssistant: staging self-contained Assistant.app into ${DEST_DIR_ABS}")

# Clean destination to avoid stale mixes of static/dynamic Assistant or plugins
if(EXISTS "${DEST_APP}")
    file(REMOVE_RECURSE "${DEST_APP}")
endif()

# Copy Assistant.app
file(COPY "${SRC_APP}" DESTINATION "${DEST_DIR_ABS}")

if(NOT EXISTS "${DST_BIN}")
    message(FATAL_ERROR "StageAssistant: staging failed; missing ${DST_BIN}")
endif()

# ---------------------------------------------------------------------------
# Strategy A: macdeployqt — produce a self-contained bundle
# ---------------------------------------------------------------------------
if(DEFINED MACDEPLOYQT AND EXISTS "${MACDEPLOYQT}")
    message(STATUS "StageAssistant: running macdeployqt: ${MACDEPLOYQT}")

    # Ensure clean deploy targets inside the app (macdeployqt will recreate them)
    file(REMOVE_RECURSE "${DEST_APP}/Contents/Frameworks")
    file(REMOVE_RECURSE "${DEST_APP}/Contents/PlugIns")
    file(REMOVE_RECURSE "${DEST_APP}/Contents/Resources/qt.conf")

    execute_process(
            COMMAND "${MACDEPLOYQT}" "${DEST_APP}" -always-overwrite -verbose=1
            RESULT_VARIABLE _MDQ_RC
    )
    if(NOT _MDQ_RC EQUAL 0)
        message(FATAL_ERROR "StageAssistant: macdeployqt failed with code ${_MDQ_RC}")
    endif()

    # Make sure Qt soname links exist (plugins often reference libQt6X.6.dylib)
    _stageassistant_make_qt_soname_links("${DEST_APP}/Contents/Frameworks")

    # Ensure QSQLITE driver exists; macdeployqt may not deploy it by default.
    set(_qsqlite_src "")

    # 1) Explicit plugin path
    if(DEFINED QSQLITE_PLUGIN AND EXISTS "${QSQLITE_PLUGIN}")
        set(_qsqlite_src "${QSQLITE_PLUGIN}")
    endif()

    # 2) Try to infer from macdeployqt (vcpkg tools layout)
    if(NOT _qsqlite_src)
        _stageassistant_guess_qt6_plugins_from_macdeployqt("${MACDEPLOYQT}" _plugins_root)
        if(_plugins_root AND EXISTS "${_plugins_root}/sqldrivers/libqsqlite.dylib")
            set(_qsqlite_src "${_plugins_root}/sqldrivers/libqsqlite.dylib")
        endif()
    endif()

    # 3) Fallback: infer from Qt install prefix (works for vcpkg, Homebrew, system Qt)
    if(NOT _qsqlite_src AND DEFINED Qt6_DIR)
        get_filename_component(_qt_prefix "${Qt6_DIR}/../../../" ABSOLUTE)
        set(_candidate "${_qt_prefix}/plugins/sqldrivers/libqsqlite.dylib")
        if(EXISTS "${_candidate}")
            set(_qsqlite_src "${_candidate}")
        endif()
    endif()

    if(_qsqlite_src)
        file(MAKE_DIRECTORY "${DEST_APP}/Contents/PlugIns/sqldrivers")
        file(COPY "${_qsqlite_src}" DESTINATION "${DEST_APP}/Contents/PlugIns/sqldrivers")
        message(STATUS "StageAssistant: staged QSQLITE plugin: ${_qsqlite_src}")
    else()
        message(WARNING "StageAssistant: could not locate libqsqlite.dylib; QtHelp search/index may not work")
    endif()

    # If the qsqlite plugin depends on libsqlite3.dylib, stage it into Frameworks.
    # We copy from vcpkg if we can infer its lib dir.
    if(EXISTS "${DEST_APP}/Contents/PlugIns/sqldrivers/libqsqlite.dylib")
        # try: triplet_root/lib/libsqlite3*.dylib
        set(_sqlite_src "")
        _stageassistant_guess_qt6_plugins_from_macdeployqt("${MACDEPLOYQT}" _plugins_root2)
        if(_plugins_root2)
            get_filename_component(_qt6_dir2 "${_plugins_root2}" DIRECTORY) # .../Qt6
            get_filename_component(_triplet_root2 "${_qt6_dir2}" DIRECTORY)  # .../<triplet>
            file(GLOB _sqlite_candidates "${_triplet_root2}/lib/libsqlite3*.dylib")
            list(LENGTH _sqlite_candidates _n_sql)
            if(_n_sql GREATER 0)
                list(GET _sqlite_candidates 0 _sqlite_src)
            endif()
        endif()

        if(_sqlite_src)
            file(MAKE_DIRECTORY "${DEST_APP}/Contents/Frameworks")
            file(COPY "${_sqlite_src}" DESTINATION "${DEST_APP}/Contents/Frameworks")
            message(STATUS "StageAssistant: staged sqlite runtime: ${_sqlite_src}")
        endif()
    endif()

    # Re-run link creation (in case sqlite copy or other libs were added)
    _stageassistant_make_qt_soname_links("${DEST_APP}/Contents/Frameworks")

    # Final sanity check (non-fatal): ensure plugins exist
    if(NOT EXISTS "${DEST_APP}/Contents/PlugIns/platforms/libqcocoa.dylib")
        message(WARNING "StageAssistant: libqcocoa.dylib missing after macdeployqt")
    endif()
    if(NOT EXISTS "${DEST_APP}/Contents/PlugIns/sqldrivers/libqsqlite.dylib")
        message(WARNING "StageAssistant: libqsqlite.dylib missing after staging")
    endif()

    message(STATUS "StageAssistant: macdeployqt deploy complete")

    # ---------------------------------------------------------------------------
    # Strategy B: vcpkg fallback — rpath patch + minimal plugin support
    # ---------------------------------------------------------------------------
else()
    message(STATUS "StageAssistant: MACDEPLOYQT not available, using vcpkg rpath fallback")
    message(STATUS "StageAssistant: Assistant.app will depend on vcpkg libs at runtime (not self-contained)")

    if(DEFINED QT_LIB_DIR AND QT_LIB_DIR AND EXISTS "${QT_LIB_DIR}")
        execute_process(COMMAND /usr/bin/otool -l "${DST_BIN}" OUTPUT_VARIABLE _OTOOL OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(NOT _OTOOL MATCHES "${QT_LIB_DIR}")
            execute_process(
                    COMMAND /usr/bin/install_name_tool -add_rpath "${QT_LIB_DIR}" "${DST_BIN}"
                    RESULT_VARIABLE _RPATH_RC
                    ERROR_VARIABLE  _RPATH_ERR
            )
            if(_RPATH_RC EQUAL 0)
                message(STATUS "StageAssistant: added rpath ${QT_LIB_DIR}")
            else()
                message(WARNING "StageAssistant: failed to add rpath: ${_RPATH_ERR}")
            endif()
        endif()
    endif()

    file(MAKE_DIRECTORY "${DEST_APP}/Contents/Resources")
    file(WRITE "${DEST_APP}/Contents/Resources/qt.conf" "[Paths]\nPlugins=PlugIns\n")

    if(DEFINED QSQLITE_PLUGIN AND EXISTS "${QSQLITE_PLUGIN}")
        file(MAKE_DIRECTORY "${DEST_APP}/Contents/PlugIns/sqldrivers")
        file(COPY "${QSQLITE_PLUGIN}" DESTINATION "${DEST_APP}/Contents/PlugIns/sqldrivers")
    endif()
endif()

execute_process(
        COMMAND install_name_tool
        -add_rpath "@executable_path/../Frameworks"
        "${DEST_APP}/Contents/MacOS/Assistant"
        ERROR_QUIET
)