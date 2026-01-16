cmake_minimum_required(VERSION 3.15)
project(qwtplot3d VERSION 6.1.4 LANGUAGES CXX)
# Enable Position Independent Code for all targets
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
# Find required packages
if(USE_QT6)
  find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL Gui OpenGLWidgets)
else()
  find_package(Qt5 REQUIRED COMPONENTS Core Widgets OpenGL Gui)
endif()
find_package(OpenGL REQUIRED)

find_package(unofficial-qwt CONFIG QUIET)
if(NOT unofficial-qwt_FOUND)
    message(STATUS "unofficial-qwt not found via config, trying manual find")

    # Find Qwt manually since vcpkg's qwt port doesn't provide CMake configs
    find_library(QWT_LIBRARY
            NAMES qwt qwt-qt5 qwt6
            REQUIRED
            )
    find_path(QWT_INCLUDE_DIR
            NAMES qwt_plot.h
            PATH_SUFFIXES qwt qwt-qt5 qwt6
            REQUIRED
            )

    # Create imported target for Qwt
    if(NOT TARGET Qwt::Qwt)
        add_library(Qwt::Qwt UNKNOWN IMPORTED)
        set_target_properties(Qwt::Qwt PROPERTIES
                IMPORTED_LOCATION "${QWT_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${QWT_INCLUDE_DIR}"
                )
        message(STATUS "Found Qwt: ${QWT_LIBRARY}")
        message(STATUS "Qwt includes: ${QWT_INCLUDE_DIR}")
    endif()
else()
    message(STATUS "Found unofficial-qwt via config")
    if(NOT TARGET Qwt::Qwt)
        add_library(Qwt::Qwt ALIAS unofficial::qwt::qwt)
    endif()
endif()

# Get source files
file(GLOB QWTPLOT3D_SOURCES "src/*.cpp")
file(GLOB QWTPLOT3D_HEADERS "src/*.h" "include/*.h")

# Debug output
message(STATUS "QwtPlot3D sources: ${QWTPLOT3D_SOURCES}")
message(STATUS "QwtPlot3D headers: ${QWTPLOT3D_HEADERS}")

# Create library
add_library(qwtplot3d STATIC ${QWTPLOT3D_SOURCES} ${QWTPLOT3D_HEADERS})

# Windows DLL settings
if(WIN32 AND BUILD_SHARED_LIBS)
    target_compile_definitions(qwtplot3d PRIVATE QT_DLL QWT3D_DLL QWT3D_MAKEDLL)
    target_compile_definitions(qwtplot3d INTERFACE QT_DLL QWT3D_DLL QWT3D_MAKEDLL)
endif()

# MOC settings
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

# Link libraries
target_link_libraries(qwtplot3d PUBLIC
        Qt::Core
        Qt::Widgets
        Qt::OpenGL
        Qt::Gui
        OpenGL::GL
        Qwt::Qwt
        )
if(USE_QT6)
    target_link_libraries(qwtplot3d PUBLIC Qt::OpenGLWidgets)
endif()

# Platform-specific OpenGL libraries
if(WIN32)
    target_link_libraries(qwtplot3d PUBLIC glu32)
elseif(UNIX AND NOT APPLE)
    if(GLU_LIBRARY)
        target_link_libraries(qwtplot3d PUBLIC ${GLU_LIBRARY})
    endif()
endif()

# GL2PS support
if(GL2PS_FOUND)
    target_link_libraries(qwtplot3d PRIVATE GL2PS::GL2PS)
    target_compile_definitions(qwtplot3d PRIVATE HAVE_GL2PS)
else()
    enable_language(C)
    target_sources(qwtplot3d PRIVATE "3rdparty/gl2ps/gl2ps.c" "3rdparty/gl2ps/gl2ps.h")
    target_include_directories(qwtplot3d PRIVATE "3rdparty/gl2ps")
endif()

# Install targets
install(TARGETS qwtplot3d
        EXPORT qwtplot3d-targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        )

# Install headers
install(DIRECTORY include/ DESTINATION include/qwtplot3d
        FILES_MATCHING PATTERN "*.h")

# Export targets
install(EXPORT qwtplot3d-targets
        FILE qwtplot3dTargets.cmake
        NAMESPACE qwtplot3d::
        DESTINATION lib/cmake/qwtplot3d
        )

# Create and install config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/qwtplot3dConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
)

# Simple config file
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qwtplot3dConfig.cmake"
        "include(CMakeFindDependencyMacro)
if(USE_QT6)
    find_dependency(Qt6 COMPONENTS Core Widgets OpenGL Gui OpenGLWidgets)
else()
    find_dependency(Qt5 COMPONENTS Core Widgets OpenGL Gui)
endif()
find_dependency(OpenGL)
include(\"\${CMAKE_CURRENT_LIST_DIR}/qwtplot3dTargets.cmake\")
")

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/qwtplot3dConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/qwtplot3dConfigVersion.cmake"
        DESTINATION lib/cmake/qwtplot3d
        )
