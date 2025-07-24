# Auto-discover GUI source files (excluding subdirectories)
file(GLOB GUI_SOURCES "*.cpp")
file(GLOB GUI_HEADERS "*.h")

# Create GUI library
add_library(gui STATIC ${GUI_SOURCES} ${GUI_HEADERS})

add_dependencies(gui utils)

# Include directories - add Qwt headers
target_include_directories(gui
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${QWT_INCLUDE_DIR}
        )

# Link libraries - add Qwt
target_link_libraries(gui
        PUBLIC
        utils
        qwtplot3d
        Qt5::Core
        Qt5::Widgets
        Qt5::Gui
        Qt5::Network
        Qt5::Sql
        Qt5::Xml
        Qt5::Svg
        Qt5::OpenGL
        Qt5::PrintSupport
        ${QWT_LIBRARY}
        )

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_source_files_properties(us_license_t.cpp
            PROPERTIES COMPILE_DEFINITIONS
            "TITLE=\"64-bit AMD Opteron\";OS_TITLE=\"Linux\";PLATFORM=\"opteron\";OS=\"linux\""
            )
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set_source_files_properties(us_license_t.cpp
            PROPERTIES COMPILE_DEFINITIONS
            "TITLE=\"Macintosh\";OS_TITLE=\"OSX\";PLATFORM=\"mac\";OS=\"osx\""
            )
elseif(WIN32)
    set_source_files_properties(us_license_t.cpp
            PROPERTIES COMPILE_DEFINITIONS
            "TITLE=\"Intel\";OS_TITLE=\"Windows\";PLATFORM=\"intel\";OS=\"win32\""
            )
endif()