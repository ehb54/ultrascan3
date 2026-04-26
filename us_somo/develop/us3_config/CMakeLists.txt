set(SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
# macOS app bundle icon
if(APPLE)
    list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../../etc/us3_config.icns")
    set(MACOSX_BUNDLE_ICON_FILE us3_config.icns)
    set_source_files_properties(
            "${CMAKE_CURRENT_SOURCE_DIR}/../../etc/us3_config.icns"
            PROPERTIES MACOSX_PACKAGE_LOCATION "Resources"
    )
endif()

# Windows icon resource
if(WIN32)
    # Create us3_config.rc if it doesn't exist
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/us3_config.rc")
        file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/us3_config.rc" "IDI_ICON1 ICON \"../../etc/somo3_icon_128x128.ico\"\n")
    endif()
    list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/us3_config.rc")
endif()

add_executable(us3_config
        ${SOURCES}
)
target_link_libraries(us3_config PRIVATE somo_generic)
target_include_directories(us3_config PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# macOS bundle properties
if(APPLE)
    set_target_properties(us3_config PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_ICON_FILE us3_config.icns
            MACOSX_BUNDLE_GUI_IDENTIFIER "com.ultrascan3.us3_config"
            MACOSX_BUNDLE_BUNDLE_NAME "UltraScan3"
            MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )
endif()

install(TARGETS us3_config
        RUNTIME DESTINATION bin
        COMPONENT Programs
)
if(APPLE)
    add_custom_command(TARGET us3_config POST_BUILD
            COMMAND codesign --force --deep --sign -
            "$<TARGET_BUNDLE_DIR:us3_config>"
            COMMENT "Ad-hoc code signing"
    )
endif()

if(WIN32 AND USE_QT6)
    file(TO_CMAKE_PATH "${VCPKG_INSTALLED_DIR}" CLEAN_VCPKG_INSTALLED_DIR)
    file(GENERATE
            OUTPUT "$<TARGET_FILE_DIR:us3_config>/qt.conf"
            CONTENT "[Paths]\nPlugins=${CLEAN_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/Qt6/plugins\n"
            CONDITION "$<CONFIG:Debug>"
    )
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET us3_config POST_BUILD
                COMMAND $<$<NOT:$<CONFIG:Debug>>:${WINDEPLOYQT_EXECUTABLE}>
                $<$<NOT:$<CONFIG:Debug>>:--no-translations>
                $<$<NOT:$<CONFIG:Debug>>:--ignore-library-errors>
                $<$<NOT:$<CONFIG:Debug>>:--release>
                $<$<NOT:$<CONFIG:Debug>>:$<TARGET_FILE:us3_config>>
                COMMENT "Running windeployqt for Release..."
                COMMAND_EXPAND_LISTS
        )
    endif()
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
        add_custom_command(TARGET us3_config POST_BUILD
                COMMAND "${CMAKE_COMMAND}"
                -DASSISTANT_SRC=${_VCPKG_ASSISTANT}
                "-DBIN_DIR=$<TARGET_FILE_DIR:us3_config>/../../.."
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
