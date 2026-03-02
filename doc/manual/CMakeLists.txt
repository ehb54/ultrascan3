# UltraScan III Documentation Build (Sphinx)
cmake_minimum_required(VERSION 3.10)

option(BUILD_DOCUMENTATION "Build Sphinx documentation" ON)
if(NOT BUILD_DOCUMENTATION)
    message(STATUS "Documentation build skipped (BUILD_DOCUMENTATION=OFF)")
    return()
endif()

# ---------------------------------------------------------------------------
# Find required tools
# ---------------------------------------------------------------------------
find_program(SPHINX_EXECUTABLE NAMES sphinx-build
        DOC "Sphinx documentation builder")

# qhelpgenerator ships with Qt tools (qttools for Qt6, qt5-tools for Qt5).
# Build a list of candidate hint paths that work regardless of where vcpkg
# is installed on any given machine or platform.
set(_QHELPGEN_HINTS "")

# 1. vcpkg-provided paths - derived from toolchain variables so they
#    resolve correctly for every developer and every CI machine.
foreach(_vcpkg_dir IN ITEMS
        "${VCPKG_INSTALLED_DIR}"
        "${_VCPKG_INSTALLED_DIR}")
    if(_vcpkg_dir AND VCPKG_TARGET_TRIPLET)
        # Strip -dynamic suffix to also check the static/default triplet where
        # host tools like qhelpgenerator are installed by vcpkg
        string(REGEX REPLACE "-dynamic$" "" _vcpkg_host_triplet "${VCPKG_TARGET_TRIPLET}")
        foreach(_triplet IN ITEMS "${VCPKG_TARGET_TRIPLET}" "${_vcpkg_host_triplet}")
            list(APPEND _QHELPGEN_HINTS
                    "${_vcpkg_dir}/${_triplet}/tools/qttools/bin"    # Qt6 (Assistant.app)
                    "${_vcpkg_dir}/${_triplet}/tools/Qt6/bin"        # Qt6 (qhelpgenerator)
                    "${_vcpkg_dir}/${_triplet}/tools/qt5-tools/bin" # Qt5
            )
        endforeach()
    endif()
endforeach()

# 2. Qt CMake dir relative paths (covers Homebrew, official Qt installer, etc.)
foreach(_qt_dir IN ITEMS "${Qt6_DIR}" "${Qt5_DIR}")
    if(_qt_dir)
        list(APPEND _QHELPGEN_HINTS "${_qt_dir}/../../../bin")
    endif()
endforeach()

# 3. Common system locations per platform
if(APPLE)
    list(APPEND _QHELPGEN_HINTS
            /opt/homebrew/opt/qt/bin        # Homebrew Qt6 (Apple Silicon)
            /opt/homebrew/opt/qt@5/bin      # Homebrew Qt5 (Apple Silicon)
            /usr/local/opt/qt/bin           # Homebrew Qt6 (Intel)
            /usr/local/opt/qt@5/bin         # Homebrew Qt5 (Intel)
    )
elseif(UNIX)
    list(APPEND _QHELPGEN_HINTS
            /usr/lib/qt6/bin
            /usr/lib/qt5/bin
            /usr/lib/x86_64-linux-gnu/qt5/bin
            /usr/lib/aarch64-linux-gnu/qt5/bin
    )
elseif(WIN32)
    list(APPEND _QHELPGEN_HINTS
            "$ENV{QTDIR}/bin"
    )
endif()

find_program(QHELPGENERATOR_EXECUTABLE NAMES qhelpgenerator
        HINTS ${_QHELPGEN_HINTS}
        DOC "Qt Help Generator executable")

find_program(QCOLLECTIONGENERATOR_EXECUTABLE NAMES qcollectiongenerator
        HINTS ${_QHELPGEN_HINTS}
        DOC "Qt Collection Generator executable")

if(NOT SPHINX_EXECUTABLE)
    message(STATUS "Documentation build disabled - sphinx-build not found")
    message(STATUS "  Install with: pip install sphinx sphinxcontrib-qthelp sphinx-subfigure")
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
        OUTPUT  ${SPHINX_QHP} ${SPHINX_QHCP}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${DOC_QTHELP_DIR}
        COMMAND ${SPHINX_EXECUTABLE}
                    -b qthelp
                    -d ${DOC_BUILD_DIR}/doctrees
                    ${DOC_SOURCE_DIR}
                    ${DOC_QTHELP_DIR}
        # Sphinx's qthelp builder ignores qthelp_virtual_folder for the .qhcp
        # homepage URL, hardcoding 'doc'. Patch BOTH .qhcp and .qhp to use
        # 'manual' to match the URL scheme us_helpdaemon uses:
        #   qthelp://ultrascaniii/manual/<page>

        COMMAND sed -i.bak
        "s|qthelp://ultrascaniii/doc/|qthelp://ultrascaniii/manual/|g"
        ${SPHINX_QHCP}
        COMMAND ${CMAKE_COMMAND} -E remove -f ${SPHINX_QHCP}.bak

        COMMAND sed -i.bak
        "s|<virtualFolder>doc</virtualFolder>|<virtualFolder>manual</virtualFolder>|g"
        ${SPHINX_QHP}
        COMMAND ${CMAKE_COMMAND} -E remove -f ${SPHINX_QHP}.bak

        # Remove empty <filterAttribute></filterAttribute> tags generated by Sphinx
        COMMAND sed -i.bak
        "\#<filterAttribute></filterAttribute>#d"
        ${SPHINX_QHP}
        COMMAND ${CMAKE_COMMAND} -E remove -f ${SPHINX_QHP}.bak

        COMMAND sed -i.bak
        "\#<filterAttribute></filterAttribute>#d"
        ${SPHINX_QHCP}
        COMMAND ${CMAKE_COMMAND} -E remove -f ${SPHINX_QHCP}.bak

        COMMAND sed -i.bak
        -e "\#<filterAttribute></filterAttribute>#d"
        -e "\#^[[:space:]]*<filterAttribute>[[:space:]]*</filterAttribute>[[:space:]]*$#d"
        ${SPHINX_QHCP}
        COMMAND ${CMAKE_COMMAND} -E remove -f ${SPHINX_QHCP}.bak

        COMMAND sed -i.bak
        -e "\#<filterAttribute></filterAttribute>#d"
        -e "\#^[[:space:]]*<filterAttribute>[[:space:]]*</filterAttribute>[[:space:]]*$#d"
        ${SPHINX_QHP}
        COMMAND ${CMAKE_COMMAND} -E remove -f ${SPHINX_QHP}.bak

        # Enable full-text search in Qt Assistant (BSD/GNU sed portable newline)
        COMMAND sed -i.bak
        -e "s|</assistant>|<enableFullTextSearchFallback>true</enableFullTextSearchFallback>\\
    </assistant>|g"
        ${SPHINX_QHCP}
        COMMAND ${CMAKE_COMMAND} -E remove -f ${SPHINX_QHCP}.bak

        DEPENDS
        ${RST_FILES}
        ${STATIC_FILES}
        ${DOC_SOURCE_DIR}/conf.py
        COMMENT "Building Sphinx qthelp output"
        VERBATIM
)

# ---------------------------------------------------------------------------
# Step 2: Compile .qhp/.qhcp into .qch/.qhc
#   - .qch: use qhelpgenerator (+ full-text search index)
#   - .qhc: use qcollectiongenerator (collection file)
# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------
# Step 2: Compile .qhp/.qhcp into .qch/.qhc
#   - .qch is sufficient for Qt Assistant Contents/Index/Search (load .qch directly)
#   - .qhc (collection) is optional and requires qcollectiongenerator
# ---------------------------------------------------------------------------

# Stage files: generate .qch and .qhc in the qthelp dir where all HTML/image
# files live (qhelpgenerator resolves file paths relative to the .qhp location).
# Then copy both to bin/ for runtime use.
set(DOC_QCH_STAGE ${DOC_QTHELP_DIR}/manual.qch)
set(DOC_QHC_STAGE ${DOC_QTHELP_DIR}/manual.qhc)
set(DOC_QCH ${DOC_OUTPUT_DIR}/manual.qch)
set(DOC_QHC ${DOC_OUTPUT_DIR}/manual.qhc)

if(QHELPGENERATOR_EXECUTABLE)

    # Step 2a: compile .qhp -> .qch in the qthelp dir (HTML files live here).
    # Output is manual.qch in the same dir so the .qhcp relative reference works.
    add_custom_command(
            OUTPUT  ${DOC_QCH_STAGE}
            COMMAND ${QHELPGENERATOR_EXECUTABLE} manual.qhp -o manual.qch
            DEPENDS ${SPHINX_QHP}
            WORKING_DIRECTORY ${DOC_QTHELP_DIR}
            COMMENT "Compiling Qt help file (manual.qch)"
            VERBATIM
    )

    # Step 2b: compile .qhcp -> .qhc using an absolute path to the QCH.
    # Qt Assistant copies the QHC to ~/Library/Application Support/QtProject/Assistant/
    # at runtime and re-resolves the QCH path relative to that new location.
    # A bare 'manual.qch' becomes a broken relative path like ../../../../jetbrains/...
    # because the QCH is not in the app-data dir. Registering the QCH with its
    # absolute bin/ path ensures the NamespaceTable entry survives the copy.
    #
    # Strategy: generate a patched .qhcp that references the QCH by its final
    # absolute path in bin/, run qcollectiongenerator from a temp dir so the
    # relative-path logic in the generator doesn't interfere, then copy to bin/.
    if(QCOLLECTIONGENERATOR_EXECUTABLE)
        set(_QHC_GENERATOR ${QCOLLECTIONGENERATOR_EXECUTABLE})
        message(STATUS "  qcollectiongenerator: ${QCOLLECTIONGENERATOR_EXECUTABLE}")
    else()
        set(_QHC_GENERATOR ${QHELPGENERATOR_EXECUTABLE})
        message(STATUS "  qcollectiongenerator not found - using qhelpgenerator for .qhc (Qt6 merged tool)")
    endif()

    add_custom_command(
            OUTPUT  ${DOC_QHC_STAGE}
            # Generate the QHC normally (qcollectiongenerator always relativizes paths).
            COMMAND ${_QHC_GENERATOR} manual.qhcp -o manual.qhc
            # Post-process: rewrite NamespaceTable to store just the bare filename
            # 'manual.qch' with no path component.  Qt Assistant resolves QCH paths
            # relative to the QHC file's own directory at runtime, so both files
            # must sit in the same directory (bin/).  A bare filename survives
            # installation to any location, whereas an absolute build-tree path
            # breaks as soon as the DMG is installed elsewhere.
            COMMAND sqlite3 manual.qhc
                "UPDATE NamespaceTable SET FilePath='manual.qch' WHERE Name='ultrascaniii';"
            DEPENDS ${SPHINX_QHCP} ${DOC_QCH_STAGE}
            WORKING_DIRECTORY ${DOC_QTHELP_DIR}
            COMMENT "Compiling Qt help collection (manual.qhc) and fixing QCH path to bare filename"
            VERBATIM
    )

    # Step 2c: copy QCH to bin/; QHC already has the absolute bin/ path embedded.
    add_custom_command(
            OUTPUT  ${DOC_QCH} ${DOC_QHC}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${DOC_OUTPUT_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy ${DOC_QCH_STAGE} ${DOC_QCH}
            COMMAND ${CMAKE_COMMAND} -E copy ${DOC_QHC_STAGE} ${DOC_QHC}
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

    # No qhelpgenerator - still build the qthelp source so the tree is usable
    add_custom_target(documentation ALL
            DEPENDS ${SPHINX_QHP}
    )
    message(STATUS "  Qt help install targets skipped (qhelpgenerator not found)")

endif()

# ---------------------------------------------------------------------------
# Stage Qt Assistant on macOS — real copy, self-contained for dev builds
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
                -P ${CMAKE_CURRENT_SOURCE_DIR}/StageAssistant.cmake
            COMMENT "Staging self-contained Assistant.app into ${DOC_OUTPUT_DIR}"
        )
        add_dependencies(documentation stage_qt_assistant)
    else()
        message(WARNING "Qt Assistant not found — help viewer will not be available in bin/")
    endif()
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
