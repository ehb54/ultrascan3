# Auto-generated CMakeLists.txt for us

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR})

set(US_SOURCES
        ${src_dir}/us_win_data.cpp
        ${src_dir}/us.cpp
)

# macOS app bundle icon
if(APPLE)
        list(APPEND US_SOURCES "${CMAKE_SOURCE_DIR}/etc/us3-icon.icns")
        set(MACOSX_BUNDLE_ICON_FILE us3-icon.icns)
        set_source_files_properties(
                "${CMAKE_SOURCE_DIR}/etc/us3-icon.icns"
                PROPERTIES MACOSX_PACKAGE_LOCATION "Resources"
        )
endif()

# Windows icon resource
if(WIN32)
        # Create us.rc if it doesn't exist
        if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/us.rc")
                file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/us.rc" "IDI_ICON1 ICON \"../../etc/us3-icon-128x128.ico\"\n")
        endif()
        list(APPEND US_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/us.rc")
endif()

# Create executable
add_executable(us ${US_SOURCES})

# macOS bundle properties
if(APPLE)
        set_target_properties(us PROPERTIES
                MACOSX_BUNDLE TRUE
                MACOSX_BUNDLE_ICON_FILE us3-icon.icns
                MACOSX_BUNDLE_GUI_IDENTIFIER "com.ultrascan3.us"
                MACOSX_BUNDLE_BUNDLE_NAME "UltraScan3"
                MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
                MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        )
endif()

target_link_libraries(us
        PRIVATE
        us_program_common
)

# Ensure us_revision.h is found
target_include_directories(us PRIVATE
        ${CMAKE_BINARY_DIR}/us       # where us_revision.h lives
)

# Ensure version headers are always generated first
add_dependencies(us us_version_headers)

target_include_directories(us
        PRIVATE
        ${src_dir}
)

if(APPLE)
        add_custom_command(TARGET us POST_BUILD
                COMMAND codesign --force --deep --sign -
                "$<TARGET_BUNDLE_DIR:us>"
                COMMENT "Ad-hoc code signing"
        )
endif()

if(WIN32 AND USE_QT6)
    file(TO_CMAKE_PATH "${VCPKG_INSTALLED_DIR}" CLEAN_VCPKG_INSTALLED_DIR)
    file(GENERATE
            OUTPUT "$<TARGET_FILE_DIR:us>/qt.conf"
            CONTENT "[Paths]\nPlugins=${CLEAN_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/Qt6/plugins\n"
            CONDITION "$<CONFIG:Debug>"
    )
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET us POST_BUILD
                COMMAND $<$<NOT:$<CONFIG:Debug>>:${WINDEPLOYQT_EXECUTABLE}>
                $<$<NOT:$<CONFIG:Debug>>:--no-translations>
                $<$<NOT:$<CONFIG:Debug>>:--ignore-library-errors>
                $<$<NOT:$<CONFIG:Debug>>:--release>
                $<$<NOT:$<CONFIG:Debug>>:$<TARGET_FILE:us>>
                COMMENT "Running windeployqt for Release..."
                COMMAND_EXPAND_LISTS
        )
    endif()
endif()

if(WIN32 AND QHELPGENERATOR_EXECUTABLE AND ASSISTANT_EXECUTABLE AND WINDEPLOYQT_EXECUTABLE)
    add_custom_command(TARGET us POST_BUILD
            COMMAND "${CMAKE_COMMAND}"
            -DDOC_QCH_STAGE=${DOC_QCH_STAGE}
            -DDOC_QHC_STAGE=${DOC_QHC_STAGE}
            -DDOC_OUTPUT_DIR=${DOC_OUTPUT_DIR}
            -DASSISTANT_SOURCE=${ASSISTANT_EXECUTABLE}
            -DWINDEPLOYQT=${WINDEPLOYQT_EXECUTABLE}
            -DVCPKG_RUNTIME_BIN_DIR=${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin
            -P "${CMAKE_SOURCE_DIR}/admin/cmake/packaging/windows/WindowsDeployQtAssistant.cmake"
            COMMENT "Staging Qt Assistant and help files into bin/"
            VERBATIM
    )
endif()

# =============================================================================
# macOS developer build: stage Assistant.app into bin/ with a qt.conf that
# points at the vcpkg plugins directory so libqcocoa.dylib is found at runtime.
# This is the dev-build equivalent of the --pkg MacDeploy staging path.
# =============================================================================
if(APPLE AND USE_QT6)
    set(_VCPKG_ASSISTANT
        "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/qttools/bin/Assistant.app")

    set(_VCPKG_QT6_PLUGINS
        "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/Qt6/plugins")
    if(NOT EXISTS "${_VCPKG_QT6_PLUGINS}")
        set(_VCPKG_QT6_PLUGINS
            "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/plugins")
    endif()

    set(_VCPKG_QT6_LIB "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib")

    if(EXISTS "${_VCPKG_ASSISTANT}/Contents/MacOS/Assistant")
        add_custom_command(TARGET us POST_BUILD
            COMMAND "${CMAKE_COMMAND}"
                -DASSISTANT_SRC=${_VCPKG_ASSISTANT}
                "-DBIN_DIR=$<TARGET_FILE_DIR:us>/../../.."
                -DQT6_PLUGINS_DIR=${_VCPKG_QT6_PLUGINS}
                -DQT6_LIB_DIR=${_VCPKG_QT6_LIB}
                -P "${CMAKE_SOURCE_DIR}/admin/cmake/packaging/macos/StageAssistantDev.cmake"
            COMMENT "Staging Qt Assistant.app for developer build"
            VERBATIM
        )
    else()
        message(STATUS "Qt Assistant.app not found at ${_VCPKG_ASSISTANT} -- Help will not work until qttools[assistant] is installed")
    endif()
endif()
