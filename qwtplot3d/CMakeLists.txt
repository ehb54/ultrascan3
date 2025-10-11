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

if( WIN32 AND BUILD_SHARED_LIBS )
    target_compile_definitions(qwtplot3d PRIVATE QT_DLL QWT3D_DLL QWT3D_MAKEDLL)
    target_compile_definitions(qwtplot3d INTERFACE QT_DLL QWT3D_DLL QWT3D_MAKEDLL)
endif()

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
        Qt5::Gui
        OpenGL::GL
        Qwt::Qwt
        )

if (WIN32)
    target_link_libraries(qwtplot3d PUBLIC glu32)
elseif(UNIX AND NOT APPLE)
    # GLU_LIBRARY is found in the top-level CMake on Linux
    target_link_libraries(qwtplot3d PUBLIC ${GLU_LIBRARY})
endif()

# GL2PS if available
if(GL2PS_FOUND)
    target_link_libraries(qwtplot3d PRIVATE GL2PS::GL2PS)
    target_compile_definitions(qwtplot3d PRIVATE HAVE_GL2PS)
else()
    enable_language( C )
    target_sources( qwtplot3d PRIVATE "3rdparty/gl2ps/gl2ps.c" "3rdparty/gl2ps/gl2ps.h" )
    target_include_directories( qwtplot3d PRIVATE "3rdparty/gl2ps" )
endif()