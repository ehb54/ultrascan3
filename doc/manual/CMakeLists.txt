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
        DEPENDS
                ${RST_FILES}
                ${STATIC_FILES}
                ${DOC_SOURCE_DIR}/conf.py
        COMMENT "Building Sphinx qthelp output"
        VERBATIM
)

# ---------------------------------------------------------------------------
# Step 2: Compile .qhp/.qhcp into .qch/.qhc (requires qhelpgenerator)
# ---------------------------------------------------------------------------
if(QHELPGENERATOR_EXECUTABLE)

    set(DOC_QCH ${DOC_OUTPUT_DIR}/manual.qch)
    set(DOC_QHC ${DOC_OUTPUT_DIR}/manual.qhc)

    add_custom_command(
            OUTPUT  ${DOC_QCH} ${DOC_QHC}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${DOC_OUTPUT_DIR}
            COMMAND ${QHELPGENERATOR_EXECUTABLE} ${SPHINX_QHP}  -o ${DOC_QCH}
            COMMAND ${QHELPGENERATOR_EXECUTABLE} ${SPHINX_QHCP} -o ${DOC_QHC}
            DEPENDS ${SPHINX_QHP} ${SPHINX_QHCP}
            COMMENT "Compiling Qt help files (manual.qch / manual.qhc)"
            VERBATIM
    )

    add_custom_target(documentation ALL
            DEPENDS ${DOC_QCH} ${DOC_QHC}
    )

    install(FILES ${DOC_QCH} ${DOC_QHC}
            DESTINATION bin
            COMPONENT documentation
    )

    if(APPLE)
        install(FILES ${DOC_QCH} ${DOC_QHC}
                DESTINATION UltraScan.app/Contents/Resources
                COMPONENT documentation
        )
    endif()

else()
    # No qhelpgenerator - still build the HTML/qthelp source so the tree is usable
    add_custom_target(documentation ALL
            DEPENDS ${SPHINX_QHP}
    )
    message(STATUS "  Qt help install targets skipped (qhelpgenerator not found)")
endif()

# ---------------------------------------------------------------------------
# Symlink Qt Assistant on macOS (same logic as before)
# ---------------------------------------------------------------------------
if(APPLE AND QHELPGENERATOR_EXECUTABLE)
    # Build candidate list using the same vcpkg variables used to find qhelpgenerator
    set(_ASSISTANT_CANDIDATES "")
    foreach(_vcpkg_dir IN ITEMS "${VCPKG_INSTALLED_DIR}" "${_VCPKG_INSTALLED_DIR}")
        if(_vcpkg_dir AND VCPKG_TARGET_TRIPLET)
            string(REGEX REPLACE "-dynamic$" "" _vcpkg_host_triplet "${VCPKG_TARGET_TRIPLET}")
            foreach(_triplet IN ITEMS "${VCPKG_TARGET_TRIPLET}" "${_vcpkg_host_triplet}")
                list(APPEND _ASSISTANT_CANDIDATES
                    "${_vcpkg_dir}/${_triplet}/tools/qttools/bin/Assistant.app"
                )
            endforeach()
        endif()
    endforeach()
    # Homebrew fallbacks
    list(APPEND _ASSISTANT_CANDIDATES
        /opt/homebrew/opt/qt/bin/Assistant.app
        /opt/homebrew/opt/qt@5/bin/Assistant.app
        /usr/local/opt/qt/bin/Assistant.app
    )
    set(ASSISTANT_SOURCE "")
    foreach(candidate IN LISTS _ASSISTANT_CANDIDATES)
        if(EXISTS "${candidate}")
            set(ASSISTANT_SOURCE "${candidate}")
            break()
        endif()
    endforeach()

    if(ASSISTANT_SOURCE)
        message(STATUS "Found Qt Assistant: ${ASSISTANT_SOURCE}")
        add_custom_target(link_qt_assistant ALL
                COMMAND ${CMAKE_COMMAND} -E make_directory "${DOC_OUTPUT_DIR}"
                COMMAND ${CMAKE_COMMAND} -E rm -f "${DOC_OUTPUT_DIR}/Assistant.app"
                COMMAND ${CMAKE_COMMAND} -E create_symlink "${ASSISTANT_SOURCE}" "${DOC_OUTPUT_DIR}/Assistant.app"
                COMMAND ${CMAKE_COMMAND} -E rm -f "${DOC_OUTPUT_DIR}/assistant"
                COMMAND ${CMAKE_COMMAND} -E create_symlink "Assistant.app/Contents/MacOS/Assistant" "${DOC_OUTPUT_DIR}/assistant"
                COMMENT "Symlinking Qt Assistant to ${DOC_OUTPUT_DIR}"
        )
        add_dependencies(documentation link_qt_assistant)
    else()
        message(WARNING "Qt Assistant not found - help viewer will not be available in bin/")
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
