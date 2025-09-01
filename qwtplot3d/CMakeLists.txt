# Find OpenGL
find_package(OpenGL REQUIRED)

# Get source files manually to ensure we catch everything
file(GLOB QWTPLOT3D_SOURCES
        "src/*.cpp"
        )

file(GLOB QWTPLOT3D_HEADERS
        "src/*.h"
        "include/*.h"
        )

# Debug: Print what files we found
message(STATUS "QwtPlot3D sources: ${QWTPLOT3D_SOURCES}")
message(STATUS "QwtPlot3D headers: ${QWTPLOT3D_HEADERS}")

# Create library with both sources and headers
add_library(qwtplot3d STATIC ${QWTPLOT3D_SOURCES} ${QWTPLOT3D_HEADERS})

# Critical MOC settings - set these BEFORE any other target operations
set_target_properties(qwtplot3d PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
        CMAKE_AUTOMOC_COMPILER_PREDEFINES ON
        )

# Include directories
target_include_directories(qwtplot3d PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include/qwtplot3d>
        )

# Link Qt libraries
target_link_libraries(qwtplot3d PUBLIC
        Qt5::Core
        Qt5::Widgets
        Qt5::OpenGL
        OpenGL::GL
        )

# GL2PS if available
if(GL2PS_FOUND)
    target_link_libraries(qwtplot3d PRIVATE GL2PS::GL2PS)
    target_compile_definitions(qwtplot3d PRIVATE HAVE_GL2PS)
endif()