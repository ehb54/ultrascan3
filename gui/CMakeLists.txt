# UltraScan3 GUI Library

# Auto-discover GUI source files
file(GLOB GUI_SOURCES "*.cpp")
file(GLOB GUI_HEADERS "*.h")

# Drop the problem files
list(FILTER GUI_SOURCES EXCLUDE REGEX "us_model_editor\\.cpp$")
list(FILTER GUI_HEADERS EXCLUDE REGEX "us_model_editor\\.h$")

# Platform-specific source filtering
if(NOT WIN32)
    # Add any other Windows-specific patterns as needed
    list(FILTER GUI_SOURCES EXCLUDE REGEX ".*_win\\.cpp$")
    list(FILTER GUI_HEADERS EXCLUDE REGEX ".*_win\\.h$")
endif()

# Create GUI library
add_library(us_gui ${GUI_SOURCES} ${GUI_HEADERS})

# Set library properties (matching qmake configuration)
set_target_properties(us_gui PROPERTIES
        VERSION ${US3_VERSION}
        SOVERSION ${US3_VERSION_MAJOR}
        OUTPUT_NAME "us_gui${US3_VERSION_MAJOR}"  # Matches qmake VER = 10
        )

if (WIN32)
    target_sources(us_gui PRIVATE ${CMAKE_CURRENT_LIST_DIR}/us_win_utils.cpp)
    target_link_libraries(us_gui PRIVATE user32)

elseif(APPLE)
    # choose .m or .mm depending on your file
    target_sources(us_gui PRIVATE ${CMAKE_CURRENT_LIST_DIR}/us_mac_utils.m)
    target_link_libraries(us_gui PRIVATE "-framework Cocoa")

elseif(UNIX) # Linux
    # try either extension depending on what you have
    if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/us_x11_utils.c")
        target_sources(us_gui PRIVATE ${CMAKE_CURRENT_LIST_DIR}/us_x11_utils.c)
    elseif (EXISTS "${CMAKE_CURRENT_LIST_DIR}/us_x11_utils.cpp")
        target_sources(us_gui PRIVATE ${CMAKE_CURRENT_LIST_DIR}/us_x11_utils.cpp)
    else()
        message(FATAL_ERROR "Found no us_x11_utils.{c,cpp} in gui/")
    endif()
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

# Dependencies - ensure utils is built first
add_dependencies(us_gui us_utils)

# Include directories
target_include_directories(us_gui
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        )

# Link libraries - all GUI dependencies
target_link_libraries(us_gui
        PUBLIC
        UltraScan3::Utils    # Our utils library with all its dependencies
        qwtplot3d            # Our 3D plotting library
        Qt5::Widgets         # Core Qt5 GUI components
        Qt5::Gui
        Qt5::Svg
        Qt5::OpenGL
        Qt5::PrintSupport
        Qwt::Qwt            # Qwt plotting library
        OpenGL::GL          # OpenGL
        )

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    # Linux-specific
    if(GLU_LIBRARY)
        target_link_libraries(us_gui PUBLIC ${GLU_LIBRARY})
    endif()
elseif(WIN32)
    # Windows-specific libraries already included via us_utils
    # Additional GUI-specific Windows libraries if needed
elseif(APPLE)
    # macOS-specific
    target_link_libraries(us_gui PUBLIC "-framework QtOpenGL")
endif()

# Export this target
add_library(UltraScan3::Gui ALIAS us_gui)

# Installation
install(TARGETS us_gui
        EXPORT UltraScan3Targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        )