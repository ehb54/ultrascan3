# UltraScan3 GUI Library

# Auto-discover GUI source files
file(GLOB GUI_SOURCES "*.cpp")
file(GLOB GUI_HEADERS "*.h")
file(GLOB GUI_RESOURCES "*.qrc")
file(GLOB GUI_FORMS "*.ui")

# Exclude legacy image qrcs; we use the auto-generated one instead
list(FILTER GUI_RESOURCES EXCLUDE REGEX "(^|/)images\\.qrc$")
list(FILTER GUI_RESOURCES EXCLUDE REGEX "gui_images_auto\\.qrc$")

# Create GUI library
add_library(us_gui ${GUI_SOURCES} ${GUI_HEADERS} ${GUI_FORMS} ${GUI_RESOURCES})

# --- Auto-embed GUI images via QRC (absolute paths + aliases) ----------------
file(GLOB_RECURSE US3_GUI_IMAGES CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/gui/images/*.png"
        "${CMAKE_SOURCE_DIR}/gui/images/*.svg"
        "${CMAKE_SOURCE_DIR}/gui/images/*.xpm"
        "${CMAKE_SOURCE_DIR}/gui/images/*.ico")

set(US3_GUI_QRC "${CMAKE_BINARY_DIR}/us3_gui_images_auto.qrc")
file(WRITE  "${US3_GUI_QRC}" "<RCC>\n  <qresource prefix=\"/images\">\n")
foreach(img IN LISTS US3_GUI_IMAGES)
    # rel will be "foo.png", "icons/bar.svg", etc., relative to gui/images
    file(RELATIVE_PATH rel "${CMAKE_SOURCE_DIR}/gui/images" "${img}")
    # Use ABSOLUTE disk path so rcc finds it from the build dir, but keep a clean runtime name via alias
    file(APPEND "${US3_GUI_QRC}" "    <file alias=\"${rel}\">${img}</file>\n")
endforeach()
file(APPEND "${US3_GUI_QRC}" "  </qresource>\n</RCC>\n")

qt5_add_resources(US3_GUI_RES "${US3_GUI_QRC}")
target_sources(us_gui PRIVATE ${US3_GUI_RES})

if(APPLE)
    # Force all symbols to be exported at link time
    target_link_options(us_gui PRIVATE    # ← us_gui not us_utils
            "LINKER:-export_dynamic"
            "LINKER:-all_load"
            )
    # Override any inherited visibility settings
    target_compile_options(us_gui PRIVATE -fvisibility=default)  # ← us_gui not us_utils
endif()

# Set library properties - MERGED with visibility settings
set_target_properties(us_gui PROPERTIES   # ← us_gui not us_utils
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        OUTPUT_NAME "us_gui${PROJECT_VERSION_MAJOR}"  # ← us_gui not us_utils
        CXX_VISIBILITY_PRESET default
        C_VISIBILITY_PRESET default
        VISIBILITY_INLINES_HIDDEN OFF
        POSITION_INDEPENDENT_CODE ON
        MACOSX_RPATH ON
        INSTALL_RPATH "@loader_path;@loader_path/../lib"
        )
# Platform-specific RPATH settings
if(UNIX)
    set_target_properties(us_gui PROPERTIES
            INSTALL_RPATH "$ORIGIN:$ORIGIN/../lib"
            )
endif()

# Ensure dynamic Qt linking - ALL PLATFORMS
target_compile_definitions(us_gui
        PUBLIC
        QT_SHARED
        PRIVATE
        QT_PLUGIN
        )

## Symbol visibility flags for GCC/Clang (Linux, macOS)
#if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
#    target_compile_options(us_gui PRIVATE
#            -fvisibility=hidden
#            -fvisibility-inlines-hidden
#            )
#endif()

# Platform-specific sources and linking
if (WIN32)
    target_link_libraries(us_gui PRIVATE user32)
    target_compile_definitions(us_gui PUBLIC US_MAKE_GUI_DLL QWT_DLL QT_DLL)
    # Note: WINDOWS_EXPORT_ALL_SYMBOLS is OFF due to visibility preset
    # Use explicit dllexport/dllimport in headers if needed

elseif(APPLE)
    target_link_libraries(us_gui PRIVATE "-framework Cocoa")

elseif(UNIX) # Linux
    find_package(X11 REQUIRED)
    target_link_libraries(us_gui PRIVATE X11::X11)
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
target_include_directories(us_gui
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        )

# UltraScan3 internal dependencies
add_dependencies(us_gui us_utils)
target_link_libraries(us_gui PUBLIC UltraScan3::Utils)

# Qt5 dependencies
target_link_libraries(us_gui
        PUBLIC
        Qt5::Widgets
        Qt5::Gui
        Qt5::Svg
        Qt5::OpenGL
        Qt5::PrintSupport
        )

# Plotting libraries - link AFTER Qt to avoid symbol conflicts
target_link_libraries(us_gui
        PUBLIC
        qwtplot3d::qwtplot3d
        unofficial::qwt::qwt # Qwt plotting library
        )

# Headers are in subdirectories: qwtplot3d/ and qwt/
if(TARGET qwtplot3d::qwtplot3d)
    get_target_property(QWTPLOT3D_INCLUDES qwtplot3d::qwtplot3d INTERFACE_INCLUDE_DIRECTORIES)
    if(QWTPLOT3D_INCLUDES)
        target_include_directories(us_gui PUBLIC
                ${QWTPLOT3D_INCLUDES}
                ${QWTPLOT3D_INCLUDES}/qwtplot3d
                ${QWTPLOT3D_INCLUDES}/qwt
                )
        message(STATUS "Added qwtplot3d includes to us_gui: ${QWTPLOT3D_INCLUDES}")
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
        target_include_directories(us_gui PUBLIC
                "${CMAKE_BINARY_DIR}/vcpkg_installed/${vcpkg_arch}/include"
                "${CMAKE_BINARY_DIR}/vcpkg_installed/${vcpkg_arch}/include/qwtplot3d"
                "${CMAKE_BINARY_DIR}/vcpkg_installed/${vcpkg_arch}/include/qwt"
                )
    endif()
else()
    message(FATAL_ERROR "qwtplot3d::qwtplot3d target not found - make sure it's found before the gui subdirectory")
endif()

# OpenGL
target_link_libraries(us_gui PUBLIC OpenGL::GL)

# Additional platform-specific libraries
if(UNIX AND NOT APPLE)
    # Linux-specific
    if(GLU_LIBRARY)
        target_link_libraries(us_gui PUBLIC ${GLU_LIBRARY})
    endif()
endif()

# Create alias for consistent naming
add_library(UltraScan3::Gui ALIAS us_gui)

# Installation
install(TARGETS us_gui
        EXPORT UltraScan3Targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        )