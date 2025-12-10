vcpkg_download_distfile(ARCHIVE
        URLS "https://sourceforge.net/projects/qwt/files/qwt/6.1.6/qwt-6.1.6.tar.bz2"
        FILENAME "qwt-6.1.6.tar.bz2"
        SHA512 0525672a2cbc07aeb4b9c0fb1c020540dcb8963d845e697a220ae0dfa8f543c529fa1fc2aa862826e627579cb5e987d0bd1da8ca95ccf02bcbced80fe5f8ed17
        )

vcpkg_extract_source_archive(
        SOURCE_PATH
        ARCHIVE "${ARCHIVE}"
)

# Create a simple CMakeLists.txt for qwt since it primarily uses qmake
file(WRITE "${SOURCE_PATH}/CMakeLists.txt" "
cmake_minimum_required(VERSION 3.15)
project(qwt VERSION 6.1.6)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Svg PrintSupport Concurrent OpenGL)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

file(GLOB QWT_SOURCES src/*.cpp)
file(GLOB QWT_HEADERS src/*.h)

add_library(qwt SHARED \${QWT_SOURCES} \${QWT_HEADERS})

target_link_libraries(qwt PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Svg
    Qt5::PrintSupport
    Qt5::Concurrent
    Qt5::OpenGL
)

target_include_directories(qwt PUBLIC
    \$<BUILD_INTERFACE:\${CMAKE_CURRENT_SOURCE_DIR}/src>
    \$<INSTALL_INTERFACE:include/qwt>
)

target_compile_definitions(qwt PRIVATE QWT_DLL QWT_MAKEDLL)

set_target_properties(qwt PROPERTIES
    VERSION 6.1.6
    SOVERSION 6
    OUTPUT_NAME qwt
)

install(TARGETS qwt EXPORT unofficial-qwt-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES \${QWT_HEADERS} DESTINATION include/qwt)

install(EXPORT unofficial-qwt-targets
    FILE unofficial-qwt-targets.cmake
    NAMESPACE unofficial::qwt::
    DESTINATION share/unofficial-qwt
)

# Create config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    \"\${CMAKE_CURRENT_BINARY_DIR}/unofficial-qwt-config-version.cmake\"
    VERSION 6.1.6
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    \"\${CMAKE_CURRENT_SOURCE_DIR}/unofficial-qwt-config.cmake.in\"
    \"\${CMAKE_CURRENT_BINARY_DIR}/unofficial-qwt-config.cmake\"
    INSTALL_DESTINATION share/unofficial-qwt
)

install(FILES
    \"\${CMAKE_CURRENT_BINARY_DIR}/unofficial-qwt-config.cmake\"
    \"\${CMAKE_CURRENT_BINARY_DIR}/unofficial-qwt-config-version.cmake\"
    DESTINATION share/unofficial-qwt
)
")

# Create config file template
file(WRITE "${SOURCE_PATH}/unofficial-qwt-config.cmake.in" "
@PACKAGE_INIT@

include(CMakeFindDependencyMacro)
find_dependency(Qt5 COMPONENTS Core Gui Widgets Svg PrintSupport Concurrent OpenGL)

include(\"\${CMAKE_CURRENT_LIST_DIR}/unofficial-qwt-targets.cmake\")

check_required_components(unofficial-qwt)
")

vcpkg_cmake_configure(
        SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/COPYING")