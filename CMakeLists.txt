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
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(BUILD_SHARED_LIBS ON)

# Enable deprecation warnings and treat them as errors
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(
            -Wdeprecated
            -Wdeprecated-declarations
            -Werror=deprecated
            -Werror=deprecated-declarations
    )
elseif(MSVC)
    add_compile_options(
            /W4           # Enable level 4 warnings (includes deprecation)
            /we4996       # Treat deprecation warnings as errors
    )
endif()

# For Qt-specific deprecation warnings
add_compile_definitions(QT_DISABLE_DEPRECATED_UP_TO=0x060900)

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
option(USE_QT6 "Use Qt6 instead of Qt5" OFF)
option(US3_BUILD_MPI "Build us_mpi_analysis" OFF)

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
    add_definitions(-DMAC -DOSX -DPLATFORM="mac" -DTITLE="Mac")
endif()

# =============================================================================
# DEPENDENCY MANAGEMENT
# =============================================================================

# Find Qt5
if(USE_QT6)
    find_package(Qt6 REQUIRED COMPONENTS
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
            OpenGLWidgets
            )
else()
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
endif()

if(US3_BUILD_TESTS)
    if(USE_QT6)
        find_package(Qt6 REQUIRED COMPONENTS Test)
    else()
        find_package(Qt5 REQUIRED COMPONENTS Test)
    endif()
endif()

if(NOT TARGET Qt::Core)
    if(USE_QT6)
        add_library(Qt::Core ALIAS Qt6::Core)
        add_library(Qt::Widgets ALIAS Qt6::Widgets)
        add_library(Qt::OpenGL ALIAS Qt6::OpenGL)
        add_library(Qt::Gui ALIAS Qt6::Gui)
        add_library(Qt::Network ALIAS Qt6::Network)
        add_library(Qt::Xml ALIAS Qt6::Xml)
        add_library(Qt::Svg ALIAS Qt6::Svg)
        add_library(Qt::Sql ALIAS Qt6::Sql)
        add_library(Qt::PrintSupport ALIAS Qt6::PrintSupport)
        add_library(Qt::DataVisualization ALIAS Qt6::DataVisualization)
        add_library(Qt::OpenGLWidgets ALIAS Qt6::OpenGLWidgets)
    else()
        add_library(Qt::Core ALIAS Qt5::Core)
        add_library(Qt::Widgets ALIAS Qt5::Widgets)
        add_library(Qt::OpenGL ALIAS Qt5::OpenGL)
        add_library(Qt::Gui ALIAS Qt5::Gui)
        add_library(Qt::Network ALIAS Qt5::Network)
        add_library(Qt::Xml ALIAS Qt5::Xml)
        add_library(Qt::Svg ALIAS Qt5::Svg)
        add_library(Qt::Sql ALIAS Qt5::Sql)
        add_library(Qt::PrintSupport ALIAS Qt5::PrintSupport)
        add_library(Qt::DataVisualization ALIAS Qt5::DataVisualization)
    endif()
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
    if(PkgConfig_FOUND)
        pkg_search_module(MYSQL QUIET mysqlclient mariadb libmariadb mysql libmysql)

        if(MYSQL_FOUND)
            message(STATUS "Found MySQL/MariaDB via pkg-config")

            add_library(MySQL::Client INTERFACE IMPORTED)
            set_property(TARGET MySQL::Client PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${MYSQL_INCLUDE_DIRS})
            set_property(TARGET MySQL::Client PROPERTY INTERFACE_LINK_LIBRARIES ${MYSQL_LIBRARIES})
            set_property(TARGET MySQL::Client PROPERTY INTERFACE_COMPILE_OPTIONS ${MYSQL_CFLAGS_OTHER})

            # so you can still use the same target name
        endif()
    endif()

    # --- If not found, try vcpkg (Windows or fallback) ---
    if(NOT MYSQL_FOUND)
        message(STATUS "pkg-config not found or no MySQL detected, trying vcpkg packages")

        # MySQL connector (Oracle version)
        find_package(unofficial-libmysql QUIET)
        if(unofficial-libmysql_FOUND)
            add_library(unofficial::libmysql::libmysql INTERFACE IMPORTED)
            add_library(MySQL::Client ALIAS unofficial::libmysql::libmysql)
            set(MYSQL_FOUND TRUE)
        endif()

        # MariaDB connector (alternative)
        if(NOT MySQL::Client)
            find_package(unofficial-mariadb QUIET CONFIG)
            if(unofficial-mariadb_FOUND)
                set(MySQL::Client unofficial::mariadb::libmariadb)
                set(MYSQL_FOUND TRUE)
            endif()
        endif()
    endif()
    if(NOT MYSQL_FOUND)
        message(FATAL_ERROR "Could not find MySQL/MariaDB client library")
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
if(USE_QT6)
    message(STATUS "  Qt version: ${Qt6_VERSION}")
else ()
    message(STATUS "  Qt version: ${Qt5_VERSION}")
endif()
message(STATUS "  Qwt version: ${QWT_VERSION} (QT ${QWT_QT_VERSION})")
if(NOT US3_NO_DB)
    message(STATUS "  MySQL found: ${MYSQL_FOUND}")
endif()
message(STATUS "  LibArchive found: ${LIBARCHIVE_FOUND}")
message(STATUS "  OpenSSL  found: ${OPENSSL_FOUND} (${OPENSSL_VERSION})")
message(STATUS "")