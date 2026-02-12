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
                file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/us.rc" "IDI_ICON1 ICON \"../../gui/images/us3-icon-256x256.png\"\n")
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
