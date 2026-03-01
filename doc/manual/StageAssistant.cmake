# StageAssistant.cmake  — called via  cmake -P  from the documentation target.
#
# Inputs (passed as -D variables):
#   ASSISTANT_SOURCE  — full path to the source Assistant.app
#   DEST_DIR          — directory that will contain Assistant.app  (build/.../bin)
#   MACDEPLOYQT       — full path to macdeployqt matching ASSISTANT_SOURCE's Qt build
#   QT_LIB_DIR        — vcpkg Qt lib dir (used only in fallback path when no MACDEPLOYQT)
#   QSQLITE_PLUGIN    — vcpkg libqsqlite.dylib (used only in fallback path)
#
# Strategy A (preferred): macdeployqt available
#   1. Deep-copy ASSISTANT_SOURCE -> DEST_DIR/Assistant.app
#   2. Run macdeployqt to bundle Qt frameworks + plugins into the app
#   3. Ad-hoc re-sign the entire bundle (macdeployqt invalidates the signature)
#   Result: fully self-contained bundle, FTS indexing works with framework Qt builds.
#
# Strategy B (fallback): no macdeployqt
#   1. Deep-copy ASSISTANT_SOURCE -> DEST_DIR/Assistant.app
#   2. Patch rpath to point to QT_LIB_DIR
#   3. Write qt.conf
#   4. Copy QSQLITE_PLUGIN and platform plugins manually
#   Result: depends on vcpkg tree at runtime; Qt5 flat-dylib FTS bug applies.

cmake_minimum_required(VERSION 3.15)

if(NOT ASSISTANT_SOURCE OR NOT EXISTS "${ASSISTANT_SOURCE}")
    message(FATAL_ERROR "ASSISTANT_SOURCE not set or does not exist: '${ASSISTANT_SOURCE}'")
endif()
if(NOT DEST_DIR)
    message(FATAL_ERROR "DEST_DIR not set")
endif()

set(DEST_APP "${DEST_DIR}/Assistant.app")
set(_SRC_BIN "${ASSISTANT_SOURCE}/Contents/MacOS/Assistant")
set(_DST_BIN "${DEST_APP}/Contents/MacOS/Assistant")

# ---------------------------------------------------------------------------
# 1. Deep-copy Assistant.app (only if source is newer or dest missing)
# ---------------------------------------------------------------------------
# Determine if we need to re-stage.
# A bundle is considered up to date only if:
#   1. The destination binary exists AND is not older than the source
#   2. The bundle has a Frameworks/ directory (i.e. macdeployqt has already run)
#      OR macdeployqt is not available (Strategy B, rpath-only path)
set(_NEEDS_COPY TRUE)
if(EXISTS "${_DST_BIN}")
    file(TIMESTAMP "${_SRC_BIN}" _ts_src "%s" UTC)
    file(TIMESTAMP "${_DST_BIN}" _ts_dst "%s" UTC)
    if(_ts_src AND _ts_dst AND NOT _ts_src STRGREATER _ts_dst)
        # Binary is up to date — but only skip if the bundle is also fully deployed
        if(MACDEPLOYQT AND EXISTS "${MACDEPLOYQT}")
            # Strategy A: only skip if Frameworks/ exists (macdeployqt has run)
            if(EXISTS "${DEST_APP}/Contents/Frameworks")
                set(_NEEDS_COPY FALSE)
            else()
                message(STATUS "StageAssistant: Frameworks/ missing, re-running macdeployqt")
            endif()
        else()
            # Strategy B: no macdeployqt, just timestamp check is sufficient
            set(_NEEDS_COPY FALSE)
        endif()
    endif()
endif()

if(_NEEDS_COPY)
    message(STATUS "StageAssistant: copying ${ASSISTANT_SOURCE} -> ${DEST_APP}")
    file(REMOVE_RECURSE "${DEST_APP}")
    file(COPY "${ASSISTANT_SOURCE}" DESTINATION "${DEST_DIR}")
else()
    message(STATUS "StageAssistant: ${DEST_APP} is up to date")
endif()

# ---------------------------------------------------------------------------
# Strategy A: macdeployqt available — self-contained framework bundle
# ---------------------------------------------------------------------------
if(MACDEPLOYQT AND EXISTS "${MACDEPLOYQT}")

    if(_NEEDS_COPY)
        message(STATUS "StageAssistant: running macdeployqt -> ${DEST_APP}")
        execute_process(
            COMMAND "${MACDEPLOYQT}" "${DEST_APP}"
            RESULT_VARIABLE _DEPLOY_RESULT
            ERROR_VARIABLE  _DEPLOY_ERR
        )
        if(_DEPLOY_RESULT EQUAL 0)
            message(STATUS "StageAssistant: macdeployqt succeeded")
        else()
            message(WARNING "StageAssistant: macdeployqt failed: ${_DEPLOY_ERR}")
        endif()

        # macdeployqt copies Qt libs into Frameworks/ but may leave the binary's
        # rpath pointing at the original vcpkg lib dir (e.g. @loader_path/../../../../../../lib).
        # Fix: remove all non-@executable_path/@loader_path rpaths and add the
        # correct one pointing at Contents/Frameworks/ relative to Contents/MacOS/.
        message(STATUS "StageAssistant: fixing rpaths on Assistant binary")
        execute_process(
            COMMAND bash -c
                "otool -l '${_DST_BIN}' | grep -A2 LC_RPATH | awk '/path /{print $2}'"
            OUTPUT_VARIABLE _EXISTING_RPATHS
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(_EXISTING_RPATHS)
            string(REPLACE "\n" ";" _RPATH_LIST "${_EXISTING_RPATHS}")
            foreach(_rp IN LISTS _RPATH_LIST)
                if(NOT _rp MATCHES "^@executable_path/../Frameworks")
                    message(STATUS "StageAssistant:   removing rpath: ${_rp}")
                    execute_process(
                        COMMAND install_name_tool -delete_rpath "${_rp}" "${_DST_BIN}"
                        ERROR_QUIET
                    )
                endif()
            endforeach()
        endif()
        execute_process(
            COMMAND install_name_tool -add_rpath "@executable_path/../Frameworks" "${_DST_BIN}"
            ERROR_QUIET
        )
        message(STATUS "StageAssistant:   added rpath: @executable_path/../Frameworks")

        # macdeployqt misses transitive dylib dependencies (e.g. libsharpyuv needed by libwebp).
        # Scan Frameworks/ for any dylib whose own dependencies are not yet present, and copy them in.
        file(GLOB _STAGED_FRAMEWORKS "${DEST_APP}/Contents/Frameworks/*.dylib")
        foreach(_dylib IN LISTS _STAGED_FRAMEWORKS)
            execute_process(
                COMMAND otool -L "${_dylib}"
                OUTPUT_VARIABLE _DEPS
                ERROR_QUIET
            )
            # Extract @rpath deps that are not yet in Frameworks/
            string(REGEX MATCHALL "@rpath/([^ ]+\.dylib)" _rpath_deps "${_DEPS}")
            foreach(_dep IN LISTS _rpath_deps)
                string(REGEX REPLACE "@rpath/" "" _depname "${_dep}")
                if(NOT EXISTS "${DEST_APP}/Contents/Frameworks/${_depname}")
                    # Search Homebrew lib for this dylib
                    foreach(_hb_prefix IN ITEMS /opt/homebrew /usr/local)
                        if(EXISTS "${_hb_prefix}/lib/${_depname}")
                            file(COPY "${_hb_prefix}/lib/${_depname}"
                                 DESTINATION "${DEST_APP}/Contents/Frameworks/")
                            message(STATUS "StageAssistant: fixed missing transitive dep ${_depname}")
                            break()
                        endif()
                    endforeach()
                endif()
            endforeach()
        endforeach()

        # macdeployqt invalidates the code signature — re-sign the whole bundle.
        execute_process(
            COMMAND codesign --force --deep --sign - "${DEST_APP}"
            RESULT_VARIABLE _SIGN_RESULT
            ERROR_VARIABLE  _SIGN_ERR
        )
        if(_SIGN_RESULT EQUAL 0)
            message(STATUS "StageAssistant: ad-hoc re-signed ${DEST_APP}")
        else()
            message(WARNING "StageAssistant: codesign failed: ${_SIGN_ERR}")
        endif()
    else()
        message(STATUS "StageAssistant: skipping macdeployqt (bundle up to date)")
    endif()

# ---------------------------------------------------------------------------
# Strategy B: vcpkg fallback — rpath patch + manual plugin copy
# ---------------------------------------------------------------------------
else()

    message(STATUS "StageAssistant: MACDEPLOYQT not available, using vcpkg rpath fallback")
    message(WARNING "StageAssistant: Qt5 flat-dylib builds have known FTS issues — install Homebrew qt@5 for working search")

    if(QT_LIB_DIR AND EXISTS "${QT_LIB_DIR}")
        execute_process(
            COMMAND otool -l "${_DST_BIN}"
            OUTPUT_VARIABLE _OTOOL_OUTPUT
            ERROR_QUIET
        )
        if(NOT _OTOOL_OUTPUT MATCHES "${QT_LIB_DIR}")
            execute_process(
                COMMAND install_name_tool -add_rpath "${QT_LIB_DIR}" "${_DST_BIN}"
                RESULT_VARIABLE _RPATH_RESULT
                ERROR_VARIABLE  _RPATH_ERR
            )
            if(_RPATH_RESULT EQUAL 0)
                message(STATUS "StageAssistant: added rpath ${QT_LIB_DIR}")
            else()
                message(WARNING "StageAssistant: install_name_tool failed: ${_RPATH_ERR}")
            endif()
            execute_process(
                COMMAND codesign --force --sign - "${_DST_BIN}"
                RESULT_VARIABLE _SIGN_RESULT
                ERROR_VARIABLE  _SIGN_ERR
            )
            if(_SIGN_RESULT EQUAL 0)
                message(STATUS "StageAssistant: ad-hoc re-signed ${_DST_BIN}")
            else()
                message(WARNING "StageAssistant: codesign failed: ${_SIGN_ERR}")
            endif()
        else()
            message(STATUS "StageAssistant: rpath ${QT_LIB_DIR} already present")
        endif()
    else()
        message(WARNING "StageAssistant: QT_LIB_DIR not set — Assistant may fail to find Qt libs")
    endif()

    # Write qt.conf
    file(MAKE_DIRECTORY "${DEST_APP}/Contents/Resources")
    file(WRITE "${DEST_APP}/Contents/Resources/qt.conf" "[Paths]\nPlugins = PlugIns\n")
    message(STATUS "StageAssistant: wrote qt.conf")

    # Copy QSQLITE plugin
    if(QSQLITE_PLUGIN AND EXISTS "${QSQLITE_PLUGIN}")
        set(_SQLDRIVERS "${DEST_APP}/Contents/PlugIns/sqldrivers")
        file(MAKE_DIRECTORY "${_SQLDRIVERS}")
        file(COPY "${QSQLITE_PLUGIN}" DESTINATION "${_SQLDRIVERS}")
        message(STATUS "StageAssistant: installed QSQLITE -> ${_SQLDRIVERS}/")
    else()
        message(WARNING "StageAssistant: QSQLITE_PLUGIN not provided — search may not work")
    endif()

    # Copy platform plugins
    if(QT_LIB_DIR)
        get_filename_component(_QT_ROOT "${QT_LIB_DIR}" DIRECTORY)
        set(_PLATFORMS_SRC "${_QT_ROOT}/plugins/platforms")
        if(EXISTS "${_PLATFORMS_SRC}")
            set(_PLATFORMS_DEST "${DEST_APP}/Contents/PlugIns/platforms")
            file(MAKE_DIRECTORY "${_PLATFORMS_DEST}")
            file(COPY "${_PLATFORMS_SRC}/" DESTINATION "${_PLATFORMS_DEST}")
            message(STATUS "StageAssistant: installed platform plugins -> ${_PLATFORMS_DEST}/")
        else()
            message(WARNING "StageAssistant: platform plugins not found at ${_PLATFORMS_SRC}")
        endif()
    endif()

endif()
