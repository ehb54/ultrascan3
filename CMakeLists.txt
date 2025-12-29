cmake_minimum_required(VERSION 3.21)

# =============================================================================
# VCPKG TRIPLET AUTO-DETECTION
# =============================================================================
# Auto-detect vcpkg triplet for macOS (only if not explicitly set)
if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    if(APPLE)
        execute_process(
                COMMAND uname -m
                OUTPUT_VARIABLE DETECTED_ARCH
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(DETECTED_ARCH STREQUAL "arm64")
            set(VCPKG_TARGET_TRIPLET "arm64-osx-dynamic")
            message(STATUS "Auto-detected Apple Silicon (arm64) - using triplet: ${VCPKG_TARGET_TRIPLET}")
        else()
            set(VCPKG_TARGET_TRIPLET "x64-osx-dynamic")
            message(STATUS "Auto-detected Intel Mac (x86_64) - using triplet: ${VCPKG_TARGET_TRIPLET}")
        endif()
    endif()
endif()

# =============================================================================
# VERSION EXTRACTION FROM us_defines.h
# =============================================================================
# Extract version before project() call
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/admin/cmake")
include(UsDefinesParser)
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
# Generate us/us_revision.h only when Git state changes (not on every build)
set(REVISION_HEADER "${CMAKE_SOURCE_DIR}/programs/us/us_revision.h")

find_package(Git)
if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
    add_custom_target(us_version_headers ALL
            COMMAND ${CMAKE_COMMAND}
            -DPROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
            -DPROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
            -DPROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
            -DUS3_VERSION=${US3_VERSION}
            -DUS3_SOURCE_DIR=${CMAKE_SOURCE_DIR}
            -DUS3_BINARY_DIR=${CMAKE_BINARY_DIR}
            -P ${CMAKE_SOURCE_DIR}/admin/cmake/GenerateVersion.cmake
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Updating us_revision.h"
            VERBATIM
    )
else()
    add_custom_target(us_version_headers)
endif()

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
link_directories(${CMAKE_BINARY_DIR}/lib)

# Enable deprecation warnings and treat them as errors
#if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
#    add_compile_options(
#            -Wdeprecated
#            -Wdeprecated-declarations
#            -Werror=deprecated
#            -Werror=deprecated-declarations
#    )
#elseif(MSVC)
#    add_compile_options(
#            /W4           # Enable level 4 warnings (includes deprecation)
#            /we4996       # Treat deprecation warnings as errors
#    )
#endif()

if(APPLE)
    add_compile_definitions(GL_SILENCE_DEPRECATION)
    add_compile_options(-w)
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
option(US3_BUILD_PROGRAMS "Build application programs" ON)

# High-level profile to make builds scriptable:
#   APP  = desktop/user build (GUI + programs + DB)
#   TEST = dev/CI build (programs + tests, prefer static US3 libs)
#   HPC  = headless / no DB / no GUI
set(US3_PROFILE "" CACHE STRING "UltraScan3 build profile: APP | TEST | HPC")
set_property(CACHE US3_PROFILE PROPERTY STRINGS APP TEST HPC)

# Lower-level knob used by gui/programs to decide static vs shared for US3 libs
option(US3_PREFER_STATIC "Prefer UltraScan3 static libraries when available" OFF)

# Apply profile -> concrete options
if(US3_PROFILE STREQUAL "APP")
    set(US3_NO_DB OFF CACHE BOOL "" FORCE)
    set(US3_BUILD_PROGRAMS ON CACHE BOOL "" FORCE)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(US3_PREFER_STATIC OFF CACHE BOOL "" FORCE)

elseif(US3_PROFILE STREQUAL "TEST")
    set(US3_NO_DB OFF CACHE BOOL "" FORCE)
    set(US3_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTING ON CACHE BOOL "" FORCE)
    set(US3_PREFER_STATIC ON CACHE BOOL "" FORCE)

elseif(US3_PROFILE STREQUAL "HPC")
    set(US3_NO_DB ON CACHE BOOL "" FORCE)
    set(US3_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(US3_PREFER_STATIC ON CACHE BOOL "" FORCE)
endif()


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

# Create symbolic link from source etc to build etc
set(US3_BUILD_ETC_DIR "${CMAKE_BINARY_DIR}/etc")
if (NOT EXISTS "${US3_BUILD_ETC_DIR}")
    file(CREATE_LINK "${US3_ETC_DIR}" "${US3_BUILD_ETC_DIR}" SYMBOLIC)
    message(STATUS "Created symbolic link: ${US3_BUILD_ETC_DIR} -> ${US3_ETC_DIR}")
endif ()

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

if(BUILD_TESTING)
    find_package(Qt5 REQUIRED COMPONENTS Test)
endif()

# Find OpenGL
find_package(OpenGL REQUIRED)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# =============================================================================
# Find Qwt - Unified approach for vcpkg and system packages
# =============================================================================
# Try vcpkg first
# Find Qwt - vcpkg or system packages
find_package(unofficial-qwt CONFIG QUIET)

if(unofficial-qwt_FOUND)
    message(STATUS "Found Qwt via vcpkg")
    if(NOT TARGET Qwt::Qwt)
        add_library(Qwt::Qwt ALIAS unofficial::qwt::qwt)
    endif()
else()
    message(STATUS "Searching for system Qwt...")
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(QWT QUIET Qt5Qwt6)
        if(NOT QWT_FOUND)
            pkg_check_modules(QWT QUIET qwt-qt5)
        endif()
    endif()

    if(NOT QWT_FOUND)
        find_path(QWT_INCLUDE_DIR NAMES qwt.h
                PATHS /usr/include/qwt /usr/include/qt5/qwt /usr/local/include/qwt)
        find_library(QWT_LIBRARY NAMES qwt-qt5 qwt qwt6-qt5
                PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)

        if(QWT_INCLUDE_DIR AND QWT_LIBRARY)
            set(QWT_FOUND TRUE)
            message(STATUS "Found system Qwt: ${QWT_LIBRARY}")
        endif()
    else()
        message(STATUS "Found system Qwt via pkg-config")
    endif()

    if(QWT_FOUND)
        # Create the main target that mimics vcpkg's structure
        add_library(unofficial_qwt_qwt INTERFACE IMPORTED)
        if(QWT_INCLUDE_DIRS)
            set_target_properties(unofficial_qwt_qwt PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES "${QWT_INCLUDE_DIRS}"
                    INTERFACE_LINK_DIRECTORIES "${QWT_LIBRARY_DIRS}"
                    INTERFACE_LINK_LIBRARIES "${QWT_LIBRARIES}")
        else()
            set_target_properties(unofficial_qwt_qwt PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES "${QWT_INCLUDE_DIR}"
                    INTERFACE_LINK_DIRECTORIES "${QWT_LIBRARY_DIR}"
                    INTERFACE_LINK_LIBRARIES "${QWT_LIBRARY}")
        endif()

        # Create both aliases for compatibility
        add_library(unofficial::qwt::qwt ALIAS unofficial_qwt_qwt)
        add_library(Qwt::Qwt ALIAS unofficial_qwt_qwt)
    else()
        message(FATAL_ERROR "Qwt not found. Install: apt-get install libqwt-qt5-dev")
    endif()
endif()

# Find QwtPlot3D - try vcpkg first, fall back to in-tree build
find_package(qwtplot3d CONFIG QUIET)

if(qwtplot3d_FOUND)
    message(STATUS "Found QwtPlot3D via vcpkg")
    if(NOT TARGET qwtplot3d::qwtplot3d)
        message(FATAL_ERROR "qwtplot3d::qwtplot3d target not created by find_package")
    endif()
else()
    message(STATUS "Using in-tree QwtPlot3D build")
    add_subdirectory(qwtplot3d)

    # Create namespaced alias for in-tree build
    if(TARGET qwtplot3d AND NOT TARGET qwtplot3d::qwtplot3d)
        add_library(qwtplot3d::qwtplot3d ALIAS qwtplot3d)
        message(STATUS "Created qwtplot3d::qwtplot3d alias for in-tree build")
    endif()
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
    find_package(unofficial-libmariadb CONFIG QUIET)

    if(unofficial-libmariadb_FOUND)
        message(STATUS "Found MariaDB via vcpkg")
        if(NOT TARGET MySQL::Client)
            add_library(MySQL::Client ALIAS unofficial::libmariadb)
        endif()
    else()
        message(STATUS "Searching for system MySQL/MariaDB...")
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(MYSQL QUIET mysqlclient)
            if(NOT MYSQL_FOUND)
                pkg_check_modules(MYSQL QUIET mariadb)
            endif()
        endif()

        if(NOT MYSQL_FOUND)
            find_path(MYSQL_INCLUDE_DIR NAMES mysql.h
                    PATHS /usr/include/mysql /usr/local/include/mysql /usr/include/mariadb)
            find_library(MYSQL_LIBRARY NAMES mysqlclient mariadb
                    PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)

            if(MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
                set(MYSQL_FOUND TRUE)
                message(STATUS "Found system MySQL: ${MYSQL_LIBRARY}")
            endif()
        else()
            message(STATUS "Found system MySQL via pkg-config")
        endif()

        if(MYSQL_FOUND)
            add_library(MySQL::Client INTERFACE IMPORTED)
            if(MYSQL_INCLUDE_DIRS)
                set_target_properties(MySQL::Client PROPERTIES
                        INTERFACE_INCLUDE_DIRECTORIES "${MYSQL_INCLUDE_DIRS}"
                        INTERFACE_LINK_LIBRARIES "${MYSQL_LIBRARIES}")
            else()
                set_target_properties(MySQL::Client PROPERTIES
                        INTERFACE_INCLUDE_DIRECTORIES "${MYSQL_INCLUDE_DIR}"
                        INTERFACE_LINK_LIBRARIES "${MYSQL_LIBRARY}")
            endif()
        else()
            message(FATAL_ERROR "MySQL not found. Install: apt-get install libmysqlclient-dev")
        endif()
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

# ---------------------------------------------------------------------------
# GoogleTest for unit testing
# ---------------------------------------------------------------------------
if(BUILD_TESTING)
    # Try config mode first (vcpkg / modern installs)
    find_package(GTest CONFIG QUIET)

    if(NOT GTest_FOUND)
        # Fall back to module / system packages
        find_package(GTest MODULE QUIET)
    endif()

    # At this point either we found it, or we didn't
    if(NOT GTest_FOUND)
        # We got here because:
        #  - user passed -DBUILD_TESTING=ON   OR
        #  - US3_PROFILE=TEST forced BUILD_TESTING=ON higher up
        # In both cases the user's intent is "I want tests".
        # Fail fast.
        message(FATAL_ERROR
                "BUILD_TESTING=ON but GoogleTest/GMock was not found.\n"
                "Install GTest in the build (Docker) image, or re-run CMake with -DBUILD_TESTING=OFF.")
    endif()

    # If we are here, GTest was found → set up the usual aliases
    message(STATUS "GoogleTest found – building test targets")

    # Create aliases for gtest (always available)
    if(NOT TARGET gtest AND TARGET GTest::gtest)
        add_library(gtest ALIAS GTest::gtest)
    endif()
    if(NOT TARGET gtest_main AND TARGET GTest::gtest_main)
        add_library(gtest_main ALIAS GTest::gtest_main)
    endif()

    # Create aliases for gmock (only if available)
    if(NOT TARGET gmock AND TARGET GTest::gmock)
        add_library(gmock ALIAS GTest::gmock)
    endif()
    if(NOT TARGET gmock_main AND TARGET GTest::gmock_main)
        add_library(gmock_main ALIAS GTest::gmock_main)
    endif()

    # Disable Qt AUTOMOC for GoogleTest
    if(TARGET GTest::gtest)
        set_target_properties(GTest::gtest PROPERTIES AUTOMOC OFF AUTOUIC OFF AUTORCC OFF)
    endif()
    if(TARGET GTest::gmock)
        set_target_properties(GTest::gmock PROPERTIES AUTOMOC OFF AUTORCC OFF AUTOUIC OFF)
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

if(BUILD_TESTING)
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
message(STATUS "  Profile: ${US3_PROFILE}")
message(STATUS "  NO_DB support: ${US3_NO_DB}")
message(STATUS "  Build tests: ${BUILD_TESTING}")
message(STATUS "  Build programs: ${US3_BUILD_PROGRAMS}")
message(STATUS "  Prefer static US3 libs: ${US3_PREFER_STATIC}")
message(STATUS "")
message(STATUS "Dependencies:")
message(STATUS "  Qt5 version: ${Qt5_VERSION}")
message(STATUS "  Qwt version: ${QWT_VERSION}")
if(NOT US3_NO_DB)
    message(STATUS "  MySQL found: ${MYSQL_FOUND}")
endif()
message(STATUS "  LibArchive found: ${LibArchive_VERSION}")
message(STATUS "  OpenSSL  found: ${OPENSSL_FOUND} (${OPENSSL_VERSION})")
message(STATUS "")
