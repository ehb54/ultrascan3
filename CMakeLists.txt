cmake_minimum_required(VERSION 3.21)

# Project definition
project(UltraScan3
        VERSION 4.0.0
        DESCRIPTION "UltraScan3 - Advanced analysis of analytical ultracentrifugation experiments"
        LANGUAGES CXX C
        )

execute_process(
        COMMAND git log --oneline
        COMMAND wc -l
        COMMAND tr -d '\n'
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# =============================================================================
# CMAKE CONFIGURATION
# =============================================================================
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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
    add_definitions(-DQT_SHAREDMEMORY -DQT_SYSTEMSEMAPHORE)
elseif(APPLE)
    add_definitions(-DMAC -DOSX -DPLATFORM="mac" -DTITLE="Mac")
endif()

# =============================================================================
# DEPENDENCY MANAGEMENT
# =============================================================================

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
        )

if(US3_BUILD_TESTS)
    find_package(Qt5 REQUIRED COMPONENTS Test)
endif()

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Find Qwt
find_package(Qwt REQUIRED)

# Find libarchive
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBARCHIVE REQUIRED libarchive)

if(LIBARCHIVE_FOUND)
    add_library(LibArchive::LibArchive INTERFACE IMPORTED)
    set_property(TARGET LibArchive::LibArchive PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${LIBARCHIVE_INCLUDE_DIRS})
    set_property(TARGET LibArchive::LibArchive PROPERTY INTERFACE_LINK_LIBRARIES ${LIBARCHIVE_LIBRARIES})
    set_property(TARGET LibArchive::LibArchive PROPERTY INTERFACE_COMPILE_OPTIONS ${LIBARCHIVE_CFLAGS_OTHER})
endif()

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
    find_package(PkgConfig REQUIRED)
    pkg_search_module(MYSQL REQUIRED mysqlclient mariadb)

    if(MYSQL_FOUND)
        add_library(MySQL::Client INTERFACE IMPORTED)
        set_property(TARGET MySQL::Client PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${MYSQL_INCLUDE_DIRS})
        set_property(TARGET MySQL::Client PROPERTY INTERFACE_LINK_LIBRARIES ${MYSQL_LIBRARIES})
        set_property(TARGET MySQL::Client PROPERTY INTERFACE_COMPILE_OPTIONS ${MYSQL_CFLAGS_OTHER})

        # Add library directories for MySQL
        link_directories(${MYSQL_LIBRARY_DIRS})
    endif()
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
    include(FetchContent)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG        release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)

    # Disable Qt AUTOMOC for GoogleTest
    set_target_properties(gmock gtest PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
endif()

# =============================================================================
# PROJECT VERSION AND BUILD INFO
# =============================================================================
set(US3_VERSION_MAJOR 10)
set(US3_VERSION_MINOR 0)
set(US3_VERSION_PATCH 0)
set(US3_VERSION ${US3_VERSION_MAJOR}.${US3_VERSION_MINOR}.${US3_VERSION_PATCH})

# Generate version header
configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/admin/cmake/us_version.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/us_version.h
        @ONLY
)

# =============================================================================
# SUBDIRECTORIES
# =============================================================================
add_subdirectory(utils)
add_subdirectory(qwtplot3d)

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
message(STATUS "  Version: ${US3_VERSION}")
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
message(STATUS "")