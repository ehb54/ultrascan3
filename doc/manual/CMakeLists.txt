# UltraScan III Documentation Build (Sphinx)
cmake_minimum_required(VERSION 3.10)

# ---------------------------------------------------------------------------
# Find required tools
# ---------------------------------------------------------------------------
find_program(SPHINX_EXECUTABLE NAMES sphinx-build
    HINTS "${PROJECT_SOURCE_DIR}/venv/bin" # Check local virtual environment first
    "${PROJECT_SOURCE_DIR}/venv/Scripts" # Windows virtual environment
    "${PROJECT_SOURCE_DIR}/.venv/Scripts" # Alternative virtual environment location
    "${PROJECT_SOURCE_DIR}/.venv/bin"
    DOC "Sphinx documentation builder")

if(NOT SPHINX_EXECUTABLE)
    message(STATUS "Documentation build disabled - sphinx-build not found")
    message(STATUS "  Install with: pip install -r doc/manual/source/requirements.txt")
    return()
endif()

message(STATUS "Documentation build enabled")
message(STATUS "  sphinx-build: ${SPHINX_EXECUTABLE}")

if(QHELPGENERATOR_EXECUTABLE)
    message(STATUS "  qhelpgenerator: ${QHELPGENERATOR_EXECUTABLE}")
else()
    message(STATUS "  qhelpgenerator: NOT FOUND - HTML output only, Qt help files will not be built")
endif()

# ---------------------------------------------------------------------------
# Directories
# ---------------------------------------------------------------------------
set(DOC_SOURCE_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/source)
set(DOC_BUILD_DIR   ${CMAKE_CURRENT_BINARY_DIR}/sphinx)
set(DOC_QTHELP_DIR  ${DOC_BUILD_DIR}/qthelp)
set(DOC_OUTPUT_DIR  ${CMAKE_BINARY_DIR}/bin)

# Project name as used in conf.py - determines output filenames
# Must match qthelp_basename in source/conf.py
set(SPHINX_PROJECT "manual")

# ---------------------------------------------------------------------------
# Collect source files for proper dependency tracking
# ---------------------------------------------------------------------------
file(GLOB_RECURSE RST_FILES "${DOC_SOURCE_DIR}/*.rst")
file(GLOB_RECURSE STATIC_FILES "${DOC_SOURCE_DIR}/_static/*")

# ---------------------------------------------------------------------------
# Step 1: Run sphinx-build with the qthelp builder
# ---------------------------------------------------------------------------
set(SPHINX_QHP  ${DOC_QTHELP_DIR}/${SPHINX_PROJECT}.qhp)
set(SPHINX_QHCP ${DOC_QTHELP_DIR}/${SPHINX_PROJECT}.qhcp)

add_custom_command(
        OUTPUT  "${SPHINX_QHP}" "${SPHINX_QHCP}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${DOC_QTHELP_DIR}"
        COMMAND "${SPHINX_EXECUTABLE}"
        -b qthelp
        -d "${DOC_BUILD_DIR}/doctrees"
        "${DOC_SOURCE_DIR}"
        "${DOC_QTHELP_DIR}"
        DEPENDS
        ${RST_FILES}
        ${STATIC_FILES}
        "${DOC_SOURCE_DIR}/conf.py"
        "${DOC_SOURCE_DIR}/auto_index.py"
        COMMENT "Building Sphinx qthelp output"
        VERBATIM
)

# ---------------------------------------------------------------------------
# Step 2: Compile QtHelp project files into runtime help files
#
#   manual.qhp   -> manual.qch   (compiled help file + full-text search index)
#   manual.qhcp  -> manual.qhc   (Qt help collection database)
#
# Qt6 merged qcollectiongenerator into qhelpgenerator. qhelpgenerator
# handles both .qhp and .qhcp compilation on all supported platforms.
#
# The .qch and .qhc files are generated in the qthelp build directory where
# the HTML files live. They are then copied into the application's runtime
# directory (bin/) where Qt Assistant loads them.
#
# IMPORTANT
# ---------
# Qt Assistant loads the help collection using:
#
#   Assistant -collectionFile <path>/manual.qhc
#
# As long as manual.qch and manual.qhc reside in the same directory at
# runtime, Qt resolves the reference correctly. Therefore we avoid embedding
# build-tree paths and simply keep both files together in bin/.
# ---------------------------------------------------------------------------

# Staging paths inside the qthelp build directory
set(DOC_QCH_STAGE ${DOC_QTHELP_DIR}/manual.qch)
set(DOC_QHC_STAGE ${DOC_QTHELP_DIR}/manual.qhc)

# Final runtime paths (UltraScan bin/)
set(DOC_QCH ${DOC_OUTPUT_DIR}/manual.qch)
set(DOC_QHC ${DOC_OUTPUT_DIR}/manual.qhc)

# Windows runtime environment for qhelpgenerator
set(_QT_TOOL_BIN_DIR "")
set(_QT_RUNTIME_BIN_DIR "")
set(_QT_PLUGIN_DIR "")

if(WIN32)
    foreach(_vcpkg_dir IN ITEMS "${VCPKG_INSTALLED_DIR}" "${_VCPKG_INSTALLED_DIR}")
        if(_vcpkg_dir AND VCPKG_TARGET_TRIPLET)
            foreach(_triplet IN ITEMS "${VCPKG_TARGET_TRIPLET}")
                if(NOT _QT_TOOL_BIN_DIR AND EXISTS "${_vcpkg_dir}/${_triplet}/tools/Qt6/bin/qhelpgenerator.exe")
                    set(_QT_TOOL_BIN_DIR "${_vcpkg_dir}/${_triplet}/tools/Qt6/bin")
                endif()

                if(NOT _QT_RUNTIME_BIN_DIR AND EXISTS "${_vcpkg_dir}/${_triplet}/bin/Qt6Core.dll")
                    set(_QT_RUNTIME_BIN_DIR "${_vcpkg_dir}/${_triplet}/bin")
                endif()

                if(NOT _QT_PLUGIN_DIR AND EXISTS "${_vcpkg_dir}/${_triplet}/plugins/sqldrivers/qsqlite.dll")
                    set(_QT_PLUGIN_DIR "${_vcpkg_dir}/${_triplet}/plugins")
                endif()
            endforeach()
        endif()
    endforeach()

    message(STATUS "  qhelpgenerator tool dir: ${_QT_TOOL_BIN_DIR}")
    message(STATUS "  Qt runtime bin dir:      ${_QT_RUNTIME_BIN_DIR}")
    message(STATUS "  Qt plugin dir:           ${_QT_PLUGIN_DIR}")
endif()

if(QHELPGENERATOR_EXECUTABLE)

    # -----------------------------------------------------------------------
    # Step 2a: Compile manual.qhp → manual.qch
    #
    # qhelpgenerator builds the compiled help file and the full-text search
    # index used by Qt Assistant.
    # -----------------------------------------------------------------------
    if(WIN32)
        add_custom_command(
                OUTPUT  ${DOC_QCH_STAGE}
                COMMAND ${CMAKE_COMMAND} -E env
                "PATH=${_QT_RUNTIME_BIN_DIR};${_QT_TOOL_BIN_DIR};$ENV{PATH}"
                "QT_PLUGIN_PATH=${_QT_PLUGIN_DIR}"
                ${QHELPGENERATOR_EXECUTABLE} ${SPHINX_PROJECT}.qhp -o manual.qch
                DEPENDS ${SPHINX_QHP}
                WORKING_DIRECTORY ${DOC_QTHELP_DIR}
                COMMENT "Compiling Qt help file (manual.qch)"
                VERBATIM
        )
    else()
        add_custom_command(
                OUTPUT  ${DOC_QCH_STAGE}
                COMMAND ${QHELPGENERATOR_EXECUTABLE} ${SPHINX_PROJECT}.qhp -o manual.qch
                DEPENDS ${SPHINX_QHP}
                WORKING_DIRECTORY ${DOC_QTHELP_DIR}
                COMMENT "Compiling Qt help file (manual.qch)"
                VERBATIM
        )
    endif()

    # -----------------------------------------------------------------------
    # Step 2b: Compile manual.qhcp → manual.qhc
    #
    # qhelpgenerator handles both .qhp and .qhcp compilation (Qt6 merged
    # qcollectiongenerator into qhelpgenerator).
    # -----------------------------------------------------------------------
    if(WIN32)
        add_custom_command(
                OUTPUT  ${DOC_QHC_STAGE}
                COMMAND ${CMAKE_COMMAND} -E env
                "PATH=${_QT_RUNTIME_BIN_DIR};${_QT_TOOL_BIN_DIR};$ENV{PATH}"
                "QT_PLUGIN_PATH=${_QT_PLUGIN_DIR}"
                ${QHELPGENERATOR_EXECUTABLE} ${SPHINX_PROJECT}.qhcp -o manual.qhc
                DEPENDS ${SPHINX_QHCP} ${DOC_QCH_STAGE}
                WORKING_DIRECTORY ${DOC_QTHELP_DIR}
                COMMENT "Compiling Qt help collection (manual.qhc)"
                VERBATIM
        )
    else()
        add_custom_command(
                OUTPUT  ${DOC_QHC_STAGE}
                COMMAND ${QHELPGENERATOR_EXECUTABLE} ${SPHINX_PROJECT}.qhcp -o manual.qhc
                DEPENDS ${SPHINX_QHCP} ${DOC_QCH_STAGE}
                WORKING_DIRECTORY ${DOC_QTHELP_DIR}
                COMMENT "Compiling Qt help collection (manual.qhc)"
                VERBATIM
        )
    endif()

    # -----------------------------------------------------------------------
    # Step 2c: Copy runtime help files into bin/
    #
    # UltraScan launches Qt Assistant pointing at bin/manual.qhc, which then
    # loads the associated manual.qch from the same directory.
    # -----------------------------------------------------------------------
    add_custom_command(
            OUTPUT  ${DOC_QCH} ${DOC_QHC}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${DOC_OUTPUT_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DOC_QCH_STAGE} ${DOC_QCH}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DOC_QHC_STAGE} ${DOC_QHC}
            DEPENDS ${DOC_QCH_STAGE} ${DOC_QHC_STAGE}
            COMMENT "Copying Qt help files to bin/"
            VERBATIM
    )

    add_custom_target(documentation ALL
            DEPENDS ${DOC_QCH} ${DOC_QHC}
    )

    install(FILES ${DOC_QCH} ${DOC_QHC}
            DESTINATION bin
            COMPONENT documentation
    )

else()

    # qhelpgenerator not available — still build the qthelp sources so the
    # documentation tree can be inspected manually.
    add_custom_target(documentation ALL
            DEPENDS ${SPHINX_QHP}
    )

    message(STATUS "  Qt help install targets skipped (qhelpgenerator not found)")

endif()

# ---------------------------------------------------------------------------
# Stage Qt Assistant
# ---------------------------------------------------------------------------
if(APPLE AND QHELPGENERATOR_EXECUTABLE)
    # Build candidate list.
    # vcpkg is preferred — sqlite3[fts5] in vcpkg.json ensures FTS indexing works.
    # Homebrew is the fallback for machines without vcpkg Qt5 tools installed.
    set(_ASSISTANT_CANDIDATES "")

    # 1. vcpkg-built Assistant
    foreach(_vcpkg_dir IN ITEMS "${VCPKG_INSTALLED_DIR}" "${_VCPKG_INSTALLED_DIR}")
        if(_vcpkg_dir AND VCPKG_TARGET_TRIPLET)
            string(REGEX REPLACE "-dynamic$" "" _vcpkg_host_triplet "${VCPKG_TARGET_TRIPLET}")
            foreach(_triplet IN ITEMS "${VCPKG_TARGET_TRIPLET}" "${_vcpkg_host_triplet}")
                list(APPEND _ASSISTANT_CANDIDATES
                    "${_vcpkg_dir}/${_triplet}/tools/qttools/bin/Assistant.app"   # Qt6
                    "${_vcpkg_dir}/${_triplet}/tools/qt5-tools/bin/Assistant.app" # Qt5
                    "${_vcpkg_dir}/${_triplet}/tools/qt5/bin/Assistant.app"       # Qt5 alt
                )
            endforeach()
        endif()
    endforeach()

    # 2. Homebrew fallback — for machines without vcpkg Qt5 tools
    list(APPEND _ASSISTANT_CANDIDATES
        /opt/homebrew/opt/qt@5/libexec/Assistant.app
        /opt/homebrew/opt/qt/libexec/Assistant.app
        /usr/local/opt/qt@5/libexec/Assistant.app
        /usr/local/opt/qt/libexec/Assistant.app
    )

    # Allow explicit override (e.g. point to a known-good Assistant.app)
    set(QT_ASSISTANT_OVERRIDE "" CACHE PATH "Override path to Assistant.app (use to pin a specific Qt version)")

    set(ASSISTANT_SOURCE "")
    if(QT_ASSISTANT_OVERRIDE AND EXISTS "${QT_ASSISTANT_OVERRIDE}")
        set(ASSISTANT_SOURCE "${QT_ASSISTANT_OVERRIDE}")
        message(STATUS "Using overridden Qt Assistant: ${ASSISTANT_SOURCE}")
    else()
        foreach(candidate IN LISTS _ASSISTANT_CANDIDATES)
            if(EXISTS "${candidate}")
                set(ASSISTANT_SOURCE "${candidate}")
                break()
            endif()
        endforeach()
    endif()

    # Locate the QSQLITE plugin — required for Assistant to read .qhc files.
    # For Homebrew Assistant, use the Homebrew SQLite plugin (same Qt build).
    # For vcpkg Assistant, use the vcpkg SQLite plugin.
    set(_QSQLITE_PLUGIN "")
    if(ASSISTANT_SOURCE MATCHES "/opt/homebrew|/usr/local/opt")
        # Homebrew Assistant — find Homebrew's own SQLite plugin
        foreach(_hb_qt IN ITEMS qt@5 qt)
            foreach(_hb_prefix IN ITEMS /opt/homebrew /usr/local)
                set(_candidate "${_hb_prefix}/opt/${_hb_qt}/plugins/sqldrivers/libqsqlite.dylib")
                if(EXISTS "${_candidate}")
                    set(_QSQLITE_PLUGIN "${_candidate}")
                    break()
                endif()
            endforeach()
            if(_QSQLITE_PLUGIN)
                break()
            endif()
        endforeach()
    endif()
    if(NOT _QSQLITE_PLUGIN)
        # vcpkg fallback — Qt6 installs plugins under Qt6/plugins/, Qt5 uses plugins/ directly
        foreach(_vcpkg_dir IN ITEMS "${VCPKG_INSTALLED_DIR}" "${_VCPKG_INSTALLED_DIR}")
            if(_vcpkg_dir AND VCPKG_TARGET_TRIPLET)
                foreach(_plugin_subdir IN ITEMS "Qt6/plugins" "plugins")
                    set(_candidate "${_vcpkg_dir}/${VCPKG_TARGET_TRIPLET}/${_plugin_subdir}/sqldrivers/libqsqlite.dylib")
                    if(EXISTS "${_candidate}")
                        set(_QSQLITE_PLUGIN "${_candidate}")
                        break()
                    endif()
                endforeach()
                if(_QSQLITE_PLUGIN)
                    break()
                endif()
            endif()
        endforeach()
    endif()

    if(ASSISTANT_SOURCE)
        message(STATUS "Found Qt Assistant: ${ASSISTANT_SOURCE}")

        # Find macdeployqt matching the Assistant source.
        # For Homebrew Assistant, use Homebrew's macdeployqt (same Qt build).
        # For vcpkg Assistant, use vcpkg's macdeployqt (though FTS will not work).
        set(_MACDEPLOYQT "")
        if(ASSISTANT_SOURCE MATCHES "/opt/homebrew")
            set(_MACDEPLOYQT "/opt/homebrew/opt/qt@5/bin/macdeployqt")
            if(NOT EXISTS "${_MACDEPLOYQT}")
                set(_MACDEPLOYQT "/opt/homebrew/opt/qt/bin/macdeployqt")
            endif()
        elseif(ASSISTANT_SOURCE MATCHES "/usr/local/opt")
            set(_MACDEPLOYQT "/usr/local/opt/qt@5/bin/macdeployqt")
            if(NOT EXISTS "${_MACDEPLOYQT}")
                set(_MACDEPLOYQT "/usr/local/opt/qt/bin/macdeployqt")
            endif()
        else()
            # vcpkg Assistant — use the top-level MACDEPLOYQT_EXECUTABLE which
            # is the build-tree wrapper (Option A relocation) with a qt.conf that
            # points at the dynamic triplet.  This is what macdeployqt needs to
            # find .dylib files rather than .a archives from the static triplet.
            # FTS works because sqlite3[fts5] is in vcpkg.json.
            if(MACDEPLOYQT_EXECUTABLE AND EXISTS "${MACDEPLOYQT_EXECUTABLE}")
                set(_MACDEPLOYQT "${MACDEPLOYQT_EXECUTABLE}")
            endif()
        endif()

        if(_MACDEPLOYQT AND EXISTS "${_MACDEPLOYQT}")
            message(STATUS "Found macdeployqt: ${_MACDEPLOYQT}")
        else()
            message(WARNING "macdeployqt not found — Assistant will use rpath fallback (FTS may not work)")
            set(_MACDEPLOYQT "")
        endif()

        # vcpkg lib dir and QSQLITE — only needed for the rpath fallback path
        # Qt6 vcpkg installs plugins under Qt6/plugins/, Qt5 uses plugins/ directly
        set(_QT_LIB_DIR "")
        set(_QSQLITE_PLUGIN_ARG "")
        if(NOT _MACDEPLOYQT)
            foreach(_vcpkg_dir IN ITEMS "${VCPKG_INSTALLED_DIR}" "${_VCPKG_INSTALLED_DIR}")
                if(_vcpkg_dir AND VCPKG_TARGET_TRIPLET)
                    set(_candidate "${_vcpkg_dir}/${VCPKG_TARGET_TRIPLET}/lib")
                    if(EXISTS "${_candidate}")
                        set(_QT_LIB_DIR "${_candidate}")
                    endif()
                    foreach(_plugin_subdir IN ITEMS "Qt6/plugins" "plugins")
                        set(_candidate "${_vcpkg_dir}/${VCPKG_TARGET_TRIPLET}/${_plugin_subdir}/sqldrivers/libqsqlite.dylib")
                        if(EXISTS "${_candidate}")
                            set(_QSQLITE_PLUGIN_ARG "${_candidate}")
                            break()
                        endif()
                    endforeach()
                endif()
            endforeach()
        endif()

        add_custom_target(stage_qt_assistant ALL
            COMMAND ${CMAKE_COMMAND}
                -DASSISTANT_SOURCE=${ASSISTANT_SOURCE}
                -DDEST_DIR=${DOC_OUTPUT_DIR}
                -DMACDEPLOYQT=${_MACDEPLOYQT}
                -DQSQLITE_PLUGIN=${_QSQLITE_PLUGIN_ARG}
                -DQT_LIB_DIR=${_QT_LIB_DIR}
                -P ${CMAKE_SOURCE_DIR}/admin/cmake/packaging/macos/MacOsDeployQtAssistant.cmake
            COMMENT "Staging self-contained Assistant.app into ${DOC_OUTPUT_DIR}"
        )
        add_dependencies(documentation stage_qt_assistant)
    else()
        message(WARNING "Qt Assistant not found — help viewer will not be available in bin/")
    endif()
endif()

if(WIN32 AND QHELPGENERATOR_EXECUTABLE)
    set(DOC_QCH_STAGE "${DOC_QCH_STAGE}" PARENT_SCOPE)
    set(DOC_QHC_STAGE "${DOC_QHC_STAGE}" PARENT_SCOPE)
    set(DOC_OUTPUT_DIR "${DOC_OUTPUT_DIR}" PARENT_SCOPE)
endif()

# ---------------------------------------------------------------------------
# Clean target
# ---------------------------------------------------------------------------
add_custom_target(clean-documentation
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${DOC_BUILD_DIR}
        COMMAND ${CMAKE_COMMAND} -E remove -f ${DOC_OUTPUT_DIR}/manual.qch
        COMMAND ${CMAKE_COMMAND} -E remove -f ${DOC_OUTPUT_DIR}/manual.qhc
        COMMENT "Cleaning documentation build files"
)
