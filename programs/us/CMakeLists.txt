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

# qt6 needs a bit post build things for it to work properly
if(WIN32 AND USE_QT6)
    file(TO_CMAKE_PATH "${VCPKG_INSTALLED_DIR}" CLEAN_VCPKG_INSTALLED_DIR)
    file(GENERATE
            OUTPUT "$<TARGET_FILE_DIR:us>/qt.conf"
            CONTENT "[Paths]\nPlugins=${CLEAN_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/Qt6/plugins\n"
            CONDITION "$<CONFIG:Debug>"
    )
    if(TARGET Qt6::windeployqt)
        set(WINDEPLOYQT_CMD "$<TARGET_FILE:Qt6::windeployqt>")
    else()
        find_program(WINDEPLOYQT windeployqt
                HINTS
                ${Qt6Core_DIR}/../../../bin
                ${CMAKE_PREFIX_PATH}
                "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/Qt6/bin"
                PATH_SUFFIXES bin
        )
        set(WINDEPLOYQT_CMD "${WINDEPLOYQT}")
    endif()
    if(WINDEPLOYQT_CMD)
        add_custom_command(TARGET us POST_BUILD
                COMMAND $<$<NOT:$<CONFIG:Debug>>:${WINDEPLOYQT_CMD}>
                $<$<NOT:$<CONFIG:Debug>>:--no-translations>
                $<$<NOT:$<CONFIG:Debug>>:--ignore-library-errors>
                $<$<NOT:$<CONFIG:Debug>>:--release>
                $<$<NOT:$<CONFIG:Debug>>:$<TARGET_FILE:us>>
                COMMENT "Running windeployqt for Release..."
                COMMAND_EXPAND_LISTS
        )
    endif()
endif()

