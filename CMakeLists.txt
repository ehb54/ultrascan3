cmake_minimum_required(VERSION 3.21)

# =============================================================================
# VERSION EXTRACTION FROM us_defines.h
# =============================================================================
# Extract version before project() call
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/admin/cmake")
include(UsDefinesParser)

# Extract version and description from us_defines.h
parse_header("${CMAKE_CURRENT_SOURCE_DIR}/utils/us_defines.h")

# Project definition with extracted version and description
project(UltraScan3
        VERSION ${US3_VERSION_MAJOR}.${US3_VERSION_MINOR}.${US3_VERSION_PATCH}
        DESCRIPTION "${US3_DESCRIPTION}"
        LANGUAGES CXX C
        )

# Make full version string available (including -dev suffix if present)
set(US3_VERSION_STRING ${US3_VERSION})

message(STATUS "UltraScan3 version: ${US3_VERSION_STRING}")
message(STATUS "UltraScan3 description: ${US3_DESCRIPTION}")

# =============================================================================
# VERSION HEADER GENERATION
# =============================================================================
# Execute version generation script immediately
include(${CMAKE_SOURCE_DIR}/admin/cmake/GenerateVersion.cmake)

include_directories(${CMAKE_BINARY_DIR})

# Set minimum macOS version - binaries will work on this version and newer
if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version" FORCE)
endif()

# =============================================================================
# CMAKE CONFIGURATION
# =============================================================================
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(BUILD_SHARED_LIBS ON)

# suppress deprecated warnings
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(
            -Wno-deprecated-declarations
            -Wno-deprecated
    )
endif()

# For Qt-specific deprecation warnings
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x000000)

# Build type configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Compiler flags based on qmake configuration
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
    set(CMAKE_C_FLAGS_RELEASE "-O3")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
    set(CMAKE_C_FLAGS_DEBUG "-g -O0")
endif()

# Platform-specific compiler flags
if(UNIX AND NOT APPLE)
    add_compile_options(-Wno-unused-local-typedefs)
endif()

# Enable Qt's MOC, RCC, and UIC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Enable testing
enable_testing()

# Add our custom cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/admin/cmake")

# =============================================================================
# CONFIGURATION OPTIONS
# =============================================================================
option(US3_NO_DB "Build without database support (for supercomputer backends)" OFF)
option(US3_BUILD_TESTS "Build unit tests" ON)
option(US3_BUILD_PROGRAMS "Build application programs" ON)

# Configuration-based definitions
if(US3_NO_DB)
    add_definitions(-DNO_DB)
endif()

# Platform-specific definitions
if(UNIX AND NOT APPLE)
    add_definitions(-DLINUX -DPLATFORM="intel" -DTITLE="Intel")
elseif(WIN32)
    add_definitions(-DUS_MAKE_DLL -D__LCC__ -DPLATFORM="intel" -DTITLE="Intel")
    add_definitions(-DQT_SHAREDMEMORY -DQT_SYSTEMSEMAPHORE -DWIN32)
elseif(APPLE)
    add_definitions(-DMAC -DOSX -DPLATFORM="mac")
endif()

# Platform configuratin
include(${CMAKE_SOURCE_DIR}/admin/cmake/platform-config.cmake)

# =============================================================================
# DEPENDENCY MANAGEMENT
# =============================================================================
# Platform-specific Qt5 handling
if(UNIX AND NOT APPLE)
    # Linux: prefer system Qt5
    list(APPEND CMAKE_PREFIX_PATH "/usr/lib/x86_64-linux-gnu/cmake")
endif()

# Find Qt5
find_package(Qt5 REQUIRED COMPONENTS
        Core
        Widgets
        Gui
        Network
        Sql
        Xml
        Svg
        OpenGL
        PrintSupport
        DataVisualization
        )

# ============================================================================
# Qt Dynamic Linking Configuration - ALL PLATFORMS
# ============================================================================

# Verify Qt is not static (with proper error handling)
if(TARGET Qt5::Core)
    get_target_property(QT_CORE_TYPE Qt5::Core TYPE)
    get_target_property(QT_CORE_LOCATION Qt5::Core LOCATION)

    message(STATUS "Qt5::Core type: ${QT_CORE_TYPE}")
    message(STATUS "Qt5::Core location: ${QT_CORE_LOCATION}")

    # Only fail if it's explicitly a STATIC_LIBRARY
    # SHARED_LIBRARY, MODULE_LIBRARY, or UNKNOWN_LIBRARY are all OK
    if(QT_CORE_TYPE STREQUAL "STATIC_LIBRARY")
        message(FATAL_ERROR "Qt5 must be built as shared libraries")
    elseif(QT_CORE_TYPE MATCHES "SHARED|MODULE|INTERFACE")
        message(STATUS "Qt5 is using dynamic/shared libraries - OK")
    else()
        # For UNKNOWN or IMPORTED types, check the actual file
        if(QT_CORE_LOCATION)
            if(QT_CORE_LOCATION MATCHES "\\.(dylib|so|dll)$")
                message(STATUS "Qt5 appears to be dynamic (based on file extension) - OK")
            else()
                message(WARNING "Qt5::Core type is ${QT_CORE_TYPE}, location: ${QT_CORE_LOCATION}")
                message(WARNING "Cannot definitively determine if Qt is static or dynamic - proceeding anyway")
            endif()
        endif()
    endif()
else()
    message(WARNING "Qt5::Core target not found - skipping static/dynamic check")
endif()

add_definitions(-DQT_SHARED)
set(QT_STATICPLUGIN 0 CACHE BOOL "Disable static Qt plugins" FORCE)

# Platform-specific Qt configuration
if(APPLE)
    message(STATUS "Configured Qt for dynamic linking on macOS")

    # Ensure all Qt modules have proper linking
    foreach(QT_MODULE Core Widgets Gui Network Sql Xml Svg OpenGL PrintSupport DataVisualization)
        if(TARGET Qt5::${QT_MODULE})
            get_target_property(QT_MOD_TYPE Qt5::${QT_MODULE} TYPE)
            message(STATUS "  Qt5::${QT_MODULE} type: ${QT_MOD_TYPE}")
        endif()
    endforeach()

elseif(UNIX)
    message(STATUS "Configured Qt for dynamic linking on Linux")

    # Linux-specific: ensure RPATH is set
    set(CMAKE_INSTALL_RPATH "$ORIGIN:$ORIGIN/../lib")

elseif(WIN32)
    message(STATUS "Configured Qt for dynamic linking on Windows")

    # Windows: Qt DLLs need to be in PATH or copied to exe directory
    # This is handled by vcpkg or windeployqt typically
endif()

message(STATUS "Qt dynamic linking configured for all platforms")

# ============================================================================
# Resources
# ============================================================================
include(GNUInstallDirs)
include(${CMAKE_SOURCE_DIR}/admin/cmake/Us3Resources.cmake)

set(US3_ETC_DIR "${CMAKE_SOURCE_DIR}/etc")

# Re-run CMake when any file in etc/ changes (including deletes/creates)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        "${US3_ETC_DIR}/*"
        "${US3_ETC_DIR}/*/*"
        "${US3_ETC_DIR}/*/*/*"
        )


# Subfolders you want to keep editable on disk (not embedded in the binary)
set(US3_ETC_EXCLUDE_DIRS
        rotors
        dmd
        mysql
        somo.cromer68.saxs_atoms
        )

# Individual files you want to keep editable on disk
set(US3_ETC_EXCLUDE_FILES
        cluster.config
        )

# Outputs
set(US3_QRC_XML "${CMAKE_BINARY_DIR}/us3_xml_auto.qrc")

us3_generate_qrc_from_dir(
        OUT_QRC "${US3_QRC_XML}" PREFIX "/us3"
        BASE "${US3_ETC_DIR}"
        GLOBS "*.xml"
        EXCLUDE_DIRS ${US3_ETC_EXCLUDE_DIRS}
        EXCLUDE_FILES ${US3_ETC_EXCLUDE_FILES}
        USE_ABSOLUTE
)

# Validation/target now only for XML
add_custom_command(
        OUTPUT "${US3_QRC_XML}.checked"
        COMMAND ${CMAKE_COMMAND}
        -DUS3_QRC="${US3_QRC_XML}"
        -DUS3_BASE="${US3_ETC_DIR}"
        -P "${CMAKE_SOURCE_DIR}/admin/cmake/Us3QrcCheck.cmake"
        COMMAND ${CMAKE_COMMAND} -E touch "${US3_QRC_XML}.checked"
        DEPENDS "${US3_QRC_XML}"
        COMMENT "Validating XML QRC references"
)

add_custom_target(us3_validate_qrc ALL
        DEPENDS "${US3_QRC_XML}.checked"
        )

# Resources library now only includes XML
add_library(us3_resources STATIC "${US3_QRC_XML}")
set_target_properties(us3_resources PROPERTIES AUTOMOC OFF)
add_dependencies(us3_resources us3_validate_qrc)
target_link_libraries(us3_resources PRIVATE Qt5::Core Qt5::Gui)
add_library(UltraScan3::Resources ALIAS us3_resources)

# Install editable trees/files we excluded from the QRC
foreach(d IN LISTS US3_ETC_EXCLUDE_DIRS)
    if(EXISTS "${US3_ETC_DIR}/${d}")
        install(DIRECTORY "${US3_ETC_DIR}/${d}/"
                DESTINATION "${CMAKE_INSTALL_DATADIR}/ultrascan/${d}")
    endif()
endforeach()
foreach(f IN LISTS US3_ETC_EXCLUDE_FILES)
    if(EXISTS "${US3_ETC_DIR}/${f}")
        install(FILES "${US3_ETC_DIR}/${f}"
                DESTINATION "${CMAKE_INSTALL_DATADIR}/ultrascan")
    endif()
endforeach()

# ============================================================================
# Find Dependencies
# ============================================================================

if(US3_BUILD_TESTS)
    find_package(Qt5 REQUIRED COMPONENTS Test)
endif()

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Find Qwt - vcpkg provides CONFIG mode
find_package(unofficial-qwt CONFIG REQUIRED)
# Create alias for backward compatibility
if(NOT TARGET Qwt::Qwt)
    add_library(Qwt::Qwt ALIAS unofficial::qwt::qwt)
endif()

# Find QwtPlot3D
option(US3_USE_VCPKG_QWTPLOT3D "Use qwtplot3d from vcpkg overlay" ON)

if(US3_USE_VCPKG_QWTPLOT3D)
    find_package(qwtplot3d CONFIG REQUIRED)
    message(STATUS "QwtPlot3D found via vcpkg overlay")

    # Verify the target was created
    if(NOT TARGET qwtplot3d::qwtplot3d)
        message(FATAL_ERROR "qwtplot3d::qwtplot3d target not created by find_package")
    endif()
else()
    # Use in-tree build
    add_subdirectory(qwtplot3d)
endif()

# Find libarchive - vcpkg provides a proper CMake target
find_package(LibArchive REQUIRED)

# Find GL2PS directly (no pkg-config file available)
find_path(GL2PS_INCLUDE_DIR
        NAMES gl2ps.h
        PATHS /usr/include /usr/local/include
        )

find_library(GL2PS_LIBRARY
        NAMES gl2ps
        PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu
        )

if(GL2PS_INCLUDE_DIR AND GL2PS_LIBRARY)
    set(GL2PS_FOUND TRUE)
    message(STATUS "GL2PS found: ${GL2PS_LIBRARY}")

    add_library(GL2PS::GL2PS INTERFACE IMPORTED)
    set_target_properties(GL2PS::GL2PS PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${GL2PS_INCLUDE_DIR}"
            INTERFACE_LINK_LIBRARIES "${GL2PS_LIBRARY}"
            )
else()
    set(GL2PS_FOUND FALSE)
    message(STATUS "GL2PS not found")
endif()

# Find MySQL/MariaDB (only if not NO_DB)
if(NOT US3_NO_DB)
    # Use vcpkg's unofficial-libmariadb package
    find_package(unofficial-libmariadb CONFIG REQUIRED)

    # Create alias for backward compatibility with your existing code
    if(NOT TARGET MySQL::Client)
        add_library(MySQL::Client ALIAS unofficial::libmariadb)
    endif()

    message(STATUS "Found MariaDB via vcpkg: unofficial::libmariadb")
endif()

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    find_library(GLU_LIBRARY GLU REQUIRED)
    find_package(X11 REQUIRED)
elseif(WIN32)
    # Windows system libraries
    set(WIN32_SYSTEM_LIBS ws2_32 advapi32 gdi32 user32 kernel32 psapi)
endif()

# GoogleTest for unit testing
if(US3_BUILD_TESTS)
    find_package(GTest CONFIG REQUIRED)

    # Create aliases for consistency if needed
    if(NOT TARGET gtest)
        add_library(gtest ALIAS GTest::gtest)
    endif()
    if(NOT TARGET gtest_main)
        add_library(gtest_main ALIAS GTest::gtest_main)
    endif()
    if(NOT TARGET gmock)
        add_library(gmock ALIAS GTest::gmock)
    endif()
    if(NOT TARGET gmock_main)
        add_library(gmock_main ALIAS GTest::gmock_main)
    endif()

    # Disable Qt AUTOMOC for GoogleTest (if the targets exist without aliases)
    if(TARGET GTest::gtest)
        set_target_properties(GTest::gtest GTest::gmock PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
    endif()
endif()

# =============================================================================
# SUBDIRECTORIES
# =============================================================================
add_subdirectory(utils)
add_subdirectory(doc/manual)

if(NOT US3_NO_DB)
    add_subdirectory(gui)
endif()

if(US3_BUILD_PROGRAMS AND NOT US3_NO_DB)
    add_subdirectory(programs)
endif()

if(US3_BUILD_TESTS)
    add_subdirectory(test)
endif()

# =============================================================================
# INSTALLATION
# =============================================================================
# Install binaries
install(TARGETS
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )

# Install headers for development
if(NOT US3_NO_DB)
    install(DIRECTORY utils/
            DESTINATION include/ultrascan3/utils
            FILES_MATCHING PATTERN "*.h"
            )

    install(DIRECTORY gui/
            DESTINATION include/ultrascan3/gui
            FILES_MATCHING PATTERN "*.h"
            )
endif()

# =============================================================================
# PACKAGING
# =============================================================================
set(CPACK_PACKAGE_NAME "UltraScan3")
set(CPACK_PACKAGE_VERSION ${US3_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Advanced analysis of analytical ultracentrifugation experiments")
set(CPACK_PACKAGE_VENDOR "University of Texas Health Science Center")

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
else()
    set(CPACK_GENERATOR "DEB;RPM")
endif()

include(CPack)

# =============================================================================
# DEVELOPMENT UTILITIES
# =============================================================================
# Add custom target for cleaning generated files (equivalent to qmakeall.sh clean)
add_custom_target(distclean
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/bin
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/lib
        COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeCache.txt
        COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/cmake_install.cmake
        COMMENT "Cleaning all generated files"
        )

# Print configuration summary
message(STATUS "")
message(STATUS "UltraScan3 Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "Options:")
message(STATUS "  NO_DB support: ${US3_NO_DB}")
message(STATUS "  Build tests: ${US3_BUILD_TESTS}")
message(STATUS "  Build programs: ${US3_BUILD_PROGRAMS}")
message(STATUS "")
message(STATUS "Dependencies:")
message(STATUS "  Qt5 version: ${Qt5_VERSION}")
message(STATUS "  Qwt version: ${QWT_VERSION}")
if(NOT US3_NO_DB)
    message(STATUS "  MySQL found: ${MYSQL_FOUND}")
endif()
message(STATUS "  LibArchive found: ${LIBARCHIVE_FOUND}")
message(STATUS "  OpenSSL  found: ${OPENSSL_FOUND} (${OPENSSL_VERSION})")
message(STATUS "")