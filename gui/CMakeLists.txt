# UltraScan3 GUI Library

# Auto-discover GUI source files
file(GLOB GUI_SOURCES "*.cpp")
file(GLOB GUI_HEADERS "*.h")
file(GLOB GUI_FORMS "*.ui")

# --- Auto-embed GUI images via QRC (absolute paths + aliases) ----------------
file(GLOB_RECURSE US3_GUI_IMAGES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/gui/images/*.png"
        "${CMAKE_SOURCE_DIR}/gui/images/*.svg"
        "${CMAKE_SOURCE_DIR}/gui/images/*.xpm"
        "${CMAKE_SOURCE_DIR}/gui/images/*.ico")

# DEBUG: Print all found images helps verify images are included
foreach(img IN LISTS US3_GUI_IMAGES)
    file(RELATIVE_PATH rel "${CMAKE_SOURCE_DIR}/gui/images" "${img}")
    if(rel MATCHES "splash")
        message(STATUS "  Found splash: ${rel}")
    endif()
endforeach()

set(US3_GUI_QRC "${CMAKE_BINARY_DIR}/us3_gui_images_auto.qrc")
file(WRITE  "${US3_GUI_QRC}" "<RCC>\n  <qresource prefix=\"/images\">\n")
foreach(img IN LISTS US3_GUI_IMAGES)
    # rel will be "foo.png", "icons/bar.svg", etc., relative to gui/images
    file(RELATIVE_PATH rel "${CMAKE_SOURCE_DIR}/gui/images" "${img}")
    # Use ABSOLUTE disk path so rcc finds it from the build dir, but keep a clean runtime name via alias
    file(APPEND "${US3_GUI_QRC}" "    <file alias=\"${rel}\">${img}</file>\n")
endforeach()
file(APPEND "${US3_GUI_QRC}" "  </qresource>\n</RCC>\n")


qt_add_resources(US3_GUI_RES "${US3_GUI_QRC}")

# ============================================================================
# COMMON CONFIGURATION FUNCTION
# ============================================================================
function(configure_us_gui_target target_name is_static)
    # Add resources
    target_sources(${target_name} PRIVATE ${US3_GUI_RES})

    # Ensure dynamic Qt linking
    target_compile_definitions(${target_name} PUBLIC QT_SHARED)

    # Platform-specific sources and linking
    if(WIN32)
        target_link_libraries(${target_name} PRIVATE user32)
        target_compile_definitions(${target_name} PUBLIC US_MAKE_GUI_DLL QWT_DLL QT_DLL)
    elseif(APPLE)
        target_link_libraries(${target_name} PRIVATE "-framework Cocoa")
    elseif(UNIX) # Linux
        find_package(X11 REQUIRED)
        target_link_libraries(${target_name} PRIVATE X11::X11)
    endif()

    # Platform-specific compile definitions for us_license_t.cpp (if exists in gui)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/us_license_t.cpp")
        if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
            set_source_files_properties(us_license_t.cpp PROPERTIES
                    COMPILE_DEFINITIONS "TITLE=\"64-bit AMD Opteron\";OS_TITLE=\"Linux\";PLATFORM=\"opteron\";OS=\"linux\""
                    )
        elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
            set_source_files_properties(us_license_t.cpp PROPERTIES
                    COMPILE_DEFINITIONS "TITLE=\"Macintosh\";OS_TITLE=\"OSX\";PLATFORM=\"mac\";OS=\"osx\""
                    )
        elseif(WIN32)
            set_source_files_properties(us_license_t.cpp PROPERTIES
                    COMPILE_DEFINITIONS "TITLE=\"Intel\";OS_TITLE=\"Windows\";PLATFORM=\"intel\";OS=\"win32\""
                    )
        endif()
    endif()

    # Include directories
    target_include_directories(${target_name}
            PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_BINARY_DIR}
            )

    # UltraScan3 utils link
    add_dependencies(${target_name} us_utils)
    if(${is_static} AND TARGET UltraScan3::Utils_Static)
        add_dependencies(${target_name} us_utils_static)
        target_link_libraries(${target_name} PUBLIC UltraScan3::Utils_Static)
    else()
        target_link_libraries(${target_name} PUBLIC UltraScan3::Utils)
    endif()

    # Qt dependencies
    target_link_libraries(${target_name}
            PUBLIC
            Qt::Widgets
            Qt::Gui
            Qt::Svg
            Qt::OpenGL
            Qt::PrintSupport
            )
    if(USE_QT6)
        target_link_libraries(us_gui PUBLIC Qt::OpenGLWidgets)
    endif()

    # Plotting libraries - link AFTER Qt to avoid symbol conflicts
    target_link_libraries(${target_name}
            PUBLIC
            qwtplot3d::qwtplot3d
            unofficial::qwt::qwt
            )

    # Headers are in subdirectories: qwtplot3d/ and qwt/
    if(TARGET qwtplot3d::qwtplot3d)
        get_target_property(QWTPLOT3D_INCLUDES qwtplot3d::qwtplot3d INTERFACE_INCLUDE_DIRECTORIES)
        if(QWTPLOT3D_INCLUDES)
            target_include_directories(${target_name} PUBLIC
                    ${QWTPLOT3D_INCLUDES}
                    ${QWTPLOT3D_INCLUDES}/qwtplot3d
                    ${QWTPLOT3D_INCLUDES}/qwt
                    )
            message(STATUS "Added qwtplot3d includes to ${target_name}: ${QWTPLOT3D_INCLUDES}")
        else()
            message(WARNING "qwtplot3d::qwtplot3d exists but has no include directories")
            # Fallback: add paths manually (platform-specific)
            if(APPLE)
                set(vcpkg_arch "x64-osx")
            elseif(WIN32)
                set(vcpkg_arch "x64-windows")
            else()
                set(vcpkg_arch "x64-linux")
            endif()
            target_include_directories(${target_name} PUBLIC
                    "${CMAKE_BINARY_DIR}/vcpkg_installed/${vcpkg_arch}/include"
                    "${CMAKE_BINARY_DIR}/vcpkg_installed/${vcpkg_arch}/include/qwtplot3d"
                    "${CMAKE_BINARY_DIR}/vcpkg_installed/${vcpkg_arch}/include/qwt"
                    )
        endif()
    else()
        message(FATAL_ERROR "qwtplot3d::qwtplot3d target not found - make sure it's found before the gui subdirectory")
    endif()

    # OpenGL
    target_link_libraries(${target_name} PUBLIC OpenGL::GL)

    # Additional platform-specific libraries
    if(UNIX AND NOT APPLE)
        # Linux-specific
        if(GLU_LIBRARY)
            target_link_libraries(${target_name} PUBLIC ${GLU_LIBRARY})
        endif()
    endif()
endfunction()

# ============================================================================
# SHARED LIBRARY
# ============================================================================
add_library(us_gui SHARED ${GUI_SOURCES} ${GUI_HEADERS} ${GUI_FORMS})

# Set library properties
set_target_properties(us_gui PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        OUTPUT_NAME "us_gui${PROJECT_VERSION_MAJOR}"
        CXX_VISIBILITY_PRESET default
        C_VISIBILITY_PRESET default
        VISIBILITY_INLINES_HIDDEN OFF
        POSITION_INDEPENDENT_CODE ON
        )

# Platform-specific properties
if(APPLE)
    set_target_properties(us_gui PROPERTIES
            MACOSX_RPATH ON
            INSTALL_RPATH "@loader_path;@loader_path/../lib"
            )
elseif(UNIX)
    set_target_properties(us_gui PROPERTIES
            INSTALL_RPATH "$ORIGIN:$ORIGIN/../lib"
            )
elseif(WIN32)
    set_target_properties(us_gui PROPERTIES
            WINDOWS_EXPORT_ALL_SYMBOLS ON
            )
endif()

# Apply common configuration
configure_us_gui_target(us_gui FALSE)

# Create alias for consistent naming
add_library(UltraScan3::Gui ALIAS us_gui)

# ============================================================================
# STATIC LIBRARY
# ============================================================================
message(STATUS "Building static us_gui library")

add_library(us_gui_static STATIC ${GUI_SOURCES} ${GUI_HEADERS} ${GUI_FORMS})

# Set properties unique to static library
set_target_properties(us_gui_static PROPERTIES
        OUTPUT_NAME "us_gui${PROJECT_VERSION_MAJOR}_static"
        POSITION_INDEPENDENT_CODE ON
        )

# Apply common configuration
configure_us_gui_target(us_gui_static TRUE)

# Create static alias
add_library(UltraScan3::Gui_Static ALIAS us_gui_static)

# ============================================================================
# INSTALLATION - Only install shared library
# ============================================================================
install(TARGETS us_gui
        EXPORT UltraScan3Targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        )
